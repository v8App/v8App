// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "Serialization/TypeSerializer.h"
#include "Serialization/BaseBuffer.h"
namespace V8App
{
    namespace Serialization
    {
        bool TypeSerializer<wchar_t *>::Serialize(BaseBuffer &inBuffer, wchar_t *inValue)
        {
            // we want to copy the /0 in the string as well
            size_t length;
            size_t swappedLength;
            if (inBuffer.IsWriter())
            {
                length = std::wcslen(inValue) + 1;
                swappedLength = length;

                if (inBuffer.IsByteSwapping())
                {
                    SwapBytes(swappedLength);
                }
                inBuffer.SerializeWrite(&swappedLength, sizeof(size_t));
            }
            else
            {
                inBuffer.SerializeRead(&swappedLength, sizeof(size_t));
                if (inBuffer.IsByteSwapping())
                {
                    SwapBytes(swappedLength);
                }
                length = swappedLength;
            }
            for (size_t i = 0; i < length; i++)
            {
                wchar_t wc = inValue[i];
                if (inBuffer.IsWriter())
                {
                    if (inBuffer.IsByteSwapping())
                    {
                        SwapBytes(wc);
                    }
                    inBuffer.SerializeWrite(&wc, sizeof(wchar_t));
                }
                else
                {
                    inBuffer.SerializeRead(&wc, sizeof(wchar_t));
                    if (inBuffer.IsByteSwapping())
                    {
                        SwapBytes(wc);
                    }
                    inValue[i] = wc;
                }
            }
            return true;
        }

        bool TypeSerializer<const wchar_t *>::Serialize(BaseBuffer &inBuffer, const wchar_t *inValue)
        {
            // for a const wchar_t array we can't serialize into it so this only works for writing
            if (inBuffer.IsReader())
            {
                return false;
            }
            // we want to copy the /0 in the string as well
            size_t length = std::wcslen(inValue) + 1;
            size_t swappedLength = length;
            if (inBuffer.IsByteSwapping())
            {
                SwapBytes(swappedLength);
            }
            inBuffer.SerializeWrite(&swappedLength, sizeof(size_t));
            for (size_t i = 0; i < length; i++)
            {
                // wchar_t wc = static_cast<wchar_t>(inValue[i]);
                wchar_t wc = inValue[i];
                if (inBuffer.IsByteSwapping())
                {
                    SwapBytes(wc);
                }
                inBuffer.SerializeWrite(&wc, sizeof(wchar_t));
            }
            return true;
        }

        bool TypeSerializer<char *>::Serialize(BaseBuffer &inBuffer, char *inValue)
        {
            // we want to copy the /0 in the string as well
            size_t length;
            if (inBuffer.IsWriter())
            {
                length = std::strlen(inValue) + 1;
                size_t swappedLength = length;
                if (inBuffer.IsByteSwapping())
                {
                    SwapBytes(swappedLength);
                }
                inBuffer.SerializeWrite(&swappedLength, sizeof(size_t));
                inBuffer.SerializeWrite(inValue, length);
            }
            else
            {
                inBuffer.SerializeRead(&length, sizeof(size_t));
                if (inBuffer.IsByteSwapping())
                {
                    SwapBytes(length);
                }
                inBuffer.SerializeRead(inValue, length);
            }
            return true;
        }

        bool TypeSerializer<const char *>::Serialize(BaseBuffer &inBuffer, const char *inValue)
        {
            // we want to copy the /0 in the string as well
            if (inBuffer.IsReader())
            {
                return false;
            }
            size_t length = std::strlen(inValue) + 1;
            size_t swappedLength = length;
            if (inBuffer.IsByteSwapping())
            {
                SwapBytes(swappedLength);
            }
            inBuffer.SerializeWrite(&swappedLength, sizeof(size_t));
            inBuffer.SerializeWrite(inValue, length);
            return true;
        }

