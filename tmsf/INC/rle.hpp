#pragma once

#include <srl.hpp>

inline void RLE_Decompress(const uint8_t* input, uint8_t* output, size_t decompressedSize)
{
    size_t in = 0;
    size_t out = 0;

    while (out < decompressedSize)
    {
        uint8_t run   = input[in++];
        uint8_t value = input[in++];

        for (uint8_t j = 0; j < run; j++)
        {
            output[out++] = value;
        }
    }
}