#pragma once

#include <memory>
#include <string>

#include "common/candle.h"
#include "common/optional.h"

namespace wedge {

class IDataLoader {
 public:
  virtual ~IDataLoader() = default;
  virtual optional<Candle> next() = 0;
};

std::unique_ptr<IDataLoader> sql_data_loader(
    const std::string& filename, optional<std::string> start_time = {},
    optional<std::string> end_time = {});

}  // namespace wedge