//
// Created by maros on 15.3.2023.
//

#include <iostream>
#include <libpak/libpak.hpp>

int main()
{
    libpak::resource resource(
      "/run/media/maros/windows_data/maros/games/Alicia/res.pak"
      );
    resource.read(true);
    printf("test");


    return 0;
}