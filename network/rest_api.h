#pragma once

#include <cassert>
#include <string>

#include "common/optional.h"
#include "network/curl_util.h"

namespace wedge {

using RestResult = ErrorOr<std::string>;

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

  optional<std::string> proxy_;
  struct CURL* curl_;

  RestResult perform_request(const std::string& url, HttpMethod method,
                             const std::string& data,
                             const HttpHeaders& headers);

  static size_t write_callback(void* contents, size_t size, size_t nmemb,
                               std::string* s);
};

}  // namespace wedge
