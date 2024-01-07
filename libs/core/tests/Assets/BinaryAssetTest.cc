// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "Assets/BinaryAsset.h"
#include "Logging/Log.h"
#include "Logging/ILogSink.h"
#include "Utils/Format.h"

#include "TestLogSink.h"

namespace v8App
{
    namespace Assets
    {
        TEST(BinaryAssetTest, GetSetContent)
        {
            std::filesystem::path tmp = std::filesystem::temp_directory_path();
            tmp /= std::filesystem::path("binraryAsset.bin");
            BinaryAsset binary(tmp);

            EXPECT_TRUE(binary.GetContent().empty());
            std::vector<uint8_t> content{1, 2, 3};
            EXPECT_TRUE(binary.SetContent(content));
            EXPECT_EQ(content, binary.GetContent());
        }

        TEST(BinaryAssetTest, ReadWriteAsset)
        {
            std::filesystem::path tmp = std::filesystem::temp_directory_path();
            tmp /= std::filesystem::path("binraryAsset.bin");
            std::filesystem::remove(tmp);
            BinaryAsset binary(tmp);

            std::vector<uint8_t> content{1, 2, 3};
            std::vector<uint8_t> empty;
            EXPECT_TRUE(binary.SetContent(content));
            EXPECT_TRUE(binary.WriteAsset());

            EXPECT_TRUE(binary.SetContent(empty));
            EXPECT_EQ(empty, binary.GetContent());

            EXPECT_TRUE(binary.ReadAsset());
            EXPECT_EQ(content, binary.GetContent());

            TestUtils::WantsLogLevelsVector error = {Log::LogLevel::Error};
            TestUtils::TestLogSink *logSink = new TestUtils::TestLogSink("TestLogSink", error);
            std::unique_ptr<Log::ILogSink> logSinkObj(logSink);
            EXPECT_TRUE(Log::Log::AddLogSink(logSinkObj));
            TestUtils::IgnoreMsgKeys ignoreKeys = {
                Log::MsgKey::AppName,
                Log::MsgKey::TimeStamp,
                Log::MsgKey::File,
                Log::MsgKey::Function,
                Log::MsgKey::Line};

            std::filesystem::permissions(tmp, std::filesystem::perms::owner_read | std::filesystem::perms::owner_write, std::filesystem::perm_options::remove);

            EXPECT_FALSE(binary.ReadAsset());
            Log::LogMessage expected = {
                {Log::MsgKey::Msg, Utils::format("Failed to open binary asset: {} for reading", tmp)},
                {Log::MsgKey::LogLevel, "Error"},
            };
            EXPECT_TRUE(logSink->ValidateMessage(expected, ignoreKeys));

            EXPECT_FALSE(binary.WriteAsset());
            expected = {
                {Log::MsgKey::Msg, Utils::format("Failed to open binary asset: {} for writing", tmp)},
                {Log::MsgKey::LogLevel, "Error"},
            };
            EXPECT_TRUE(logSink->ValidateMessage(expected, ignoreKeys));

            std::filesystem::remove(tmp);
            Log::Log::RemoveLogSink(logSink->GetName());
        }

    } // namespace Assets

} // namespace v8App
