// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <string>
#include <filesystem>
#include <fstream>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "Utils/VersionString.h"
#include "Logging/Log.h"
#include "Logging/ILogSink.h"
#include "TestLogSink.h"
#include "Assets/AppAssetRoots.h"

namespace v8App
{
    namespace Assets
    {
        namespace Testing
        {
            std::filesystem::path CreateValidAppRoot()
            {
                std::filesystem::path tmp = std::filesystem::temp_directory_path();
                std::filesystem::path appRoot = tmp / "testApp";
                // make sure the test directory is deleted or the creation fails
                std::filesystem::remove_all(appRoot);
                EXPECT_TRUE(std::filesystem::create_directories(appRoot / c_RootJS));
                EXPECT_TRUE(std::filesystem::create_directories(appRoot / c_RootModules));
                EXPECT_TRUE(std::filesystem::create_directories(appRoot / c_RootResource));
                return appRoot;
            }
        }
        TEST(AppAssetRootsTest, SetGetAppRootPath)
        {
            TestUtils::WantsLogLevelsVector error = {Log::LogLevel::Error};
            TestUtils::TestLogSink *logSink = new TestUtils::TestLogSink("TestLogSink", error);
            std::unique_ptr<Log::ILogSink> logSinkObj(logSink);
            EXPECT_TRUE(Log::Log::AddLogSink(logSinkObj));
            TestUtils::IgnoreMsgKeys ignoreKeys = {Log::MsgKey::AppName, Log::MsgKey::TimeStamp};
            Log::Log::SetLogLevel(Log::LogLevel::Error);

            std::filesystem::path tmp = std::filesystem::temp_directory_path();
            std::filesystem::path appRoot = tmp / "testApp";
            std::error_code code;
            // make sure the test directory is deleted or the creation fails
            std::filesystem::remove_all(appRoot);
            EXPECT_TRUE(std::filesystem::create_directories(appRoot, code));

            // direcotry doesn't exist
            std::unique_ptr<AppAssetRoots> appAssetRoot = std::make_unique<AppAssetRoots>();
            EXPECT_FALSE(appAssetRoot->SetAppRootPath(appRoot / "nonExistant"));
            EXPECT_EQ(appAssetRoot->GetAppRoot(), "");

            // root path is a file
            std::filesystem::path filePath = appRoot / "testFile";
            std::ofstream file(filePath);
            file.close();
            appAssetRoot = std::make_unique<AppAssetRoots>();
            EXPECT_FALSE(appAssetRoot->SetAppRootPath(filePath));
            EXPECT_EQ(appAssetRoot->GetAppRoot(), "");

            // root valid js directory not found
            appAssetRoot = std::make_unique<AppAssetRoots>();
            EXPECT_FALSE(appAssetRoot->SetAppRootPath(appRoot));
            Log::LogMessage expected = {
                {Log::MsgKey::Msg, std::format("Failed to find the {} directory in the app root", c_RootJS)},
                {Log::MsgKey::LogLevel, "Error"},
            };
            EXPECT_TRUE(logSink->ValidateMessage(expected, ignoreKeys));
            EXPECT_EQ(appAssetRoot->GetAppRoot(), "");

            // root valid modules directory not found
            EXPECT_TRUE(std::filesystem::create_directories(appRoot / c_RootJS));
            appAssetRoot = std::make_unique<AppAssetRoots>();
            EXPECT_FALSE(appAssetRoot->SetAppRootPath(appRoot));
            expected = {
                {Log::MsgKey::Msg, std::format("Failed to find the {} directory in the app root", c_RootModules)},
                {Log::MsgKey::LogLevel, "Error"},
            };
            EXPECT_TRUE(logSink->ValidateMessage(expected, ignoreKeys));
            EXPECT_EQ(appAssetRoot->GetAppRoot(), "");

            // root valid resources directory not found
            std::filesystem::path moduleRoot = appRoot / c_RootModules;
            EXPECT_TRUE(std::filesystem::create_directories(moduleRoot));
            appAssetRoot = std::make_unique<AppAssetRoots>();
            EXPECT_FALSE(appAssetRoot->SetAppRootPath(appRoot));
            expected = {
                {Log::MsgKey::Msg, std::format("Failed to find the {} directory in the app root", c_RootResource)},
                {Log::MsgKey::LogLevel, "Error"},
            };
            EXPECT_TRUE(logSink->ValidateMessage(expected, ignoreKeys));
            EXPECT_EQ(appAssetRoot->GetAppRoot(), "");

            // core root directories found
            EXPECT_TRUE(std::filesystem::create_directories(appRoot / c_RootResource));
            appAssetRoot = std::make_unique<AppAssetRoots>();
            EXPECT_TRUE(appAssetRoot->SetAppRootPath(appRoot));
            EXPECT_EQ(appAssetRoot->GetAppRoot(), appRoot);

            // app root was set already
            EXPECT_FALSE(appAssetRoot->SetAppRootPath(appRoot));

            EXPECT_FALSE(appAssetRoot->FindModuleRootPath(c_RootJS).empty());
            EXPECT_FALSE(appAssetRoot->FindModuleRootPath(c_RootResource).empty());

            // some modules found
            std::filesystem::path module1 = moduleRoot / "test" / "1.2.3";
            std::filesystem::path module2 = moduleRoot / "test" / "2.0.0";
            std::filesystem::path module3 = moduleRoot / "test" / "NotAVersion";
            EXPECT_TRUE(std::filesystem::create_directories(module1));
            EXPECT_TRUE(std::filesystem::create_directories(module2));
            EXPECT_TRUE(std::filesystem::create_directories(module3));
            appAssetRoot = std::make_unique<AppAssetRoots>();
            EXPECT_TRUE(appAssetRoot->SetAppRootPath(appRoot));

            EXPECT_EQ(appAssetRoot->FindModuleRootPath("test/1.2.3"), module1);
            EXPECT_EQ(appAssetRoot->FindModuleRootPath("test/2.0.0"), module2);
            EXPECT_EQ(appAssetRoot->GetModulesLatestVersion("test"), Utils::VersionString("2.0.0"));
            EXPECT_TRUE(appAssetRoot->FindModuleRootPath("test/NotAVersion").empty());

            // set via a string
            appAssetRoot = std::make_unique<AppAssetRoots>();
            EXPECT_TRUE(appAssetRoot->SetAppRootPath(appRoot.string()));
            EXPECT_EQ(appAssetRoot->GetAppRoot(), appRoot);
            Log::Log::RemoveLogSink(logSink->GetName());
        }

