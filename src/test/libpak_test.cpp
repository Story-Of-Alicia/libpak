//
// Created by maros on 15.3.2023.
//

#include <libpak/libpak.hpp>

struct Dummy {
  int a;
  int b;
};

int main(int argc, char** argv)
{

  libpak::resource resource("/run/media/maros/windows_data/maros/games/Alicia/res.pak");
  resource.read();

  for(auto& pair : resource.assets) {
    auto& asset = pair.second;
    std::u16string t(asset.header.path);
    if(t.find(u"libconfig") != std::wstring::npos) {
      resource.read_asset_data(asset);
    }
  }

  return 0;
}