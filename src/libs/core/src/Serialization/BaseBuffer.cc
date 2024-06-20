// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <algorithm>
#include <iterator>

#include "Serialization/BaseBuffer.h"

namespace V8App
{
    namespace Serialization
    {
        BaseBuffer::BaseBuffer(size_t inInitialSize)
        {
            m_Buffer.reserve(inInitialSize);
        }

        BaseBuffer::BaseBuffer(const char *inBuffer, size_t inSize)
        {
            m_Buffer.reserve(inSize);
            std::copy(inBuffer, inBuffer + inSize, std::back_inserter(m_Buffer));
        }

        BaseBuffer::~BaseBuffer()
        {
        }

        BaseBuffer::BaseBuffer(BaseBuffer &inBuffer)
        {
            m_Buffer = inBuffer.m_Buffer;
            m_StreamState = inBuffer.m_StreamState;
            m_Error = inBuffer.m_Error;
        }

        BaseBuffer::BaseBuffer(BaseBuffer &&inBuffer)
        {
            m_Buffer = std::move(inBuffer.m_Buffer);
            m_StreamState = inBuffer.m_StreamState;
            m_Error = inBuffer.m_Error;
        }

        const char *BaseBuffer::GetDataNew()
        {
            // Note: The caller is expected to free the memory
            // The can get the size of the data by calling BufferSize
            char *data = new char[m_Buffer.size()];
            std::copy(m_Buffer.begin(), m_Buffer.end(), data);
            return data;
        }
    }
}