        bool TypeSerializer<std::string>::Serialize(BaseBuffer &inBuffer, std::string &inValue)
        {
            // we want to copy the /0 in the string as well
            size_t length;
            size_t swappedLength;
            if (inBuffer.IsWriter())
            {
                length = inValue.size();
                swappedLength = length;

                if (inBuffer.IsByteSwapping())
                {
                    SwapBytes(swappedLength);
                }
                inBuffer.SerializeWrite(&swappedLength, sizeof(size_t));
            }
            else
            {
                inBuffer.SerializeRead(&swappedLength, sizeof(size_t));
                if (inBuffer.IsByteSwapping())
                {
                    SwapBytes(swappedLength);
                }
                length = swappedLength;
                inValue.reserve(length);
            }
            for (size_t i = 0; i < length; i++)
            {
                char c = inValue[i];
                if (inBuffer.IsWriter())
                {
                    if (inBuffer.IsByteSwapping())
                    {
                        SwapBytes(c);
                    }
                    inBuffer.SerializeWrite(&c, sizeof(char));
                }
                else
                {
                    inBuffer.SerializeRead(&c, sizeof(char));
                    if (inBuffer.IsByteSwapping())
                    {
                        SwapBytes(c);
                    }
                    inValue+= c;
                }
            }
            return true;
        }

        bool TypeSerializer<const std::string>::Serialize(BaseBuffer &inBuffer, const std::string &inValue)
        {
            // we want to copy the /0 in the string as well
            if (inBuffer.IsReader())
            {
                return false;
            }
            size_t length = inValue.size();
            size_t swappedLength = length;
            if (inBuffer.IsByteSwapping())
            {
                SwapBytes(swappedLength);
            }
            inBuffer.SerializeWrite(&swappedLength, sizeof(size_t));
            inBuffer.SerializeWrite(inValue.c_str(), length);
            return true;
        }

        bool TypeSerializer<std::wstring>::Serialize(BaseBuffer &inBuffer, std::wstring &inValue)
        {
            // we want to copy the /0 in the string as well
            size_t length;
            size_t swappedLength;
            if (inBuffer.IsWriter())
            {
                length = inValue.size();
                swappedLength = length;

                if (inBuffer.IsByteSwapping())
                {
                    SwapBytes(swappedLength);
                }
                inBuffer.SerializeWrite(&swappedLength, sizeof(size_t));
            }
            else
            {
                inBuffer.SerializeRead(&swappedLength, sizeof(size_t));
                if (inBuffer.IsByteSwapping())
                {
                    SwapBytes(swappedLength);
                }
                length = swappedLength;
                inValue.reserve(length);
            }
            for (size_t i = 0; i < length; i++)
            {
                wchar_t wc = inValue[i];
                if (inBuffer.IsWriter())
                {
                    if (inBuffer.IsByteSwapping())
                    {
                        SwapBytes(wc);
                    }
                    inBuffer.SerializeWrite(&wc, sizeof(wchar_t));
                }
                else
                {
                    inBuffer.SerializeRead(&wc, sizeof(wchar_t));
                    if (inBuffer.IsByteSwapping())
                    {
                        SwapBytes(wc);
                    }
                    inValue += wc;
                }
            }
            return true;
        }

        bool TypeSerializer<const std::wstring>::Serialize(BaseBuffer &inBuffer, const std::wstring &inValue)
        {
            // for a const wchar_t array we can't serialize into it so this only works for writing
            if (inBuffer.IsReader())
            {
                return false;
            }
            // we want to copy the /0 in the string as well
            size_t length = inValue.size();
            size_t swappedLength = length;
            if (inBuffer.IsByteSwapping())
            {
                SwapBytes(swappedLength);
            }
            inBuffer.SerializeWrite(&swappedLength, sizeof(size_t));
            for (size_t i = 0; i < length; i++)
            {
                // wchar_t wc = static_cast<wchar_t>(inValue[i]);
                wchar_t wc = inValue[i];
                if (inBuffer.IsByteSwapping())
                {
                    SwapBytes(wc);
                }
                inBuffer.SerializeWrite(&wc, sizeof(wchar_t));
            }
            return true;
        }

        BaseBuffer &operator<<(BaseBuffer &inBuffer, wchar_t *inValue)
        {
            if (TypeSerializer<wchar_t *>::Serialize(inBuffer, inValue) == false)
            {
                inBuffer.SetError();
            }
            return inBuffer;
        }

