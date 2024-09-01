// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "gtest/gtest.h"
#include "Serialization/WriteBuffer.h"

namespace v8App
{
    namespace Serialization
    {
        TEST(WriteBufferTest, Constructor)
        {
            std::unique_ptr<WriteBuffer> buffer = std::make_unique<WriteBuffer>();
            EXPECT_EQ(131072, buffer->BufferCapacity());
            EXPECT_TRUE(buffer->IsWriter());
            EXPECT_TRUE(buffer->AtEnd());

            buffer = std::make_unique<WriteBuffer>(124);
            EXPECT_EQ(124, buffer->BufferCapacity());

            const char *testData = "Hello";
            size_t len = strlen(testData) + 1;

            buffer = std::make_unique<WriteBuffer>(testData, len);
            EXPECT_STREQ(testData, buffer->GetData());
            EXPECT_EQ(len, buffer->BufferSize());
            EXPECT_TRUE(buffer->AtEnd());
        }

        TEST(WriteBufferTest, Serialize)
        {
            const char *testData = "Hello";

            std::unique_ptr<WriteBuffer> buffer = std::make_unique<WriteBuffer>();

            char testChar = 'H';
            char testChars[4] = "ell";

            buffer->SerializeRead(&testChar, 1);
            EXPECT_EQ('H', testChar);

            buffer->SerializeWrite(&testChar, 1);
            EXPECT_EQ('H', buffer->GetData()[0]);

            buffer->SerializeWrite(testChars, 3);
            EXPECT_EQ('e', buffer->GetData()[1]);
            EXPECT_EQ('l', buffer->GetData()[2]);
            EXPECT_EQ('l', buffer->GetData()[3]);

            testChar = 'o';
            buffer->SerializeWrite(&testChar, 1);
            EXPECT_EQ('o', buffer->GetData()[4]);

            buffer->ResetPos();
            EXPECT_EQ(0, buffer->BufferSize());
            testChar = 'H';
            buffer->SerializeWrite(&testChar, 1);
            EXPECT_EQ('H', buffer->GetData()[0]);
            EXPECT_EQ(1, buffer->BufferSize());
        }
    }
}
