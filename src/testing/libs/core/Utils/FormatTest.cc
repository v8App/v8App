// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <string>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "Utils/Format.h"

namespace v8App
{
    namespace Utils
    {
        TEST(FormatTest, format)
        {
            //test string 
            EXPECT_EQ("test", Utils::format("{}", "test"));
            //test int
            EXPECT_EQ("1", Utils::format("{}", 1));
            //look at maybe adding others
            EXPECT_EQ("{ }", Utils::format("{ }"));
            //tets { and } in string not not next to each other
            EXPECT_EQ("test {h}", Utils::format("{} {h}", "test"));
        }
    } // Utils
} // v8App