// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <iostream>
#include <string>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "TestLogSink.h"

#include "TestSnapshotProvider.h"
#include "Utils/Environment.h"

#include "JSApp.h"
#include "JSRuntime.h"
#include "V8AppPlatform.h"
#include "V8RuntimeProvider.h"
#include "V8ContextProvider.h"

#include "test_main.h"

namespace v8App
{
    namespace JSRuntime
    {
        struct TemplateInfo
        {
            bool m_Bool;
        };

        TEST(JSRuntimeDeathTest, SetObjectTemplate)
        {
            GTEST_FLAG_SET(death_test_style, "threadsafe");
            ASSERT_DEATH({
                JSAppSharedPtr app = std::make_shared<JSApp>();

                JSRuntimeSharedPtr runtimePtr = std::make_shared<JSRuntime>();

                V8LObjTpl objTemplate;
                struct TemplateInfo info;
                runtimePtr->SetObjectTemplate(&info, objTemplate);
                std::exit(0); }, "");
        }

        TEST(JSRuntimeDeathTest, GetObjectTemplate)
        {
            GTEST_FLAG_SET(death_test_style, "threadsafe");
            ASSERT_DEATH({
                JSAppSharedPtr app = std::make_shared<JSApp>();

                std::shared_ptr<JSRuntime> runtimePtr = std::make_shared<JSRuntime>();

                struct TemplateInfo info;
                runtimePtr->GetObjectTemplate(&info);
                std::exit(0); }, "");
        }

        TEST(JSRuntimeDeathTest, GetForegroundTaskRunner)
        {
#ifdef V8APP_DEBUG
            GTEST_FLAG_SET(death_test_style, "threadsafe");
            ASSERT_DEATH({
                JSRuntimeIsolateHelper helper;
                V8TaskRunnerSharedPtr runner = helper.GetForegroundTaskRunner(nullptr, V8TaskPriority::kBestEffort);
                std::exit(0); }, "");
#endif
        }

        TEST(JSRuntimeDeathTest, IdleTasksEnabled)
        {
#ifdef V8APP_DEBUG
            GTEST_FLAG_SET(death_test_style, "threadsafe");
            ASSERT_DEATH({
                JSRuntimeIsolateHelper helper;
                bool enabled = helper.IdleTasksEnabled(nullptr);
                std::exit(0); }, "");
#endif
        }
    }
}