#include <cassert>
#include <chrono>
#include <fstream>
#include <iostream>
#include <sstream>
#include <thread>

#include <sqlite3.h>
#include <nlohmann/json.hpp>

#include "network/rest_api.h"

using json = nlohmann::json;
using namespace wedge;

class BinanceDownloader {
 public:
  BinanceDownloader(const std::string& symbol, const std::string& interval,
                    const std::string& end_date)
      : symbol_(symbol),
        interval_(interval),
        end_time_(parse_date_to_timestamp(end_date)) {
    client_.set_proxy("http://127.0.0.1:7890");

    // 初始化 SQLite 数据库
    int rc =
        sqlite3_open((symbol_ + "_" + interval_ + "_klines.db").c_str(), &db_);
    assert(rc == SQLITE_OK && "Failed to open SQLite database");

    // 创建表结构
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
    rc = sqlite3_exec(db_, create_table_sql, nullptr, nullptr, nullptr);
    assert(rc == SQLITE_OK && "Failed to create table in SQLite database");
  }

  ~BinanceDownloader() {
    if (db_) {
      sqlite3_close(db_);
    }
  }

  void download_all_klines() {
    int64_t start_time = 0;
    const int limit = 1000;
    const int request_interval_ms = 50;  // 设置请求间隔为50毫秒

    while (start_time < end_time_) {
      // 记录当前时间，计算下次请求的发送时间
      auto next_request_time = std::chrono::steady_clock::now() +
                               std::chrono::milliseconds(request_interval_ms);

      std::string url = build_url(start_time, end_time_, limit);
      RestResult result = client_.get(url);

      if (result.has_value()) {
        process_response(result.value());
        // 更新 start_time 为下一个时间段的开始时间
        json j = json::parse(result.value());
        if (!j.empty()) {
          start_time = j.back()[6];  // K 线结束时间戳
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
  std::string symbol_;
  std::string interval_;
  int64_t end_time_;
  RestClient client_;
  sqlite3* db_ = nullptr;

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

  int64_t parse_date_to_timestamp(const std::string& date_str) const {
    std::tm t = {};
    std::istringstream ss(date_str);
    ss >> std::get_time(&t, "%Y-%m-%d");
    assert(!ss.fail() && "Failed to parse date string");
    return std::chrono::system_clock::from_time_t(std::mktime(&t))
               .time_since_epoch() /
           std::chrono::milliseconds(1);
  }
};

int main() {
  auto curl_global = CurlGlobal::init();
  assert(curl_global.has_value() && "CURL Initialization Error");

  BinanceDownloader downloader("ETHUSDT", "1m", "2024-08-01");
  downloader.download_all_klines();
  return 0;
}
