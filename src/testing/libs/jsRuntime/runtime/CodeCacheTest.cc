
// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <ostream>
#include <fstream>

#include "V8Fixture.h"

#include "Assets/TextAsset.h"
#include "Assets/BinaryAsset.h"
#include "Logging/Log.h"
#include "Logging/ILogSink.h"
#include "Utils/Format.h"

#include "TestLogSink.h"

#include "CodeCache.h"
#include "JSUtilities.h"

namespace v8App
{
    namespace JSRuntime
    {
        using CodeCacheTest = V8Fixture;

        namespace CodeCacheTestInternal
        {
            V8MaybeLocalModule UnresovledCallback(
                V8LocalContext inContet, V8LocalString inSpecifier,
                V8LocalFixedArray inImportAssertions, V8LocalModule inReferrer)
            {
                Log::LogMessage msg;
                msg.emplace(Log::MsgKey::Msg, Utils::format("Unresloved callback called"));
                LOG_ERROR(msg);
                return V8MaybeLocalModule();
            }

            int32_t ExecuteScript(V8Isolate *isolate, V8ScriptSource *source, bool useCache)
            {
                V8Isolate::Scope iScope(isolate);
                v8::HandleScope hScope(isolate);
                V8LocalContext context = v8::Context::New(isolate);
                v8::Context::Scope cScope(context);
                v8::ScriptCompiler::CompileOptions option = v8::ScriptCompiler::kNoCompileOptions;
                if (useCache)
                {
                    option = v8::ScriptCompiler::kConsumeCodeCache;
                }
                v8::TryCatch tryCatch(isolate);
                V8LocalModule module;
                EXPECT_TRUE(v8::ScriptCompiler::CompileModule(isolate, source, option).ToLocal(&module));
                if (tryCatch.HasCaught())
                {
                    std::string stack = JSUtilities::GetStackTrace(context, tryCatch);
                    std::cout << stack << std::endl;
                    return -1;
                }
                module->InstantiateModule(context, UnresovledCallback);
                if (tryCatch.HasCaught())
                {
                    std::string stack = JSUtilities::GetStackTrace(context, tryCatch);
                    std::cout << stack << std::endl;
                    return -1;
                }
                if (useCache)
                {
                    EXPECT_FALSE(source->GetCachedData()->rejected);
                }
                V8LocalValue value = module->Evaluate(context)
                                         .ToLocalChecked();
                V8LocalPromise promise(V8LocalPromise::Cast(value));
                EXPECT_EQ(promise->State(), v8::Promise::kFulfilled);
                if (tryCatch.HasCaught())
                {
                    std::string stack = JSUtilities::GetStackTrace(context, tryCatch);
                    std::cout << stack << std::endl;
                    return -1;
                }

                return context->Global()->Get(context, JSUtilities::StringToV8(isolate, "Result")).ToLocalChecked()->Int32Value(context).FromJust();
            }

            V8ScriptCachedData *GenerateCodeCache(V8Isolate *inIsolate, V8LocalContext inContet, V8ScriptSource *inSource)
            {
                V8LocalModule module = v8::ScriptCompiler::CompileModule(inIsolate, inSource).ToLocalChecked();
                module->InstantiateModule(inContet, UnresovledCallback);

                V8LocalUnboundModuleScript unbound = module->GetUnboundModuleScript();

                V8LocalValue result = module->Evaluate(inContet).ToLocalChecked();
                V8LocalPromise promise(V8LocalPromise::Cast(result));
                CHECK_EQ(promise->State(), v8::Promise::kFulfilled);

                return v8::ScriptCompiler::CreateCodeCache(unbound);
            }
        }

        class TestCodeCache : public CodeCache
        {
        public:
            TestCodeCache(JSAppSharedPtr inApp) : CodeCache(inApp) {}

            ScriptCacheInfo *TestGetCachedScript(std::string inFilePath) { return GetCachedScript(inFilePath); }
            ScriptCacheInfo *TestCreateCacheInfo(std::string inFilePath) { return CreateCacheInfo(inFilePath); }
            std::filesystem::path TestGenerateCachePath(std::filesystem::path inFileName) { return GenerateCachePath(inFileName); }

            bool TestReadScriptFile(std::string inFileName, CodeCache::ScriptCacheInfo *inInfo) { return ReadScriptFile(inFileName, inInfo); }
            bool TestWriteCacheDataToFile(std::filesystem::path inCacheFile, const uint8_t *inData, int inDataLength) { return WriteCacheDataToFile(inCacheFile, inData, inDataLength); }
            bool TestReadCachedDataFile(std::filesystem::path inCacheFile, CodeCache::ScriptCacheInfo *inInfo) { return ReadCachedDataFile(inCacheFile, inInfo); }
        };

