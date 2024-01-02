// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <string>
#include <filesystem>
#include <fstream>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "Utils/VersionString.h"
#include "Utils/Format.h"
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
                std::filesystem::path appRoot = tmp / std::filesystem::path("testApp");
                // make sure the test directory is deleted or the creation fails
                std::filesystem::remove_all(appRoot);
                EXPECT_TRUE(std::filesystem::create_directories(appRoot / c_RootJS));
                EXPECT_TRUE(std::filesystem::create_directories(appRoot / c_RootModules));
                EXPECT_TRUE(std::filesystem::create_directories(appRoot / c_RootResource));
                return appRoot;
            }
        }

        class TestAppAssetRoots : public Assets::AppAssetRoots
        {
        public:
            std::filesystem::path TestReplaceTokens(std::filesystem::path inPath) { return ReplaceTokens(inPath); }
        };

        TEST(AppAssetRootsTest, SetGetAppRootPath)
        {
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
            Log::Log::SetLogLevel(Log::LogLevel::Error);

            std::filesystem::path tmp = std::filesystem::temp_directory_path();
            std::filesystem::path appRoot = tmp / std::filesystem::path("testApp");
            std::error_code code;
            // make sure the test directory is deleted or the creation fails
            std::filesystem::remove_all(appRoot);
            EXPECT_TRUE(std::filesystem::create_directories(appRoot, code));

            // direcotry doesn't exist
            std::unique_ptr<AppAssetRoots> appAssetRoot = std::make_unique<AppAssetRoots>();
            EXPECT_FALSE(appAssetRoot->SetAppRootPath(appRoot / std::filesystem::path("nonExistant")));
            EXPECT_EQ(appAssetRoot->GetAppRoot(), "");

            // root path is a file
            std::filesystem::path filePath = appRoot / std::filesystem::path("testFile");
            std::ofstream file(filePath);
            file.close();
            appAssetRoot = std::make_unique<AppAssetRoots>();
            EXPECT_FALSE(appAssetRoot->SetAppRootPath(filePath));
            EXPECT_EQ(appAssetRoot->GetAppRoot(), "");

            // root valid js directory not found
            appAssetRoot = std::make_unique<AppAssetRoots>();
            EXPECT_FALSE(appAssetRoot->SetAppRootPath(appRoot));
            Log::LogMessage expected = {
                {Log::MsgKey::Msg, Utils::format("Failed to find the {} directory in the app root", c_RootJS)},
                {Log::MsgKey::LogLevel, "Error"},
            };
            EXPECT_TRUE(logSink->ValidateMessage(expected, ignoreKeys));
            EXPECT_EQ(appAssetRoot->GetAppRoot(), "");

            // root valid modules directory not found
            EXPECT_TRUE(std::filesystem::create_directories(appRoot / c_RootJS));
            appAssetRoot = std::make_unique<AppAssetRoots>();
            EXPECT_FALSE(appAssetRoot->SetAppRootPath(appRoot));
            expected = {
                {Log::MsgKey::Msg, Utils::format("Failed to find the {} directory in the app root", c_RootModules)},
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
                {Log::MsgKey::Msg, Utils::format("Failed to find the {} directory in the app root", c_RootResource)},
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
            std::filesystem::path module1 = moduleRoot / std::filesystem::path("test/1.2.3");
            std::filesystem::path module2 = moduleRoot / std::filesystem::path("test/2.0.0");
            std::filesystem::path module3 = moduleRoot / std::filesystem::path("test/NotAVersion");
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
            std::filesystem::path appRoot = tmp / std::filesystem::path("testApp");
            std::unique_ptr<AppAssetRoots> appAssetRoot = std::make_unique<AppAssetRoots>();

            std::filesystem::path moduleRoot = appRoot / std::filesystem::path("testModule");
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

        TEST(AppAssetRootsTest, MakeRelativePathToAppRoot)
        {
            std::filesystem::path root = Testing::CreateValidAppRoot();
            std::unique_ptr<AppAssetRoots> appAssetRoot = std::make_unique<AppAssetRoots>();
            EXPECT_TRUE(appAssetRoot->SetAppRootPath(root));

            std::filesystem::path path1 = root / std::filesystem::path("/test/test2");
            std::filesystem::path path2 = std::filesystem::path("test/test2");
            std::filesystem::path path3 = std::filesystem::path("test/../test2");
            std::filesystem::path path4 = std::filesystem::path("../../../test/test2");
            std::filesystem::path win = root / std::filesystem::path("test\\test2");
            std::filesystem::path win2 =  std::filesystem::path("C:\\test\\test2");

            //string version
            EXPECT_EQ(appAssetRoot->MakeRelativePathToAppRoot(path1).string(), "test/test2");

            EXPECT_EQ(appAssetRoot->MakeRelativePathToAppRoot(path1).string(), "test/test2");
            EXPECT_EQ(appAssetRoot->MakeRelativePathToAppRoot(path2).string(), "test/test2");
            EXPECT_EQ(appAssetRoot->MakeRelativePathToAppRoot(path3).string(), "test2");
            EXPECT_EQ(appAssetRoot->MakeRelativePathToAppRoot(path4).string(), "");
            EXPECT_EQ(appAssetRoot->MakeRelativePathToAppRoot(win).string(), "test/test2");
            EXPECT_EQ(appAssetRoot->MakeRelativePathToAppRoot(win2).string(), "test/test2");
        }

        TEST(AppAssetRootsTest, MakeAbsolutePathToAppRoot)
        {
            std::filesystem::path root = Testing::CreateValidAppRoot();
            std::unique_ptr<AppAssetRoots> appAssetRoot = std::make_unique<AppAssetRoots>();
            EXPECT_TRUE(appAssetRoot->SetAppRootPath(root));

            std::filesystem::path path1 = root / std::filesystem::path("/test/test2");
            std::filesystem::path path2 = std::filesystem::path("test/test2");
            std::filesystem::path path3 = std::filesystem::path("test/../test2");
            std::filesystem::path path4 = std::filesystem::path("../../../test/test2");
            std::filesystem::path win = root / std::filesystem::path("test\\test2");
            std::filesystem::path win2 =  std::filesystem::path("C:\\test\\test2");

            //string version
            EXPECT_EQ(appAssetRoot->MakeAbsolutePathToAppRoot(path1).string(), (root / std::filesystem::path("test/test2")).string());

            EXPECT_EQ(appAssetRoot->MakeAbsolutePathToAppRoot(path1).string(), (root / std::filesystem::path("test/test2")).string());
            EXPECT_EQ(appAssetRoot->MakeAbsolutePathToAppRoot(path2).string(), (root / std::filesystem::path("test/test2")).string());
            EXPECT_EQ(appAssetRoot->MakeAbsolutePathToAppRoot(path3).string(), (root / std::filesystem::path("test2")).string());
            EXPECT_EQ(appAssetRoot->MakeAbsolutePathToAppRoot(path4).string(), "");
            EXPECT_EQ(appAssetRoot->MakeAbsolutePathToAppRoot(win).string(), (root / std::filesystem::path("test/test2")).string());
            EXPECT_EQ(appAssetRoot->MakeAbsolutePathToAppRoot(win2).string(), (root / std::filesystem::path("test/test2")).string());
        }

        TEST(AppAssetRootsTest, ReplaceTokens)
        {
            std::filesystem::path root = Testing::CreateValidAppRoot();
            std::unique_ptr<TestAppAssetRoots> appAssetRoot = std::make_unique<TestAppAssetRoots>();
            EXPECT_TRUE(appAssetRoot->SetAppRootPath(root));

            std::filesystem::path appToken = std::filesystem::path("%APPROOT%/js/test.js");
            std::filesystem::path jsToken = std::filesystem::path("%JS%/test.js");
            std::filesystem::path modulesToken = std::filesystem::path("%MODULES%/js/test.js");
            std::filesystem::path resourcesToken = std::filesystem::path("%RESOURCES%/test.txt");

            std::filesystem::path expectedAppToken = std::filesystem::path("js/test.js");
            std::filesystem::path expectedJsToken = std::filesystem::path("js/test.js");
            std::filesystem::path expectedModulesToken = std::filesystem::path("modules/js/test.js");
            std::filesystem::path expexctedResourcesToken = std::filesystem::path("resources/test.txt");

            EXPECT_EQ(appAssetRoot->TestReplaceTokens(appToken).string(), (root / expectedAppToken).string());
            EXPECT_EQ(appAssetRoot->TestReplaceTokens(jsToken).string(), (root / expectedJsToken).string());
            EXPECT_EQ(appAssetRoot->TestReplaceTokens(modulesToken).string(), (root / expectedModulesToken).string());
            EXPECT_EQ(appAssetRoot->TestReplaceTokens(resourcesToken).string(), (root / expexctedResourcesToken).string());
            
            // check that the Make versions replace the token as well
            EXPECT_EQ(appAssetRoot->MakeRelativePathToAppRoot(jsToken).string(), expectedJsToken.string());
            EXPECT_EQ(appAssetRoot->MakeAbsolutePathToAppRoot(jsToken).string(), (root / expectedJsToken).string());

            //that the tokenis only replace when it's at the beginning
            std::filesystem::path path = std::filesystem::path("js/%RESOURCES%/test.js");
            EXPECT_EQ(appAssetRoot->TestReplaceTokens(path).string(), path.string());
        }

    }
}