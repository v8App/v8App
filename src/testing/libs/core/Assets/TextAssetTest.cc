// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "test_main.h"

#include "Assets/TextAsset.h"
#include "Logging/Log.h"
#include "Logging/ILogSink.h"
#include "Utils/Format.h"
#include "Utils/Paths.h"

#include "TestLogSink.h"

namespace v8App
{
    namespace Assets
    {
        TEST(TextAssetTest, GetSetContent)
        {
            std::filesystem::path tmp = s_TestDir /"textAsset.txt";
            TextAsset text(tmp);

            EXPECT_EQ("", text.GetContent());
            std::string content("test");
            EXPECT_TRUE(text.SetContent(content));
            EXPECT_EQ(content, text.GetContent());
        }

        TEST(TextAssetTest, ReadWriteAsset)
        {
            std::filesystem::path tmp = s_TestDir / "textAsset.txt";
            std::filesystem::remove(tmp);
            TextAsset text(tmp);

            std::string content("test");
            std::string empty;
            EXPECT_TRUE(text.SetContent(content));
            EXPECT_TRUE(text.WriteAsset());

            EXPECT_TRUE(text.SetContent(empty));
            EXPECT_EQ(empty, text.GetContent());

            EXPECT_TRUE(text.ReadAsset());
            EXPECT_EQ(content, text.GetContent());

#ifndef V8APP_WINDOWS
            //Set file permissions doersn't seem to work on windows so skip this test on windows for now.
            TestUtils::TestLogSink *logSink = TestUtils::TestLogSink::GetGlobalSink();
            Log::Log::SetLogLevel(Log::LogLevel::Error);

            TestUtils::IgnoreMsgKeys ignoreKeys = {
                Log::MsgKey::AppName,
                Log::MsgKey::TimeStamp,
                Log::MsgKey::File,
                Log::MsgKey::Function,
                Log::MsgKey::Line};

            std::filesystem::permissions(tmp, std::filesystem::perms::owner_read | std::filesystem::perms::owner_write, std::filesystem::perm_options::remove);

            EXPECT_FALSE(text.ReadAsset());
                        Log::LogMessage expected = {
                {Log::MsgKey::Msg, Utils::format("Failed to open text asset: {} for reading", tmp)},
                {Log::MsgKey::LogLevel, "Error"},
            };
            EXPECT_TRUE(logSink->ValidateMessage(expected, ignoreKeys));

            EXPECT_FALSE(text.WriteAsset());
            expected = {
                {Log::MsgKey::Msg, Utils::format("Failed to open text asset: {} for writing", tmp)},
                {Log::MsgKey::LogLevel, "Error"},
            };
            EXPECT_TRUE(logSink->ValidateMessage(expected, ignoreKeys));
            Log::Log::RemoveLogSink(logSink->GetName());
#endif
            std::filesystem::remove(tmp);
        }

    } // namespace Assets

} // namespace v8App
