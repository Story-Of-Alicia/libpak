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

#include "libpak/libpak.hpp"
#include "libpak/algorithms.hpp"
#include "libpak/util.hpp"

#include <filesystem>
#include <format>
#include <ranges>
#include <stdexcept>

#include <zlib.h>

namespace
{

uLongf capitalized_string_crc32(const std::string& string)
{
  uLongf value{0};

  for (char c : string)
  {
    c = static_cast<char>(std::toupper(c));

    value = crc32(
      value,
      reinterpret_cast<const Bytef*>(&c),
      sizeof(c));
  }

  return value;
}

/**
 * Calculate buffer's alicia checksum.
 * @param buffer Buffer
 * @param length Length
 * @return Checksum
 */
int32_t alicia_checksum(const char* buffer, uint64_t length)
{
  int32_t result = 0;
  do
  {
    result += *buffer;
    buffer++;
    length--;
  } while (length != 0);
  return result;
}

} // namespace

bool libpak::stream::read(
  std::byte* const buffer,
  const int64_t size,
  const int64_t offset,
  const std::ios::seekdir dir)
{
  if (this->source == nullptr || this->source->fail())
    throw std::runtime_error("stream source is not available");

  int64_t origin = 0;
  if (offset != 0)
  {
    origin = this->source->tellg();
    this->source->seekg(offset, dir);
  }

  this->source->read(reinterpret_cast<char*>(buffer), size);
  if (origin != 0)
    this->source->seekg(origin);

  return this->source->good();
}

bool libpak::stream::write(
  uint8_t const* const buffer,
  const int64_t size,
  const int64_t offset,
  const std::ios::seekdir dir)
{
  if (this->sink == nullptr)
    throw std::runtime_error("stream sink is not available");

  int64_t origin = 0;
  if (offset != 0)
  {
    origin = this->sink->tellp();
    this->sink->seekp(offset, dir);
  }

  this->sink->write(reinterpret_cast<const char*>(buffer), size);
  if (origin != 0)
    this->sink->seekp(origin);

  return this->sink->good();
}

int64_t libpak::stream::set_writer_cursor(const int64_t pos, const std::ios::seekdir dir)
{
  if (this->sink == nullptr)
    return -1;

  const auto origin = get_writer_cursor();
  this->sink->seekp(pos, dir);
  return origin;
}

int64_t libpak::stream::get_writer_cursor()
{
  if (this->sink == nullptr)
    return -1;

  return this->sink->tellp();
}

int64_t libpak::stream::set_reader_cursor(const int64_t pos, const std::ios::seekdir dir)
{
  if (this->source == nullptr)
    return -1;

  const auto origin = get_reader_cursor();
  this->source->seekg(pos, dir);
  return origin;
}

int64_t libpak::stream::get_reader_cursor()
{
  if (this->source == nullptr)
    return -1;
  return this->source->tellg();
}

libpak::stream::stream(
  const std::shared_ptr<std::istream>& source,
  const std::shared_ptr<std::ostream>& sink)
  : source(source)
  , sink(sink)
{
}

void libpak::resource::create() {}

void libpak::resource::read(const bool data)
{
  // input stream
  this->input_stream = std::make_shared<std::ifstream>(
    this->resource_path, std::ios::binary);
  // resource stream wrapper
  this->resource_stream = std::make_shared<stream>(
    this->input_stream, this->output_stream);

  // reset to known state
  this->resource_stream->set_reader_cursor(0);

  // read the intro header
  if (!this->resource_stream->read(this->pak_header))
    throw std::runtime_error("failed to read pak header");

  // read the content header
  this->resource_stream->set_reader_cursor(PAK_CONTENT_SECTOR);
  if (!this->resource_stream->read(this->content_header))
    throw std::runtime_error("failed to read content header");

  // reserve the size of asset count
  this->assets.reserve(this->content_header.assets_count);

  // read the assets
  for (uint32_t assetIndex{0}; assetIndex < content_header.assets_count; assetIndex++)
  {
    try
    {
      asset asset;

      // read asset
      this->read_asset_header(asset);

      // read the asset data
      try
      {
        if (data)
          this->read_asset_data(asset);
      }
      catch (const std::runtime_error& err)
      {
        throw std::runtime_error(std::format("failed read asset data: {}", err.what()));
      }

      // index asset
      this->assets[asset.path()] = std::move(asset);
    }
    catch (const std::runtime_error& e)
    {
      throw std::runtime_error(std::format("failed to read asset: {}", e.what()));
    }
  }
}

