#pragma once

#include <string>

#include "common/error.h"

namespace wedge {

class CurlError : public Error {
 public:
  explicit CurlError(int code) : code_(code) {}
  std::string message() const override;

 private:
  int code_;
};

class CurlGlobal {
 public:
  static ErrorOr<CurlGlobal> init();
  ~CurlGlobal();
};

class HttpHeaders {
 public:
  HttpHeaders() : headers_(nullptr) {}
  ~HttpHeaders();
  bool add_header(const std::string& header);
  void* get() const { return headers_; }

 private:
  void* headers_;
};

class HttpError : public Error {
 public:
  explicit HttpError(long code) : code_(code) {}
  std::string message() const override;

 private:
  long code_;
};

}  // namespace wedge