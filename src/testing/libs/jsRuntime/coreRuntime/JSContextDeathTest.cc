// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <iostream>
#include <string>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "JSContext.h"

#include "V8InitApp.h"

namespace v8App
{
    namespace JSRuntime
    {
        TEST(JSContextDeathTest, Constructor)
        {
            GTEST_FLAG_SET(death_test_style, "threadsafe");
            ASSERT_DEATH({
                JSRuntimeSharedPtr runtime = nullptr;
                JSContext context(runtime, "test");
                std::exit(0);
            },
                         "");
        }
    }
}