        TEST(AppAssetRootsTest, AddFindRemoveModuleRootPath)
        {
            std::filesystem::path tmp = std::filesystem::temp_directory_path();
            std::filesystem::path appRoot = tmp / "testApp";
            std::unique_ptr<AppAssetRoots> appAssetRoot = std::make_unique<AppAssetRoots>();

            std::filesystem::path moduleRoot = appRoot / "testModule";
            EXPECT_TRUE(appAssetRoot->AddModuleRootPath("test", moduleRoot));
            EXPECT_EQ(appAssetRoot->FindModuleRootPath("test"), moduleRoot);
            EXPECT_TRUE(appAssetRoot->FindModuleRootPath("NonExistant").empty());
            appAssetRoot->RemoveModuleRootPath("test");
            EXPECT_TRUE(appAssetRoot->FindModuleRootPath("test").empty());
        }

        TEST(AppAssetRootsTest, SetGetRemoveModulesLatestVersion)
        {
            std::unique_ptr<AppAssetRoots> appAssetRoot = std::make_unique<AppAssetRoots>();

            Utils::VersionString version("1.0.0");
            appAssetRoot->SetModulesLatestVersion("test", version);
            EXPECT_EQ(appAssetRoot->GetModulesLatestVersion("test"), version);
            Utils::VersionString version2("2.0.0");
            appAssetRoot->SetModulesLatestVersion("test", version2);
            EXPECT_EQ(appAssetRoot->GetModulesLatestVersion("test"), version2);
            appAssetRoot->RemoveModulesLatestVersion("test");
            EXPECT_FALSE(appAssetRoot->GetModulesLatestVersion("test").IsVersionString());
        }

