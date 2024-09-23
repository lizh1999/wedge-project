#pragma once

#include <nlohmann/json.hpp>

namespace wedge {

struct Credentials {
  std::string api_key;
  std::string secret_key;
  NLOHMANN_DEFINE_TYPE_INTRUSIVE(Credentials, api_key, secret_key)
};

}  // namespace wedge