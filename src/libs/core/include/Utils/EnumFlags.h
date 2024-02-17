// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef __ENUM_FLAGS_H_
#define __ENUM_FLAGS_H_

#include <type_traits>

namespace v8App
{
    namespace Utils
    {
        template <typename T, typename = std::enable_if<std::is_enum<T>::value, T>::type>
        inline T operator~(T a) { return (T) ~(int)a; }
        template <typename T, typename = std::enable_if<std::is_enum<T>::value, T>::type>
        inline T operator|(T a, T b) { return (T)((int)a | (int)b); }
        template <typename T, typename = std::enable_if<std::is_enum<T>::value, T>::type>
        inline T operator&(T a, T b) { return (T)((int)a & (int)b); }
        template <typename T, typename = std::enable_if<std::is_enum<T>::value, T>::type>
        inline T operator^(T a, T b) { return (T)((int)a ^ (int)b); }
        template <typename T, typename = std::enable_if<std::is_enum<T>::value, T>::type>
        inline T &operator|=(T &a, T b) { return (T &)((int &)a |= (int)b); }
        template <typename T, typename = std::enable_if<std::is_enum<T>::value, T>::type>
        inline T &operator&=(T &a, T b) { return (T &)((int &)a &= (int)b); }
        template <typename T, typename = std::enable_if<std::is_enum<T>::value, T>::type>
        inline T &operator^=(T &a, T b) { return (T &)((int &)a ^= (int)b); }

    } // namespace Utils

} // namespace v8App

#endif //__ENUM_FLAGS_H_