        TEST(AppAssetRootsTest, DidPathEscapeRoot)
        {
            std::filesystem::path tmp = std::filesystem::temp_directory_path();
            std::unique_ptr<AppAssetRoots> appAssetRoot = std::make_unique<AppAssetRoots>();

            EXPECT_FALSE(appAssetRoot->DidPathEscapeRoot(tmp, tmp / "test"));
            EXPECT_FALSE(appAssetRoot->DidPathEscapeRoot(tmp, tmp / "test/test"));
            EXPECT_TRUE(appAssetRoot->DidPathEscapeRoot(tmp, tmp / "../test"));
        }

        TEST(AppAssetRootsTest, MakeRelativePathTo)
        {
            std::filesystem::path root = Testing::CreateValidAppRoot();
            std::unique_ptr<AppAssetRoots> appAssetRoot = std::make_unique<AppAssetRoots>();
            EXPECT_TRUE(appAssetRoot->SetAppRootPath(root));

            std::filesystem::path tmp2 = root / "test/test2";
            std::filesystem::path tmp3 = "test/test2";
            std::filesystem::path tmpWin = root / "test\\test2";
            std::filesystem::path tmp4 = std::filesystem::temp_directory_path() / "/test/test2";

            EXPECT_EQ(appAssetRoot->MakeRelativePathToRoot(tmp2, root), "test/test2");
            EXPECT_EQ(appAssetRoot->MakeRelativePathToRoot(tmp3, root), "test/test2");
            EXPECT_EQ(appAssetRoot->MakeRelativePathToRoot(tmpWin, root), "test/test2");
            EXPECT_EQ(appAssetRoot->MakeRelativePathToRoot(tmp2.string(), root.string()), "test/test2");
            EXPECT_TRUE(appAssetRoot->MakeRelativePathToRoot(tmp4, root.string()).empty());

            EXPECT_EQ(appAssetRoot->MakeRelativePathToAppRoot(tmp2), "test/test2");
            EXPECT_EQ(appAssetRoot->MakeRelativePathToAppRoot(tmpWin), "test/test2");
            EXPECT_EQ(appAssetRoot->MakeRelativePathToAppRoot(tmp2.string()), "test/test2");
            EXPECT_TRUE(appAssetRoot->MakeRelativePathToAppRoot(tmp4).empty());
        }

        TEST(AppAssetRootsTest, MakeAbsolutePath)
        {
            std::filesystem::path root = Testing::CreateValidAppRoot();
            std::unique_ptr<AppAssetRoots> appAssetRoot = std::make_unique<AppAssetRoots>();
            EXPECT_TRUE(appAssetRoot->SetAppRootPath(root));

            std::filesystem::path tmp2 = root / "test/test2";
            std::filesystem::path tmpWin = root / "test\\test2";
            std::filesystem::path tmp3 = std::filesystem::temp_directory_path() / "/test/test2";
            std::filesystem::path abs1 = std::filesystem::absolute(root / "test/test2");
            std::filesystem::path abs2 = std::filesystem::absolute(tmp3);

            EXPECT_EQ(appAssetRoot->MakeAbsolutePathChecked(tmp2), abs1);
            EXPECT_EQ(appAssetRoot->MakeAbsolutePathChecked(tmpWin), abs1);
            EXPECT_EQ(appAssetRoot->MakeAbsolutePathChecked(abs1), abs1);
            EXPECT_EQ(appAssetRoot->MakeAbsolutePathChecked(tmp2.string()), abs1);
            EXPECT_TRUE(appAssetRoot->MakeAbsolutePathChecked(tmp3).empty());

            EXPECT_EQ(appAssetRoot->MakeAbsolutePath(tmp2), abs1);
            EXPECT_EQ(appAssetRoot->MakeAbsolutePath(tmpWin), abs1);
            EXPECT_EQ(appAssetRoot->MakeAbsolutePath(abs1), abs1);
            EXPECT_EQ(appAssetRoot->MakeAbsolutePath(tmp2.string()), abs1);
            EXPECT_EQ(appAssetRoot->MakeAbsolutePath(tmp3), abs2);
        }

    }
}