#pragma once

#include <boost/beast/http/verb.hpp>
#include <boost/optional.hpp>

#include "wedge/binance/credentials.h"

namespace wedge {

namespace http = boost::beast::http;

struct Request {
  http::verb method;
  std::string path;
  std::vector<std::pair<std::string, std::string>> params;
  boost::optional<Credentials> credentials;
  bool sign;
};

template <class Self>
class RequestBuilder {
 public:
  RequestBuilder(http::verb method, std::string_view path, bool sign = false)
      : method_(method), path_(path), sign_(sign) {}

  operator Request() const {
    return Request{
        .method = method_,
        .path = std::move(path_),
        .params = std::move(params_),
        .credentials = std::move(credentials_),
        .sign = sign_,
    };
  }

 protected:
  Self& credentials(const Credentials& credentials) {
    credentials_ = credentials;
    return static_cast<Self&>(*this);
  }

  Self& add_param(std::string_view key, std::string_view value) {
    params_.push_back(std::pair<std::string, std::string>(key, value));
    return static_cast<Self&>(*this);
  }

  Self& add_param(std::string_view key, double value) {
    char buffer[128];
    sprintf(buffer, "%.8lf", value);
    return add_param(key, buffer);
  }

  Self& add_param(std::string_view key, uint64_t value) {
    char buffer[128];
    sprintf(buffer, "%llu", value);
    return add_param(key, buffer);
  }

  Self& add_param(std::string_view key, int64_t value) {
    char buffer[128];
    sprintf(buffer, "%lld", (long long)value);
    return add_param(key, buffer);
  }

  Self& add_param(std::string_view key, uint32_t value) {
    char buffer[128];
    sprintf(buffer, "%u", value);
    return add_param(key, buffer);
  }

 private:
  http::verb method_;
  std::string path_;
  std::vector<std::pair<std::string, std::string>> params_;
  boost::optional<Credentials> credentials_;
  bool sign_;
};

}  // namespace wedge