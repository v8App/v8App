// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "Containers/NamedIndexes.h"
#include "Serialization/ReadBuffer.h"
#include "Serialization/WriteBuffer.h"

namespace v8App
{
    namespace Containers
    {
        TEST(NamedIndexesTest, Constrcutor)
        {
            NamedIndexes indexes;

            EXPECT_EQ(1024, indexes.GetMaxSupportedIndexes());
            EXPECT_EQ(0, indexes.GetNumberOfIndexes());

            NamedIndexes indexes2(2056);
            EXPECT_EQ(2056, indexes2.GetMaxSupportedIndexes());
            EXPECT_EQ(0, indexes2.GetNumberOfIndexes());
        }

        TEST(NamedIndexesTest, AddGet)
        {
            NamedIndexes indexes;

            EXPECT_EQ("", indexes.GetNameFromIndex(0));
            EXPECT_EQ(indexes.GetMaxSupportedIndexes(), indexes.GetIndexForName("test"));

            EXPECT_TRUE(indexes.AddNamedIndex(0, "test"));
            EXPECT_FALSE(indexes.AddNamedIndex(1, "test"));
            EXPECT_FALSE(indexes.AddNamedIndex(0, "test1"));

            EXPECT_EQ("test", indexes.GetNameFromIndex(0));
            EXPECT_EQ(0, indexes.GetIndexForName("test"));

            EXPECT_EQ(1, indexes.GetNumberOfIndexes());
        }

        TEST(NamedIndexesTest, SerializeDeserialize)
        {
            NamedIndexes indexes(2056);

            indexes.AddNamedIndex(0, "test");
            indexes.AddNamedIndex(1, "test2");

            Serialization::WriteBuffer wBuffer;

            EXPECT_TRUE(indexes.SerializeNameIndexes(wBuffer));

            Serialization::ReadBuffer rBuffer(wBuffer.GetData(), wBuffer.BufferSize());
            NamedIndexes indexes2;

            EXPECT_TRUE(indexes2.DeserializeNameIndexes(rBuffer));

            EXPECT_EQ(0, indexes2.GetIndexForName("test"));
            EXPECT_EQ("test", indexes2.GetNameFromIndex(0));

            EXPECT_EQ(1, indexes2.GetIndexForName("test2"));
            EXPECT_EQ("test2", indexes2.GetNameFromIndex(1));

            EXPECT_EQ(2056, indexes2.GetMaxSupportedIndexes());
            EXPECT_EQ(2, indexes2.GetNumberOfIndexes());
        }
    }
}