// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <iostream>
#include <string>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "TestLogSink.h"
#include "Utils/Environment.h"

#include "JSRuntime.h"
#include "V8Platform.h"

#include "test_main.h"

namespace v8App
{
    namespace JSRuntime
    {
#ifdef V8APP_DEBUG
        TEST(JSAppDeathTest, SetObjectTemplate)
        {
            GTEST_FLAG_SET(death_test_style, "threadsafe");
            ASSERT_DEATH({
                JSAppSharedPtr app = std::make_shared<JSApp>("test", nullptr);
                std::exit(0);
            },
                         "");
        }
#endif
    }
}