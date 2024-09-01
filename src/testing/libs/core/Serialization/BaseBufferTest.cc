// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "gtest/gtest.h"

#include "Serialization/BaseBuffer.h"

namespace v8App
{
    namespace Serialization
    {
        class TestBuffer : public BaseBuffer
        {
        public:
            TestBuffer(size_t inInitialSize = 131072) : BaseBuffer(inInitialSize) {}
            TestBuffer(const char *inBuffer, size_t inSize) : BaseBuffer(inBuffer, inSize){}
            virtual ~TestBuffer() = default;

            TestBuffer(TestBuffer &) = default;
            TestBuffer(TestBuffer &&) = default;

            virtual void SerializeRead(void *inBytes, size_t inSize) override {};
            virtual void SerializeWrite(const void *inBytes, size_t inSize) override {};
            virtual bool AtEnd() override { return true; }
            void TestSetStreamState(BaseBuffer::StreamState inState) { SetStreamState(inState); }
            void ResetPos() {};
        };

        TEST(BaseBuffertest, TestConstrcutorAndMethods)
        {
            std::unique_ptr<TestBuffer> buffer = std::make_unique<TestBuffer>();

            EXPECT_EQ(0, buffer->BufferSize());
            EXPECT_EQ(131072, buffer->BufferCapacity());
            EXPECT_FALSE(buffer->HasErrored());
            EXPECT_FALSE(buffer->IsReader());
            EXPECT_TRUE(buffer->IsWriter());
            EXPECT_FALSE(buffer->IsByteSwapping());
            if (std::endian::native == std::endian::big)
            {
                EXPECT_FALSE(buffer->IsLittleEndian());
                EXPECT_TRUE(buffer->IsBigEndian());
            }
            else
            {
                EXPECT_FALSE(buffer->IsBigEndian());
                EXPECT_TRUE(buffer->IsLittleEndian());
            }

            BaseBuffer::StreamState state;
            state.m_Reader = true;
            state.m_BigEndian = false;
            buffer->TestSetStreamState(state);
            buffer->SetError();

            EXPECT_FALSE(buffer->IsWriter());
            EXPECT_TRUE(buffer->IsReader());
            EXPECT_TRUE(buffer->HasErrored());

            if (std::endian::native == std::endian::big)
            {
                EXPECT_TRUE(buffer->IsByteSwapping());
            }
            else
            {
                EXPECT_FALSE(buffer->IsByteSwapping());
            }

            buffer = std::make_unique<TestBuffer>(nullptr, 10);
            EXPECT_EQ(10, buffer->BufferCapacity());
            EXPECT_EQ(0, buffer->BufferSize());

            const char *testData = "Hello";
            //we want the null terminator as well so we can do comaprisions below
            size_t len = strlen(testData)+1;


            buffer = std::make_unique<TestBuffer>(testData, len);
            //we set the state so we can see that copies or moves below
            buffer->TestSetStreamState(state);

            EXPECT_EQ(len, buffer->BufferSize());
            EXPECT_STREQ(testData, buffer->GetData());
            const char *newData = buffer->GetDataNew();
            EXPECT_STREQ(testData, newData);
            delete[] newData;

            // test copy constrcutor
            std::unique_ptr<TestBuffer> buffer2 = std::make_unique<TestBuffer>(*buffer);
            EXPECT_EQ(len, buffer->BufferSize());
            EXPECT_STREQ(testData, buffer->GetData());
            EXPECT_EQ(len, buffer2->BufferSize());
            EXPECT_STREQ(testData, buffer2->GetData());
            EXPECT_TRUE(buffer2->IsReader());
            EXPECT_FALSE(buffer2->IsBigEndian());

            // test move constructor
            std::unique_ptr<TestBuffer> buffer3 = std::make_unique<TestBuffer>(std::move(*buffer));
            EXPECT_EQ(0, buffer->BufferSize());
            EXPECT_EQ(len, buffer2->BufferSize());
            EXPECT_STREQ(testData, buffer3->GetData());
            EXPECT_TRUE(buffer3->IsReader());
            EXPECT_FALSE(buffer3->IsBigEndian());
        }
    }
}