        TEST_F(CodeCacheTest, LoadScriptFileSetCacheData)
        {
            TestUtils::TestLogSink *logSink = TestUtils::TestLogSink::GetGlobalSink();
            Log::Log::SetLogLevel(Log::LogLevel::Error);

            TestCodeCache codeCache(m_App);

            EXPECT_EQ(nullptr, codeCache.LoadScriptFile(std::filesystem::path(""), m_Isolate));
            Log::LogMessage expected = {
                {Log::MsgKey::Msg, Utils::format("Empty file name passed for a script file")},
                {Log::MsgKey::LogLevel, "Error"},
            };
            EXPECT_TRUE(logSink->ValidateMessage(expected, m_IgnoreKeys));

            EXPECT_EQ(nullptr, codeCache.LoadScriptFile(std::filesystem::path("test"), m_Isolate));
            expected = {
                {Log::MsgKey::Msg, Utils::format("Unsupported file extension passed, only .mjs, .js allowed. File: \"test\"")},
                {Log::MsgKey::LogLevel, "Error"},
            };
            EXPECT_TRUE(logSink->ValidateMessage(expected, m_IgnoreKeys));

            EXPECT_EQ(nullptr, codeCache.LoadScriptFile(std::filesystem::path("js/test.js"), m_Isolate));
            expected = {
                {Log::MsgKey::Msg, Utils::format("File does not exists. File: \"js/test.js\"")},
                {Log::MsgKey::LogLevel, "Error"},
            };
            EXPECT_TRUE(logSink->ValidateMessage(expected, m_IgnoreKeys));

            std::filesystem::path appRoot = m_Runtime->GetApp()->GetAppRoots()->GetAppRoot();
            std::filesystem::path testPath = appRoot / std::filesystem::path("resources/empty.js");

            EXPECT_EQ(nullptr, codeCache.LoadScriptFile(std::filesystem::path(testPath), m_Isolate));
            expected = {
                {Log::MsgKey::Msg, Utils::format("Script file is not in the js or modules directories. File: {}", testPath)},
                {Log::MsgKey::LogLevel, "Error"},
            };

            EXPECT_TRUE(logSink->ValidateMessage(expected, m_IgnoreKeys));

            testPath = appRoot / std::filesystem::path("js/cacheTest.js");
            Assets::TextAsset srcFile(testPath);
            srcFile.SetContent("function f(){return 2;}(function() { globalThis.Result=f(); })()");
            ASSERT_TRUE(srcFile.WriteAsset());

            V8Isolate::Scope iScope(m_Isolate);
            v8::HandleScope hScope(m_Isolate);
            V8LocalContext context = m_Context->GetLocalContext();
            v8::Context::Scope cScope(context);

            V8ScriptSourceUniquePtr source = codeCache.LoadScriptFile(testPath, m_Isolate);
            ASSERT_NE(nullptr, source);
            EXPECT_EQ(nullptr, source->GetCachedData());
            EXPECT_FALSE(codeCache.HasCodeCache(testPath));
            EXPECT_EQ(2, CodeCacheTestInternal::ExecuteScript(m_Isolate, source.get(), false));

            source = codeCache.LoadScriptFile(testPath, m_Isolate);
            V8ScriptCachedData *cache = CodeCacheTestInternal::GenerateCodeCache(m_Isolate, context, source.get());
            ASSERT_NE(nullptr, cache);

            // test the setting of the cache
            EXPECT_FALSE(codeCache.SetCodeCache(std::filesystem::path("test"), nullptr));
            expected = {
                {Log::MsgKey::Msg, Utils::format("SetCodeCache passed a nullptr for cached data")},
                {Log::MsgKey::LogLevel, "Error"},
            };
            EXPECT_TRUE(logSink->ValidateMessage(expected, m_IgnoreKeys));

            EXPECT_FALSE(codeCache.SetCodeCache(std::filesystem::path("js/test"), cache));
            expected = {
                {Log::MsgKey::Msg, Utils::format("File doesn't exist: \"js/test\"")},
                {Log::MsgKey::LogLevel, "Error"},
            };
            EXPECT_TRUE(logSink->ValidateMessage(expected, m_IgnoreKeys));

            ASSERT_TRUE(codeCache.SetCodeCache(testPath, cache));
            EXPECT_TRUE(codeCache.HasCodeCache(testPath));

            source = codeCache.LoadScriptFile(testPath, m_Isolate);
            ASSERT_NE(nullptr, source);
            ASSERT_NE(nullptr, source->GetCachedData());

            EXPECT_EQ(2, CodeCacheTestInternal::ExecuteScript(m_Isolate, source.get(), true));

            // use a second time to make sure the buffer wasn't deleted
            source = codeCache.LoadScriptFile(testPath, m_Isolate);
            ASSERT_NE(nullptr, source);
            ASSERT_NE(nullptr, source->GetCachedData());

            EXPECT_EQ(2, CodeCacheTestInternal::ExecuteScript(m_Isolate, source.get(), true));
            {
                // test loads the cache data from file.
                CodeCache codeCache2(m_App);

                V8ScriptSourceUniquePtr source2 = codeCache2.LoadScriptFile(testPath, m_Isolate);
                EXPECT_NE(nullptr, source2);
                EXPECT_NE(nullptr, source2->GetCachedData());
                EXPECT_EQ(2, CodeCacheTestInternal::ExecuteScript(m_Isolate, source2.get(), true));
            }

            // js file changed
            srcFile.SetContent("function f(){return 4;}(function() { globalThis.Result=f(); })()");
            ASSERT_TRUE(srcFile.WriteAsset());
            source = codeCache.LoadScriptFile(testPath, m_Isolate);
            ASSERT_NE(nullptr, source);
            EXPECT_EQ(nullptr, source->GetCachedData());

            EXPECT_EQ(4, CodeCacheTestInternal::ExecuteScript(m_Isolate, source.get(), false));
        }

