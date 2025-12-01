/**
 *
 * @author Anders Lind (96395432+imgibson@users.noreply.github.com)
 * @date 2024-06-10
 *
 * Copyright (c) 2024 Anders Lind (https://github.com/imgibson). All rights reserved.
 *
 */

#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <tuple>

namespace base64 {

enum class mode_t { standard, url_safe };

template <mode_t mode = mode_t::standard>
static std::size_t encoded_length(const void* buffer, std::size_t size) noexcept {
#ifdef _DEBUG
    assert(buffer);
#endif
    std::ignore = buffer;
    if constexpr (mode == mode_t::standard) {
        return (size + 2) / 3 * 4;
    } else {
        std::size_t t = size % 3;
        return (size / 3) * 4 + (t == 0 ? 0 : t + 1);
    }
}

template <mode_t mode = mode_t::standard, std::size_t N>
static std::size_t encode(const void* buffer, std::size_t size, char (&result)[N]) noexcept
    requires(N > 0) {
#ifdef _DEBUG
    assert(buffer);
#endif
    const char* kCharSet = []() consteval -> const char* {
        if constexpr (mode == mode_t::standard) {
            return R"(ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/)";
        } else {
            return R"(ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_)";
        }
    }();
    std::size_t length = encoded_length<mode>(buffer, size);
    if (length >= N) {
        return length;
    }
    length = 0;
    const char* bufptr = static_cast<const char*>(buffer);
    while (size >= 3) {
        result[length++] = kCharSet[(bufptr[0] & 0xfc) >> 2];
        result[length++] = kCharSet[(bufptr[0] & 0x03) << 4 | (bufptr[1] & 0xf0) >> 4];
        result[length++] = kCharSet[(bufptr[1] & 0x0f) << 2 | (bufptr[2] & 0xc0) >> 6];
        result[length++] = kCharSet[(bufptr[2] & 0x3f)];
        bufptr += 3;
        size -= 3;
    }
    if (size >= 2) {
        result[length++] = kCharSet[(bufptr[0] & 0xfc) >> 2];
        result[length++] = kCharSet[(bufptr[0] & 0x03) << 4 | (bufptr[1] & 0xf0) >> 4];
        result[length++] = kCharSet[(bufptr[1] & 0x0f) << 2];
        if constexpr (mode == mode_t::standard) {
            result[length++] = '=';
        }
    } else if (size >= 1) {
        result[length++] = kCharSet[(bufptr[0] & 0xfc) >> 2];
        result[length++] = kCharSet[(bufptr[0] & 0x03) << 4];
        if constexpr (mode == mode_t::standard) {
            result[length++] = '=';
            result[length++] = '=';
        }
    }
    result[length] = '\0';
    return length;
}

template <mode_t mode = mode_t::standard>
static std::ptrdiff_t decoded_length(const char* str, std::size_t length) noexcept {
#ifdef _DEBUG
    assert(str);
#endif
    if (length == 0) {
        return 0;
    }
    std::size_t t = length % 4;
    if constexpr (mode == mode_t::standard) {
        if (t != 0) {
            return -1;
        }
    } else {
        if (t == 1) {
            return -1;
        }
    }
    const char* kCharSet = []() consteval -> const char* {
        if constexpr (mode == mode_t::standard) {
            return R"(ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/)";
        } else {
            return R"(ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_)";
        }
    }();
    std::size_t i = 0;
    while (i < length && std::strchr(kCharSet, str[i]) != nullptr) {
        ++i;
    }
    if constexpr (mode == mode_t::standard) {
        std::size_t j = i;
        while (j < length && str[j] == '=') {
            ++j;
        }
        if (j != length || (j - i) > 2) {
            return -1;
        }
        return length / 4 * 3 - (j - i);
    } else {
        if (i != length) {
            return -1;
        }
        return length / 4 * 3 + (t == 0 ? 0 : t - 1);
    }
}

template <mode_t mode = mode_t::standard>
static std::ptrdiff_t decoded_length(const char* str) noexcept {
    return decoded_length<mode>(str, std::strlen(str));
}

template <mode_t mode = mode_t::standard, std::size_t N>
static std::ptrdiff_t decode(const char* str, std::size_t length, char (&result)[N]) noexcept
    requires(N > 0) {
#ifdef _DEBUG
    assert(str);
#endif
    const char* kCharSet = []() consteval -> const char* {
        if constexpr (mode == mode_t::standard) {
            return R"(ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/)";
        } else {
            return R"(ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_)";
        }
    }();
    std::size_t size = 0;
    const auto append = [&](char c) -> bool {
        if (size >= N) {
            return false;
        }
        result[size++] = c;
        return true;
    };
    while (length > 4) {
        char buffer[4]{};
        for (std::size_t i = 0; i < 4; ++i) {
            const char* pos = std::strchr(kCharSet, *str++);
            if (!pos) {
                return -1;
            }
            buffer[i] = static_cast<char>(pos - kCharSet);
        }
        if (!append(buffer[0] << 2 | buffer[1] >> 4)) return -1;
        if (!append(buffer[1] << 4 | buffer[2] >> 2)) return -1;
        if (!append(buffer[2] << 6 | buffer[3])) return -1;
        length -= 4;
    }
    if (length >= 2) {
        if constexpr (mode == mode_t::standard) {
            while (length > 2 && str[length - 1] == '=') {
                --length;
            }
        }
        char buffer[4]{};
        for (std::size_t i = 0; i < length; ++i) {
            const char* pos = std::strchr(kCharSet, *str++);
            if (!pos) {
                return -1;
            }
            buffer[i] = static_cast<char>(pos - kCharSet);
        }
        if (length >= 4) {
            if (!append(buffer[0] << 2 | buffer[1] >> 4)) return -1;
            if (!append(buffer[1] << 4 | buffer[2] >> 2)) return -1;
            if (!append(buffer[2] << 6 | buffer[3])) return -1;
        } else if (length >= 3) {
            if (!append(buffer[0] << 2 | buffer[1] >> 4)) return -1;
            if (!append(buffer[1] << 4 | buffer[2] >> 2)) return -1;
        } else {
            if (!append(buffer[0] << 2 | buffer[1] >> 4)) return -1;
        }
    } else {
        return -1;
    }
    return static_cast<std::ptrdiff_t>(size);
}

template <mode_t mode = mode_t::standard, std::size_t N>
static std::ptrdiff_t decode(const char* str, char (&result)[N]) noexcept {
    return decode<mode>(str, std::strlen(str), result);
}

} // namespace base64
