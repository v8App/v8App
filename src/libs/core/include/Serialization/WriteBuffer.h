// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef __WRITE_BUFFER_H__
#define __WRITE_BUFFER_H__

#include "Serialization/BaseBuffer.h"

namespace V8App
{
    namespace Serialization
    {
        class WriteBuffer : public BaseBuffer
        {
        public:
            WriteBuffer(size_t inInitialSize = 131072);
            WriteBuffer(const char *inBuffer, size_t inSize);
            virtual ~WriteBuffer() = default;

            WriteBuffer(WriteBuffer &inBuffer);
            WriteBuffer(WriteBuffer &&inBuffer);

            //for the writer do nothing
            virtual void SerializeRead(void *inBytes, size_t inSize) override {}
            /**
             * Serialize the bytes either into or out of the buffer.
             * Subclasses should override it to do the work
             */
            virtual void SerializeWrite(const void *inBytes, size_t inSize) override;
            /**
             * Is the buffer at the end of it.
             * Write is always at the end
             */
            virtual bool AtEnd() override { return true; }
        };
    }
}

#endif //__WRITE_BUFFER_H__
