// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef __TYPE_SERIALIZER_H__
#define __TYPE_SERIALIZER_H__

#include <type_traits>
#include <string>

#include "Serialization/BaseBuffer.h"

namespace v8App
{
    namespace Serialization
    {

        template<typename T>
        concept IsConstType = std::is_const<T>::value;

        /**
         * Templated byte swapper that onyl works for pod types that are
         * single or even byte sizes
         */
        template <typename T>
        void SwapBytes(T &inValue)
        {
            static_assert(std::is_integral<T>::value || std::is_floating_point<T>::value);
            char *p = reinterpret_cast<char *>(const_cast<std::remove_const<T>::type *>(&inValue));
            // if the size is only one byte return no swapping needed
            if constexpr (sizeof(T) == 1)
            {
                return;
            }
            if constexpr (sizeof(T) == 2)
            {
                std::swap(p[0], p[1]);
                return;
            }
            else // 4+
            {
                for (size_t i = 0; i < sizeof(T) / 2; i++)
                {
                    std::swap(p[sizeof(T) - 1 - i], p[i]);
                }
            }
        }

        template <typename T>
        concept IsFixedSizedType = (std::is_floating_point<T>::value || std::is_integral<T>::value);

        template <typename T>
        struct TypeSerializer
        {
            static bool Serialize(BaseBuffer &inBuffer, T &inValue)
                requires IsFixedSizedType<T>
            {
                if (inBuffer.IsWriter())
                {
                    T swapped = inValue;
                    if (inBuffer.IsByteSwapping())
                    {
                        SwapBytes(swapped);
                    }
                    inBuffer.SerializeWrite(&swapped, sizeof(T));
                }
                else
                {
                    inBuffer.SerializeRead(&inValue, sizeof(T));
                    if (inBuffer.IsByteSwapping())
                    {
                        SwapBytes(inValue);
                    }
                }
                return true;
            }

            static bool Serialize(BaseBuffer &inBuffer, T &inValue)
                requires IsFixedSizedType<T> && IsConstType<T>
            {
                // cant serialize read into a const value
                if (inBuffer.IsReader())
                {
                    return false;
                }
                T swapped = inValue;
                if (inBuffer.IsByteSwapping())
                {
                    SwapBytes(swapped);
                }
                inBuffer.SerializeWrite(&swapped, sizeof(T));

                return true;
            }
        };

        /**
         * Specialization for strings.
         * NOTE: they should be null terminated strings since
         * we have to figure out the lengths
         */
        template <>
        struct TypeSerializer<wchar_t *>
        {
            static bool Serialize(BaseBuffer &inBuffer, wchar_t *inValue);
        };

        template <>
        struct TypeSerializer<const wchar_t *>
        {
            static bool Serialize(BaseBuffer &inBuffer, const wchar_t *inValue);
        };

        template <>
        struct TypeSerializer<char *>
        {
            static bool Serialize(BaseBuffer &inBuffer, char *inValue);
        };

        template <>
        struct TypeSerializer<const char *>
        {
            static bool Serialize(BaseBuffer &inBuffer, const char *inValue);
        };

        template <>
        struct TypeSerializer<std::string>
        {
            static bool Serialize(BaseBuffer &inBuffer, std::string &inValue);
        };

        template <>
        struct TypeSerializer<const std::string>
        {
            static bool Serialize(BaseBuffer &inBuffer, const std::string &inValue);
        };

        template <>
        struct TypeSerializer<std::wstring>
        {
            static bool Serialize(BaseBuffer &inBuffer, std::wstring &inValue);
        };

        template <>
        struct TypeSerializer<const std::wstring>
        {
            static bool Serialize(BaseBuffer &inBuffer, const std::wstring &inValue);
        };

        BaseBuffer &operator<<(BaseBuffer &inBuffer, wchar_t *inValue);
        BaseBuffer &operator<<(BaseBuffer &inBuffer, const wchar_t *inValue);
        BaseBuffer &operator<<(BaseBuffer &inBuffer, char *inValue);
        BaseBuffer &operator<<(BaseBuffer &inBuffer, const char *inValue);
        BaseBuffer &operator<<(BaseBuffer &inBuffer, std::string &inValue);
        BaseBuffer &operator<<(BaseBuffer &inBuffer, const std::string &inValue);
        BaseBuffer &operator<<(BaseBuffer &inBuffer, std::wstring &inValue);
        BaseBuffer &operator<<(BaseBuffer &inBuffer, const std::wstring&inValue);

        /**
         * Templated function to serialize/deserilize a value from a buffer
         * We subclassing we wil be able to pass a reader/writer and it will
         * take care of the direction of the data.
         */
        template <typename T>
        BaseBuffer &operator<<(BaseBuffer &inBuffer, T inValue)
        {
            if (TypeSerializer<T>::Serialize(inBuffer, inValue) == false)
            {
                inBuffer.SetError();
            }
            return inBuffer;
        }
 
        BaseBuffer &operator>>(BaseBuffer &inBuffer, wchar_t *inValue);
        BaseBuffer &operator>>(BaseBuffer &inBuffer, const wchar_t *inValue);
        BaseBuffer &operator>>(BaseBuffer &inBuffer, char *inValue);
        BaseBuffer &operator>>(BaseBuffer &inBuffer, const char *inValue);
        BaseBuffer &operator>>(BaseBuffer &inBuffer, std::string &inValue);
        BaseBuffer &operator>>(BaseBuffer &inBuffer, const std::string &inValue);
        BaseBuffer &operator>>(BaseBuffer &inBuffer, std::wstring &inValue);
        BaseBuffer &operator>>(BaseBuffer &inBuffer, const std::wstring&inValue);

        /**
         * Templated function to serialize/deserilize a value from a buffer
         * We subclassing we wil be able to pass a reader/writer and it will
         * take care of the direction of the data.
         */
        template <typename T>
        BaseBuffer &operator>>(BaseBuffer &inBuffer, T &inValue)
        {
            if ( TypeSerializer<T>::Serialize(inBuffer, inValue) == false)
            {
                inBuffer.SetError();
            }
            return inBuffer;
        }

    }
}

#endif //__TYPE_SERIALIZER_H__