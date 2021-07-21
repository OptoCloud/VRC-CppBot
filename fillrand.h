#ifndef FILLRAND_H
#define FILLRAND_H

#include <cstdlib>
#include <cstdint>

namespace VRChad::Utils {
void FillRandHex(char* data, std::size_t size);
void FillRandBytes(std::uint8_t* data, std::size_t size);
}

#endif // FILLRAND_H