        TEST_F(CodeCacheTest, CreateCacheInfo)
        {
            TestUtils::TestLogSink *logSink = TestUtils::TestLogSink::GetGlobalSink();
            Log::Log::SetLogLevel(Log::LogLevel::Error);

            TestCodeCache codeCache(m_App);

            // bad file path
            EXPECT_EQ(nullptr, codeCache.TestCreateCacheInfo("test.js"));
            Log::LogMessage expected = {
                {Log::MsgKey::Msg, Utils::format("Script file is not in the js or modules directories. File: \"test.js\"")},
                {Log::MsgKey::LogLevel, "Error"},
            };
            EXPECT_TRUE(logSink->ValidateMessage(expected, m_IgnoreKeys));

            // file doesn't exist
            std::filesystem::path appRoot = m_Runtime->GetApp()->GetAppRoots()->GetAppRoot();
            std::filesystem::path testPath = appRoot / std::filesystem::path("js/cacheTests.js");
            EXPECT_EQ(nullptr, codeCache.TestCreateCacheInfo(testPath));
            expected = {
                {Log::MsgKey::Msg, Utils::format("File doesn't exist: {}", testPath)},
                {Log::MsgKey::LogLevel, "Error"},
            };
            EXPECT_TRUE(logSink->ValidateMessage(expected, m_IgnoreKeys));

            // file exists info created
            testPath = appRoot / std::filesystem::path("js/cacheTest.js");
            CodeCache::ScriptCacheInfo *info = codeCache.TestCreateCacheInfo(testPath);
            ASSERT_NE(nullptr, info);
            EXPECT_NE(nullptr, codeCache.TestGetCachedScript(testPath));

            std::string sourceStr = "function f(){return 4;}(function() { globalThis.Result=f(); })()";
            Assets::TextAsset srcFile(testPath);
            srcFile.SetContent(sourceStr);
            ASSERT_TRUE(srcFile.WriteAsset());

            EXPECT_EQ(testPath, info->m_FilePath);
            EXPECT_EQ(codeCache.TestGenerateCachePath(testPath), info->m_CachedFilePath);
            EXPECT_EQ(nullptr, info->m_Compiled);
            EXPECT_EQ(0, info->m_CompiledLength);
            EXPECT_EQ(sourceStr, info->m_SourceStr);

            // fail to insert
            EXPECT_EQ(nullptr, codeCache.TestCreateCacheInfo(testPath));
            expected = {
                {Log::MsgKey::Msg, Utils::format("Failed to insert the cache info")},
                {Log::MsgKey::LogLevel, "Error"},
            };
            EXPECT_TRUE(logSink->ValidateMessage(expected, m_IgnoreKeys));
        }

        TEST_F(CodeCacheTest, GenerateCachePath)
        {
            TestUtils::TestLogSink *logSink = TestUtils::TestLogSink::GetGlobalSink();
            Log::Log::SetLogLevel(Log::LogLevel::Error);

            TestCodeCache codeCache(m_App);
            std::filesystem::path appRoot = m_Runtime->GetApp()->GetAppRoots()->GetAppRoot();

            std::filesystem::path testPath = appRoot / std::filesystem::path("test/test.js");
            EXPECT_TRUE(codeCache.TestGenerateCachePath(testPath).empty());
            Log::LogMessage expected = {
                {Log::MsgKey::Msg, Utils::format("Script file is not in the js or modules directories. File: {}", testPath)},
                {Log::MsgKey::LogLevel, "Error"},
            };
            EXPECT_TRUE(logSink->ValidateMessage(expected, m_IgnoreKeys));

            testPath = appRoot / std::filesystem::path("js/test.js");
            EXPECT_EQ(appRoot / std::filesystem::path(".code_cache/js/test.jscc"), codeCache.TestGenerateCachePath(testPath));

            testPath = appRoot / std::filesystem::path("modules/test.js");
            EXPECT_EQ(appRoot / std::filesystem::path(".code_cache/modules/test.jscc"), codeCache.TestGenerateCachePath(testPath));
        }

