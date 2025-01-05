// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "Serialization/TypeSerializer.h"
#include "Serialization/BaseBuffer.h"

namespace v8App
{
    namespace Serialization
    {
        bool TypeSerializer<wchar_t *>::SerializeRead(ReadBuffer &inBuffer, wchar_t *inValue)
        {
            // we want to copy the /0 in the string as well
            size_t length;
            size_t swappedLength;

            inBuffer.SerializeRead(&swappedLength, sizeof(size_t));
            if (inBuffer.IsByteSwapping())
            {
                SwapBytes(swappedLength);
            }
            length = swappedLength;
            for (size_t i = 0; i < length; i++)
            {
                wchar_t wc = inValue[i];
                inBuffer.SerializeRead(&wc, sizeof(wchar_t));
                if (inBuffer.IsByteSwapping())
                {
                    SwapBytes(wc);
                }
                inValue[i] = wc;
            }
            return true;
        }

        bool TypeSerializer<wchar_t *>::SerializeWrite(WriteBuffer &inBuffer, const wchar_t *inValue)
        {
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

        bool TypeSerializer<char *>::SerializeRead(ReadBuffer &inBuffer, char *inValue)
        {
            // we want to copy the /0 in the string as well
            size_t length;
            inBuffer.SerializeRead(&length, sizeof(size_t));
            if (inBuffer.IsByteSwapping())
            {
                SwapBytes(length);
            }
            inBuffer.SerializeRead(inValue, length);
            return true;
        }

        bool TypeSerializer<char *>::SerializeWrite(WriteBuffer &inBuffer, const char *inValue)
        {
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

        bool TypeSerializer<std::string>::SerializeRead(ReadBuffer &inBuffer, std::string &inValue)
        {
            // we want to copy the /0 in the string as well
            size_t length;
            size_t swappedLength;
            inBuffer.SerializeRead(&swappedLength, sizeof(size_t));
            if (inBuffer.IsByteSwapping())
            {
                SwapBytes(swappedLength);
            }
            length = swappedLength;
            inValue.reserve(length);
            for (size_t i = 0; i < length; i++)
            {
                char c = inValue[i];
                inBuffer.SerializeRead(&c, sizeof(char));
                if (inBuffer.IsByteSwapping())
                {
                    SwapBytes(c);
                }
                if (c != '\0')
                {
                    inValue += c;
                }
            }
            return true;
        }

        bool TypeSerializer<std::string>::SerializeWrite(WriteBuffer &inBuffer, const std::string &inValue)
        {
            size_t length = inValue.size() + 1;
            size_t swappedLength = length;
            if (inBuffer.IsByteSwapping())
            {
                SwapBytes(swappedLength);
            }
            inBuffer.SerializeWrite(&swappedLength, sizeof(size_t));
            inBuffer.SerializeWrite(inValue.c_str(), length);
            return true;
        }

        bool TypeSerializer<std::wstring>::SerializeRead(ReadBuffer &inBuffer, std::wstring &inValue)
        {
            // we want to copy the /0 in the string as well
            size_t length;
            size_t swappedLength;
            inBuffer.SerializeRead(&swappedLength, sizeof(size_t));
            if (inBuffer.IsByteSwapping())
            {
                SwapBytes(swappedLength);
            }
            length = swappedLength;
            inValue.reserve(length);
            for (size_t i = 0; i < length; i++)
            {
                wchar_t wc = inValue[i];
                inBuffer.SerializeRead(&wc, sizeof(wchar_t));
                if (inBuffer.IsByteSwapping())
                {
                    SwapBytes(wc);
                }
                if (wc != L'\0')
                {
                    inValue += wc;
                }
            }
            return true;
        }

        bool TypeSerializer<std::wstring>::SerializeWrite(WriteBuffer &inBuffer, const std::wstring &inValue)
        {
            // we want to copy the /0 in the string as well
            size_t length = inValue.size() + 1;
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

        BaseBuffer &operator<<(WriteBuffer &inBuffer, const wchar_t *inValue)
        {
            if (TypeSerializer<wchar_t *>::SerializeWrite(inBuffer, inValue) == false)
            {
                inBuffer.SetError();
            }
            return inBuffer;
        }

        BaseBuffer &operator<<(WriteBuffer &inBuffer, const char *inValue)
        {
            if (TypeSerializer<char *>::SerializeWrite(inBuffer, inValue) == false)
            {
                inBuffer.SetError();
            }
            return inBuffer;
        }

        BaseBuffer &operator<<(WriteBuffer &inBuffer, const std::string &inValue)
        {
            if (TypeSerializer<std::string>::SerializeWrite(inBuffer, inValue) == false)
            {
                inBuffer.SetError();
            }
            return inBuffer;
        }

        BaseBuffer &operator<<(WriteBuffer &inBuffer, const std::wstring &inValue)
        {
            if (TypeSerializer <std::wstring>::SerializeWrite(inBuffer, inValue) == false)
            {
                inBuffer.SetError();
            }
            return inBuffer;
        }

        BaseBuffer &operator>>(ReadBuffer &inBuffer, wchar_t *inValue)
        {
            if (TypeSerializer<wchar_t *>::SerializeRead(inBuffer, inValue) == false)
            {
                inBuffer.SetError();
            }
            return inBuffer;
        }

        BaseBuffer &operator>>(ReadBuffer &inBuffer, char *inValue)
        {
            if (TypeSerializer<char *>::SerializeRead(inBuffer, inValue) == false)
            {
                inBuffer.SetError();
            }
            return inBuffer;
        }

        BaseBuffer &operator>>(ReadBuffer &inBuffer, std::string &inValue)
        {
            if (TypeSerializer<std::string>::SerializeRead(inBuffer, inValue) == false)
            {
                inBuffer.SetError();
            }
            return inBuffer;
        }

        BaseBuffer &operator>>(ReadBuffer &inBuffer, std::wstring &inValue)
        {
            if (TypeSerializer<std::wstring>::SerializeRead(inBuffer, inValue) == false)
            {
                inBuffer.SetError();
            }
            return inBuffer;
        }
    }
}