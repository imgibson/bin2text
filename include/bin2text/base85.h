/**
 *
 * @author Anders Lind (96395432+imgibson@users.noreply.github.com)
 * @date 2024-06-17
 *
 * Copyright (c) 2024 Anders Lind (https://github.com/imgibson). All rights reserved.
 *
 */

#pragma once

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>

namespace base85 {

static std::ptrdiff_t decoded_length(const char* charset, const char* str, std::size_t len) noexcept {
#ifdef _DEBUG
    assert(str);
    assert(charset && std::strlen(charset) == 85);
#endif
    std::size_t t = len % 5;
    if (t == 1) {
        return -1;
    }
    for (std::size_t i = 0; i < len; i++) {
        if (std::strchr(charset, str[i]) == nullptr) {
            return -1;
        }
    }
    return (len / 5) * 4 + (t == 0 ? 0 : 4 - (5 - t));
}

static std::ptrdiff_t decoded_length(const char* charset, const char* str) noexcept {
    return decoded_length(charset, str, std::strlen(str));
}

template <std::size_t N>
static std::ptrdiff_t decode(const char* charset, const char* str, std::size_t len, char (&result)[N]) noexcept
    requires(N > 0) {
#ifdef _DEBUG
    assert(str);
    assert(charset && std::strlen(charset) == 85);
#endif
    std::size_t size = 0;
    const auto b85 = [&]<std::size_t M>(const std::array<char, 5>& arr5) -> bool {
        std::uint32_t value = 0;
        std::uint32_t divisor = 85 * 85 * 85 * 85;
        for (std::size_t i = 0; i < 5; i++) {
            const char* pos = std::strchr(charset, arr5[i]);
            if (!pos) {
                return false;
            }
            value += static_cast<std::uint32_t>(static_cast<char>(pos - charset)) * divisor;
            divisor /= 85;
        }
        for (std::size_t i = 0; i < M; i++) {
            if (size >= N) {
                return false;
            }
            result[size++] = static_cast<char>(value >> (24 - i * 8));
        }
        return true;
    };
    while (len > 5) {
        if (!b85.template operator()<4>({str[0], str[1], str[2], str[3], str[4]})) return -1;
        str += 5;
        len -= 5;
    }
    if (len == 5) {
        if (!b85.template operator()<4>({str[0], str[1], str[2], str[3], str[4]})) return -1;
    } else if (len == 4) {
        if (!b85.template operator()<3>({str[0], str[1], str[2], str[3], '#'})) return -1;
    } else if (len == 3) {
        if (!b85.template operator()<2>({str[0], str[1], str[2], '#', '#'})) return -1;
    } else if (len == 2) {
        if (!b85.template operator()<1>({str[0], str[1], '#', '#', '#'})) return -1;
    } else {
        return -1;
    }
    return static_cast<std::ptrdiff_t>(size);
}

template <std::size_t N>
static std::ptrdiff_t decode(const char* charset, const char* str, char (&result)[N]) noexcept {
    return decode(charset, str, std::strlen(str), result);
}

} // namespace base85