void libpak::resource::write()
{
  // input stream
  this->output_stream = std::make_shared<std::ofstream>(
    this->resource_path, std::ios::binary);
  // resource stream wrapper
  this->resource_stream = std::make_shared<stream>(
    this->input_stream, this->output_stream);

  this->resource_stream->set_writer_cursor(0);

  // Update the content header
  this->content_header.assets_count = static_cast<uint32_t>(this->assets.size());

  // write the content header
  this->resource_stream->set_writer_cursor(PAK_CONTENT_SECTOR);
  if (!this->resource_stream->write(this->content_header))
    throw std::runtime_error("failed to write content header");

  int64_t data_offset = PAK_DATA_SECTOR;
  for (auto& asset : this->assets | std::views::values)
  {
    const auto header_origin = this->resource_stream->set_writer_cursor(
      data_offset);
    this->write_asset_data(asset);

    // Return to the asset header origin
    this->resource_stream->set_writer_cursor(header_origin);

    this->write_asset_header(asset);

    // Offset the data cursor by the length of the embedded data.
    data_offset += asset.header.embedded_data_length;
  }

  if (!this->resource_stream->write(this->data_header))
    throw std::runtime_error("failed to write data header");

  // Update the intro PAKS header assets counts
  this->pak_header.assets_count = static_cast<uint32_t>(this->assets.size());
  this->pak_header.used_assets_count = static_cast<uint32_t>(this->assets.size());
  this->pak_header.deleted_assets_count = 0;

  this->pak_header.file_size = static_cast<uint32_t>(
    this->resource_stream->get_writer_cursor());

  // Write the intro PAKS header
  this->resource_stream->set_writer_cursor(0);
  if (!this->resource_stream->write(this->pak_header))
    throw std::runtime_error("failed to write pak header");

  this->output_stream->close();
}

void libpak::resource::read_asset_header(asset& asset)
{
  auto& header = asset.header;
  // read asset header
  if (!this->resource_stream->read(header, header.header_offset))
    throw std::runtime_error("failed to read asset header");

  // handle invalid asset
  if (header.path_length == 0x0)
    throw std::runtime_error("invalid asset header read");
}

void libpak::resource::read_asset_data(asset& asset)
{
  auto& header = asset.header;
  auto& data = asset.data;
  if (!header.are_data_embedded)
    return;

  uLongf embedded_size = asset.header.embedded_data_length;
  const int64_t embedded_data_offset = header.embedded_data_offset;

  // allocate embedded data buffer
  std::vector<std::byte> embedded_data;
  try
  {
    embedded_data.resize(embedded_size);
  }
  catch (std::bad_alloc&)
  {
    throw std::runtime_error("not enough memory for embedded buffer");
  }

  // read the embedded data
  if (!this->resource_stream->read(embedded_data.data(), embedded_size, embedded_data_offset))
    throw std::runtime_error("couldn't read embedded data");

  // if data is not compressed, return the unprocessed buffer
  if (not header.are_data_compressed)
  {
    asset.data.buffer = std::move(embedded_data);
    return;
  }

  // NPAK can compress small buffers and inflate them. Because to this,
  // choose the largest data size for the decompressed data buffer.
  uLongf decompressed_data_size = std::max(
    header.embedded_data_length,
    header.data_decompressed_length);

  // allocate buffer for data
  try
  {
    data.buffer.resize(decompressed_data_size);
  }
  catch (std::bad_alloc&)
  {
    throw std::runtime_error("not enough memory for data buffer");
  }

  // uncompress
  const auto compression_result = uncompress2(
    reinterpret_cast<Bytef*>(data.buffer.data()),
    &decompressed_data_size,
    reinterpret_cast<Bytef*>(embedded_data.data()),
    &embedded_size);

  if (decompressed_data_size > data.buffer.size())
    data.buffer.resize(decompressed_data_size);

  switch (compression_result)
  {
    case Z_BUF_ERROR:
    case Z_MEM_ERROR:
      throw std::runtime_error("not enough memory for uncompressed data");
    case Z_DATA_ERROR:
      throw std::runtime_error("corrupted compressed data");
    default:
      {};
      break;
  }
}

