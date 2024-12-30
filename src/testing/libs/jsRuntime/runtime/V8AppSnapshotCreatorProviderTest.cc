// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "V8Fixture.h"
#include "TestFiles.h"

#include "Utils/Format.h"

#include "JSApp.h"
#include "JSRuntime.h"
#include "V8AppSnapshotCreator.h"
#include "V8AppSnapshotProvider.h"

namespace v8App
{
    namespace JSRuntime
    {
        using V8AppSnapshotCreatorProviderTest = V8Fixture;

        TEST_F(V8AppSnapshotCreatorProviderTest, SingleBareRuntime)
        {
            TestUtils::TestLogSink *logSink = TestUtils::TestLogSink::GetGlobalSink();
            Log::Log::SetLogLevel(Log::LogLevel::Error);

            std::filesystem::path testRoot = s_TestDir / "SingleBareRutime";
            EXPECT_TRUE(TestUtils::CreateAppDirectory(testRoot));

            std::filesystem::path snapshotFile = "V8AppSingleBareRuntime.dat";
            snapshotFile = testRoot / snapshotFile;

            V8AppSnapshotCreator creator;

            // empty file path
            EXPECT_FALSE(creator.CreateSnapshot(*m_App, ""));
            Log::LogMessage expected = {
                {Log::MsgKey::Msg, "CreateSnapshot passed an empty file path"},
                {Log::MsgKey::LogLevel, "Error"},
            };
            EXPECT_TRUE(logSink->ValidateMessage(expected, m_IgnoreKeys));

            // snap object isn't app
            EXPECT_FALSE(creator.CreateSnapshot(*m_Runtime, snapshotFile));
            expected = {
                {Log::MsgKey::Msg, "CreateSnapshot expected a JSApp Object"},
                {Log::MsgKey::LogLevel, "Error"},
            };
            EXPECT_TRUE(logSink->ValidateMessage(expected, m_IgnoreKeys));

            // snap object isn't app
            EXPECT_FALSE(creator.CreateSnapshot(*m_App, snapshotFile));
            expected = {
                {Log::MsgKey::Msg, Utils::format("The app {} is not snapshottable", m_App->GetName())},
                {Log::MsgKey::LogLevel, "Error"},
            };
            EXPECT_TRUE(logSink->ValidateMessage(expected, m_IgnoreKeys));

            JSAppSharedPtr snapApp = m_App->CloneAppForSnapshotting();

            // nno need to create a context since the runtime always creates a bare v8 context as default
            if (creator.CreateSnapshot(*snapApp, snapshotFile) == false)
            {
                // so the snap app is disposed of or we'll error on test shutdown
                snapApp->DisposeApp();
                ASSERT_FALSE(true);
            }
            snapApp->DisposeApp();

            // test that we can load and runa some scripts in it
            V8AppSnapshotProviderSharedPtr snapProvider = std::make_shared<V8AppSnapshotProvider>();
            ASSERT_TRUE(snapProvider->LoadSnapshotData(snapshotFile));

            AppProviders providers = m_Providers;
            providers.m_SnapshotProvider = snapProvider;

            JSAppSharedPtr restore = std::make_shared<JSApp>();
            ASSERT_TRUE(restore->Initialize("SingleBareRuntimeRestored", testRoot, providers));
            {
                JSRuntimeSharedPtr runtime = restore->GetMainRuntime();
                JSContextSharedPtr jsContext = runtime->CreateContext("SingleBareRuntimeRestored", "");

                V8IsolateScope iScope(runtime->GetIsolate());
                V8HandleScope hScope(runtime->GetIsolate());
                V8LValue testValue;

                const char *source1 = R"(
                function test42() {
                    return 42;
                }
            )";

                testValue = jsContext->RunScript(source1);

                const char *source2 = R"(
                test42();
            )";
                testValue = jsContext->RunScript(source2);
                if (testValue.IsEmpty())
                {
                    // if this is hit then it means the script failed
                    EXPECT_FALSE(1);
                }
                else
                {
                    EXPECT_EQ(42, testValue->IntegerValue(jsContext->GetLocalContext()).FromJust());
                }
            }
            restore->DisposeApp();
        }

        TEST_F(V8AppSnapshotCreatorProviderTest, MultipleRuntimes)
        {
            std::filesystem::path testRoot = s_TestDir / "MultipleRuntimes";
            EXPECT_TRUE(TestUtils::CreateAppDirectory(testRoot));

            std::filesystem::path snapshotFile = "V8AppMultipleBareRuntime.dat";
            snapshotFile = testRoot / snapshotFile;

            JSAppSharedPtr snapApp = m_App->CloneAppForSnapshotting();
            JSRuntimeSharedPtr secondRuntime = snapApp->CreateJSRuntimeFromIndex("SecondRuntime", 0,
            JSRuntimeSnapshotAttributes::NotSnapshottable);
            JSRuntimeSharedPtr thirdRuntime = snapApp->CreateJSRuntimeFromIndex("ThirdRuntime", 0,
            JSRuntimeSnapshotAttributes::NotSnapshottable);
            V8AppSnapshotCreator creator;

            // nno need to create a context since the runtime always creates a bare v8 context as default
            if (creator.CreateSnapshot(*snapApp, snapshotFile) == false)
            {
                // so the snap app is disposed of or we'll error on test shutdown
                snapApp->DisposeApp();
                ASSERT_FALSE(true);
            }
            //snapApp->DisposeRuntime(thirdRuntime);
            snapApp->DisposeApp();

            V8AppSnapshotProviderSharedPtr snapProvider = std::make_shared<V8AppSnapshotProvider>();
            ASSERT_TRUE(snapProvider->LoadSnapshotData(snapshotFile));

            AppProviders providers = m_Providers;
            providers.m_SnapshotProvider = snapProvider;

            JSAppSharedPtr restore = std::make_shared<JSApp>();
            ASSERT_TRUE(restore->Initialize("MultipleRuntimeRestored", testRoot, providers));
            {
                JSRuntimeSharedPtr runtime = restore->CreateJSRuntimeFromIndex("SecondRuntime", 1);
                if (runtime == nullptr)
                {
                    restore->DisposeApp();
                    ASSERT_TRUE(false);
                }
                JSContextSharedPtr jsContext = runtime->CreateContext("MultipleRuntimeRestored", "");

                V8IsolateScope iScope(runtime->GetIsolate());
                V8HandleScope hScope(runtime->GetIsolate());
                V8LValue testValue;

                const char *source1 = R"(
                function test42() {
                    return 42;
                }
            )";

                testValue = jsContext->RunScript(source1);

                const char *source2 = R"(
                test42();
            )";
                testValue = jsContext->RunScript(source2);
                if (testValue.IsEmpty())
                {
                    // if this is hit then it means the script failed
                    EXPECT_FALSE(1);
                }
                else
                {
                    EXPECT_EQ(42, testValue->IntegerValue(jsContext->GetLocalContext()).FromJust());
                }
            }
            restore->DisposeApp();
        }
    }
}
