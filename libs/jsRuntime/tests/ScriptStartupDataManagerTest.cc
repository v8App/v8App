
// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <ostream>
#include <fstream>

#include "V8TestFixture.h"
#include "ScriptStartupDataManager.h"

namespace v8App
{
    namespace JSRuntime
    {
        using ScriptStartupDataManTest = V8TestFixture;

        namespace ScrtipStartupTestInternal
        {
            bool WriteTestFile(std::filesystem::path inFileName, std::string inCOntents)
            {
                std::ofstream fileStream(inFileName, std::ios::out);
                if (fileStream.is_open() == false)
                {
                    return false;
                }
                fileStream << inCOntents;
                fileStream.close();
                return true;
            }
        }
        TEST_F(ScriptStartupDataManTest, TestLoadScriptFile)
        {
            std::string source;

            std::filesystem::path testFilesDir = m_RunFiles->Rlocation("com_github_v8app_v8app/libs/jsRuntime/tests/test-files/cache");
            ASSERT_FALSE(testFilesDir.empty());

            //test empty path
            EXPECT_FALSE(ScriptStartupDataManager::LoadScriptFile("", source));
            EXPECT_EQ(0, source.length());

            std::filesystem::path filePath = testFilesDir;
            filePath /= "notExist";

            //test file doesn't exist
            EXPECT_FALSE(ScriptStartupDataManager::LoadScriptFile(filePath, source));
            EXPECT_EQ(0, source.length());

            //test invalid extension
            filePath = testFilesDir;
            filePath /= "notExist.png";

            EXPECT_FALSE(ScriptStartupDataManager::LoadScriptFile(filePath, source));
            EXPECT_EQ(0, source.length());

            //test no extension finds .mjs file
            filePath = testFilesDir;
            filePath /= "script";

            EXPECT_TRUE(ScriptStartupDataManager::LoadScriptFile(filePath, source));
#ifdef V8APP_DEBUG
            EXPECT_EQ("//DO NOT EDIT ME Script.mjs", source);
#else
            EXPECT_EQ("//EMBEDDED TEST FILE Script.mjs", source);
#endif

            //test no extnesion finds .js extension
            filePath = testFilesDir;
            filePath /= "script2";

            EXPECT_TRUE(ScriptStartupDataManager::LoadScriptFile(filePath, source));
#ifdef V8APP_DEBUG
            EXPECT_EQ("//DO NOT EDIT ME Script2.js", source);
#else
            EXPECT_EQ("//EMBEDDED TEST FILE Script2.js", source);
#endif

            //test .mjs extension
            filePath = testFilesDir;
            filePath /= "script3.mjs";

            EXPECT_TRUE(ScriptStartupDataManager::LoadScriptFile(filePath, source));
#ifdef V8APP_DEBUG
            EXPECT_EQ("//DO NOT EDIT ME Script3.mjs", source);
#else
            EXPECT_EQ("//EMBEDDED TEST FILE Script3.mjs", source);
#endif

            //test .js extension
            filePath = testFilesDir;
            filePath /= "script4.js";

            EXPECT_TRUE(ScriptStartupDataManager::LoadScriptFile(filePath, source));
#ifdef V8APP_DEBUG
            EXPECT_EQ("//DO NOT EDIT ME Script4.js", source);
#else
            EXPECT_EQ("//EMBEDDED TEST FILE Script4.js", source);
#endif

#ifdef V8APP_DEBUG
            //test cache of file
            filePath = testFilesDir;
            filePath /= "cacheTest.mjs";

            //write a test file that is correct and shuld load
            ASSERT_TRUE(ScrtipStartupTestInternal::WriteTestFile(filePath, "CacheTest.mjs"));

            //load the file
            EXPECT_TRUE(ScriptStartupDataManager::LoadScriptFile(filePath, source));
            EXPECT_EQ("CacheTest.mjs", source);

            //now remove it and try to load from cache
            std::filesystem::remove(filePath);

            EXPECT_TRUE(ScriptStartupDataManager::LoadScriptFile(filePath, source));
            EXPECT_EQ("CacheTest.mjs", source);
#endif
        }

        TEST_F(ScriptStartupDataManTest, TestSetCacheFile)
        {
            std::string source;

            std::filesystem::path testFiles = m_RunFiles->Rlocation("com_github_v8app_v8app/libs/jsRuntime/tests/test-files/cache");
            ASSERT_FALSE(testFiles.empty());

            std::filesystem::path filePath = testFiles;
            filePath /= "scriptLoad.js";

            ScriptStartupDataManager::SetCachedFile(filePath, "SET CACHED FILE");
            EXPECT_TRUE(ScriptStartupDataManager::LoadScriptFile(filePath, source));

#ifdef V8APP_DEBUG
            EXPECT_EQ("SET CACHED FILE", source);
#else
            EXPECT_EQ("//EMBEDDED TEST FILE", source);
#endif
        }
    }
}