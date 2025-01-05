// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "Serialization/ReadBuffer.h"

namespace v8App
{
    namespace Serialization
    {
        ReadBuffer::ReadBuffer(size_t inInitialSize) : BaseBuffer(inInitialSize)
        {
            BaseBuffer::StreamState state;
            state.m_Reader = true;
            SetStreamState(state);
        }

        ReadBuffer::ReadBuffer(const char *inBuffer, size_t inSize) : BaseBuffer(inBuffer, inSize)
        {
            BaseBuffer::StreamState state;
            state.m_Reader = true;
            SetStreamState(state);
        }

        ReadBuffer::ReadBuffer(ReadBuffer &inBuffer) : BaseBuffer(inBuffer)
        {
            m_ReadPos = inBuffer.m_ReadPos;
        }

        ReadBuffer::ReadBuffer(ReadBuffer &&inBuffer) : BaseBuffer(inBuffer)
        {
            m_ReadPos = inBuffer.m_ReadPos;
            inBuffer.m_ReadPos = 0;
        }

        void ReadBuffer::SerializeRead(void *inBytes, size_t inSize)
        {
            if (m_ReadPos + inSize > m_Buffer.size())
            {
                SetError();
                return;
            }
            char *p = reinterpret_cast<char *>(inBytes);
            for (size_t i = 0; i < inSize; i++, m_ReadPos++)
            {
                p[i] = m_Buffer[m_ReadPos];
            }
        }

        bool ReadBuffer::AtEnd()
        {
            return m_ReadPos >= m_Buffer.size();
        }

        void ReadBuffer::Peek(void* inData, size_t inSize)
        {
            SerializeRead(inData, inSize);
            if(HasErrored() == false)
            {
                m_ReadPos -= inSize;
            }
        }
    }
}