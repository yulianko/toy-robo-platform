#pragma once

#include <cctype>
#include <cstdlib>
#include <cstring>

inline int urlDecode(const char* src, char* dst, int dstSize) {
    int di = 0;
    for (int si = 0; src[si] != '\0' && di < dstSize - 1; ++si) {
        if (src[si] == '+') {
            dst[di++] = ' ';
        } else if (src[si] == '%' && src[si + 1] != '\0' && src[si + 2] != '\0' && isxdigit((uint8_t)src[si + 1]) &&
                   isxdigit((uint8_t)src[si + 2])) {
            char hex[3] = {src[si + 1], src[si + 2], '\0'};
            dst[di++] = static_cast<char>(strtol(hex, nullptr, 16));
            si += 2;
        } else {
            dst[di++] = src[si];
        }
    }
    dst[di] = '\0';
    return di;
}
