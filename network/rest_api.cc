#include "rest_api.h"

#include <curl/curl.h>

namespace wedge {

RestClient::RestClient() : curl_((struct CURL*)curl_easy_init()) {
  if (!curl_) {
    curl_ = nullptr;
  }
}

RestClient::~RestClient() {
  if (curl_) {
    curl_easy_cleanup(curl_);
  }
}

size_t RestClient::write_callback(void* contents, size_t size, size_t nmemb,
                                  std::string* s) {
  s->append(static_cast<char*>(contents), size * nmemb);
  return size * nmemb;
}

void RestClient::set_proxy(const std::string& proxy) { proxy_ = proxy; }

RestResult RestClient::perform_request(const std::string& url,
                                       HttpMethod method,
                                       const std::string& data,
                                       const HttpHeaders& headers) {
  if (!curl_) {
    return std::make_unique<CurlError>(CURLE_FAILED_INIT);
  }

  std::string response;
  curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, write_callback);
  curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &response);

  if (proxy_.has_value()) {
    curl_easy_setopt(curl_, CURLOPT_PROXY, proxy_->c_str());
  }

  switch (method) {
    case HttpMethod::Post:
      curl_easy_setopt(curl_, CURLOPT_POST, 1L);
      curl_easy_setopt(curl_, CURLOPT_POSTFIELDS, data.c_str());
      break;
    case HttpMethod::Put:
      curl_easy_setopt(curl_, CURLOPT_CUSTOMREQUEST, "PUT");
      curl_easy_setopt(curl_, CURLOPT_POSTFIELDS, data.c_str());
      break;
    case HttpMethod::Delete:
      curl_easy_setopt(curl_, CURLOPT_CUSTOMREQUEST, "DELETE");
      break;
    case HttpMethod::Get:
      // GET 请求不需要额外设置
      break;
  }

  if (headers.get()) {
    curl_easy_setopt(curl_, CURLOPT_HTTPHEADER, headers.get());
  }

  CURLcode res = curl_easy_perform(curl_);
  if (res != CURLE_OK) {
    return std::make_unique<CurlError>(res);
  }

  long http_code = 0;
  curl_easy_getinfo(curl_, CURLINFO_RESPONSE_CODE, &http_code);
  if (http_code >= 400) {
    return std::make_unique<HttpError>(http_code, response);
  }

  return response;
}

RestResult RestClient::get(const std::string& url, const HttpHeaders& headers) {
  return perform_request(url, HttpMethod::Get, "", headers);
}

RestResult RestClient::post(const std::string& url, const std::string& data,
                            const HttpHeaders& headers) {
  return perform_request(url, HttpMethod::Post, data, headers);
}

RestResult RestClient::put(const std::string& url, const std::string& data,
                           const HttpHeaders& headers) {
  return perform_request(url, HttpMethod::Put, data, headers);
}

RestResult RestClient::del(const std::string& url, const HttpHeaders& headers) {
  return perform_request(url, HttpMethod::Delete, "", headers);
}

}  // namespace wedge