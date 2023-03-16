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
    explicit resource(std::string_view path, bool create = true) : resourcePath(path)
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
     * @param asset Indexed asset.
     * @param data  Whether to read the asset data.
     * @throws std::runtime_error
     */
    void read_asset(asset& asset, bool data = false);

    /**
     * Reads asset data from the resource.
     * @param asset Indexed asset.
     * @return True if asset data were read.
     * @throws std::runtime_error
     */
    void read_asset_data(asset& asset);

    /**
     * Writes the assets to the resource.
     * @param patch Whether to write only modified assets.
     * @param data  Whether to write asset data.
     * @throws std::runtime_error
     */
    void write(bool patch = true, bool data = true);


    void write_asset(const libpak::asset& asset, bool data = true);
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
     * @param name Asset ID.
     * @return Indexed asset.
     */
    inline libpak::asset& operator[](const std::u16string_view & name) {
      return this->assets.at(name);
    }


  private:
    std::string_view resourcePath;

  protected:
    std::shared_ptr<std::ifstream> input_stream;
    std::shared_ptr<std::ofstream> output_stream;

  protected:
    libpak::pak_header pak_header;
    libpak::content_header content_header;
    libpak::data_header data_header;

  public:
    asset_map assets;
  };

} // namespace libpak

#endif // libpak_libpak_HPP
