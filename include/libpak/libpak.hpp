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

  /**
   * Represents a single resource which holds assets and their accompanying data.
   */
  class resource {
  private:
    std::string_view resourcePath;

  protected:
    std::shared_ptr<std::ifstream> input_stream;
    std::shared_ptr<std::ofstream> output_stream;

  protected:
    libpak::pak_header pak_header;
    libpak::content_header content_header;
    libpak::data_header data_header;

    std::unordered_map<std::wstring_view, libpak::asset> asset_index;

  public:
    /**
     * Default constructor.
     * @param path Path to resource.
     * @param create Whether to create the resource.
     */
    explicit resource(std::string_view path,
                      bool create = true)
        : resourcePath(path) {
              if(create)
                this->create();
          };

  public:
    /**
     * Read the whole resource and write it to memory.
     * @param data Whether to read the data.
     * @return True if successful, false if failed.
     */
    bool read(bool data = false);

    /**
     * Write the resource to persistent memory.
     * @param cleanup Whether to cleanup memory after.
     * @return True if successful, false if failed.
     */
    bool write(bool cleanup = false);

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
     * Get asset by its identifier.
     * @param asset_id Identifier of the asset.
     * @param data Whether to read it's data to memory.
     * @return Reference to asset.
     */
    asset& get_asset(const std::wstring_view& asset_id,
                 bool data = false);
  };

} // namespace libpak

#endif // libpak_libpak_HPP