        TEST_F(CodeCacheTest, ReadScriptFile)
        {
            TestUtils::TestLogSink *logSink = TestUtils::TestLogSink::GetGlobalSink();
            Log::Log::SetLogLevel(Log::LogLevel::Error);

            TestCodeCache codeCache(m_App);
            std::filesystem::path appRoot = m_Runtime->GetApp()->GetAppRoots()->GetAppRoot();

            std::filesystem::path testPath = appRoot / std::filesystem::path("js/cacheTest.js");

            EXPECT_FALSE(codeCache.TestReadScriptFile("test.js", nullptr));
            Log::LogMessage expected = {
                {Log::MsgKey::Msg, Utils::format("RreadScriptFile passed a nullptr for info.")},
                {Log::MsgKey::LogLevel, "Error"},
            };
            EXPECT_TRUE(logSink->ValidateMessage(expected, m_IgnoreKeys));

            CodeCache::ScriptCacheInfo info;

            EXPECT_FALSE(codeCache.TestReadScriptFile("test.js", &info));
            expected = {
                {Log::MsgKey::Msg, Utils::format("File doesn't exist: \"test.js\"")},
                {Log::MsgKey::LogLevel, "Error"},
            };
            EXPECT_TRUE(logSink->ValidateMessage(expected, m_IgnoreKeys));

            std::string sourceStr = "function f(){return 4;}(function() { globalThis.Result=f(); })()";
            Assets::TextAsset srcFile(testPath);
            srcFile.SetContent(sourceStr);
            ASSERT_TRUE(srcFile.WriteAsset());

            EXPECT_TRUE(codeCache.TestReadScriptFile(testPath, &info));
            EXPECT_EQ(sourceStr, info.m_SourceStr);
        }

        TEST_F(CodeCacheTest, ReadWriteCachedDataFile)
        {
            TestUtils::TestLogSink *logSink = TestUtils::TestLogSink::GetGlobalSink();
            Log::Log::SetLogLevel(Log::LogLevel::Error);

            TestCodeCache codeCache(m_App);

            uint8_t cacheData[] = {'f', 'g', 'h'};
            int dataLength = 3;

            EXPECT_FALSE(codeCache.TestWriteCacheDataToFile(std::filesystem::path("test.js"), nullptr, 0));
            Log::LogMessage expected = {
                {Log::MsgKey::Msg, Utils::format("WriteCacheDataToFile passed a nllptr for data")},
                {Log::MsgKey::LogLevel, "Error"},
            };
            EXPECT_TRUE(logSink->ValidateMessage(expected, m_IgnoreKeys));

            std::filesystem::path appRoot = m_Runtime->GetApp()->GetAppRoots()->GetAppRoot();
            std::filesystem::path testPath = appRoot / std::filesystem::path("js/readWriteTest.js");
            std::filesystem::path testCachePath = codeCache.TestGenerateCachePath(testPath);

            EXPECT_TRUE(codeCache.TestWriteCacheDataToFile(testCachePath, cacheData, dataLength));

            // Now read it back in
            CodeCache::ScriptCacheInfo info;
            EXPECT_FALSE(codeCache.TestReadCachedDataFile(std::filesystem::path("test.js"), nullptr));
            expected = {
                {Log::MsgKey::Msg, Utils::format("ReadCachedDataFile passed a nllptr for info")},
                {Log::MsgKey::LogLevel, "Error"},
            };
            EXPECT_TRUE(logSink->ValidateMessage(expected, m_IgnoreKeys));

            EXPECT_FALSE(codeCache.TestReadCachedDataFile(std::filesystem::path("test.js"), &info));
            expected = {
                {Log::MsgKey::Msg, Utils::format("Cached data file doesn't exist: \"test.js\"")},
                {Log::MsgKey::LogLevel, "Error"},
            };
            EXPECT_TRUE(logSink->ValidateMessage(expected, m_IgnoreKeys));

            EXPECT_TRUE(codeCache.TestReadCachedDataFile(testCachePath, &info));
            EXPECT_EQ(dataLength, info.m_CompiledLength);
            for (int x = 0; x < dataLength; x++)
            {
                EXPECT_EQ(cacheData[x], info.m_Compiled[x]);
            }
        }
    }
}