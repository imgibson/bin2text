/**
 * 
 * @author Anders Lind (96395432+imgibson@users.noreply.github.com)
 * @date 2024-06-17
 * 
 * Copyright (c) 2024 Anders Lind (https://github.com/imgibson). All rights reserved.
 * 
 */

#pragma once

#include <cassert>
#include <cstdint>
#include <cstring>
#include <exception>

namespace base85 {

static std::size_t decoded_length(const char* charset, const char* str, std::size_t len) {
#ifdef _DEBUG
    assert(str);
    assert(charset && std::strlen(charset) == 85);
#endif
    const auto is_valid = [&charset](char c) -> bool {
        return std::strchr(charset, c) != nullptr;
    };
    std::size_t t = len % 5;
    if (t == 1) {
        throw std::exception();
    }
    for (std::size_t i = 0; i < len; i++) {
        if (!is_valid(str[i])) {
            throw std::exception();
        }
    }
    return (len / 5) * 4 + (t == 0 ? 0 : 4 - (5 - t));
}

template <std::size_t N>
static std::size_t decode(const char* charset, const char* str, std::size_t len, char (&result)[N]) {
#ifdef _DEBUG
    assert(str);
    assert(charset && std::strlen(charset) == 85);
#endif
    static_assert(N > 0);
    const auto strmap = [&charset](char c) -> char {
        const char* pos = std::strchr(charset, c);
        if (!pos) {
            throw std::exception();
        }
        return static_cast<char>(pos - charset);
    };
    std::size_t size = 0;
    auto append = [&](char c) -> void {
        if (size >= N) {
            throw std::exception();
        }
        result[size++] = c;
    };
    struct puple_t {
        char v[5];
    };
    const auto b85 = [&]<std::size_t M>(puple_t puple) -> void {
        std::uint32_t value = 0;
        std::uint32_t divisor = 85 * 85 * 85 * 85;
        for (std::size_t i = 0; i < 5; i++) {
            value += static_cast<std::uint32_t>(strmap(puple.v[i])) * divisor;
            divisor /= 85;
        }
        for (std::size_t i = 0; i < M; i++) {
            append(static_cast<char>(value >> (24 - i * 8)));
        }
    };
    while (len > 5) {
        b85.template operator()<4>({str[0], str[1], str[2], str[3], str[4]});
        str += 5;
        len -= 5;
    };
    if (len == 5) {
        b85.template operator()<4>({str[0], str[1], str[2], str[3], str[4]});
    } else if (len == 4) {
        b85.template operator()<3>({str[0], str[1], str[2], str[3], '#'});
    } else if (len == 3) {
        b85.template operator()<2>({str[0], str[1], str[2], '#', '#'});
    } else if (len == 2) {
        b85.template operator()<1>({str[0], str[1], '#', '#', '#'});
    } else {
        throw std::exception();
    }
    return size;
}

} // namespace base85
