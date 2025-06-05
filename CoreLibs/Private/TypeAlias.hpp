#pragma once

#include <cstdint>
#include <cstring>

using int8 = int8_t;
using int16 = int16_t;
using int32 = int32_t;
using int64 = int64_t;

using uint8 = uint8_t;
using uint16 = uint16_t;
using uint32 = uint32_t;
using uint64 = uint64_t;

using autosize = size_t;
using sautosize = std::make_signed_t<autosize>;

template <typename T>
using vec = std::vector<T>;
template <typename T, autosize Size>
using arr = std::array<T, Size>;
template <typename T>
using uset = std::unordered_set<T>;
template <typename T>
using que = std::queue<T>;

using str = std::string;
