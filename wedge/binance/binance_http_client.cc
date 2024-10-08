#include "wedge/binance/binance_http_client.h"

#include <boost/beast.hpp>

#include <cinttypes>

namespace wedge {

const char* kBaseURL = "api.binance.com";
const char* kHttpsPort = "443";

BinanceHttpClient::BinanceHttpClient(asio::io_context& executor,
                                     ssl::context& ssl_context)
    : stream_(executor, ssl_context) {}

BinanceHttpClient::BinanceHttpClient(asio::any_io_executor& executor,
                                     ssl::context& ssl_context)
    : stream_(executor, ssl_context) {}

asio::awaitable<error_code> BinanceHttpClient::connect() {
  auto executor = co_await asio::this_coro::executor;
  asio::ip::tcp::resolver resolver(executor);

  // Set SNI Hostname (many hosts need this to handshake successfully)
  if (!SSL_set_tlsext_host_name(stream_.native_handle(), kBaseURL)) {
    co_return error_code(static_cast<int>(::ERR_get_error()),
                         asio::error::get_ssl_category());
  }

  // Look up the domain name
  auto const results = co_await resolver.async_resolve(kBaseURL, kHttpsPort);

  // Set the timeout.
  beast::get_lowest_layer(stream_).expires_after(std::chrono::seconds(5));

  error_code ec;

  // Make the connection on the IP address we get from a lookup
  std::tie(ec, std::ignore) =
      co_await beast::get_lowest_layer(stream_).async_connect(results,
                                                              asio::as_tuple);
  if (ec) {
    co_return ec;
  }

  // Set the timeout.
  beast::get_lowest_layer(stream_).expires_after(std::chrono::seconds(5));

  // Perform the SSL handshake
  std::tie(ec) = co_await stream_.async_handshake(ssl::stream_base::client,
                                                  asio::as_tuple);

  co_return ec;
}

class URLBuilder {
 public:
  URLBuilder& set(std::string_view key, std::string_view value) {
    if (!query_string_.empty()) {
      query_string_.push_back('&');
    }
    query_string_.append(key);
    query_string_.push_back('=');
    query_string_.append(value);
    return *this;
  }

  URLBuilder& sign(std::string_view secret_key) {
    set("timestamp", current_timestamp());
    std::string signature = create_signature(secret_key);
    return set("signature", signature);
  }

  std::string url(std::string_view target) const {
    std::string result;
    result.append(target);
    if (!query_string_.empty()) {
      result.push_back('?');
      result.append(query_string_);
    }
    return result;
  }

 private:
  URLBuilder& set(std::string_view key, int64_t value) {
    char buffer[128];
    sprintf(buffer, "%" PRId64, value);
    return set(key, buffer);
  }

  static std::string bin2hex(const unsigned char* bin, unsigned int len) {
    std::string hex_str(len * 2, '\0');
    for (unsigned int i = 0; i < len; ++i) {
      sprintf(&hex_str[i * 2], "%02x", bin[i]);
    }
    return hex_str;
  }

  static int64_t current_timestamp() {
    using namespace std::chrono;
    auto duration = system_clock::now().time_since_epoch();
    return duration_cast<milliseconds>(duration).count();
  }

  std::string create_signature(std::string_view secret_key) const {
    unsigned char hash[EVP_MAX_MD_SIZE];
    uint32_t length;
    HMAC(EVP_sha256(), secret_key.data(), secret_key.size(),
         (uint8_t*)query_string_.data(), query_string_.size(), hash, &length);
    return bin2hex(hash, length);
  }

  std::string query_string_;
};

asio::awaitable<result<BinanceResponce>> BinanceHttpClient::send(
    const Request& request) {
  auto& [method, path, params, credentials, sign] = request;
  URLBuilder url_builder;
  for (auto& [key, value] : params) {
    url_builder.set(key, value);
  }
  if (sign && credentials) {
    url_builder.sign(credentials->secret_key);
  }
  std::string target = url_builder.url(path);
  http::request<http::empty_body> http_request(method, target, 11);
  http_request.set(http::field::host, kBaseURL);
  http_request.set(http::field::user_agent, "wedge-agent");
  if (credentials.has_value()) {
    http_request.set("X-MBX-APIKEY", credentials->api_key);
  }

  // Set the timeout.
  beast::get_lowest_layer(stream_).expires_after(std::chrono::seconds(5));

  error_code ec;
  std::tie(ec, std::ignore) =
      co_await http::async_write(stream_, http_request, asio::as_tuple);

  if (ec) {
    co_return ec;
  }

  // This buffer is used for reading and must be persisted
  beast::flat_buffer buffer;

  // Declare a container to hold the response
  BinanceResponce response;

  // Set the timeout.
  beast::get_lowest_layer(stream_).expires_after(std::chrono::seconds(5));

  // Receive the HTTP response
  std::tie(ec, std::ignore) =
      co_await http::async_read(stream_, buffer, response, asio::as_tuple);

  if (ec) {
    co_return ec;
  }

  co_return response;
}

asio::awaitable<error_code> BinanceHttpClient::shutdown() {
  auto [ec] = co_await stream_.async_shutdown(asio::as_tuple);
  if (ec == ssl::error::stream_truncated) {
    ec = {};
  }
  co_return ec;
}

error_code BinanceHttpClient::connect_sync() {
  asio::ip::tcp::resolver resolver(
      beast::get_lowest_layer(stream_).get_executor());
  auto results = resolver.resolve(kBaseURL, kHttpsPort);

  // Set SNI Hostname
  if (!SSL_set_tlsext_host_name(stream_.native_handle(), kBaseURL)) {
    return error_code(static_cast<int>(::ERR_get_error()),
                      asio::error::get_ssl_category());
  }

  error_code ec;

  // Make the connection on the IP address we get from a lookup
  beast::get_lowest_layer(stream_).connect(results, ec);

  if (ec) {
    return ec;
  }

  // Perform the SSL handshake
  stream_.handshake(ssl::stream_base::client, ec);

  return ec;
}

error_code BinanceHttpClient::shutdown_sync() {
  error_code ec;
  stream_.shutdown(ec);
  if (ec == ssl::error::stream_truncated) {
    ec = {};
  }
  return ec;
}

result<BinanceResponce> BinanceHttpClient::send_sync(const Request& request) {
  URLBuilder url_builder;
  for (auto& [key, value] : request.params) {
    url_builder.set(key, value);
  }
  if (request.sign && request.credentials) {
    url_builder.sign(request.credentials->secret_key);
  }
  std::string target = url_builder.url(request.path);

  http::request<http::empty_body> http_request(request.method, target, 11);
  http_request.set(http::field::host, kBaseURL);
  http_request.set(http::field::user_agent, "wedge-agent");
  if (request.credentials.has_value()) {
    http_request.set("X-MBX-APIKEY", request.credentials->api_key);
  }

  error_code ec;

  http::write(stream_, http_request, ec);

  if (ec) {
    return ec;
  }

  // This buffer is used for reading and must be persisted
  beast::flat_buffer buffer;
  BinanceResponce response;

  http::read(stream_, buffer, response, ec);

  if (ec) {
    return ec;
  }

  return response;
}

}  // namespace wedge