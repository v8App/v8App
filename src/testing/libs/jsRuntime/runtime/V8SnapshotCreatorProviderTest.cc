// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <fstream>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "test_main.h"
#include "V8Fixture.h"
#include "TestFiles.h"

#include "Utils/Format.h"

#include "JSApp.h"
#include "JSRuntime.h"
#include "V8SnapshotCreator.h"
#include "V8SnapshotProvider.h"
#include "ISnapshotObject.h"

namespace v8App
{
    namespace JSRuntime
    {
        using V8SnapshotCreaterProviderTest = V8Fixture;

        class V8TestSnapObject : public ISnapshotObject
        {
            virtual bool MakeSnapshot(Serialization::WriteBuffer &inBuffer, void *inData = nullptr) {}
            virtual bool RestoreSnapshot(Serialization::ReadBuffer &inBufffer, void *inData = nullptr) {}
        };

        TEST_F(V8SnapshotCreaterProviderTest, CreateLoadSnapshot)
        {
            TestUtils::TestLogSink *logSink = TestUtils::TestLogSink::GetGlobalSink();
            Log::Log::SetLogLevel(Log::LogLevel::Error);
            logSink->FlushMessages();

            std::filesystem::path testRoot = s_TestDir / "ConstrcutorInitializeDispose";
            EXPECT_TRUE(TestUtils::CreateAppDirectory(testRoot));
            std::string appName = "testJSAppV8BaseSnapshotCreatorProvider";

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
            V8TestSnapObject testSnapObject;
            EXPECT_FALSE(creator.CreateSnapshot(testSnapObject, snapshotFile));
            expected = {
                {Log::MsgKey::Msg, "CreateSnapshot epxects to be passed a JSRuntime or JSApp object"},
                {Log::MsgKey::LogLevel, "Error"},
            };
            EXPECT_TRUE(logSink->ValidateMessage(expected, m_IgnoreKeys));

            // test app runtime is null
            JSAppSharedPtr testApp = std::make_shared<JSApp>();
            EXPECT_FALSE(creator.CreateSnapshot(*testApp, snapshotFile));
            expected = {
                {Log::MsgKey::Msg, "CreateSNaphot failed to get the main runtime from the app"},
                {Log::MsgKey::LogLevel, "Error"},
            };
            EXPECT_TRUE(logSink->ValidateMessage(expected, m_IgnoreKeys));

            // test runtime isn't initialized
            JSRuntimeSharedPtr testRuntime = std::make_shared<JSRuntime>();
            EXPECT_FALSE(creator.CreateSnapshot(*testRuntime, snapshotFile));
            expected = {
                {Log::MsgKey::Msg, Utils::format("The runtime {} is not initialized", testRuntime->GetName())},
                {Log::MsgKey::LogLevel, "Error"},
            };
            EXPECT_TRUE(logSink->ValidateMessage(expected, m_IgnoreKeys));

            // test runtime isn't snapshottable
            ASSERT_TRUE(testApp->Initialize(appName, testRoot, m_Providers, true));
            ASSERT_TRUE(testRuntime->Initialize(testApp, "testV8SnapshotCreateProviderRuntime"));
            EXPECT_FALSE(creator.CreateSnapshot(*testRuntime, snapshotFile));
            expected = {
                {Log::MsgKey::Msg, Utils::format("The runtime {} is not snapshottable", testRuntime->GetName())},
                {Log::MsgKey::LogLevel, "Error"},
            };
            EXPECT_TRUE(logSink->ValidateMessage(expected, m_IgnoreKeys));

            // test the actuall creation
            EXPECT_TRUE(creator.CreateSnapshot(*testApp, snapshotFile));
            testRuntime->DisposeRuntime();
            testApp->DisposeApp();

            // test the loader
            std::filesystem::path testFile("../../test.dat");
            std::filesystem::path testFile2(testRoot / "resources/test2.dat");

            V8SnapshotProviderSharedPtr snapProvider = std::make_shared<V8SnapshotProvider>();

            EXPECT_EQ(nullptr, snapProvider->GetSnapshotData()->data);
            EXPECT_EQ(0, snapProvider->GetSnapshotData()->raw_size);
            EXPECT_EQ("", snapProvider->GetSnapshotPath().generic_string());

            // no path passed on construction or in function call
            EXPECT_FALSE(snapProvider->LoadSnapshotData(""));
            expected = {
                {Log::MsgKey::Msg, "A path needs to be passed to LoadSnapshotData"},
                {Log::MsgKey::LogLevel, "Error"}};
            EXPECT_TRUE(logSink->ValidateMessage(expected, m_IgnoreKeys));

            // path doesn't exist
            testFile = testRoot / "resources/test.dat";
            EXPECT_FALSE(snapProvider->LoadSnapshotData(testFile));
            expected = {
                {Log::MsgKey::Msg, Utils::format("Passed snapshot path doesn't exist {}", testFile)},
                {Log::MsgKey::LogLevel, "Error"}};
            EXPECT_TRUE(logSink->ValidateMessage(expected, m_IgnoreKeys));

            // loads the snapshot file
            {
                Assets::BinaryAsset dataFile(testFile);
                Assets::BinaryByteVector data{1, 2, 3, 4};
                dataFile.SetContent(data);
                EXPECT_TRUE(dataFile.WriteAsset());

                Assets::BinaryAsset dataFile2(testFile2);
                Assets::BinaryByteVector data2{5, 6};
                dataFile2.SetContent(data2);
                EXPECT_TRUE(dataFile2.WriteAsset());
            }
            EXPECT_TRUE(snapProvider->LoadSnapshotData(testFile));
            EXPECT_EQ(4, snapProvider->GetSnapshotData()->raw_size);
            EXPECT_TRUE(snapProvider->SnapshotLoaded());

            // return true since data is already loaded
            EXPECT_TRUE(snapProvider->LoadSnapshotData(testFile));

            // loads new file over old
            EXPECT_TRUE(snapProvider->LoadSnapshotData(testFile2));
            EXPECT_EQ(2, snapProvider->GetSnapshotData()->raw_size);

            // load the created snapshot and test
            // test that we can load and runa some scripts in it
            ASSERT_TRUE(snapProvider->LoadSnapshotData(snapshotFile));

            AppProviders providers = m_Providers;
            providers.m_SnapshotProvider = snapProvider;

            testApp = std::make_shared<JSApp>();
            ASSERT_TRUE(testApp->Initialize(appName, testRoot, providers));
            {
                JSRuntimeSharedPtr runtime = testApp->GetMainRuntime();
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
            }
            testApp->DisposeApp();
        }
    }
}
