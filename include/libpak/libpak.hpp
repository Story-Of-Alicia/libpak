//
// Created by maros on 15.3.2023.
//

#ifndef libpak_libpak_HPP
#define libpak_libpak_HPP

#include "definitions.hpp"

#include <memory>
#include <fstream>
#include <string_view>
#include <unordered_map>
#include <utility>

namespace libpak {

  using asset_map = std::unordered_map<std::u16string_view, libpak::asset>;

  /**
   * Represents a single resource which holds assets and their accompanying data.
   */
  class resource {
  public:
    /**
     * Default constructor.
     * @param path Path to resource.
     * @param create Whether to create the resource.
     */
    explicit resource(std::string path, bool create = true) : resource_path(std::move(path))
    {
      if(create)
        this->create();
    };

  public:
    /**
     * Reads the resource and indexes the assets.
     * @param data Whether to read the data of the indexed assets.
     * @throws std::runtime_error
     */
    void read(bool data = false);

    /**
     * Reads asset from the resource.
     * @param asset Asset. Must contain a valid offset or the read cursor must be before a valid header.
     * @param data  Whether to read the assets data.
     * @throws std::runtime_error
     */
    void read_asset(asset& asset, bool data = false);

    /**
     * Reads assets data from the resource.
     * @param asset Asset. Must contain a valid data offset or the read cursor must be before valid data.
     * @throws std::runtime_error
     */
    void read_asset_data(asset& asset);

    /**
     * Writes the asset to the resource.
     * @param patch Whether to write only patched assets.
     * @param data  Whether to write asset data.
     * @throws std::runtime_error
     */
    void write(bool patch = true, bool data = true);

    /**
     * Writes the assets data to the resource.
     * @param asset Asset.
     * @param data  Whether to write the assets data
     * @throws std::runtime_error
     */
    void write_asset(const libpak::asset& asset, bool data = true);

    /**
     * Writes the
     * @param asset
     */
    void write_asset_data(const libpak::asset& asset);

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
    inline libpak::asset& operator[](const std::u16string_view & name) {
      return this->assets.at(name);
    }

  public:
    /**
     * Path to resource.
     */
    const std::string resource_path;

  public:
    /**
     * Resource input stream.
     */
    std::shared_ptr<std::ifstream> input_stream;

    /**
     * Resource output stream.
     */
    std::shared_ptr<std::ofstream> output_stream;

  public:
    /**
     * PAK header
     */
    libpak::pak_header pak_header;
    /**
     * Content header
     */
    libpak::content_header content_header;

    /**
     * Data header
     */
    libpak::data_header data_header;

  public:
    /**
     * Map of all assets indexed by their name.
     */
    asset_map assets;
  };

} // namespace libpak

#endif // libpak_libpak_HPP
