// Copyright 2020 The v8App Authors. All rights reserved.
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
#ifdef V8_DEBUG
        TEST(JSContextDeathTest, Constructor)
        {
            GTEST_FLAG_SET(death_test_style, "threadsafe");
            ASSERT_DEATH({
                // V8PlatformInitFixture::SetUpTestSuite();
                JSRuntimeSharedPtr runtime = nullptr;
                JSContext context(runtime, "test");
                // V8PlatformInitFixture::TearDownTestSuite();
                std::exit(0);
            },
                         "");
        }
#endif
    }
}
