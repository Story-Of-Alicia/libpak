//
// Created by maros on 16.3.2023.
//

#ifndef LIBPAK_UTIL_HPP
#define LIBPAK_UTIL_HPP

#include <functional>

namespace libpak
{
  namespace util {

    using defer_func = std::function<void(void)>;

    struct defer {
      const defer_func func;
      explicit defer(defer_func&& func) noexcept : func(std::move(func)) {}
      ~defer() {
        func();
      }
    };

  }
}

#endif // LIBPAK_UTIL_HPP
