//
// Created by maros on 15.3.2023.
//

#include <string.h>
#include <zlib.h>
#include <libpak/libpak.hpp>

int main()
{
    libpak::resource resource(
      "/run/media/maros/windows_data/maros/games/Alicia/res.pak"
      );
    resource.read(true);


    for(auto& asset : resource) {
      printf("asset: %ls\n", asset.first.cbegin());
    }


    return 0;
}