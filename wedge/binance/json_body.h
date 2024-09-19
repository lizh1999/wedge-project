#pragma once

#include <boost/asio.hpp>
#include <boost/beast/http/message.hpp>
#include <boost/optional.hpp>
#include <boost/system.hpp>
#include <nlohmann/json.hpp>

namespace wedge {

namespace asio = boost::asio;
namespace beast = boost::beast;
namespace http = beast::http;

using json = nlohmann::json;
using boost::system::error_code;

struct json_body {
  using value_type = json;

  struct writer {
    using ConstBuffer = asio::const_buffer;
    using GetResult = boost::optional<std::pair<ConstBuffer, bool>>;

    template <bool isRequest, class Fields>
    writer(boost::beast::http::header<isRequest, Fields> const&,
           value_type const& body) {
      serialized_body = body.dump();
    }

    void init(error_code& ec) { ec = {}; }

    GetResult get(error_code& ec) {
      ec = {};
      return std::make_pair(
          ConstBuffer(serialized_body.data(), serialized_body.size()), false);
    }

   private:
    std::string serialized_body;
  };

  struct reader {
    template <bool isRequest, class Fields>
    reader(http::header<isRequest, Fields>&, value_type& body) : body(body) {}

    void init(boost::optional<std::uint64_t> const& content_length,
              error_code& ec) {
      ec = {};
      if (content_length) {
        buffer.reserve(*content_length);
      }
    }

    template <class ConstBufferSequence>
    std::size_t put(ConstBufferSequence const& buffers, error_code& ec) {
      ec = {};
      buffer.append(static_cast<const char*>(buffers.data()), buffers.size());
      return buffers.size();
    }

    void finish(error_code& ec) {
      ec = {};
      auto parsed_json = json::parse(buffer, nullptr, false);
      if (parsed_json.is_discarded()) {
        ec = make_error_code(boost::system::errc::invalid_argument);
      } else {
        body = std::move(parsed_json);
      }
    }

   private:
    std::string buffer;
    value_type& body;
  };
};

}  // namespace wedge
