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
#include <cstdint>
#include <cstring>
#include <exception>
#include <tuple>

namespace base64 {

enum class mode_t {
    standard,
    url_safe
};

template <mode_t mode = mode_t::standard>
static std::size_t encoded_length(const void* buffer, std::size_t length) noexcept {
#ifdef _DEBUG
    assert(buffer);
#endif
    std::ignore = buffer;
    if constexpr (mode == mode_t::standard) {
        return (length + 2) / 3 * 4;
    } else {
        std::size_t t = length % 3;
        return (length / 3) * 4 + (t == 0 ? 0 : t + 1);
    }
}

template <mode_t mode = mode_t::standard, std::size_t N>
static std::size_t encode(const void* buffer, std::size_t length, char (&result)[N]) noexcept {
#ifdef _DEBUG
    assert(buffer);
#endif
    static_assert(N > 0);
    const char* kCharMap = []() consteval -> const char* {
        if constexpr (mode == mode_t::standard) {
            return R"(ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/)";
        } else {
            return R"(ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_)";
        }
    }();
    std::size_t size = encoded_length<mode>(buffer, length);
    if (size >= N) {
        return size;
    }
    size = 0;
    const char* bufptr = static_cast<const char*>(buffer);
    while (length >= 3) {
        result[size++] = kCharMap[(bufptr[0] & 0xfc) >> 2];
        result[size++] = kCharMap[(bufptr[0] & 0x03) << 4 | (bufptr[1] & 0xf0) >> 4];
        result[size++] = kCharMap[(bufptr[1] & 0x0f) << 2 | (bufptr[2] & 0xc0) >> 6];
        result[size++] = kCharMap[(bufptr[2] & 0x3f)];
        bufptr += 3;
        length -= 3;
    };
    if (length >= 2) {
        result[size++] = kCharMap[(bufptr[0] & 0xfc) >> 2];
        result[size++] = kCharMap[(bufptr[0] & 0x03) << 4 | (bufptr[1] & 0xf0) >> 4];
        result[size++] = kCharMap[(bufptr[1] & 0x0f) << 2];
        if constexpr (mode == mode_t::standard) {
            result[size++] = '=';
        }
    } else if (length >= 1) {
        result[size++] = kCharMap[(bufptr[0] & 0xfc) >> 2];
        result[size++] = kCharMap[(bufptr[0] & 0x03) << 4];
        if constexpr (mode == mode_t::standard) {
            result[size++] = '=';
            result[size++] = '=';
        }
    }
    result[size] = '\0';
    return size;
}

template <mode_t mode = mode_t::standard>
static std::size_t decoded_length(const char* str, std::size_t length) {
#ifdef _DEBUG
    assert(str);
#endif
    if (length == 0) {
        return 0;
    }
    std::size_t t = length % 4;
    if constexpr (mode == mode_t::standard) {
        if (t != 0) {
            throw std::exception();
        }
    } else {
        if (t == 1) {
            throw std::exception();
        }
    }
    const auto is_valid = [](char c) -> bool {
        const char* kCharMap = []() consteval -> const char* {
            if constexpr (mode == mode_t::standard) {
                return R"(ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/)";
            } else {
                return R"(ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_)";
            }
        }();
        return std::strchr(kCharMap, c) != nullptr;
    };
    std::size_t i = 0;
    while (i < length && is_valid(str[i])) {
        ++i;
    };
    if constexpr (mode == mode_t::standard) {
        const auto is_trail = [](char c) -> bool {
            return c == '=';
        };
        std::size_t j = i;
        while (j < length && is_trail(str[j])) {
            ++j;
        };
        if (j != length || (j - i) > 2) {
            throw std::exception();
        }
        return length / 4 * 3 - (j - i);
    } else {
        if (i != length) {
            throw std::exception();
        }
        return length / 4 * 3 + (t == 0 ? 0 : t - 1);
    }
}

template <mode_t mode = mode_t::standard, std::size_t N>
static std::size_t decode(const char* str, std::size_t length, char (&result)[N]) {
#ifdef _DEBUG
    assert(str);
#endif
    static_assert(N > 0);
    const auto strmap = [](char c) -> char {
        const char* kCharMap = []() consteval -> const char* {
            if constexpr (mode == mode_t::standard) {
                return R"(ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/)";
            } else {
                return R"(ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_)";
            }
        }();
        const char* pos = std::strchr(kCharMap, c);
        if (!pos) {
            throw std::exception();
        }
        return static_cast<char>(pos - kCharMap);
    };
    std::size_t size = 0;
    const auto append = [&](char c) -> void {
        if (size >= N) {
            throw std::exception();
        }
        result[size++] = c;
    };
    while (length > 4) {
        char buffer[4];
        for (std::size_t i = 0; i < 4; ++i) {
            buffer[i] = strmap(*str++);
        }
        append(buffer[0] << 2 | buffer[1] >> 4);
        append(buffer[1] << 4 | buffer[2] >> 2);
        append(buffer[2] << 6 | buffer[3]);
        length -= 4;
    };
    if (length >= 2) {
        if constexpr (mode == mode_t::standard) {
            while (length > 2 && str[length - 1] == '=') {
                --length;
            };
        }
        char buffer[4]{};
        for (std::size_t i = 0; i < length; ++i) {
            buffer[i] = strmap(*str++);
        }
        if (length >= 4) {
            append(buffer[0] << 2 | buffer[1] >> 4);
            append(buffer[1] << 4 | buffer[2] >> 2);
            append(buffer[2] << 6 | buffer[3]);
        } else if (length >= 3) {
            append(buffer[0] << 2 | buffer[1] >> 4);
            append(buffer[1] << 4 | buffer[2] >> 2);
        } else {
            append(buffer[0] << 2 | buffer[1] >> 4);
        }
    } else {
        throw std::exception();
    }
    return size;
}

template <mode_t mode = mode_t::standard>
static std::size_t decoded_length(const char* str) {
#ifdef _DEBUG
    assert(str);
#endif
    return decoded_length<mode>(str, std::strlen(str));
}

template <mode_t mode = mode_t::standard, std::size_t N>
static std::size_t decode(const char* str, char (&result)[N]) {
#ifdef _DEBUG
    assert(str);
#endif
    return decode<mode>(str, std::strlen(str), result);
}

} // namespace base64
