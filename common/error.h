#pragma once

#include <memory>
#include <string>
#include <variant>

namespace wedge {

class Error {
 public:
  virtual std::string message() const = 0;
  virtual ~Error() = default;
};

template <class T>
class ErrorOr {
  using ErrorPointer = std::unique_ptr<Error>;

 public:
  ErrorOr(T&& value) : inner_(std::move(value)) {}
  ErrorOr(const T& value) : inner_(value) {}
  ErrorOr(ErrorPointer error) : inner_(std::move(error)) {}

  bool has_value() const { return std::get_if<T>(&inner_); }
  T& value() { return std::get<T>(inner_); }
  Error& error() { return *std::get<ErrorPointer>(inner_); }
  ErrorPointer take_error() {
    return std::move(std::get<ErrorPointer>(inner_));
  }

 private:
  std::variant<T, ErrorPointer> inner_;
};

}  // namespace wedge