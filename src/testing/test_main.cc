// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <chrono>
#include <cstdlib>
#include <thread>

#include "gtest/gtest.h"

#ifdef USE_JSRUNTIME
#include "Utils/Environment.h"
#include "Logging/LogJSONFile.h"

#include "V8Types.h"
#include "V8Platform.h"
#include "JSRuntime.h"
#endif

#include "test_main.h"

std::unique_ptr<Runfiles> s_Runfiles;
std::filesystem::path s_TestDir;
v8::StartupData s_V8StartupData{nullptr, 0};

int main(int argc, char **argv)
{
    std::vector<char *> new_argv(argv, argv + argc);
    // needs to be available at the main scpe level
    std::string testDirArg("--test-dir=");
    bool setupDone = false;
    bool listTest = false;

    // gtest when running a death test passes an argument about death tests
    // so if the flag is passd skip setup
    for (int x = 0; x < argc; x++)
    {
        std::string arg(argv[x]);

        if (arg.starts_with("--test-dir="))
        {
            setupDone = true;
            size_t arg_sepe = arg.find("=");

            arg = arg.replace(0, 11, "");
            s_TestDir = std::filesystem::path(arg);
        }
        if (arg.starts_with("--gtest_list_tests"))
        {
            listTest = true;
        }
    }
    // if just listing tests then skip all the setup
    if (listTest)
    {
        testing::InitGoogleTest(&argc, new_argv.data());
        return RUN_ALL_TESTS();
    }

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
    if (s_TestDir.empty())
    {
        const char *testDirEnv = std::getenv("TEST_TMPDIR");

        if (testDirEnv == nullptr)
        {
            s_TestDir = std::filesystem::temp_directory_path();
        }
        else
        {
            s_TestDir = std::filesystem::path(testDirEnv);
        }

        auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        std::string time_str(19, '\0');
        std::strftime(&time_str[0], time_str.size(), "%Y_%m_%d-%H_%M_%S", std::localtime(&now));

        // create temp directory for test
        s_TestDir /= std::filesystem::path(time_str);

        // add it to the arguments so it gets passed to any future invocation of the app
        testDirArg += s_TestDir.string();
        new_argv.push_back(const_cast<char *>(testDirArg.c_str()));
        new_argv.push_back(nullptr);
        argc++;
    }
    if (std::filesystem::exists(s_TestDir) == false)
    {
        std::cout << "Creating testDir: " << s_TestDir << std::endl;
        if (std::filesystem::create_directories(s_TestDir) == false)
        {
            std::cout << "main.cc:Failed to create the test directoery: " << s_TestDir << std::endl;
            std::exit(1);
        }

        // Get The runfiles name for the path to the test files if empty then no test files needed
        const char *testFilesRunLoc = std::getenv("TestFiles");

        if (testFilesRunLoc != nullptr)
        {
            std::filesystem::path testFileDir = s_Runfiles->Rlocation(testFilesRunLoc);
            if (testFileDir.empty())
            {
                std::cout << "Failed to get the test files source directory" << std::endl;
                std::exit(1);
            }

            // copy over test files into it
            std::cout << "Copying test files from: " << testFileDir << std::endl;

            std::string cmd = "cp -r " + testFileDir.string() + "/* " + s_TestDir.string();
#if defined(V8_APP_WIN)
            cmd = "xcopy "+testFileDir.string()+"\\* "+s_TestDir.string)+" /E /R /K /O /Y ";
#endif
            if (std::system(cmd.c_str()) != 0)
            {
                std::cout << "Failed to copy the test files to the test direcotry: " << s_TestDir << std::endl;
                std::exit(1);
            }
        }
    }

#ifdef USE_JSRUNTIME
    std::filesystem::path logPath = s_TestDir / std::filesystem::path("log");
    std::filesystem::create_directories(logPath);
    logPath /= std::filesystem::path("UnitTestLog.json");
    //only omit it if we detected we are a child run from like  death test
    if (setupDone == false)
    {
        std::cout << "Log File: " << logPath << std::endl;
    }
    std::unique_ptr<v8App::Log::ILogSink> jsonLog = std::make_unique<v8App::Log::LogJSONFile>("UnitTestLog", logPath);

    v8App::Log::Log::AddLogSink(jsonLog);
    std::string icu_name = v8App::Utils::GetEnvironmentVar("V8_ICU_DATA");
    std::string snapshot_name = v8App::Utils::GetEnvironmentVar("V8_SNAPSHOT_BIN");

    if (icu_name.empty() || snapshot_name.empty())
    {
        std::cout << "Failed to find one or both of env vars V8_ICU_DATA, V8_SNAPSHOT_BIN";
    }

    std::string icuData = s_Runfiles->Rlocation(icu_name);
    std::string snapshotData = s_Runfiles->Rlocation(snapshot_name);

    EXPECT_NE("", icuData);
    EXPECT_NE("", snapshotData);

    v8::V8::InitializeICU(icuData.c_str());

    std::ifstream sData(snapshotData, std::ios_base::binary | std::ios_base::ate);
    if(sData.is_open() == false || sData.fail())
    {
        std::cout << "Failed to open " << snapshotData << std::endl;
        std::exit(1);
    }
    int dataLength = sData.tellg();
    sData.seekg(0, std::ios::beg);
    std::unique_ptr<char> buf = std::unique_ptr<char>(new char[dataLength]);
    sData.read(buf.get(), dataLength);
    s_V8StartupData =  v8::StartupData{buf.release(), dataLength};

    v8App::JSRuntime::PlatformIsolateHelperUniquePtr helper = std::make_unique<v8App::JSRuntime::JSRuntimeIsolateHelper>();
    v8App::JSRuntime::V8Platform::InitializeV8(std::move(helper));
#endif
    std::cout << std::endl
              << std::endl;
    testing::InitGoogleTest(&argc, new_argv.data());
    int exitCode = RUN_ALL_TESTS();

#ifdef USE_JSRUNTIME
    v8App::JSRuntime::V8Platform::ShutdownV8();
#endif

    return exitCode;
}