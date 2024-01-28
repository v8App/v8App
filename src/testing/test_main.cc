// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <chrono>
#include <cstdlib>
#include <thread>

#include "gtest/gtest.h"

#include "Utils/Environment.h"
#include "Utils/Format.h"

#ifdef USE_JSRUNTIME
#include "V8Types.h"
#include "V8Platform.h"
#include "JSRuntime.h"
#endif

#include "test_main.h"

std::unique_ptr<Runfiles> s_Runfiles;
std::filesystem::path s_TestDir;

int main(int argc, char **argv)
{
    std::string error;
    if (s_Runfiles == nullptr)
    {
        s_Runfiles.reset(Runfiles::CreateForTest(&error));
        if (error.empty() == false)
        {
            std::cout << "Got an error creating run files. Error: " << errno << std::endl;
            std::exit(1);
        }
    }
    s_TestDir = v8App::Utils::GetEnvironmentVar("TEST_TMPDIR");
    if (s_TestDir.empty())
    {
        s_TestDir = std::filesystem::temp_directory_path();
    }

    auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::string time_str(19, '\0');
    std::strftime(&time_str[0], time_str.size(), "%Y_%m_%d-%H_%M_%S", std::localtime(&now));

    // create temp directory for test
    s_TestDir /= std::filesystem::path(time_str);

    if (std::filesystem::exists(s_TestDir) == false)
    {
        std::cout << "Creating testDir: " << s_TestDir << std::endl;
        if (std::filesystem::create_directories(s_TestDir) == false)
        {
            std::cout << "main.cc:Failed to create the test directoery: " << s_TestDir << std::endl;
            std::exit(1);
        }

        // Get The runfiles name for the path to the test files if empty then no test files needed
        std::string testFilesRunLoc = v8App::Utils::GetEnvironmentVar("TestFiles");
        if (testFilesRunLoc.empty() == false)
        {
            std::filesystem::path testFileDir = s_Runfiles->Rlocation(testFilesRunLoc);
            if (testFileDir.empty())
            {
                std::cout << "Failed to get the test files source directory" << std::endl;
                std::exit(1);
            }

            // copy over test files into it
            std::cout << "Copying test files from: " << testFileDir << std::endl;

            std::string cmd = v8App::Utils::format("cp -r {}/* {}", testFileDir, s_TestDir);
#if defined(V8_APP_WIN)
            cmd = v8App::Utils::format("xcopy {}\\* {} /E /R /K /O /Y ", testFileDir, s_TestDir);
#endif
            if (std::system(cmd.c_str()) != 0)
            {
                std::cout << "Failed to copy the test files to the test direcotry: " << s_TestDir << std::endl;
                std::exit(1);
            }
        }
    }

#ifdef USE_JSRUNTIME
    std::string icu_name = v8App::Utils::GetEnvironmentVar("V8_ICU_DATA");
    std::string snapshot_name = v8App::Utils::GetEnvironmentVar("V8_SNAPSHOT_BIN");

    if (icu_name.empty() || snapshot_name.empty())
    {
        EXPECT_TRUE(false) << "Failed to find one or both of env vars V8_ICU_DATA, V8_SNAPSHOT_BIN";
    }

    std::string icuData = s_Runfiles->Rlocation(icu_name);
    std::string snapshotData = s_Runfiles->Rlocation(snapshot_name);

    EXPECT_NE("", icuData);
    EXPECT_NE("", snapshotData);

    v8::V8::InitializeICU(icuData.c_str());
    v8::V8::InitializeExternalStartupDataFromFile(snapshotData.c_str());

    v8App::JSRuntime::PlatformIsolateHelperUniquePtr helper = std::make_unique<v8App::JSRuntime::JSRuntimeIsolateHelper>();
    v8App::JSRuntime::V8Platform::InitializeV8(std::move(helper));
#endif

    testing::InitGoogleTest(&argc, argv);
    int exitCode = RUN_ALL_TESTS();

#ifdef USE_JSRUNTIME
    v8App::JSRuntime::V8Platform::ShutdownV8();
#endif

    return exitCode;
}