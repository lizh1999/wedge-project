#include <cassert>
#include <chrono>
#include <fstream>
#include <iostream>
#include <sstream>
#include <thread>

#include <nlohmann/json.hpp>
#include <sqlite3.h>

#include "common/format.h"
#include "network/rest_api.h"

using json = nlohmann::json;
using namespace wedge;

class BinanceDownloader {
 public:
  BinanceDownloader(const std::string filename, const std::string& symbol,
                    const std::string& interval)
      : filename_(filename), symbol_(symbol), interval_(interval), db_(nullptr) {
    open_database();
    if (is_new_database()) {
      create_table();
    }
    start_time_ = get_max_start_time();
    end_time_ = get_current_timestamp();
  }

  ~BinanceDownloader() {
    if (db_) {
      sqlite3_close(db_);
    }
  }

  void download_all_klines() {
    const int limit = 1000;
    const int request_interval_ms = 50;  // 设置请求间隔为50毫秒

    while (start_time_ < end_time_) {
      // 记录当前时间，计算下次请求的发送时间
      auto next_request_time = std::chrono::steady_clock::now() +
                               std::chrono::milliseconds(request_interval_ms);

      std::string url = build_url(start_time_, end_time_, limit);
      RestResult result = client_.get(url);

      if (result.has_value()) {
        process_response(result.value());
        // 更新 start_time_ 为下一个时间段的开始时间
        json j = json::parse(result.value());
        if (!j.empty()) {
          start_time_ = j.back()[6];  // K 线结束时间戳
        }
      } else {
        const RestError& error = result.error();
        std::cerr << "Error: " << error.message() << "\nurl: " << url
                  << std::endl;
      }

      // 等待直到下次请求时间，以避免超过API访问频率限制
      std::this_thread::sleep_until(next_request_time);
    }
  }

 private:
  std::string filename_;
  std::string symbol_;
  std::string interval_;
  int64_t start_time_;
  int64_t end_time_;
  RestClient client_;
  sqlite3* db_;

  void open_database() {
    int rc = sqlite3_open(filename_.c_str(), &db_);
    assert(rc == SQLITE_OK && "Failed to open SQLite database");
  }

  bool is_new_database() {
    const char* check_table_sql =
        "SELECT name FROM sqlite_master WHERE type='table' AND name='klines';";
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_, check_table_sql, -1, &stmt, nullptr);
    assert(rc == SQLITE_OK && "Failed to prepare check statement");

    rc = sqlite3_step(stmt);
    bool is_new_db = (rc != SQLITE_ROW);
    sqlite3_finalize(stmt);

    return is_new_db;
  }

  void create_table() {
    const char* create_table_sql = R"(
      CREATE TABLE IF NOT EXISTS klines (
        open_time INTEGER PRIMARY KEY,
        close_time INTEGER,
        open_price DOUBLE,
        high_price DOUBLE,
        low_price DOUBLE,
        close_price DOUBLE,
        volume DOUBLE,
        quote_volume DOUBLE,
        traders INTEGER,
        taker_buy_base DOUBLE,
        taker_buy_quote DOUBLE
      );
    )";
    int rc = sqlite3_exec(db_, create_table_sql, nullptr, nullptr, nullptr);
    assert(rc == SQLITE_OK && "Failed to create table in SQLite database");
  }

  int64_t get_max_start_time() {
    const char* select_max_time_sql = "SELECT MAX(close_time) FROM klines;";
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_, select_max_time_sql, -1, &stmt, nullptr);
    assert(rc == SQLITE_OK && "Failed to prepare select statement");

    rc = sqlite3_step(stmt);
    int64_t max_start_time =
        (rc == SQLITE_ROW && sqlite3_column_type(stmt, 0) != SQLITE_NULL)
            ? sqlite3_column_int64(stmt, 0)
            : 0;
    sqlite3_finalize(stmt);

    return max_start_time;
  }

  std::string build_url(int64_t start_time, int64_t end_time, int limit) const {
    return "https://api1.binance.com/api/v3/klines?symbol=" + symbol_ +
           "&interval=" + interval_ +
           "&startTime=" + std::to_string(start_time) +
           "&endTime=" + std::to_string(end_time) +
           "&limit=" + std::to_string(limit);
  }

  void process_response(const std::string& response) {
    // 解析 JSON 数据并处理 K 线数据
    json j = json::parse(response);

    // 构建插入SQL语句
    std::string insert_sql =
        "INSERT OR IGNORE INTO klines (open_time, close_time, open_price, "
        "high_price, low_price, close_price, volume, quote_volume, traders, "
        "taker_buy_base, taker_buy_quote) VALUES ";

    for (const auto& kline : j) {
      insert_sql += build_values_sql(kline) + ",";
    }

    // 去掉最后一个逗号
    if (!j.empty()) {
      insert_sql.pop_back();
      insert_sql += ";";
    }

    // 执行插入SQL语句
    execute_sql(insert_sql);
  }

  std::string build_values_sql(const json& kline) const {
    std::ostringstream oss;
    // clang-format off
    oss << "(" 
        << kline[0].get<int64_t>() << ", "
        << kline[6].get<int64_t>() << ", "
        << kline[1].get<std::string>() << ", "
        << kline[2].get<std::string>() << ", "
        << kline[3].get<std::string>() << ", "
        << kline[4].get<std::string>() << ", "
        << kline[5].get<std::string>() << ", "
        << kline[7].get<std::string>() << ", "
        << kline[8].get<int>() << ", "
        << kline[9].get<std::string>() << ", "
        << kline[10].get<std::string>() 
        << ")";
    // clang-format on
    return oss.str();
  }

  void execute_sql(const std::string& sql) {
    char* err_msg = nullptr;
    int rc = sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &err_msg);
    if (rc != SQLITE_OK) {
      std::cerr << "Failed to execute SQL: " << err_msg << std::endl;
      sqlite3_free(err_msg);
    }
  }

  int64_t get_current_timestamp() const {
    auto now = std::chrono::system_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(
               now.time_since_epoch())
        .count();
  }
};

struct Task {
  std::string symbol;
  std::string interval;
  NLOHMANN_DEFINE_TYPE_INTRUSIVE(Task, symbol, interval)
};

struct Config {
  std::vector<Task> tasks;
  std::string path;
  NLOHMANN_DEFINE_TYPE_INTRUSIVE(Config, tasks, path)
};

int main() {
  std::string config_path = format("{}/.wedge/dataset.json", PROJECT_ROOT_DIR);
  std::ifstream config_file(config_path);
  assert(config_file.is_open() && "Open config file error");
  json j;
  config_file >> j;
  auto config = j.get<Config>();

  auto curl_global = CurlGlobal::init();
  assert(curl_global.has_value() && "CURL Initialization Error");

  for (const auto& task : config.tasks) {
    auto filename = format("{}/{}/{}USDT_{}_klines.db", PROJECT_ROOT_DIR,
                           config.path, task.symbol, task.interval);
    BinanceDownloader downloader(filename, "BTCUSDT", "1m");
    downloader.download_all_klines();
  }
  return 0;
}
