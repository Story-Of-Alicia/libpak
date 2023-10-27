# libpak
libpak is platform-agnostic implementation for reading and writing in the ntreev paks format.
Example:
```cpp
#include <libpak/libpak.hpp>

int main() {
  libpak::resource resource("res.pak");
  resource.read();

  for(auto& [path, asset] : resource.assets) {
    if(path.find(u"libconfig") != std::wstring::npos) {
      resource.read_asset_data(asset);
    }
  }
}
```
