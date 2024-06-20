// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef __BASE_BUFFER_H__
#define __BASE_BUFFER_H__

#include <vector>

namespace V8App
{
    namespace Serialization
    {
        /**
         * Base buffer stream class where subclasses override it to implment
         * reader or writer. Stream can't be voth a reader and writer
         */
        class BaseBuffer
        {
        public:
            /**
             * Info to Track information about the stream
             */
            struct StreamState
            {
                bool m_Reader{false};
                bool m_BigEndian{std::endian::native == std::endian::big};
            };

            BaseBuffer(size_t inInitialSize = 131072);
            BaseBuffer(const char *inBuffer, size_t inSize);
            virtual ~BaseBuffer();

            BaseBuffer(BaseBuffer &inBUffer);
            BaseBuffer(BaseBuffer &&inBuffer);

            /**
             * Has the stream entered an error state
             */
            bool HasErrored() { return m_Error; }

            /**
             * Is this a reader
             */
            bool IsReader() { return m_StreamState.m_Reader; }
            /**
             * Is this a writer
             */
            bool IsWriter() { return m_StreamState.m_Reader == false; }
            /**
             * Is the stream in a little endian bye format
             */
            bool IsLittleEndian() { return m_StreamState.m_BigEndian == false; }
            /**
             * Is the data in big endian byte format
             */
            bool IsBigEndian() { return m_StreamState.m_BigEndian; }
            /**
             * Is the stream in Host Network Order ie big endian
             */
            bool IsHostNetworkOrder() { return m_StreamState.m_BigEndian; }

            /**
             * Compares the platform's endianess to the stream's endianess to see if they are the same if not then byte swapping is needed
             */
            bool IsByteSwapping()
            {
                return (m_StreamState.m_BigEndian && (std::endian::native != std::endian::big)) ||
                       (m_StreamState.m_BigEndian == false && (std::endian::native == std::endian::big));
            }

            /**
             * The total size fo the buffer
             */
            size_t BufferSize() { return m_Buffer.size(); }
            size_t BufferCapacity() { return m_Buffer.capacity(); }

            /**
             * Returns the pointer to the vectors data
             */
            const char *GetData() { return m_Buffer.data(); }

            /**
             * Create a copy of the buffer using new. Caller takes
             * ownership of the returned data and will need to call
             * delete on it.
             */
            const char *GetDataNew();

            /**
             * Sets the stream to an error state
             */
            void SetError() { m_Error = true; }

            /**
             * Serialize the bytes either into or out of the buffer.
             * Subclasses should override it to do the work
             */
            virtual void SerializeRead(void *inBytes, size_t inSize) = 0;
            virtual void SerializeWrite(const void *inBytes, size_t inSize) = 0;
            /**
             * Is the buffer at the end of it.
             */
            virtual bool AtEnd() = 0;

        protected:
            /**
             * Sets info about the stream state by subclasses
             */
            void SetStreamState(StreamState &inState) { m_StreamState = inState; }

            /**
             * Has the stream errored
             */
            bool m_Error{false};
            /**
             * The buffer for the stream
             */
            std::vector<char> m_Buffer;

            /**
             * The state of the stream
             */
            StreamState m_StreamState;

        private:
            BaseBuffer &operator=(const BaseBuffer &) = delete;
            BaseBuffer &operator=(const BaseBuffer *) = delete;
        };
    }
}
#endif //__BASE_BUFFER_H__