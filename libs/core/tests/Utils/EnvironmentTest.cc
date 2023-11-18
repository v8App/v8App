// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <string>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "Utils/Environment.h"

namespace v8App
{
    namespace Utils
    {
        TEST(EnvironmentTest, GetEnvironmentVar)
        {
            EXPECT_EQ("", GetEnvironmentVar("NotExist"));
            EXPECT_EQ("test", GetEnvironmentVar("VarExists"));

        }
    } //Utils
} // v8App