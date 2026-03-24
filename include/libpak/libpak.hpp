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

#ifndef libpak_libpak_HPP
#define libpak_libpak_HPP

#include "definitions.hpp"

#include <fstream>
#include <memory>
#include <unordered_map>
#include <utility>

namespace libpak
{

using asset_map = std::unordered_map<std::u16string, asset>;

/**
 * Provides encapsulation for read and write operations on streams.
 */
class stream
{
public:
  /**
   * Reads buffer from stream source. If offset is specified
   * @param buffer Buffer.
   * @param size   Buffer size.
   * @param offset Offset.
   * @param dir    Offset direction.
   * @throws std::runtime_exception when stream source is not available
   * @returns True if reading was successful, otherwise returns false.
   */
  bool read(std::byte* buffer, int64_t size, int64_t offset = 0, std::ios::seekdir dir = std::ios::beg);

  /**
   * Reads blob from stream source
   * @tparam Blob  Blob type.
   * @param blob   Blob.
   * @param offset Offset.
   * @param dir    Offset direction.
   * @return True if reading was successful, otherwise returns false.
   */
  template <typename Blob>
  bool read(Blob& blob, int64_t offset = 0, std::ios::seekdir dir = std::ios::beg)
  {
    return read(reinterpret_cast<std::byte*>(&blob), sizeof blob, offset, dir);
  }

  /**
   * Writes buffer to stream sink.
   * @param buffer Buffer.
   * @param size   Buffer size.
   * @param offset Offset.
   * @param dir    Offset direction.
   * @throws std::runtime_exception when stream sink is not available
   * @returns True if writing was successful, otherwise returns false.
   */
  bool write(
    const uint8_t* buffer,
    int64_t size,
    int64_t offset = 0,
    std::ios::seekdir dir = std::ios::beg);

  /**
   * Writes blob to stream sink.
   * @tparam Blob  Blob type.
   * @param blob   Blob.
   * @param offset Offset.
   * @param dir    Offset direction.
   * @return True if writing was successful, otherwise returns false.
   */
  template <typename Blob>
  bool write(const Blob& blob, int64_t offset = 0, std::ios::seekdir dir = std::ios::beg)
  {
    return write(reinterpret_cast<const uint8_t*>(&blob), sizeof blob, offset, dir);
  }

  /**
   * Sets writer cursor position.
   * @param pos Position.
   * @param dir Direction.
   * @return Previous writer cursor position.
   */
  int64_t set_writer_cursor(int64_t pos, std::ios::seekdir dir = std::ios::beg);

  /**
   * @return Current writer cursor position.
   */
  int64_t get_writer_cursor();

  /**
   * Sets reader cursor position.
   * @param pos Position.
   * @param dir Direction.
   * @return Previous reader cursor position.
   */
  int64_t set_reader_cursor(int64_t pos, std::ios::seekdir dir = std::ios::beg);

  /**
   * @return Current reader cursor position.
   */
  int64_t get_reader_cursor();

  /**
   * Construct stream with source and sink streams.
   * @param source Source (input).
   * @param sink   Sink (output).
   */
  stream(const std::shared_ptr<std::istream>& source, const std::shared_ptr<std::ostream>& sink);

  /**
   * Resource source stream.
   */
  std::shared_ptr<std::istream> source;

  /**
   * Resource sink stream.
   */
  std::shared_ptr<std::ostream> sink;
};

/**
 * Represents a single resource which holds assets and their accompanying data.
 */
class resource
{
public:
  /**
   * Default constructor.
   * @param path Path to resource.
   * @param create Whether to create the resource.
   */
  explicit resource(std::string path, bool create = true)
    : resource_path(std::move(path))
  {
    if (create)
      this->create();
  };

  /**
   * Reads the resource and indexes the assets.
   * @param data Whether to read the data of the indexed assets.
   * @throws std::runtime_error
   */
  void read(bool data = false);

  /**
   * Reads asset from the resource.
   * @param asset Asset. Must contain a valid offset or the read cursor must be before a valid
   * header.
   * @param data  Whether to read the assets data.
   * @throws std::runtime_error
   */
  void read_asset_header(asset& asset);

  /**
   * Reads assets data from the resource.
   * @param asset Asset. Must contain a valid data offset or the read cursor must be before valid
   * data.
   * @throws std::runtime_error
   */
  void read_asset_data(asset& asset);

  /**
   * Writes the resource.
   * @throws std::runtime_error
   */
  void write();

  /**
   * Writes the asset header.
   * @param asset Asset.
   * @throws std::runtime_error
   */
  void write_asset_header(const asset& asset);

  /**
   * Writes the asset's data.
   * @param asset Asset.
   */
  void write_asset_data(asset& asset);

  /**
   * Create the resource file descriptors.
   */
  void create();

  /**
   * Destroy all file descriptors and free memory.
   * @return
   */
  void destroy() noexcept;

  /**
   * Overloaded subscript operator used for fetching assets by their name.
   * @param name Asset name.
   * @return Indexed asset.
   */
  asset& operator[](const std::u16string& name) { return this->assets.at(name); }

  /**
   * Path to resource.
   */
  std::string resource_path;

  /**
   * PAK header
   */
  pak_header pak_header;
  /**
   * Content header
   */
  content_header content_header;

  /**
   * Data header
   */
  data_header data_header;

  /**
   * Map of all assets indexed by their name.
   */
  asset_map assets;

  /**
   * Resource stream.
   */
  std::shared_ptr<stream> resource_stream;

  /**
   * Resource input stream.
   */
  std::shared_ptr<std::ifstream> input_stream;

  /**
   * Resource output stream.
   */
  std::shared_ptr<std::ofstream> output_stream;
};

} // namespace libpak

#endif // libpak_libpak_HPP
