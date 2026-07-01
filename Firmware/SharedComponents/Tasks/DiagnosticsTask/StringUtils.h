#pragma once

#include <algorithm>
#include <cstddef>
#include <cstring>

namespace strutil {

inline void copyWithTrailingNewline(const char* src, char* dst, size_t dstSize) {
    if (dst == nullptr || dstSize < 2) {
        return;
    }

    dst[0] = '\0';
    if (src == nullptr) {
        return;
    }

    const size_t srcLen = std::strlen(src);
    const size_t copyLen = std::min(srcLen, dstSize - 2);
    std::memcpy(dst, src, copyLen);

    if (copyLen > 0 && dst[copyLen - 1] == '\n') {
        dst[copyLen] = '\0';
    } else {
        dst[copyLen] = '\n';
        dst[copyLen + 1] = '\0';
    }
}

}  // namespace strutil
