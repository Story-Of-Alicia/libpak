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

  using asset_entry = std::pair<std::wstring_view, libpak::asset>;
  using asset_map = std::unordered_map<std::wstring_view, libpak::asset>;

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
     * @param name Asset id
     * @return Asset reference
     */
    inline libpak::asset& operator[](const std::wstring_view& name) {
      return this->asset_index.at(name);
    }

    /**
     * @return Asset iterator beginning
     */
    inline asset_map::const_iterator begin() const noexcept { return this->asset_index.begin(); }

    /**
     * @return Asset iterator end
     */
    inline asset_map::const_iterator end() const noexcept { return this->asset_index.end(); }

  private:
    std::string_view resourcePath;

  protected:
    std::shared_ptr<std::ifstream> input_stream;
    std::shared_ptr<std::ofstream> output_stream;

  protected:
    libpak::pak_header pak_header;
    libpak::content_header content_header;
    libpak::data_header data_header;

    asset_map asset_index;
  };

} // namespace libpak

#endif // libpak_libpak_HPP
