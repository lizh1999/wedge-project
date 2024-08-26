#include "backtest/data_loader.h"

#include <cassert>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>

#include <sqlite3.h>

namespace wedge {

class SqlDataLoader : public IDataLoader {
 public:
  SqlDataLoader(const std::string& filename,
                std::optional<std::string> start_time,
                std::optional<std::string> end_time) {
    int rc = sqlite3_open(filename.c_str(), &db_);
    assert(rc == SQLITE_OK && "Failed to open SQLite database");

    int64_t start_timestamp =
        start_time.transform(as_unix_timestamp).value_or(0);
    int64_t end_timestamp =
        end_time.transform(as_unix_timestamp).value_or(LLONG_MAX);
    const char* sql = R"(
      SELECT open_time, close_time, open_price, high_price, low_price, close_price, volume,
             quote_volume, traders, taker_buy_base, taker_buy_quote 
      FROM klines
      WHERE open_time >= ? AND open_time <= ?
      ORDER BY open_time ASC)";
    rc = sqlite3_prepare_v2(db_, sql, -1, &stmt_, nullptr);
    assert(rc == SQLITE_OK && "Failed to prepare SQL statement");

    rc = sqlite3_bind_int64(stmt_, 1, start_timestamp);
    assert(rc == SQLITE_OK && "Failed to bind start_time");

    rc = sqlite3_bind_int64(stmt_, 2, end_timestamp);
    assert(rc == SQLITE_OK && "Failed to bind end_time");
  }

  ~SqlDataLoader() override {
    if (stmt_) {
      sqlite3_finalize(stmt_);
    }
    if (db_) {
      sqlite3_close(db_);
    }
  }

  std::optional<Candle> next() override {
    int rc = sqlite3_step(stmt_);
    if (rc == SQLITE_ROW) {
      Candle candle;
      candle.open_time = sqlite3_column_int64(stmt_, 0);
      candle.close_time = sqlite3_column_int64(stmt_, 1);
      candle.open_price = sqlite3_column_double(stmt_, 2);
      candle.high_price = sqlite3_column_double(stmt_, 3);
      candle.low_price = sqlite3_column_double(stmt_, 4);
      candle.close_price = sqlite3_column_double(stmt_, 5);
      candle.volume = sqlite3_column_double(stmt_, 6);
      candle.quote_volume = sqlite3_column_double(stmt_, 7);
      candle.traders = sqlite3_column_int(stmt_, 8);
      candle.taker_buy_base = sqlite3_column_double(stmt_, 9);
      candle.taker_buy_quote = sqlite3_column_double(stmt_, 10);
      return candle;
    } else if (rc == SQLITE_DONE) {
      return std::nullopt;
    } else {
      std::cerr << "Error while fetching data: " << sqlite3_errmsg(db_)
                << std::endl;
      return std::nullopt;
    }
  }

 private:
  static int64_t as_unix_timestamp(const std::string& time_str) {
    std::tm tm = {};
    std::istringstream ss(time_str);
    ss >> std::get_time(&tm, "%Y-%m-%d");
    assert(!ss.fail() && "Failed to parse date string");
    return static_cast<int64_t>(std::mktime(&tm)) * 1000;
  }

  sqlite3* db_ = nullptr;
  sqlite3_stmt* stmt_ = nullptr;
};

std::unique_ptr<IDataLoader> sql_data_loader(
    const std::string& filename, std::optional<std::string> start_time,
    std::optional<std::string> end_time) {
  return std::make_unique<SqlDataLoader>(filename, start_time, end_time);
}

}  // namespace wedge