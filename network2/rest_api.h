#pragma once

#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/system/result.hpp>
#include <string>

namespace wedge {

template <class T>
using ErrorOr = boost::system::result<T>;

class RestClient {
  using tcp = boost::asio::ip::tcp;
  using ssl_stream = boost::asio::ssl::stream<tcp::socket>;
  using http_request =
      boost::beast::http::request<boost::beast::http::string_body>;
  using http_response =
      boost::beast::http::response<boost::beast::http::string_body>;
  using error_code = boost::system::error_code;

 public:
  explicit RestClient(const std::string& host)
      : ssl_context_(boost::asio::ssl::context::tlsv12_client), host_(host) {}

  ErrorOr<std::string> get(const std::string& target) {
    return make_request(boost::beast::http::verb::get, target);
  }

  ErrorOr<std::string> post(const std::string& target,
                            const std::string& body) {
    return make_request(boost::beast::http::verb::post, target, body);
  }

  ErrorOr<std::string> put(const std::string& target, const std::string& body) {
    return make_request(boost::beast::http::verb::put, target, body);
  }

  ErrorOr<std::string> del(const std::string& target) {
    return make_request(boost::beast::http::verb::delete_, target);
  }

 private:
  ErrorOr<std::string> make_request(boost::beast::http::verb method,
                                    const std::string& target,
                                    const std::string& body = "");

  boost::asio::io_context io_context_;
  boost::asio::ssl::context ssl_context_;
  std::string host_;
};

}  // namespace wedge