//
// Created by maros on 15.3.2023.
//

#ifndef LIBPAK_ALGORITHMS_HPP
#define LIBPAK_ALGORITHMS_HPP

#include <cstdint>

namespace libpak {
  namespace alg {

    /**
     * Perform alicia checksum on a buffer.
     * @param buffer Buffer
     * @param length Length
     * @return Checksum
     */
    int32_t alicia_checksum(const char* buffer, uint64_t length)
    {
      int32_t result = 0;
      do {
        result += *buffer;
        buffer++;
        length--;
      } while(length != 0);
      return result;
    }



  } // namespace alg
} // namespace libpak

#endif // LIBPAK_ALGORITHMS_HPP
