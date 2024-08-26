#pragma once

#include <cassert>
#include <optional>
#include <string>

#include "curl_util.h"

namespace wedge {

class RestError {
 public:
  enum class Kind { Curl, Http };

  RestError(const CurlError& curl_error)
      : kind_(Kind::Curl), curl_error_(curl_error) {}

  RestError(const HttpError& http_error)
      : kind_(Kind::Http), http_error_(http_error) {}

  Kind kind() const { return kind_; }

  const CurlError& curl_error() const {
    assert(kind_ == Kind::Curl &&
           "Accessing CurlError when the kind is not Curl");
    return curl_error_;
  }

  const HttpError& http_error() const {
    assert(kind_ == Kind::Http &&
           "Accessing HttpError when the kind is not Http");
    return http_error_;
  }

  std::string message() const {
    switch (kind_) {
      case Kind::Curl:
        return curl_error_.message();
      case Kind::Http:
        return http_error_.message();
    }
  }

 private:
  Kind kind_;
  union {
    CurlError curl_error_;
    HttpError http_error_;
  };
};

class RestResult {
 public:
  RestResult(std::string value) : result_(std::move(value)) {}
  RestResult(RestError error) : result_(std::unexpected(std::move(error))) {}

  bool has_value() const { return result_.has_value(); }
  std::string& value() { return result_.value(); }
  const RestError& error() const { return result_.error(); }

 private:
  std::expected<std::string, RestError> result_;
};

class RestClient {
 public:
  RestClient();
  ~RestClient();

  void set_proxy(const std::string& proxy);

  RestResult get(const std::string& url,
                 const HttpHeaders& headers = HttpHeaders());
  RestResult post(const std::string& url, const std::string& data,
                  const HttpHeaders& headers = HttpHeaders());
  RestResult put(const std::string& url, const std::string& data,
                 const HttpHeaders& headers = HttpHeaders());
  RestResult del(const std::string& url,
                 const HttpHeaders& headers = HttpHeaders());

 private:
  enum class HttpMethod { Get, Post, Put, Delete };

  std::optional<std::string> proxy_;
  CURL* curl_;

  RestResult perform_request(const std::string& url, HttpMethod method,
                             const std::string& data,
                             const HttpHeaders& headers);

  static size_t write_callback(void* contents, size_t size, size_t nmemb,
                               std::string* s);
};

}  // namespace wedge
