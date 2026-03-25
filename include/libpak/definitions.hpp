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

#ifndef LIBPAK_DEFINITIONS_HPP
#define LIBPAK_DEFINITIONS_HPP

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace libpak
{

#pragma pack(push, 1)

static constexpr size_t PAK_CONTENT_SECTOR = 0x7D000;
static constexpr size_t PAK_DATA_SECTOR = 0xF00000;

/**
 * Represents pak header.
 */
struct pak_header
{
  uint32_t header_magic{};
  uint32_t unknown0{};
  uint32_t paklib_version{};
  uint32_t locale{};

  uint32_t assets_count{};
  uint32_t used_assets_count{};
  uint32_t deleted_assets_count{};

  uint32_t file_size{};
  uint32_t team_version{};
  uint32_t header_sign{};
};

/**
 * Represents content header.
 */
struct content_header
{
  uint32_t first_magic{};  // FILS
  uint32_t second_magic{}; // FILZ

  uint32_t assets_count{};
};

/**
 * Represents asset header.
 */
struct asset_header
{
  uint32_t prefix{};
  uint32_t path_length{};

  uint32_t embedded_data_offset{};
  uint32_t embedded_data_length{};

  uint32_t data_decompressed_length{};
  uint32_t are_data_compressed{};
  uint32_t data_decompressed_length0{};
  uint32_t unknown0{};
  uint32_t data_decompressed_length1{};

  uint32_t timestamp{};
  uint32_t path_hash{};
  uint32_t filename_hash{};
  uint32_t extension_hash{};
  uint32_t parent_path_hash{};
  uint32_t is_asset_deleted{};
  uint32_t header_offset{};
  uint32_t are_data_embedded{};

  uint32_t unknown_type_l{};
  uint32_t unknown_type_h{};
  uint32_t date_created{};
  uint32_t time_created{};

  uint32_t crc_decompressed{};
  uint32_t crc_embedded{};
  uint32_t crc_identity{};
  uint32_t checksum_decompressed{};
  uint32_t checksum_embedded{};
  uint32_t unknown6{};
  char16_t path[256]{};
};

/**
 * Represents data header.
 */
struct data_header
{
  uint32_t magic{0x454C4946}; // ASCII: FILE
};

/**
 * Represents asset data.
 */
struct asset_data
{
  std::vector<std::byte> buffer;
};

/**
 * Represents asset.
 */
struct asset
{
  /**
   * Asset header
   */
  asset_header header{};

  /**
   * Asset data
   */
  asset_data data{};

  /**
   * @return String view of the asset path.
   */
  std::u16string path() { return header.path; }

  /**
   * Mark the asset as patched.
   */
  void markAsPatched() { this->patched = true; }

private:
  bool patched = false;
};

#pragma pack(pop)

} // namespace libpak

#endif // LIBPAK_DEFINITIONS_HPP