        BaseBuffer &operator<<(BaseBuffer &inBuffer, const wchar_t *inValue)
        {
            if (TypeSerializer<const wchar_t *>::Serialize(inBuffer, inValue) == false)
            {
                inBuffer.SetError();
            }
            return inBuffer;
        }

        BaseBuffer &operator<<(BaseBuffer &inBuffer, char *inValue)
        {
            if (TypeSerializer<char *>::Serialize(inBuffer, inValue) == false)
            {
                inBuffer.SetError();
            }
            return inBuffer;
        }

        BaseBuffer &operator<<(BaseBuffer &inBuffer, const char *inValue)
        {
            if (TypeSerializer<const char *>::Serialize(inBuffer, inValue) == false)
            {
                inBuffer.SetError();
            }
            return inBuffer;
        }

        BaseBuffer &operator<<(BaseBuffer &inBuffer, std::string &inValue)
        {
            if (TypeSerializer<std::string>::Serialize(inBuffer, inValue) == false)
            {
                inBuffer.SetError();
            }
            return inBuffer;
        }

        BaseBuffer &operator<<(BaseBuffer &inBuffer, const std::string &inValue)
        {
            if (TypeSerializer<const std::string>::Serialize(inBuffer, inValue) == false)
            {
                inBuffer.SetError();
            }
            return inBuffer;
        }

        BaseBuffer &operator<<(BaseBuffer &inBuffer, std::wstring &inValue)
        {
            if (TypeSerializer<std::wstring>::Serialize(inBuffer, inValue) == false)
            {
                inBuffer.SetError();
            }
            return inBuffer;
        }

        BaseBuffer &operator<<(BaseBuffer &inBuffer, const std::wstring &inValue)
        {
            if (TypeSerializer<const std::wstring>::Serialize(inBuffer, inValue) == false)
            {
                inBuffer.SetError();
            }
            return inBuffer;
        }

        BaseBuffer &operator>>(BaseBuffer &inBuffer, wchar_t *inValue)
        {
            if (TypeSerializer<wchar_t *>::Serialize(inBuffer, inValue) == false)
            {
                inBuffer.SetError();
            }
            return inBuffer;
        }

        BaseBuffer &operator>>(BaseBuffer &inBuffer, const wchar_t *inValue)
        {
            if (TypeSerializer<const wchar_t *>::Serialize(inBuffer, inValue) == false)
            {
                inBuffer.SetError();
            }
            return inBuffer;
        }

        BaseBuffer &operator>>(BaseBuffer &inBuffer, char *inValue)
        {
            if (TypeSerializer<char *>::Serialize(inBuffer, inValue) == false)
            {
                inBuffer.SetError();
            }
            return inBuffer;
        }

        BaseBuffer &operator>>(BaseBuffer &inBuffer, const char *inValue)
        {
            if (TypeSerializer<const char *>::Serialize(inBuffer, inValue) == false)
            {
                inBuffer.SetError();
            }
            return inBuffer;
        }

        BaseBuffer &operator>>(BaseBuffer &inBuffer, std::string &inValue)
        {
           if (TypeSerializer<std::string>::Serialize(inBuffer, inValue) == false)
            {
                inBuffer.SetError();
            }
            return inBuffer;
        }

        BaseBuffer &operator>>(BaseBuffer &inBuffer, const std::string &inValue)
        {
           if (TypeSerializer<const std::string>::Serialize(inBuffer, inValue) == false)
            {
                inBuffer.SetError();
            }
            return inBuffer;
        }

        BaseBuffer &operator>>(BaseBuffer &inBuffer, std::wstring &inValue)
        {
           if (TypeSerializer<std::wstring>::Serialize(inBuffer, inValue) == false)
            {
                inBuffer.SetError();
            }
            return inBuffer;
        }

        BaseBuffer &operator>>(BaseBuffer &inBuffer, const std::wstring &inValue)
        {
           if (TypeSerializer<const std::wstring>::Serialize(inBuffer, inValue) == false)
            {
                inBuffer.SetError();
            }
            return inBuffer;
        }
    }
}