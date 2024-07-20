// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <iostream>
#include <string>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "test_main.h"
#include "tools/cpp/runfiles/runfiles.h"
#include "TestSnapshotProvider.h"
#include "TestLogSink.h"

#include "Utils/Environment.h"

#include "V8AppPlatform.h"
#include "JSRuntime.h"
#include "JSContext.h"
#include "V8ContextProvider.h"
#include "V8RuntimeProvider.h"

using bazel::tools::cpp::runfiles::Runfiles;

namespace v8App
{
    namespace JSRuntime
    {
        class V8InitApp : public testing::Test
        {
        public:
            void SetUp() override
            {
                // setup the global test log
                TestUtils::TestLogSink *testSink = TestUtils::TestLogSink::GetGlobalSink();
                testSink->FlushMessages();

                const char *suiteName = ::testing::UnitTest::GetInstance()->current_test_info()->test_suite_name();
                AppProviders providers;
                providers.m_SnapshotProvider = std::make_shared<TestSnapshotProvider>();
                providers.m_ContextProvider = std::make_shared<V8ContextProvider>();
                providers.m_RuntimeProvider = std::make_shared<V8RuntimeProvider>();

                m_App = std::make_shared<JSApp>(suiteName, providers);
                m_App->Initialize(s_TestDir, false);
            }

            void TearDown() override
            {
                m_App->DisposeApp();
                m_App.reset();
            }

            JSAppSharedPtr m_App;
        };

    }
}