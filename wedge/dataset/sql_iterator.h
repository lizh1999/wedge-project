#pragma once

#include <optional>
#include <string>

#include "wedge/common/candle.h"

struct sqlite3;
struct sqlite3_stmt;

namespace wedge {

class SqlIterator {
 public:
  SqlIterator(struct sqlite3* db, std::optional<std::string> start_time,
              std::optional<std::string> end_time);

  SqlIterator(const SqlIterator&) = delete;
  SqlIterator& operator=(const SqlIterator&) = delete;

  ~SqlIterator();

  std::optional<Candle> next();

 private:
  struct sqlite3_stmt* stmt_;
};

}  // namespace wedge
