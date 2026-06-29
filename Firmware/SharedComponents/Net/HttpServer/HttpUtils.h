#pragma once

#include <cctype>
#include <cstdlib>
#include <cstring>

namespace HttpUtils {
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

inline bool joinUri(char* dst, size_t dstSize, const char* route, const char* uri) {
    if (dst == nullptr || dstSize == 0 || route == nullptr || uri == nullptr) {
        return false;
    }

    int written;
    if (route[0] == '\0' && uri[0] == '\0') {
        written = snprintf(dst, dstSize, "/");
    } else if (route[0] == '\0') {
        written = snprintf(dst, dstSize, "/%s", uri);
    } else if (uri[0] == '\0') {
        written = snprintf(dst, dstSize, "/%s", route);
    } else {
        written = snprintf(dst, dstSize, "/%s/%s", route, uri);
    }

    return written > 0 && (size_t)written < dstSize;
}

}  // namespace HttpUtils
