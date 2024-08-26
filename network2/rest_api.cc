#include "network2/rest_api.h"

namespace wedge {

using namespace boost::asio;
using namespace boost::beast;

ErrorOr<std::string> RestClient::make_request(boost::beast::http::verb method,
                                              const std::string& target,
                                              const std::string& body) {
  error_code ec;

  tcp::resolver resolver(io_context_);
  auto const results = resolver.resolve(host_, "443", ec);
  if (ec) {
    return ErrorOr<std::string>(ec);
  }

  ssl_stream stream(io_context_, ssl_context_);
  if (!SSL_set_tlsext_host_name(stream.native_handle(), host_.c_str())) {
    ec = {static_cast<int>(::ERR_get_error()),
          boost::asio::error::get_ssl_category()};
    return ErrorOr<std::string>(ec);
  }
  boost::asio::connect(stream.next_layer(), results.begin(), results.end(), ec);
  if (ec) {
    return ErrorOr<std::string>(ec);
  }

  stream.handshake(boost::asio::ssl::stream_base::client, ec);
  if (ec) {
    return ErrorOr<std::string>(ec);
  }

  http_request req{method, target, 11};
  req.set(boost::beast::http::field::host, host_);
  req.set(boost::beast::http::field::user_agent, BOOST_BEAST_VERSION_STRING);
  if (!body.empty()) {
    req.set(boost::beast::http::field::content_type, "application/json");
    req.body() = body;
    req.prepare_payload();
  }

  boost::beast::http::write(stream, req, ec);
  if (ec) {
    return ErrorOr<std::string>(ec);
  }

  boost::beast::flat_buffer buffer;
  http_response res;
  boost::beast::http::read(stream, buffer, res, ec);
  if (ec) {
    return ErrorOr<std::string>(ec);
  }
  stream.shutdown(ec);
  if (ec == boost::asio::ssl::error::stream_truncated) {
    ec.clear();
  }
  if (ec) {
    return ErrorOr<std::string>(ec);
  }

  return ErrorOr<std::string>(res.body());
}

}  // namespace wedge