void libpak::resource::write_asset_header(asset& asset)
{
  auto& header = asset.header;

  // update the header offset
  header.header_offset = static_cast<uint32_t>(
    this->resource_stream->get_writer_cursor());

  const std::filesystem::path path(asset.header.path);

  // update path hash
  const auto path_string = path.string();
  // path length includes the zero terminator
  header.path_length = static_cast<uint32_t>(
    path_string.length() + 1);
  header.path_hash = capitalized_string_crc32(path_string);

  // update filename hash
  const std::string filename_string = path.filename().string();
  header.filename_hash = capitalized_string_crc32(filename_string);

  // update extension hash
  const std::string extension_string = path.extension().string();
  header.extension_hash = capitalized_string_crc32(extension_string);

  // update parent path hash
  const std::string parent_path_string = path.parent_path().string();
  header.parent_path_hash = capitalized_string_crc32(parent_path_string);

  // write the asset header
  if (!this->resource_stream->write(header))
    throw std::runtime_error("failed to write asset header");
}

void libpak::resource::write_asset_data(asset& asset)
{
  if (not asset.header.are_data_embedded || asset.data.buffer.empty())
    return;

  asset.header.data_decompressed_length = static_cast<uint32_t>(
      asset.data.buffer.size());

  // calculate the CRC and checksum of the decompressed data.
  const uLongf decompressed_crc = crc32(
    0, // initial crc cycle value
    reinterpret_cast<const Bytef*>(asset.data.buffer.data()),
    asset.header.data_decompressed_length);

  const uint32_t decompressed_checksum = alicia_checksum(
    reinterpret_cast<const char*>(asset.data.buffer.data()),
    asset.header.data_decompressed_length);

  uLongf embedded_crc{};
  uLongf embedded_checksum{};

  asset.header.embedded_data_offset = static_cast<uint32_t>(
    resource_stream->get_writer_cursor());

  if (asset.header.are_data_compressed)
  {
    uLongf compressed_size = std::max(
      asset.header.data_decompressed_length,
      asset.header.embedded_data_length);

    std::vector<std::byte> compressed_data_buffer;
    compressed_data_buffer.resize(compressed_size);

    compress2(
      reinterpret_cast<Bytef*>(compressed_data_buffer.data()),
      &compressed_size,
      reinterpret_cast<Bytef*>(asset.data.buffer.data()),
      asset.header.data_decompressed_length,
      9 /* compression level*/);

    // calculate the crc and checksum of the now compressed data

    embedded_crc = crc32(
      0, // initial crc cycle value
      reinterpret_cast<Bytef*>(compressed_data_buffer.data()),
      compressed_size);

    embedded_checksum = alicia_checksum(
      reinterpret_cast<const char*>(compressed_data_buffer.data()),
      compressed_size);

    // write the compressed data
    resource_stream->write(
      reinterpret_cast<const uint8_t*>(compressed_data_buffer.data()),
      compressed_size);

    asset.header.embedded_data_length = compressed_size;
  }
  else
  {
    // Both embedded CRC and checksums are identical.
    embedded_crc = decompressed_crc;
    embedded_checksum = decompressed_checksum;

    // write the decompresssed data
    resource_stream->write(
      reinterpret_cast<const uint8_t*>(asset.data.buffer.data()),
      asset.header.data_decompressed_length);

    asset.header.embedded_data_length = asset.header.data_decompressed_length;
  }

  // update the header crcs and checksums

  asset.header.crc_decompressed = decompressed_crc;
  asset.header.checksum_decompressed = decompressed_checksum;
  asset.header.crc_embedded = embedded_crc;
  asset.header.checksum_embedded = embedded_checksum;
}

void libpak::resource::destroy() noexcept
{
  this->pak_header = {};
  this->content_header = {};
  this->data_header = {};
  this->assets.clear();
}
