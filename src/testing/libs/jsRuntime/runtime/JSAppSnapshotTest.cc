// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "V8Fixture.h"
#include "TestFiles.h"
#include "TestSnapshotCreator.h"
#include "TestSnapshotProvider.h"

#include "JSApp.h"
#include "V8RuntimeProvider.h"
#include "V8ContextProvider.h"

namespace v8App
{
    namespace JSRuntime
    {
        using JSAppSnapshotTest = V8Fixture;

        class TestJSApp : public JSApp
        {
        public:
            TestJSApp(std::string inName, AppProviders inProviders) : JSApp(inName, inProviders) {}

            virtual JSAppSharedPtr CreateApp()
            {
                m_AppCreated = true;
                return std::make_shared<TestJSApp>();
            }

            static inline bool m_AppCreated{false};
        };

        TEST_F(JSAppSnapshotTest, CreateSnapshotTestCreator)
        {
            TestUtils::TestLogSink *logSink = TestUtils::TestLogSink::GetGlobalSink();
            Log::Log::SetLogLevel(Log::LogLevel::Error);
            Log::LogMessage expected;

            std::filesystem::path testRoot = s_TestDir / "CreateSnapshotTestCreator";
            std::filesystem::path snapFile = testRoot / "testSnapTestCreator.bin";
            EXPECT_TRUE(TestUtils::CreateAppDirectory(testRoot));

            std::string appName = "testJSAppSnapshotTest";
            AppProviders providers(std::make_shared<TestSnapshotProvider>(),
                                   std::make_shared<V8RuntimeProvider>(),
                                   std::make_shared<V8ContextProvider>());
            IJSSnapshotCreatorSharedPtr snapCreator = std::make_shared<TestSnapshotCreator>();
            TestSnapshotCreator *testCreator = dynamic_cast<TestSnapshotCreator*>(snapCreator.get());

            JSAppSharedPtr app = std::make_shared<TestJSApp>();
            ASSERT_TRUE(app->Initialize(appName, testRoot, providers, false));
            // file is empty
            EXPECT_FALSE(app->CreateSnapshot(snapCreator, ""));
            expected = {
                {Log::MsgKey::Msg, "A snapshot file must be passed to write the data to"},
                {Log::MsgKey::LogLevel, "Error"},
            };
            EXPECT_TRUE(logSink->ValidateMessage(expected, m_IgnoreKeys));

            // snap creator is null
            EXPECT_FALSE(app->CreateSnapshot(nullptr, snapFile));
            expected = {
                {Log::MsgKey::Msg, "The snapshot creator must not be a nullptr when creating a snapshot"},
                {Log::MsgKey::LogLevel, "Error"},
            };
            EXPECT_TRUE(logSink->ValidateMessage(expected, m_IgnoreKeys));
            
            // snapshot creator fails
            testCreator->SetSuccess(false);
            EXPECT_FALSE(app->CreateSnapshot(snapCreator, snapFile));
            // since the app ins't a snapshot version it creates it.
            EXPECT_TRUE(TestJSApp::m_AppCreated);

            app->DisposeApp();
            return;

            // snapshot create returns true no snapshot created though for the test creator
            testCreator->SetSuccess(true);
            EXPECT_TRUE(app->CreateSnapshot(snapCreator, snapFile));

            app->DisposeApp();

            //test where the app is already a snapshot app
            app = std::make_shared<TestJSApp>();
            ASSERT_TRUE(app->Initialize(appName + "AppSnap", testRoot, providers, true));
            TestJSApp::m_AppCreated = false;
            EXPECT_TRUE(app->CreateSnapshot(snapCreator, snapFile));
            EXPECT_FALSE(TestJSApp::m_AppCreated);
            app->DisposeApp();
        }

        TEST_F(JSAppSnapshotTest, TestV8Snapshot)
        {
        }
    }
}
