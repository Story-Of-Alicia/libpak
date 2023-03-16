//
// Created by maros on 15.3.2023.
//

#include <zlib.h>
#include <fmt/format.h>

#include <cstdio>
#include <stdexcept>

#include "libpak/util.hpp"
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

void read_data(std::ifstream& stream, libpak::asset& asset)
{
  auto& header = asset.header;
  auto& data = asset.data;
  if(!header.is_asset_embedded)
    return;

  // allocate embedded data buffer
  uint64_t embeddedSize = asset.header.embedded_data_length;
  std::shared_ptr<uint8_t> embeddedData(nullptr);
  try {
    embeddedData.reset(new uint8_t[embeddedSize]);
  }
  catch(std::bad_alloc& alloc) {
    throw std::runtime_error("not enough memory for embedded buffer");
  }

  // store origin offset
  auto origin = stream.tellg();
  libpak::util::defer defer([&stream, &origin](){
    // restore origin offset
    stream.seekg(origin, std::ios::beg);
  });

  // read embedded data
  if(!::read(stream, embeddedData, embeddedSize, header.embedded_data_offset))
    throw std::runtime_error("couldn't read embedded data");



  // if data is not compressed, return the unprocessed buffer
  if(!header.is_data_compressed) {
    asset.data.buffer = embeddedData;
    return;
  }

  // npak can compress small buffers and inflate them
  // due to this, the largest data length has to be chosen for the uncompressed data buffer
  uint64_t dataSize = std::max(header.embedded_data_length,
                               header.data_decompressed_length);
  // allocate buffer for data
  try {
    data.buffer.reset(new uint8_t[dataSize]);
  }
  catch(std::bad_alloc& alloc) {
    throw std::runtime_error("not enough memory for data buffer");
  }

  // uncompress
  auto result = uncompress2(data.buffer.get(), &dataSize, embeddedData.get(), &embeddedSize);
  switch(result) {
  case Z_BUF_ERROR:
  case Z_MEM_ERROR:
    throw std::runtime_error("not enough memory for uncompressed data");
  case Z_DATA_ERROR:
    throw std::runtime_error("corrupted compressed data");
  default: {
  }; break;
  }
}

void libpak::resource::read(bool data)
{
  // read the pak header
  auto& input = *this->input_stream;
  if(input.fail())
    throw std::runtime_error("failed to read the resource file");

  if(!::read(input, this->pak_header))
    throw std::runtime_error("failed to read pak header");

  // read the content header
  if(!::read(input, this->content_header, PAK_ASSETS_ADDR))
    throw std::runtime_error("failed to read content header");

  // reserve the size of asset count
  this->assets.reserve(this->content_header.assets_count);

  uint32_t registeredAssetCount = content_header.assets_count;
  uint32_t assetCount = 0;
  uint32_t deletedAssetCount = 0;

  // read the assets
  for(uint32_t assetIndex{0}; assetIndex < registeredAssetCount; assetIndex++) {
    try {
      libpak::asset asset;
      // read asset
      this->read_asset(asset, data);

      assetCount++;
      if(asset.header.is_asset_deleted)
        deletedAssetCount++;

      if(registeredAssetCount == assetCount)
        printf("aa\n");

      this->assets[asset.path()] = asset;
    } catch(const std::runtime_error& e)
    {
      throw std::runtime_error(fmt::format("failed to read asset: {}", e.what()));
    }
  }
}

void libpak::resource::read_asset(libpak::asset& asset,
                                  bool data) {
  auto& input = *this->input_stream;

  // read asset header
  if(!::read(input, asset.header, asset.header.asset_offset))
    throw std::runtime_error("failed to read asset header");

  // handle invalid asset
  if(asset.header.asset_magic == 0x0)
    throw std::runtime_error("invalid asset header read");

  if(data) {
    try {
      this->read_asset_data(asset);
    }
    catch(const std::runtime_error& err) {
      throw std::runtime_error(fmt::format("failed read asset data: {}", err.what()));
    }
  }
}

void libpak::resource::read_asset_data(libpak::asset& asset) {
  auto& input = *this->input_stream;
  ::read_data(input, asset);
}

void libpak::resource::write(bool patch, bool data) { }

void libpak::resource::write_asset(const libpak::asset& asset, bool data) {}
void libpak::resource::write_asset_data(const libpak::asset& asset) {}

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
  this->assets.clear();
}
