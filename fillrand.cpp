#include "fillrand.h"

#include <ctime>
#include <random>

std::mt19937_64 randgen;

void VRChad::Utils::FillRandHex(char* data, size_t size)
{
    VRChad::Utils::FillRandBytes((std::uint8_t*)data, size);

    std::uint8_t* begin = (std::uint8_t*)data;
    std::uint8_t* end = begin + size;
    std::uint8_t* it = begin;
    while (it != end) {
        std::uint8_t val16 = '0' + (*it % 16);

        if (val16 > '9') {
            val16 += 7;
        }

        *(it++) = val16;
    }
}

void VRChad::Utils::FillRandBytes(uint8_t* data, size_t size)
{
    randgen.seed(std::time(nullptr));

    std::uint8_t* begin = data;
    std::uint8_t* end = begin + size;
    std::uint8_t* it = begin;
    while (it + 8 <= end) {
        *reinterpret_cast<std::uint64_t*>(it) = randgen();
        it += 8;
    }

    int i = 0;
    std::uint64_t rest = randgen();
    while (it <= end) {
        *(it++) = ((std::uint8_t*)&rest)[i++];
    }
}
