#include "curl_util.h"

#include <curl/curl.h>

#include "common/format.h"

namespace wedge {

ErrorOr<CurlGlobal> CurlGlobal::init() {
  CURLcode res = curl_global_init(CURL_GLOBAL_DEFAULT);
  if (res != CURLE_OK) {
    return std::make_unique<CurlError>(res);
  }
  return CurlGlobal();
}

CurlGlobal::~CurlGlobal() { curl_global_cleanup(); }

std::string CurlError::message() const {
  const char* error_message = curl_easy_strerror((CURLcode)code_);
  return format("CURL error: {}", error_message);
}

HttpHeaders::~HttpHeaders() {
  if (headers_) {
    curl_slist_free_all((curl_slist *)headers_);
  }
}

bool HttpHeaders::add_header(const std::string& header) {
  headers_ = curl_slist_append((curl_slist *)headers_, header.c_str());
  return headers_ != nullptr;
}

std::string HttpError::message() const {
  return format("HTTP error code: {}", code_);
}

}  // namespace wedge