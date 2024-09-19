#include "wedge/dataset/sql_dataset.h"

#include <fmt/core.h>
#include <sqlite3.h>

namespace wedge {

static bool is_new_dataset(sqlite3* db) {
  const char* check_table_sql =
      "SELECT name FROM sqlite_master WHERE type='table' AND name='klines';";
  sqlite3_stmt* stmt;
  sqlite3_prepare_v2(db, check_table_sql, -1, &stmt, nullptr);

  int rc = sqlite3_step(stmt);
  sqlite3_finalize(stmt);

  return rc != SQLITE_ROW;
}

static void create_table(sqlite3* db) {
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
  sqlite3_exec(db, create_table_sql, nullptr, nullptr, nullptr);
}

SqlDataset::SqlDataset(const std::string& name) {
  sqlite3_open(name.c_str(), &db_);
  if (is_new_dataset(db_)) {
    create_table(db_);
  }
}

SqlDataset::~SqlDataset() { sqlite3_close(db_); }

void SqlDataset::insert(const Candle& candle) {
  const char* insert_sql = R"(
      INSERT INTO klines (open_time, close_time, open_price, high_price, low_price, close_price, 
                          volume, quote_volume, traders, taker_buy_base, taker_buy_quote)
      VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);
    )";

  sqlite3_stmt* stmt;
  sqlite3_prepare_v2(db_, insert_sql, -1, &stmt, nullptr);

  sqlite3_bind_int64(stmt, 1, candle.open_time);
  sqlite3_bind_int64(stmt, 2, candle.close_time);
  sqlite3_bind_double(stmt, 3, candle.open_price);
  sqlite3_bind_double(stmt, 4, candle.high_price);
  sqlite3_bind_double(stmt, 5, candle.low_price);
  sqlite3_bind_double(stmt, 6, candle.close_price);
  sqlite3_bind_double(stmt, 7, candle.volume);
  sqlite3_bind_double(stmt, 8, candle.quote_volume);
  sqlite3_bind_int(stmt, 9, candle.traders);
  sqlite3_bind_double(stmt, 10, candle.taker_buy_base);
  sqlite3_bind_double(stmt, 11, candle.taker_buy_quote);

  sqlite3_step(stmt);
  sqlite3_finalize(stmt);
}

SqlIterator SqlDataset::iterator(std::optional<std::string> start_time,
                                 std::optional<std::string> end_time) {
  return SqlIterator(db_, start_time, end_time);
}

int64_t SqlDataset::get_max_start_time() {
  const char* select_max_time_sql = "SELECT MAX(close_time) FROM klines;";
  sqlite3_stmt* stmt;
  int rc = sqlite3_prepare_v2(db_, select_max_time_sql, -1, &stmt, nullptr);

  rc = sqlite3_step(stmt);
  int64_t max_start_time =
      (rc == SQLITE_ROW && sqlite3_column_type(stmt, 0) != SQLITE_NULL)
          ? sqlite3_column_int64(stmt, 0)
          : 0;
  sqlite3_finalize(stmt);

  return max_start_time;
}

}  // namespace wedge