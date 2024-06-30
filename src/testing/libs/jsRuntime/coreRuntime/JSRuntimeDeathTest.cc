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
#include "V8AppPlatform.h"

#include "test_main.h"

namespace v8App
{
    namespace JSRuntime
    {
        struct TemplateInfo
        {
            bool m_Bool;
        };

        TEST(JSRuntimeDeathTest, CreateJSRuntimeAppNull)
        {
#ifdef V8APP_DEBUG
            GTEST_FLAG_SET(death_test_style, "threadsafe");
            ASSERT_DEATH({
                JSAppSharedPtr app;

                std::shared_ptr<JSRuntime> runtimePtr = JSRuntime::CreateJSRuntime(app, IdleTasksSupport::kIdleTasksEnabled, "CreateJSRuntimeAppNull", nullptr); 

                std::exit(0); }, "");
#endif
        }

        TEST(JSRuntimeDeathTest, SetObjectTemplate)
        {
            GTEST_FLAG_SET(death_test_style, "threadsafe");
            ASSERT_DEATH({
                std::shared_ptr<TestSnapshotProvider> snapProvider = std::make_shared<TestSnapshotProvider>();
                JSAppSharedPtr app = std::make_shared<JSApp>("test", snapProvider);

                std::shared_ptr<JSRuntime> runtimePtr = std::make_shared<JSRuntime>(app, IdleTasksSupport::kIdleTasksEnabled, "SetObjectTemplate");

                V8LObjTpl objTemplate;
                struct TemplateInfo info;
                runtimePtr->SetObjectTemplate(&info, objTemplate);
                std::exit(0); }, "");
        }

        TEST(JSRuntimeDeathTest, GetObjectTemplate)
        {
            GTEST_FLAG_SET(death_test_style, "threadsafe");
            ASSERT_DEATH({
                std::shared_ptr<TestSnapshotProvider> snapProvider = std::make_shared<TestSnapshotProvider>();
                JSAppSharedPtr app = std::make_shared<JSApp>("test", snapProvider);

                std::shared_ptr<JSRuntime> runtimePtr = std::make_shared<JSRuntime>(app, IdleTasksSupport::kIdleTasksEnabled, "GetObjectTemplate");

                struct TemplateInfo info;
                runtimePtr->GetObjectTemplate(&info);
                // V8PlatformInitFixture::TearDownTestSuite();
                std::exit(0); }, "");
        }

        TEST(JSRuntimeDeathTest, CreateContextNotNullCreator)
        {
            GTEST_FLAG_SET(death_test_style, "threadsafe");
            ASSERT_DEATH({
                std::shared_ptr<TestSnapshotProvider> snapProvider = std::make_shared<TestSnapshotProvider>();
                JSAppSharedPtr app = std::make_shared<JSApp>("test", snapProvider);

                std::shared_ptr<JSRuntime> runtimePtr = std::make_shared<JSRuntime>(app, IdleTasksSupport::kIdleTasksEnabled, "CreateContextNotNullCreator");

                runtimePtr->CreateContext("test", "");
                std::exit(0); }, "");
        }

        TEST(JSRuntimeDeathTest, GetNamespaceForSnapIndexNullCreator)
        {
#ifdef V8APP_DEBUG
            GTEST_FLAG_SET(death_test_style, "threadsafe");
            ASSERT_DEATH({
                std::shared_ptr<TestSnapshotProvider> snapProvider = std::make_shared<TestSnapshotProvider>();
                JSAppSharedPtr app = std::make_shared<JSApp>("test", snapProvider);

                std::shared_ptr<JSRuntime> runtimePtr = std::make_shared<JSRuntime>(app, IdleTasksSupport::kIdleTasksEnabled, "GetNamespaceForSnapIndexNullCreator");

                runtimePtr->GetNamespaceForSnapIndex(0);
                std::exit(0); }, "");
#endif
        }

        TEST(JSRuntimeDeathTest, GetSnapIndexForNamespaceNullCreator)
        {
#ifdef V8APP_DEBUG
            GTEST_FLAG_SET(death_test_style, "threadsafe");
            ASSERT_DEATH({
                std::shared_ptr<TestSnapshotProvider> snapProvider = std::make_shared<TestSnapshotProvider>();
                JSAppSharedPtr app = std::make_shared<JSApp>("test", snapProvider);

                std::shared_ptr<JSRuntime> runtimePtr = std::make_shared<JSRuntime>(app, IdleTasksSupport::kIdleTasksEnabled, "GetSnapIndexForNamespaceNullCreator");

                runtimePtr->GetSnapIndexForNamespace("test");
                std::exit(0); }, "");
#endif
        }

        TEST(JSRuntimeDeathTest, AddSnapIndexNamespaceNullCreator)
        {
#ifdef V8APP_DEBUG
            GTEST_FLAG_SET(death_test_style, "threadsafe");
            ASSERT_DEATH({
                std::shared_ptr<TestSnapshotProvider> snapProvider = std::make_shared<TestSnapshotProvider>();
                JSAppSharedPtr app = std::make_shared<JSApp>("test", snapProvider);

                std::shared_ptr<JSRuntime> runtimePtr = std::make_shared<JSRuntime>(app, IdleTasksSupport::kIdleTasksEnabled, "AddSnapIndexNamespaceNullCreator");

                runtimePtr->AddSnapIndexNamespace(0, "test");
                std::exit(0); }, "");
#endif
        }

        TEST(JSRuntimeDeathTest, DisposeContextNullCreator)
        {
#ifdef V8APP_DEBUG
            GTEST_FLAG_SET(death_test_style, "threadsafe");
            ASSERT_DEATH({
                std::shared_ptr<TestSnapshotProvider> snapProvider = std::make_shared<TestSnapshotProvider>();
                JSAppSharedPtr app = std::make_shared<JSApp>("test", snapProvider);

                std::shared_ptr<JSRuntime> runtimePtr = std::make_shared<JSRuntime>(app, IdleTasksSupport::kIdleTasksEnabled, "DisposeContextNullCreator");

                runtimePtr->DisposeContext(JSContextSharedPtr());
                std::exit(0); }, "");
#endif
        }
    }
}