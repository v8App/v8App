// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "gtest/gtest.h"

#include "Serialization/ReadBuffer.h"

namespace v8App
{
    namespace Serialization
    {
        TEST(ReadBufferTest, Constructor)
        {
            std::unique_ptr<ReadBuffer> buffer = std::make_unique<ReadBuffer>();
            EXPECT_EQ(131072, buffer->BufferCapacity());
            EXPECT_TRUE(buffer->IsReader());
            EXPECT_TRUE(buffer->AtEnd());

            buffer = std::make_unique<ReadBuffer>(124);
            EXPECT_EQ(124, buffer->BufferCapacity());

            const char *testData = "Hello";
            size_t len = strlen(testData) + 1;

            buffer = std::make_unique<ReadBuffer>(testData, len);
            EXPECT_STREQ(testData, buffer->GetData());
            EXPECT_EQ(len, buffer->BufferSize());
            EXPECT_FALSE(buffer->AtEnd());
        }

        TEST(ReadBufferTest, SerializeReadWrite)
        {
            const char *testData = "Hello";
            size_t len = strlen(testData);

            std::unique_ptr<ReadBuffer> buffer = std::make_unique<ReadBuffer>(testData, len);

            char testChar = 'a';
            char testChars[3];

            // should do nothing
            buffer->SerializeWrite(&testChar, 1);
            EXPECT_EQ('a', testChar);

            buffer->SerializeRead(&testChar, 1);
            EXPECT_EQ('H', testChar);
            EXPECT_FALSE(buffer->AtEnd());
            EXPECT_FALSE(buffer->HasErrored());

            buffer->SerializeRead(testChars, 3);
            EXPECT_FALSE(buffer->AtEnd());
            EXPECT_FALSE(buffer->HasErrored());
            EXPECT_EQ('e', testChars[0]);
            EXPECT_EQ('l', testChars[1]);
            EXPECT_EQ('l', testChars[2]);

            buffer->SerializeRead(&testChar, 1);
            EXPECT_EQ('o', testChar);
            EXPECT_TRUE(buffer->AtEnd());
            EXPECT_FALSE(buffer->HasErrored());

            buffer->SerializeRead(&testChar, 1);
            EXPECT_EQ('o', testChar);
            EXPECT_TRUE(buffer->AtEnd());
            EXPECT_TRUE(buffer->HasErrored());

            buffer->ResetPos();
            buffer->SerializeRead(&testChar, 1);
            EXPECT_EQ('H', testChar);
        }

        TEST(ReadBufferTest, Peek)
        {
            int testData = 100;
            int value = 0;
            size_t size_int = sizeof(int);

            std::unique_ptr<ReadBuffer> buffer = std::make_unique<ReadBuffer>((const char *)&testData, size_int);

            buffer->Peek(&value, size_int);
            EXPECT_EQ(value, testData);
            EXPECT_FALSE(buffer->AtEnd());

            value = 0;
            buffer->SerializeRead(&value, size_int);
            EXPECT_EQ(value, testData);
            EXPECT_TRUE(buffer->AtEnd());
        }
    }
}
