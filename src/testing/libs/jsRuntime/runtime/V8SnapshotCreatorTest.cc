// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <fstream>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "test_main.h"
#include "V8Fixture.h"
#include "TestFiles.h"

#include "JSApp.h"
#include "JSRuntime.h"
#include "V8SnapshotCreator.h"
#include "V8SnapshotProvider.h"
#include "ISnapshotObject.h"

namespace v8App
{
    namespace JSRuntime
    {
        using V8SnapshotCreaterTest = V8Fixture;

        class TestSnapObject : public ISnapshotObject
        {
            virtual bool MakeSnapshot(Serialization::WriteBuffer &inBuffer, void *inData = nullptr) {}
            virtual bool RestoreSnapshot(Serialization::ReadBuffer &inBufffer, void *inData = nullptr) {}
        };

        TEST_F(V8SnapshotCreaterTest, CreateSnapshot)
        {
            TestUtils::TestLogSink *logSink = TestUtils::TestLogSink::GetGlobalSink();
            Log::Log::SetLogLevel(Log::LogLevel::Error);

            std::filesystem::path testRoot = s_TestDir / "ConstrcutorInitializeDispose";
            EXPECT_TRUE(TestUtils::CreateAppDirectory(testRoot));

            std::filesystem::path snapshotFile = "v8snapshot.bin";
            snapshotFile = testRoot / snapshotFile;

            V8SnapshotCreator creator;

            // empty file path
            EXPECT_FALSE(creator.CreateSnapshot(*m_App, ""));
            Log::LogMessage expected = {
                {Log::MsgKey::Msg, "CreateSnapshot passed an empty file path"},
                {Log::MsgKey::LogLevel, "Error"},
            };
            EXPECT_TRUE(logSink->ValidateMessage(expected, m_IgnoreKeys));

            // snap object isn't a runtime or app
            TestSnapObject testSnapObject;
            EXPECT_FALSE(creator.CreateSnapshot(testSnapObject, snapshotFile));
            expected = {
                {Log::MsgKey::Msg, "CreateSnapshot epxects to be passed a JSRuntime or JSApp object"},
                {Log::MsgKey::LogLevel, "Error"},
            };
            EXPECT_TRUE(logSink->ValidateMessage(expected, m_IgnoreKeys));

            // test app runtime is null
            JSAppSharedPtr testApp = std::make_shared<JSApp>("V8SnapshotCreatorNullRuntime", m_Providers);
            EXPECT_FALSE(creator.CreateSnapshot(*testApp, snapshotFile));
            expected = {
                {Log::MsgKey::Msg, "CreateSNaphot failed to get the main runtime from the app"},
                {Log::MsgKey::LogLevel, "Error"},
            };
            EXPECT_TRUE(logSink->ValidateMessage(expected, m_IgnoreKeys));

            // test runtime isn't initialized
            JSRuntimeSharedPtr testRuntime = std::make_shared<JSRuntime>(m_App, IdleTaskSupport::kEnabled, "V8SnapshotCreatorTest", false, 0);
            EXPECT_FALSE(creator.CreateSnapshot(*testRuntime, snapshotFile));
            expected = {
                {Log::MsgKey::Msg, Utils::format("The runtime {} is not initialized", testRuntime->GetName())},
                {Log::MsgKey::LogLevel, "Error"},
            };
            EXPECT_TRUE(logSink->ValidateMessage(expected, m_IgnoreKeys));

            // test runtime isn't snapshottable
            ASSERT_TRUE(testRuntime->Initialize());
            EXPECT_FALSE(creator.CreateSnapshot(*testRuntime, snapshotFile));
            expected = {
                {Log::MsgKey::Msg, Utils::format("The runtime {} is not snapshottable", testRuntime->GetName())},
                {Log::MsgKey::LogLevel, "Error"},
            };
            EXPECT_TRUE(logSink->ValidateMessage(expected, m_IgnoreKeys));

            // test the actuall creation
            ASSERT_TRUE(testApp->Initialize(testRoot, true, m_Providers));
            EXPECT_TRUE(creator.CreateSnapshot(*testApp, snapshotFile));
            testRuntime->DisposeRuntime();

            // test that we can load and runa some scripts in it
            V8SnapshotProviderSharedPtr snapProvider = std::make_shared<V8SnapshotProvider>();
            ASSERT_TRUE(snapProvider->LoadSnapshotData(testApp, snapshotFile));
            testApp->DisposeApp();

            AppProviders providers = m_Providers;
            providers.m_SnapshotProvider = snapProvider;

            JSAppSharedPtr newSnapApp = std::make_shared<JSApp>("V8SnapshotCreatorTestSaveSnapshot", providers);
            ASSERT_TRUE(newSnapApp->Initialize(testRoot));

            JSRuntimeSharedPtr runtime = m_App->GetMainRuntime();
            JSContextSharedPtr context = runtime->CreateContext("V8SnapshotCreatorTest", "");
            ASSERT_NE(nullptr, context);

            V8IsolateScope iScope(runtime->GetIsolate());
            V8HandleScope hScope(runtime->GetIsolate());
            V8LValue testValue;

            const char *source1 = R"(
                function test42() {
                    return 42;
                }
            )";

            testValue = context->RunScript(source1);

            const char *source2 = R"(
                test42();
            )";
            testValue = context->RunScript(source2);
            if (testValue.IsEmpty())
            {
                // if this is hit then it means the script failed
                EXPECT_FALSE(1);
            }
            else
            {
                EXPECT_EQ(42, testValue->IntegerValue(context->GetLocalContext()).FromJust());
            }
            newSnapApp->DisposeApp();
        }
    }
}
