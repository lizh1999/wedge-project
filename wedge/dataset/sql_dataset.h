#pragma once

#include <string>

#include "wedge/common/candle.h"
#include "wedge/dataset/sql_iterator.h"

struct sqlite3;

namespace wedge {

class SqlDataset {
 public:
  explicit SqlDataset(const std::string& name);

  void insert(const Candle& candle);

  SqlIterator iterator(std::optional<std::string> start_time = {},
                       std::optional<std::string> end_time = {});

  int64_t get_max_start_time();

  ~SqlDataset();

 private:
  struct sqlite3* db_;
};

}  // namespace wedge