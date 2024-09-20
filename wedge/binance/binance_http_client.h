#pragma once

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast/core/tcp_stream.hpp>
#include <boost/beast/http/message.hpp>
#include <boost/system.hpp>

#include "wedge/binance/http.h"
#include "wedge/binance/json_body.h"

namespace wedge {

namespace asio = boost::asio;
namespace ssl = asio::ssl;
namespace beast = boost::beast;

using boost::system::error_code;
using boost::system::result;
using BinanceResponce = http::response<json_body>;

class BinanceHttpClient {
 public:
  BinanceHttpClient(asio::any_io_executor& executor, ssl::context& ssl_context);
  asio::awaitable<error_code> connect();
  asio::awaitable<error_code> shutdown();
  asio::awaitable<result<BinanceResponce>> send(const Request& request);

  error_code connect_sync();
  error_code shutdown_sync();
  result<BinanceResponce> send_sync(const Request &request);

 private:
  ssl::stream<beast::tcp_stream> stream_;
};

}  // namespace wedge