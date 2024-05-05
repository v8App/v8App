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

#include "JSRuntime.h"
#include "V8Platform.h"

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
                std::shared_ptr<TestSnapshotProvider> snapProvider = std::make_shared<TestSnapshotProvider>();
                JSAppSharedPtr app = std::make_shared<JSApp>("test", snapProvider);

                std::shared_ptr<JSRuntime> runtimePtr = std::make_shared<JSRuntime>(app, IdleTasksSupport::kIdleTasksEnabled, "SetObjectTemplate");

                v8::Local<v8::ObjectTemplate> objTemplate;
                struct TemplateInfo info;
                runtimePtr->SetObjectTemplate(&info, objTemplate);
                std::exit(0);
            },
                         "");
        }

        TEST(JSRuntimeDeathTest, GetObjectTemplate)
        {
            GTEST_FLAG_SET(death_test_style, "threadsafe");
            ASSERT_DEATH({
                std::shared_ptr<TestSnapshotProvider> snapProvider = std::make_shared<TestSnapshotProvider>();
                JSAppSharedPtr app = std::make_shared<JSApp>("test", snapProvider);

                std::shared_ptr<JSRuntime> runtimePtr = std::make_shared<JSRuntime>(app, IdleTasksSupport::kIdleTasksEnabled, "GetObjectTemplate");

                v8::Local<v8::ObjectTemplate> objTemplate;
                struct TemplateInfo info;
                runtimePtr->GetObjectTemplate(&info);
                // V8PlatformInitFixture::TearDownTestSuite();
                std::exit(0);
            },
                         "");
        }

        TEST(JSRuntimeDeathTest, SetFunctionTemplate)
        {
            GTEST_FLAG_SET(death_test_style, "threadsafe");
            ASSERT_DEATH({
                std::shared_ptr<TestSnapshotProvider> snapProvider = std::make_shared<TestSnapshotProvider>();
                JSAppSharedPtr app = std::make_shared<JSApp>("test", snapProvider);

                std::shared_ptr<JSRuntime> runtimePtr = std::make_shared<JSRuntime>(app, IdleTasksSupport::kIdleTasksEnabled, "SetFunctionTemplate");

                v8::Local<v8::FunctionTemplate> funcTemplate;
                struct TemplateInfo info;
                runtimePtr->SetFunctionTemplate(&info, funcTemplate);
                std::exit(0);
            },
                         "");
        }

        TEST(JSRuntimeDeathTest, GetFunctionTemplate)
        {
            GTEST_FLAG_SET(death_test_style, "threadsafe");
            ASSERT_DEATH({
                std::shared_ptr<TestSnapshotProvider> snapProvider = std::make_shared<TestSnapshotProvider>();
                JSAppSharedPtr app = std::make_shared<JSApp>("test", snapProvider);

                std::shared_ptr<JSRuntime> runtimePtr = std::make_shared<JSRuntime>(app, IdleTasksSupport::kIdleTasksEnabled, "GetFunctionTemplate");

                struct TemplateInfo info;
                runtimePtr->GetFunctionTemplate(&info);
                std::exit(0);
            },
                         "");
        }

        TEST(JSRuntimeDeathTest, CreateContextNotNullCreator)
        {
            GTEST_FLAG_SET(death_test_style, "threadsafe");
            ASSERT_DEATH({
                std::shared_ptr<TestSnapshotProvider> snapProvider = std::make_shared<TestSnapshotProvider>();
                JSAppSharedPtr app = std::make_shared<JSApp>("test", snapProvider);

                std::shared_ptr<JSRuntime> runtimePtr = std::make_shared<JSRuntime>(app, IdleTasksSupport::kIdleTasksEnabled, "CreateContextNotNullCreator");

                runtimePtr->CreateContext("test");
                std::exit(0);
            },
                         "");
        }
    }
}