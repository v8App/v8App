// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <fstream>

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "TestLogSink.h"

#include "JSAppCreatorRegistry.h"
#include "JSApp.h"

namespace v8App
{
    namespace JSRuntime
    {
        namespace Testing
        {
            JSAppSharedPtr JSAppCreatorRegisteryTestAppCreator() { return nullptr; }
        }

        TEST(JSAppCreatorRegistryTest, RegisterCreate)
        {
            TestUtils::TestLogSink *logSink = TestUtils::TestLogSink::GetGlobalSink();
            Log::Log::SetLogLevel(Log::LogLevel::Warn);
            logSink->FlushMessages();

            TestUtils::IgnoreMsgKeys ignoreKeys = {
                Log::MsgKey::AppName,
                Log::MsgKey::TimeStamp,
                Log::MsgKey::File,
                Log::MsgKey::Function,
                Log::MsgKey::Line};

            JSAppCreatorRegistry::RegisterCreator("JSApp", Testing::JSAppCreatorRegisteryTestAppCreator);
            Log::LogMessage expected = {
                {Log::MsgKey::Msg, "Found an app already registered with class Type 'JSApp'"},
                {Log::MsgKey::LogLevel, "Warn"},
            };
            EXPECT_TRUE(logSink->ValidateMessage(expected, ignoreKeys));

            JSAppCreatorRegistry::RegisterCreator("JSApp", JSApp::AppCreator);
            EXPECT_TRUE(logSink->NoMessages());

            JSAppSharedPtr app = JSAppCreatorRegistry::CreateApp("NotExist");
                EXPECT_EQ(nullptr, app);

            app = JSAppCreatorRegistry::CreateApp("JSApp");
                EXPECT_NE(nullptr, app);
        }
    }
}