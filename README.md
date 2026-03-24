# libpak
libpak is library for manipulating the Ntreev PAK format

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
