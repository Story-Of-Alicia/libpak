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
//    auto write = std::make_shared<std::ofstream>("test.binary", std::ios::binary);
//    auto read = std::make_shared<std::ifstream>("test.binary", std::ios::binary);
//    auto stream = libpak::stream(read, write);
//    {
//      auto d = Dummy{5,5};
//      stream.write(d);
//    }
//    write->flush();
//    {
//      auto d = Dummy{0,0};
//      if(!stream.read(d))
//        printf("a");
//      else
//        printf("b");
//    }

  libpak::resource resource("res.pak");
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