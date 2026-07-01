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

inline bool getQueryInt(httpd_req_t* req, const char* key, int& out, long minVal = 1, long maxVal = INT_MAX) {
    char query[64];
    esp_err_t err = httpd_req_get_url_query_str(req, query, sizeof(query));
    if (err != ESP_OK) {
        return false;
    }

    char value[16];
    if (httpd_query_key_value(query, key, value, sizeof(value)) != ESP_OK) {
        return false;
    }

    char* end = nullptr;
    long parsed = std::strtol(value, &end, 10);
    if (end == value || *end != '\0' || parsed < minVal || parsed > maxVal) {
        return false;
    }

    out = static_cast<int>(parsed);
    return true;
}

}  // namespace HttpUtils
