// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "Serialization/WriteBuffer.h"

namespace V8App
{
    namespace Serialization
    {
        WriteBuffer::WriteBuffer(size_t inInitialSize) : BaseBuffer(inInitialSize)
        {
            BaseBuffer::StreamState state;
            state.m_Reader = false;
            SetStreamState(state);
        }

        WriteBuffer::WriteBuffer(const char *inBuffer, size_t inSize) : BaseBuffer(inBuffer, inSize)
        {
            BaseBuffer::StreamState state;
            state.m_Reader = false;
            SetStreamState(state);
        }

        WriteBuffer::WriteBuffer(WriteBuffer &inBuffer) : BaseBuffer(inBuffer)
        {
        }
        WriteBuffer::WriteBuffer(WriteBuffer &&inBuffer) : BaseBuffer(inBuffer)
        {
        }

        void WriteBuffer::SerializeWrite(const void *inBytes, size_t inSize)
        {
            const char *p = reinterpret_cast<const char *>(inBytes);
            for (size_t i = 0; i < inSize; i++)
            {
                m_Buffer.push_back(p[i]);
            }
        }
    }
}
