#pragma once

#include <memory>
#include <optional>
#include <string>

#include "common/candle.h"

namespace wedge {

class IDataLoader {
 public:
  virtual ~IDataLoader() = default;
  virtual std::optional<Candle> next() = 0;
};

std::unique_ptr<IDataLoader> sql_data_loader(
    const std::string& filename, std::optional<std::string> start_time = {},
    std::optional<std::string> end_time = {});

}  // namespace wedge