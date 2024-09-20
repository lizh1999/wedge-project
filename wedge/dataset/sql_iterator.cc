#include "wedge/dataset/sql_iterator.h"

#include <sqlite3.h>

#include <ctime>
#include <iomanip>
#include <sstream>

namespace wedge {

static int64_t as_unix_timestamp(const std::string& time_str) {
  std::tm tm = {};
  std::istringstream ss(time_str);
  ss >> std::get_time(&tm, "%Y-%m-%d");
  return static_cast<int64_t>(std::mktime(&tm)) * 1000;
}

SqlIterator::SqlIterator(sqlite3* db, std::optional<std::string> start_time,
                         std::optional<std::string> end_time) {
  int64_t start_timestamp = 0;
  int64_t end_timestamp = std::numeric_limits<int64_t>::max();
  if (start_time) {
    start_timestamp = as_unix_timestamp(*start_time);
  }
  if (end_time) {
    end_timestamp = as_unix_timestamp(*end_time);
  }

  const char* sql = R"(
      SELECT open_time, close_time, open_price, high_price, low_price, close_price, volume,
             quote_volume, traders, taker_buy_base, taker_buy_quote 
      FROM klines
      WHERE open_time >= ? AND open_time <= ?
      ORDER BY open_time ASC)";
  sqlite3_prepare_v2(db, sql, -1, &stmt_, nullptr);
  sqlite3_bind_int64(stmt_, 1, start_timestamp);
  sqlite3_bind_int64(stmt_, 2, end_timestamp);
}

SqlIterator::~SqlIterator() {
  if (stmt_) {
    sqlite3_finalize(stmt_);
  }
}

std::optional<Candle> SqlIterator::next() {
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
    std::abort();
  }
}

}  // namespace wedge