#pragma once

#include <charconv>
#include <memory>
#include <optional>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace wedge {

template <typename T>
struct is_optional : std::false_type {};

template <typename T>
struct is_optional<std::optional<T>> : std::true_type {};

template <typename T>
inline constexpr bool is_optional_v = is_optional<T>::value;

class CommandLine {
 public:
 private:
  class IArgument {
   public:
    virtual ~IArgument() = default;
    virtual bool parse(std::string_view value) = 0;
    virtual bool ok() const = 0;
  };

  template <class T>
  class Argument {
   public:
   private:
    auto& inner() {
      if constexpr (is_optional_v<T>) {
        return value_->value();
      } else {
        return *value_;
      }
    }

    T* value_;
  };

  std::unordered_map<std::string, IArgument*> map_;
  std::vector<std::unique_ptr<IArgument>> arguments_;
};

}  // namespace wedge