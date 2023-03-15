//
// Created by maros on 15.3.2023.
//

#include <zlib.h>
#include <fmt/format.h>

#include <cstdio>
#include <stdexcept>

#include "libpak/libpak.hpp"
#include "libpak/algorithms.hpp"

template <typename T>
bool read(std::ifstream& stream, T& header, int64_t offset = 0, std::ios::seekdir seekdir = std::ios::beg)
{
  if(offset != 0)
    stream.seekg(offset, seekdir);
  stream.read(reinterpret_cast<char*>(&header), sizeof(header));
  return !stream.fail();
}

bool read(
    std::ifstream& stream,
    std::shared_ptr<uint8_t> data,
    int64_t size,
    int64_t offset = 0,
    std::ios::seekdir seekdir = std::ios::beg)
{
  if(offset != 0)
    stream.seekg(offset, seekdir);
  stream.read(reinterpret_cast<char*>(data.get()), size);
  return !stream.fail();
}

void read_data(std::ifstream& stream, libpak::asset asset)
{
  auto& header = asset.header;
  auto& data = asset.data;
  if(!header.is_asset_embedded)
    return;

  uint64_t bufferSize = header.is_data_compressed
                        ? header.embedded_data_length
                        : header.data_decompressed_length;

  try {
    data.buffer.reset(new uint8_t[bufferSize]);
  } catch(std::bad_alloc& alloc) {
    throw std::logic_error("not enough memory for data buffer");
  }

  // return unprocessed data if not compressed
  if(!header.is_data_compressed)
    if(!::read(stream, data.buffer, bufferSize))
      throw std::logic_error("couldn't read embedded data");

  // store origin offset
  auto origin = stream.tellg();

  // jump to data and read
  uint64_t compressedSize = header.embedded_data_length;
  std::shared_ptr<uint8_t> compressedData(nullptr);
  try {
    compressedData.reset(new uint8_t[compressedSize]);
  } catch(std::bad_alloc& alloc) {
    throw std::logic_error("not enough memory for compressed buffer");
  }

  if(!::read(stream, compressedData, compressedSize, header.embedded_data_offset))
    throw std::logic_error("couldn't read compressed embedded data");

  // restore origin offset
  stream.seekg(origin, std::ios::beg);

  uint64_t destinationLength = header.data_decompressed_length;
  // uncompress
  auto result = uncompress2(data.buffer.get(),&destinationLength,
                            compressedData.get(), &compressedSize);
  switch(result) {
  case Z_BUF_ERROR:
  case Z_MEM_ERROR:
    throw std::logic_error("not enough memory for uncompressed data");
  case Z_DATA_ERROR:
    throw std::logic_error("corrupted compressed data");
  default: {
  }; break;
  }
}

bool libpak::resource::read(bool data)
{

  // read the pak header
  auto& input = *this->input_stream;
  if(!::read(input, this->pak_header))
    throw std::logic_error("failed to read pak header");

  // read the content header
  if(!::read(input, this->content_header, PAK_ASSETS_ADDR))
    throw std::logic_error("failed to read content header");

  // reserve the size of asset count
  this->asset_index.reserve(this->content_header.asset_count);

  int hitCount = 0;
  // read the assets
  for(;;) {
    // read the header
    libpak::asset asset{};
    if(!::read(input, asset.header))
      throw std::logic_error("failed to read asset header");

    // break if there are no assets are left
    if(asset.header.asset_magic == 0x0)
      break;
    this->asset_index[asset.path()] = asset;
    hitCount++;

    // continue if data shouldn't be loaded
    if(!data)
      continue;

    try {
      ::read_data(input, asset);
    }
    catch(const std::logic_error& err) {
      throw std::logic_error(fmt::format("couldn't process data for asset: {}", err.what()));
    }
  }

  return true;
}
bool libpak::resource::write(bool cleanup) { return false; }

void libpak::resource::create()
{
  this->input_stream = std::make_shared<std::ifstream>(this->resourcePath.cbegin(), std::ios_base::binary);
}

void libpak::resource::destroy() noexcept
{
  this->input_stream->close();
  this->output_stream->close();
  this->pak_header = {};
  this->content_header = {};
  this->data_header = {};
  this->asset_index.clear();
}

libpak::asset& libpak::resource::get_asset(const std::wstring_view& asset_id, bool data)
{
  return this->asset_index[asset_id];
}
