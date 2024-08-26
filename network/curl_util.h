#pragma once

#include <expected>
#include <string>

#include <curl/curl.h>

namespace wedge {

class CurlError {
 public:
  explicit CurlError(CURLcode code) : code_(code) {}

  CURLcode code() const { return code_; }

  std::string message() const {
    return std::string("CURL error: ") + curl_easy_strerror(code_);
  }

 private:
  CURLcode code_;
};

class CurlGlobal {
 public:
  static std::expected<CurlGlobal, CurlError> init();
  ~CurlGlobal();
};

class HttpHeaders {
 public:
  HttpHeaders() : headers_(nullptr) {}

  ~HttpHeaders() {
    if (headers_) {
      curl_slist_free_all(headers_);
    }
  }

  bool add_header(const std::string& header) {
    headers_ = curl_slist_append(headers_, header.c_str());
    return headers_ != nullptr;
  }

  curl_slist* get() const { return headers_; }

 private:
  curl_slist* headers_;
};

class HttpError {
 public:
  explicit HttpError(long code) : code_(code) {}

  long code() const { return code_; }

  std::string message() const {
    return "HTTP error code: " + std::to_string(code_);
  }

 private:
  long code_;
};

}