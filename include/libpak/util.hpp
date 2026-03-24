/**
 * libpak - library for PAK manipulation
 * Copyright (C) 2026 Story Of Alicia
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 **/

#ifndef LIBPAK_UTIL_HPP
#define LIBPAK_UTIL_HPP

#include <functional>

namespace libpak::util
{

//! Deferred function signature.
using defer_func = std::function<void(void)>;

/**
 * Defers execution until destruction.
 */
struct defer
{
  const defer_func func;
  explicit defer(defer_func&& func) noexcept
    : func(std::move(func))
  {
  }
  ~defer() { func(); }
};

} // namespace libpak::util

#endif // LIBPAK_UTIL_HPP
