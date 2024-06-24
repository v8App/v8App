// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef __READ_BUFFER_H__
#define __READ_BUFFER_H__

#include "Serialization/BaseBuffer.h"

namespace v8App
{
    namespace Serialization
    {
        class ReadBuffer : public BaseBuffer
        {
        public:
            ReadBuffer(size_t inInitialSize = 131072);
            ReadBuffer(const char *inBuffer, size_t inSize);
            virtual ~ReadBuffer() = default;

            ReadBuffer(ReadBuffer &);
            ReadBuffer(ReadBuffer &&);

            /**
             * Serialize the bytes either into or out of the buffer.
             * Subclasses should override it to do the work
             */
            virtual void SerializeRead(void *inBytes, size_t inSize) override;
            //for the reader do nothing
            virtual void SerializeWrite(const void *inBytes, size_t inSize) override {};
            /**
             * Is the buffer at the end of it.
             */
            virtual bool AtEnd() override;

            /**
             * allow peeking at the data with out affecting the read position.
             * Usefult for strings who have their size written into the bufffer
             * before the actual string.
             */
            void Peek(void* inData, size_t inSize);

        protected:
            size_t m_ReadPos{0};
        };
    }
}

#endif //__READ_BUFFER_H__
