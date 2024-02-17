// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <iostream>
#include <string>
#include <fstream>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "V8Fixture.h"

#include "Assets/TextAsset.h"

#include "JSUtilities.h"
#include "JSContextModules.h"

namespace v8App
{
    namespace JSRuntime
    {
        using JSContextModulesTest = V8Fixture;

        class TestJSContextModules : public JSContextModules
        {
        public:
            TestJSContextModules(JSContextSharedPtr inContext) : JSContextModules(inContext) {}

            JSContextSharedPtr GetJSContext() { return m_Context; }
            void SetJSContext(JSContextSharedPtr inContext) { m_Context = inContext; }

            JSModuleInfoSharedPtr TestBuildModuleInfo(JSModuleInfo::AssertionInfo &inAssertionInfo, const std::filesystem::path &inImportPath, const std::filesystem::path &inCurrentModPath)
            {
                return BuildModuleInfo(inAssertionInfo, inImportPath, inCurrentModPath);
            }

            JSModuleInfoSharedPtr TestLoadModuleTree(JSContextSharedPtr inContext, JSModuleInfoSharedPtr inModInfo) { return LoadModuleTree(inContext, inModInfo); }

            bool TestAddModule(const JSModuleInfoSharedPtr &inModule, std::string inFileName, JSModuleInfo::ModuleType inModuleType) { return AddModule(inModule, inFileName, inModuleType); }
            JSModuleInfoSharedPtr TestGetModuleInfoByModule(V8LocalModule inModule, JSModuleInfo::ModuleType inType) { return GetModuleInfoByModule(inModule, inType); }
        };

        TEST_F(JSContextModulesTest, ConstrcutorGetIsolate)
        {
            TestJSContextModules modules(m_Context);

            EXPECT_EQ(m_Context, modules.GetJSContext());
            EXPECT_EQ(m_Isolate, modules.GetIsolate());
        }

        TEST_F(JSContextModulesTest, BuildModuleInfo)
        {
            TestJSContextModules jsModules(m_Context);
            JSModuleInfoSharedPtr info;
            JSModuleInfo::AssertionInfo assertInfo;
            std::filesystem::path rootPath = m_App->GetAppRoots()->GetAppRoot();

            V8Isolate::Scope iScope(m_Isolate);
            v8::HandleScope hScope(m_Isolate);
            v8::TryCatch tryCatch(m_Isolate);
            v8::Context::Scope cScope(m_Context->GetLocalContext());

            std::filesystem::path testPath("testModule.js");
            // test no module for assert
            assertInfo.m_Module = "NoModuleTest";
            info = jsModules.TestBuildModuleInfo(assertInfo, testPath, rootPath);
            EXPECT_EQ(info, nullptr);
            EXPECT_TRUE(tryCatch.HasCaught());
            EXPECT_EQ(JSUtilities::V8ToString(m_Isolate, tryCatch.Message()->Get()),
                      Utils::format("Uncaught SyntaxError: Failed to find asserted module: {}, ImportPath: {}", assertInfo.m_Module, testPath));

            // JS Folder
            tryCatch.Reset();
            // module assertion in js path
            testPath = std::filesystem::path("%JS%/testModule.js");
            info = jsModules.TestBuildModuleInfo(assertInfo, testPath, rootPath);
            EXPECT_EQ(info, nullptr);
            EXPECT_TRUE(tryCatch.HasCaught());
            EXPECT_EQ(JSUtilities::V8ToString(m_Isolate, tryCatch.Message()->Get()),
                      Utils::format("Uncaught SyntaxError: Can not use a module assetion in the {} root, ImportPath: {}", Assets::c_RootJS, testPath));

            tryCatch.Reset();
            // extension doesn't match allowed type
            testPath = std::filesystem::path("%JS%/testModule.txt");
            assertInfo.m_Module = "";
            assertInfo.m_Type = JSModuleInfo::ModuleType::kJavascript;
            assertInfo.m_TypeString = "js";
            info = jsModules.TestBuildModuleInfo(assertInfo, testPath, rootPath);
            EXPECT_EQ(info, nullptr);
            EXPECT_TRUE(tryCatch.HasCaught());
            EXPECT_EQ(JSUtilities::V8ToString(m_Isolate, tryCatch.Message()->Get()),
                      Utils::format("Uncaught TypeError: File type doesn't match specified type {}. Importpath: {}", assertInfo.m_TypeString, testPath));

            tryCatch.Reset();
            // non module info returned
            testPath = std::filesystem::path("%JS%/testModule.js");
            info = jsModules.TestBuildModuleInfo(assertInfo, testPath, rootPath);
            ASSERT_NE(info, nullptr);
            EXPECT_FALSE(tryCatch.HasCaught());
            EXPECT_EQ(info->GetName(), "testModule");
            EXPECT_EQ(info->GetModulePath().string(), (rootPath / std::filesystem::path("js/testModule.js")).string());

            tryCatch.Reset();
            // non module info returned with abs path
            testPath = std::filesystem::path("/js/testModule.js");
            info = jsModules.TestBuildModuleInfo(assertInfo, testPath, rootPath);
            ASSERT_NE(info, nullptr);
            EXPECT_FALSE(tryCatch.HasCaught());
            EXPECT_EQ(info->GetName(), "testModule");
            EXPECT_EQ(info->GetModulePath().string(), (rootPath / std::filesystem::path("js/testModule.js")).string());

            // Resources Folder
            tryCatch.Reset();
            // module assertion in resources path
            testPath = std::filesystem::path("%RESOURCES%/testModule.txt");
            assertInfo.m_Module = "tetsMofule";
            info = jsModules.TestBuildModuleInfo(assertInfo, testPath, rootPath);
            EXPECT_EQ(info, nullptr);
            EXPECT_TRUE(tryCatch.HasCaught());
            EXPECT_EQ(JSUtilities::V8ToString(m_Isolate, tryCatch.Message()->Get()),
                      Utils::format("Uncaught SyntaxError: Can not use a module assetion in the {} root, ImportPath: {}", Assets::c_RootResource, testPath));

            // js or mjs in resources
            testPath = std::filesystem::path("%RESOURCES%/testModule.js");
            assertInfo.m_Module = "";
            info = jsModules.TestBuildModuleInfo(assertInfo, testPath, rootPath);
            EXPECT_EQ(info, nullptr);
            EXPECT_TRUE(tryCatch.HasCaught());
            EXPECT_EQ(JSUtilities::V8ToString(m_Isolate, tryCatch.Message()->Get()),
                      Utils::format("Uncaught SyntaxError: Files ending in .js or .mjs can not be in resources, ImportPath: {}", testPath));

            testPath = std::filesystem::path("%RESOURCES%/testModule.mjs");
            info = jsModules.TestBuildModuleInfo(assertInfo, testPath, rootPath);
            EXPECT_EQ(info, nullptr);
            EXPECT_TRUE(tryCatch.HasCaught());
            EXPECT_EQ(JSUtilities::V8ToString(m_Isolate, tryCatch.Message()->Get()),
                      Utils::format("Uncaught SyntaxError: Files ending in .js or .mjs can not be in resources, ImportPath: {}", testPath));

            tryCatch.Reset();
            // extension doesn't match allowed type
            testPath = std::filesystem::path("%RESOURCES%/testModule.txt");
            assertInfo.m_Module = "";
            assertInfo.m_Type = JSModuleInfo::ModuleType::kJSON;
            assertInfo.m_TypeString = "json";
            info = jsModules.TestBuildModuleInfo(assertInfo, testPath, rootPath);
            EXPECT_EQ(info, nullptr);
            EXPECT_TRUE(tryCatch.HasCaught());
            EXPECT_EQ(JSUtilities::V8ToString(m_Isolate, tryCatch.Message()->Get()),
                      Utils::format("Uncaught TypeError: File type doesn't match specified type json. Importpath: {}", testPath));

            tryCatch.Reset();
            // non module info returned
            testPath = std::filesystem::path("%RESOURCES%/testModule.json");
            info = jsModules.TestBuildModuleInfo(assertInfo, testPath, rootPath);
            ASSERT_NE(info, nullptr);
            EXPECT_FALSE(tryCatch.HasCaught());
            EXPECT_EQ(info->GetName(), "testModule");
            EXPECT_EQ(info->GetModulePath().string(), (rootPath / std::filesystem::path("resources/testModule.json")).string());

            // Modules Folder
            tryCatch.Reset();
            // extension doesn't match allowed type
            testPath = std::filesystem::path("%MODULES%/builModInfo/test.json");
            assertInfo.m_Module = "";
            assertInfo.m_Type = JSModuleInfo::ModuleType::kJavascript;
            assertInfo.m_TypeString = "js";
            info = jsModules.TestBuildModuleInfo(assertInfo, testPath, rootPath);
            EXPECT_EQ(info, nullptr);
            EXPECT_TRUE(tryCatch.HasCaught());
            EXPECT_EQ(JSUtilities::V8ToString(m_Isolate, tryCatch.Message()->Get()),
                      Utils::format("Uncaught TypeError: File type doesn't match specified type {}. Importpath: {}", assertInfo.m_TypeString, testPath));

            // module asserted but module not the module
            tryCatch.Reset();
            testPath = std::filesystem::path("%MODULES%/NotBuilModInfo/test.js");
            assertInfo.m_Module = "buildModInfo";
            info = jsModules.TestBuildModuleInfo(assertInfo, testPath, rootPath);
            EXPECT_EQ(info, nullptr);
            EXPECT_TRUE(tryCatch.HasCaught());
            EXPECT_EQ(JSUtilities::V8ToString(m_Isolate, tryCatch.Message()->Get()),
                      Utils::format("Uncaught SyntaxError: Import path was not in asserted module's path. Module:{}, ImportPath: {}", assertInfo.m_Module, testPath));

            // No module version
            tryCatch.Reset();
            testPath = std::filesystem::path("%MODULES%/NoModVersion/test.js");
            assertInfo.m_Module = "";
            info = jsModules.TestBuildModuleInfo(assertInfo, testPath, rootPath);
            EXPECT_EQ(info, nullptr);
            EXPECT_TRUE(tryCatch.HasCaught());
            EXPECT_EQ(JSUtilities::V8ToString(m_Isolate, tryCatch.Message()->Get()),
                      Utils::format("Uncaught SyntaxError: Failed to find module's version: NoModVersion, ImportPath: {}", testPath));

            // test relative module path to specified root
            std::filesystem::path modPath = m_App->GetAppRoots()->FindModuleVersionRootPath("buildModInfo/1.0.0");
            ASSERT_NE("", modPath.string());

            tryCatch.Reset();
            testPath = std::filesystem::path("test.js");
            assertInfo.m_Module = "";
            info = jsModules.TestBuildModuleInfo(assertInfo, testPath, modPath);
            ASSERT_NE(info, nullptr);
            EXPECT_FALSE(tryCatch.HasCaught());
            EXPECT_EQ(info->GetName(), "buildModInfo");
            EXPECT_EQ(info->GetVersion(), "1.0.0");
            EXPECT_EQ(info->GetModulePath().string(), (rootPath / std::filesystem::path("modules/buildModInfo/1.0.0/test.js")).string());

            // test relative module to asserted module
            tryCatch.Reset();
            testPath = std::filesystem::path("test.js");
            assertInfo.m_Module = "buildModInfo";
            info = jsModules.TestBuildModuleInfo(assertInfo, testPath, modPath);
            ASSERT_NE(info, nullptr);
            EXPECT_FALSE(tryCatch.HasCaught());
            EXPECT_EQ(info->GetName(), "buildModInfo");
            EXPECT_EQ(info->GetVersion(), "1.0.0");
            EXPECT_EQ(info->GetModulePath().string(), (rootPath / std::filesystem::path("modules/buildModInfo/1.0.0/test.js")).string());

            // test absolute path to module
            tryCatch.Reset();
            testPath = std::filesystem::path("/modules/buildModInfo/test.js");
            info = jsModules.TestBuildModuleInfo(assertInfo, testPath, rootPath);
            ASSERT_NE(info, nullptr);
            EXPECT_FALSE(tryCatch.HasCaught());
            EXPECT_EQ(info->GetName(), "buildModInfo");
            EXPECT_EQ(info->GetVersion(), "1.0.0");
            EXPECT_EQ(info->GetModulePath().string(), (rootPath / std::filesystem::path("modules/buildModInfo/1.0.0/test.js")).string());
        }

        TEST_F(JSContextModulesTest, LoadModuleTreeJSNoImports)
        {
            TestUtils::TestLogSink *logSink = TestUtils::TestLogSink::GetGlobalSink();
            Log::Log::SetLogLevel(Log::LogLevel::Error);

            TestJSContextModules jsModules(m_Context);

            V8Isolate::Scope iScope(m_Isolate);
            v8::HandleScope hScope(m_Isolate);
            v8::TryCatch tryCatch(m_Isolate);
            V8LocalContext context = m_Context->GetLocalContext();
            v8::Context::Scope cScope(context);

            std::filesystem::path appRoot = m_App->GetAppRoots()->GetAppRoot();
            JSModuleInfoSharedPtr info = std::make_shared<JSModuleInfo>(m_Context);
            JSModuleInfo::AssertionInfo assertInfo;
            assertInfo.m_Type = JSModuleInfo::ModuleType::kJavascript;
            assertInfo.m_TypeString = "js";
            info->SetAssertionInfo(assertInfo);

            // failed to load script
            info->SetPath(appRoot / std::filesystem::path("js/NotExists.js"));

            EXPECT_EQ(nullptr, jsModules.TestLoadModuleTree(m_Context, info));
            ASSERT_TRUE(tryCatch.HasCaught());
            EXPECT_EQ(JSUtilities::V8ToString(m_Isolate, tryCatch.Message()->Get()),
                      Utils::format("Uncaught Error: Failed to load the module file: {}", info->GetModulePath()));
            Log::LogMessage expected;
            expected = {
                {Log::MsgKey::Msg, Utils::format("Failed to load the module file: {}", info->GetModulePath())},
                {Log::MsgKey::LogLevel, "Error"},
            };
            EXPECT_TRUE(logSink->ValidateMessage(expected, m_IgnoreKeys, 1));

            // compile error
            tryCatch.Reset();
            info->SetPath(appRoot / std::filesystem::path("js/compileError.js"));
            ASSERT_EQ(nullptr, jsModules.TestLoadModuleTree(m_Context, info));
            ASSERT_TRUE(tryCatch.HasCaught());
            EXPECT_EQ(JSUtilities::V8ToString(m_Isolate, tryCatch.Message()->Get()),
                      "Uncaught SyntaxError: Unexpected token '-'");
            expected = {
                {Log::MsgKey::Msg, Utils::format("Uncaught SyntaxError: Unexpected token '-'\n{}:1:SyntaxError: Unexpected token '-'\n", info->GetModulePath().string())},
                {Log::MsgKey::LogLevel, "Error"},
            };
            EXPECT_TRUE(logSink->ValidateMessage(expected, m_IgnoreKeys));

            // loads script
            tryCatch.Reset();
            info->SetPath(appRoot / std::filesystem::path("js/loadModule.js"));
            JSModuleInfoSharedPtr moduleInfo = jsModules.TestLoadModuleTree(m_Context, info);
            EXPECT_FALSE(tryCatch.HasCaught());
            EXPECT_FALSE(moduleInfo->GetLocalModule().IsEmpty());

            // the module is added to the jsModules attached to the context and not our test one
            JSModuleInfoSharedPtr addInfo = m_Context->GetJSModules()->GetModuleBySpecifier(info->GetModulePath());
            ASSERT_NE(nullptr, addInfo);
            ASSERT_EQ(addInfo.get(), info.get());

            // load the same module again should error
            tryCatch.Reset();
            info->SetPath(appRoot / std::filesystem::path("js/loadModule.js"));
            moduleInfo = jsModules.TestLoadModuleTree(m_Context, info);
            ASSERT_EQ(nullptr, moduleInfo);
            EXPECT_FALSE(tryCatch.HasCaught());
            expected = {
                {Log::MsgKey::Msg, Utils::format("Failed to add module into map. File: {}", info->GetModulePath())},
                {Log::MsgKey::LogLevel, "Error"},
            };
            EXPECT_TRUE(logSink->ValidateMessage(expected, m_IgnoreKeys));
        }

        TEST_F(JSContextModulesTest, LoadModuleTreeJSON)
        {
            TestUtils::TestLogSink *logSink = TestUtils::TestLogSink::GetGlobalSink();
            Log::Log::SetLogLevel(Log::LogLevel::Error);

            TestJSContextModules jsModules(m_Context);

            V8Isolate::Scope iScope(m_Isolate);
            v8::HandleScope hScope(m_Isolate);
            v8::TryCatch tryCatch(m_Isolate);
            V8LocalContext context = m_Context->GetLocalContext();
            v8::Context::Scope cScope(context);

            std::filesystem::path appRoot = m_App->GetAppRoots()->GetAppRoot();
            JSModuleInfoSharedPtr info = std::make_shared<JSModuleInfo>(m_Context);
            JSModuleInfo::AssertionInfo assertInfo;
            assertInfo.m_Type = JSModuleInfo::ModuleType::kJSON;
            assertInfo.m_TypeString = "json";
            info->SetAssertionInfo(assertInfo);

            // failed to load the json
            info->SetPath(appRoot / std::filesystem::path("js/NotExists.json"));

            JSModuleInfoSharedPtr moduleInfo =  jsModules.TestLoadModuleTree(m_Context, info);
            EXPECT_EQ(nullptr, moduleInfo);
            EXPECT_TRUE(tryCatch.HasCaught());
            EXPECT_EQ(JSUtilities::V8ToString(m_Isolate, tryCatch.Message()->Get()),
                      Utils::format("Uncaught Error: Failed to load the module file: {}", info->GetModulePath()));
            Log::LogMessage expected;
            expected = {
                {Log::MsgKey::Msg, Utils::format("Failed to load the module file: {}", info->GetModulePath())},
                {Log::MsgKey::LogLevel, "Error"},
            };
            EXPECT_TRUE(logSink->ValidateMessage(expected, m_IgnoreKeys, 1));

            // failed to parse json file
            info->SetPath(appRoot / std::filesystem::path("resources/ParseError.json"));

            moduleInfo = jsModules.TestLoadModuleTree(m_Context, info);
            EXPECT_EQ(nullptr, moduleInfo);
            EXPECT_TRUE(tryCatch.HasCaught());
            EXPECT_EQ(JSUtilities::V8ToString(m_Isolate, tryCatch.Message()->Get()),
                      "Uncaught SyntaxError: Unexpected token 'e', \"{\"test\":test}\" is not valid JSON");
            expected = {
                {Log::MsgKey::Msg, Utils::format("Uncaught SyntaxError: Unexpected token 'e', \"{\"test\":test}\" is not valid JSON\n{}:1:SyntaxError: Unexpected token 'e', \"{\"test\":test}\" is not valid JSON\n", info->GetModulePath().string())},
                {Log::MsgKey::LogLevel, "Error"},
            };
            EXPECT_TRUE(logSink->ValidateMessage(expected, m_IgnoreKeys));

            // load the json module
            tryCatch.Reset();
            info->SetPath(appRoot / std::filesystem::path("resources/loadModule.json"));
            moduleInfo = jsModules.TestLoadModuleTree(m_Context, info);
            ASSERT_NE(nullptr, moduleInfo);
            EXPECT_FALSE(tryCatch.HasCaught());
            EXPECT_FALSE(moduleInfo->GetLocalModule().IsEmpty());

            JSModuleInfoSharedPtr addInfo = m_Context->GetJSModules()->GetModuleBySpecifier(info->GetModulePath());
            ASSERT_NE(nullptr, addInfo);
            ASSERT_EQ(addInfo.get(), info.get());

            // load the same module again should error
            tryCatch.Reset();
            info->SetPath(appRoot / std::filesystem::path("resources/loadModule.json"));
            moduleInfo = jsModules.TestLoadModuleTree(m_Context, info);
            ASSERT_EQ(nullptr, moduleInfo);
            EXPECT_FALSE(tryCatch.HasCaught());
            expected = {
                {Log::MsgKey::Msg, Utils::format("Failed to add module into map. File: {}", info->GetModulePath())},
                {Log::MsgKey::LogLevel, "Error"},
            };
            EXPECT_TRUE(logSink->ValidateMessage(expected, m_IgnoreKeys));
        }

        TEST_F(JSContextModulesTest, LoadModuleTreeJSImports)
        {
            TestUtils::TestLogSink *logSink = TestUtils::TestLogSink::GetGlobalSink();
            Log::Log::SetLogLevel(Log::LogLevel::Error);

            TestJSContextModules jsModules(m_Context);

            V8Isolate::Scope iScope(m_Isolate);
            v8::HandleScope hScope(m_Isolate);
            v8::TryCatch tryCatch(m_Isolate);
            V8LocalContext context = m_Context->GetLocalContext();
            v8::Context::Scope cScope(context);

            std::filesystem::path appRoot = m_App->GetAppRoots()->GetAppRoot();
            JSModuleInfoSharedPtr info = std::make_shared<JSModuleInfo>(m_Context);
            JSModuleInfo::AssertionInfo assertInfo;
            assertInfo.m_Type = JSModuleInfo::ModuleType::kJavascript;
            assertInfo.m_TypeString = "js";
            info->SetAssertionInfo(assertInfo);

            info->SetPath(appRoot / std::filesystem::path("js/loadModuleImport.mjs"));
            info->SetName("loadModuleImport");

            info = jsModules.TestLoadModuleTree(m_Context, info);
            ASSERT_FALSE(info->GetLocalModule().IsEmpty());
        }

        TEST_F(JSContextModulesTest, LoadModuleTreeJSImportDynamic)
        {
            TestUtils::TestLogSink *logSink = TestUtils::TestLogSink::GetGlobalSink();
            Log::Log::SetLogLevel(Log::LogLevel::Error);

            TestJSContextModules jsModules(m_Context);

            V8Isolate::Scope iScope(m_Isolate);
            v8::HandleScope hScope(m_Isolate);
            v8::TryCatch tryCatch(m_Isolate);
            V8LocalContext context = m_Context->GetLocalContext();
            v8::Context::Scope cScope(context);

            std::filesystem::path appRoot = m_App->GetAppRoots()->GetAppRoot();
            JSModuleInfoSharedPtr info = std::make_shared<JSModuleInfo>(m_Context);
            JSModuleInfo::AssertionInfo assertInfo;
            assertInfo.m_Type = JSModuleInfo::ModuleType::kJavascript;
            assertInfo.m_TypeString = "js";
            info->SetAssertionInfo(assertInfo);

            info->SetPath(appRoot / std::filesystem::path("js/importModuleDynamic.js"));

            info = jsModules.TestLoadModuleTree(m_Context, info);
            ASSERT_NE(nullptr, info);
        }

        TEST_F(JSContextModulesTest, ImportAssertions)
        {
            TestUtils::TestLogSink *logSink = TestUtils::TestLogSink::GetGlobalSink();
            Log::Log::SetLogLevel(Log::LogLevel::Warn);

            TestJSContextModules jsModules(m_Context);

            V8Isolate::Scope iScope(m_Isolate);
            v8::HandleScope hScope(m_Isolate);
            v8::TryCatch tryCatch(m_Isolate);
            V8LocalContext context = m_Context->GetLocalContext();
            v8::Context::Scope cScope(context);

            std::filesystem::path appRoot = m_App->GetAppRoots()->GetAppRoot();
            JSModuleInfoSharedPtr info = std::make_shared<JSModuleInfo>(m_Context);
            JSModuleInfo::AssertionInfo assertInfo;
            assertInfo.m_Type = JSModuleInfo::ModuleType::kJavascript;
            assertInfo.m_TypeString = "js";
            info->SetAssertionInfo(assertInfo);

            // unknown asssertion
            info->SetPath(appRoot / std::filesystem::path("js/importAssertions/assertUnknown.js"));
            JSModuleInfoSharedPtr moduleInfo = jsModules.TestLoadModuleTree(m_Context, info);
            ASSERT_NE(nullptr, moduleInfo);
            EXPECT_FALSE(moduleInfo->GetLocalModule().IsEmpty());

            Log::LogMessage expected = {
                {Log::MsgKey::Msg, "Unknown assertion: unknown"},
                {Log::MsgKey::LogLevel, "Warn"},
            };
            EXPECT_TRUE(logSink->ValidateMessage(expected, m_IgnoreKeys));

            // modules assertions
            info->SetPath(appRoot / std::filesystem::path("js/importAssertions/assertModule.js"));
            moduleInfo = jsModules.TestLoadModuleTree(m_Context, info);
            ASSERT_EQ(nullptr, moduleInfo);
            EXPECT_TRUE(tryCatch.HasCaught());
            EXPECT_EQ(JSUtilities::V8ToString(m_Isolate, tryCatch.Message()->Get()),
                      "Uncaught SyntaxError: Failed to find asserted module: test, ImportPath: \"./importFile.js\"");

            // type json assertion
            tryCatch.Reset();
            info->SetPath(appRoot / std::filesystem::path("js/importAssertions/assertJSONType.js"));
            moduleInfo = jsModules.TestLoadModuleTree(m_Context, info);
            ASSERT_EQ(nullptr, moduleInfo);
            EXPECT_TRUE(tryCatch.HasCaught());
            EXPECT_EQ(JSUtilities::V8ToString(m_Isolate, tryCatch.Message()->Get()),
                      "Uncaught TypeError: File type doesn't match specified type json. Importpath: \"./importFile.js\"");

            // type js assertion
            tryCatch.Reset();
            info->SetPath(appRoot / std::filesystem::path("js/importAssertions/assertTypeJS.js"));
            moduleInfo = jsModules.TestLoadModuleTree(m_Context, info);
            ASSERT_NE(nullptr, moduleInfo);
            EXPECT_FALSE(moduleInfo->GetLocalModule().IsEmpty());
            EXPECT_FALSE(tryCatch.HasCaught());

            // type native assertion
            tryCatch.Reset();
            info->SetPath(appRoot / std::filesystem::path("js/importAssertions/assertTypeNative.js"));
            moduleInfo = jsModules.TestLoadModuleTree(m_Context, info);
            ASSERT_EQ(nullptr, moduleInfo);
            EXPECT_TRUE(tryCatch.HasCaught());
            EXPECT_EQ(JSUtilities::V8ToString(m_Isolate, tryCatch.Message()->Get()),
                      "Uncaught TypeError: File type doesn't match specified type native. Importpath: \"./importFile.js\"");

            // type unknown assertion
            tryCatch.Reset();
            info->SetPath(appRoot / std::filesystem::path("js/importAssertions/assertTypeInvalid.js"));
            moduleInfo = jsModules.TestLoadModuleTree(m_Context, info);
            ASSERT_EQ(nullptr, moduleInfo);
            EXPECT_FALSE(tryCatch.HasCaught());
            expected = {
                {Log::MsgKey::Msg, "Unknown type assertion: unknown"},
                {Log::MsgKey::LogLevel, "Warn"},
            };
            EXPECT_TRUE(logSink->ValidateMessage(expected, m_IgnoreKeys));
        }

        TEST_F(JSContextModulesTest, LoadModule)
        {
            std::filesystem::path root = m_App->GetAppRoots()->GetAppRoot();
            JSContextModulesSharedPtr jsModules = m_Context->GetJSModules();

            V8MaybeLocalModule maybeModule;
            V8Isolate::Scope iScope(m_Isolate);
            v8::HandleScope hScope(m_Isolate);
            v8::TryCatch tryCatch(m_Isolate);
            V8LocalContext context = m_Context->GetLocalContext();
            v8::Context::Scope cScope(context);

            std::filesystem::path srcPath = root / std::filesystem::path("js/compileError.js");

            JSModuleInfoSharedPtr moduleInfo = jsModules->LoadModule(srcPath);
            ASSERT_EQ(moduleInfo, nullptr);

            srcPath = root / std::filesystem::path("js/loadModule.js");
            moduleInfo = jsModules->LoadModule(srcPath);
            V8LocalModule module = moduleInfo->GetLocalModule();
            ASSERT_FALSE(module.IsEmpty());
            EXPECT_NE(nullptr, jsModules->GetModuleBySpecifier(srcPath));
        }

        TEST_F(JSContextModulesTest, LoadJSON)
        {
            std::filesystem::path root = m_App->GetAppRoots()->GetAppRoot();
            JSContextModulesSharedPtr jsModules = m_Context->GetJSModules();

            V8Isolate::Scope iScope(m_Isolate);
            v8::HandleScope hScope(m_Isolate);
            v8::TryCatch tryCatch(m_Isolate);
            V8LocalContext context = m_Context->GetLocalContext();
            v8::Context::Scope cScope(context);

            std::filesystem::path srcPath = root / std::filesystem::path("resources/loadModule.json");

            JSModuleInfoSharedPtr moduleInfo =  jsModules->LoadModule(srcPath);
            ASSERT_NE(moduleInfo, nullptr);
            JSModuleInfoSharedPtr info = jsModules->GetModuleBySpecifier(srcPath);
            V8LocalModule module = moduleInfo->GetLocalModule();
            ASSERT_NE(nullptr, info);
            V8LocalValue jsonValue = info->GetLocalJSON();
            EXPECT_FALSE(jsonValue.IsEmpty());
            EXPECT_TRUE(jsonValue->IsObject());
        }

        TEST_F(JSContextModulesTest, InstantiateModuleJS)
        {
            TestUtils::TestLogSink *logSink = TestUtils::TestLogSink::GetGlobalSink();
            Log::Log::SetLogLevel(Log::LogLevel::Error);

            std::filesystem::path root = m_App->GetAppRoots()->GetAppRoot();
            JSContextModulesSharedPtr jsModules = m_Context->GetJSModules();

            V8Isolate::Scope iScope(m_Isolate);
            v8::HandleScope hScope(m_Isolate);
            v8::TryCatch tryCatch(m_Isolate);
            V8LocalContext context = m_Context->GetLocalContext();
            v8::Context::Scope cScope(context);

            // passed nullptr
            JSModuleInfoSharedPtr info;
            EXPECT_FALSE(jsModules->InstantiateModule(info));
            Log::LogMessage expected = {
                {Log::MsgKey::Msg, "InstantiateModule passed a null module ptr"},
                {Log::MsgKey::LogLevel, "Error"}};
            logSink->ValidateMessage(expected, m_IgnoreKeys);

            // info hasn't had module loaded
            info = std::make_shared<JSModuleInfo>(m_Context);
            EXPECT_FALSE(jsModules->InstantiateModule(info));
            expected = {
                {Log::MsgKey::Msg, "InstantiateModule passed module info's module is empty"},
                {Log::MsgKey::LogLevel, "Error"}};
            logSink->ValidateMessage(expected, m_IgnoreKeys);

            std::filesystem::path srcPath = root / std::filesystem::path("js/loadModuleImport.mjs");
            info =  jsModules->LoadModule(srcPath);
            ASSERT_NE(nullptr, info);

            EXPECT_TRUE(jsModules->InstantiateModule(info));
        }

        TEST_F(JSContextModulesTest, InstantiateModuleJSDynamic)
        {
            std::filesystem::path root = m_App->GetAppRoots()->GetAppRoot();
            JSContextModulesSharedPtr jsModules = m_Context->GetJSModules();

            V8Isolate::Scope iScope(m_Isolate);
            v8::HandleScope hScope(m_Isolate);
            v8::TryCatch tryCatch(m_Isolate);
            V8LocalContext context = m_Context->GetLocalContext();
            v8::Context::Scope cScope(context);

            std::filesystem::path srcPath = root / std::filesystem::path("js/importModuleDynamic.js");
            JSModuleInfoSharedPtr info = jsModules->LoadModule(srcPath);
            ASSERT_NE(nullptr, info);
            EXPECT_TRUE(jsModules->InstantiateModule(info));
        }

        TEST_F(JSContextModulesTest, InstantiateModuleJSON)
        {
            std::filesystem::path root = m_App->GetAppRoots()->GetAppRoot();
            JSContextModulesSharedPtr jsModules = m_Context->GetJSModules();

            V8Isolate::Scope iScope(m_Isolate);
            v8::HandleScope hScope(m_Isolate);
            v8::TryCatch tryCatch(m_Isolate);
            V8LocalContext context = m_Context->GetLocalContext();
            v8::Context::Scope cScope(context);

            std::filesystem::path srcPath = root / std::filesystem::path("resources/loadModule.json");
            JSModuleInfoSharedPtr info = jsModules->LoadModule(srcPath);
            ASSERT_NE(nullptr, info);
            EXPECT_TRUE(jsModules->InstantiateModule(info));
        }

        TEST_F(JSContextModulesTest, RunModuleJS)
        {
            TestUtils::TestLogSink *logSink = TestUtils::TestLogSink::GetGlobalSink();
            Log::Log::SetLogLevel(Log::LogLevel::Error);

            std::filesystem::path root = m_App->GetAppRoots()->GetAppRoot();
            JSContextModulesSharedPtr jsModules = m_Context->GetJSModules();

            V8Isolate::Scope iScope(m_Isolate);
            v8::HandleScope hScope(m_Isolate);
            v8::TryCatch tryCatch(m_Isolate);
            V8LocalContext context = m_Context->GetLocalContext();
            v8::Context::Scope cScope(context);

            // passed nullptr
            JSModuleInfoSharedPtr info;
            EXPECT_FALSE(jsModules->RunModule(info));
            Log::LogMessage expected = {
                {Log::MsgKey::Msg, "RunModule passed a null module ptr"},
                {Log::MsgKey::LogLevel, "Error"}};
            EXPECT_TRUE(logSink->ValidateMessage(expected, m_IgnoreKeys));

            // info hasn't had module loaded
            info = std::make_shared<JSModuleInfo>(m_Context);
            EXPECT_FALSE(jsModules->RunModule(info));
            expected = {
                {Log::MsgKey::Msg, "RunModule passed module info's module is empty"},
                {Log::MsgKey::LogLevel, "Error"}};
            EXPECT_TRUE(logSink->ValidateMessage(expected, m_IgnoreKeys));

            std::filesystem::path srcPath = root / std::filesystem::path("js/loadModuleImport.mjs");
            info = jsModules->LoadModule(srcPath);
            ASSERT_NE(nullptr, info);
            ASSERT_TRUE(jsModules->InstantiateModule(info));
            EXPECT_TRUE(jsModules->RunModule(info));
        }

        TEST_F(JSContextModulesTest, RunModuleJSDynamic)
        {
            std::filesystem::path root = m_App->GetAppRoots()->GetAppRoot();
            JSContextModulesSharedPtr jsModules = m_Context->GetJSModules();

            V8Isolate::Scope iScope(m_Isolate);
            v8::HandleScope hScope(m_Isolate);
            v8::TryCatch tryCatch(m_Isolate);
            V8LocalContext context = m_Context->GetLocalContext();
            v8::Context::Scope cScope(context);

            std::filesystem::path srcPath = root / std::filesystem::path("js/importModuleDynamic.js");
            JSModuleInfoSharedPtr moduleInfo = jsModules->LoadModule(srcPath);
            V8LocalModule module = moduleInfo->GetLocalModule();
            JSModuleInfoSharedPtr info = jsModules->GetModuleBySpecifier(srcPath);
            ASSERT_NE(nullptr, info);
            ASSERT_TRUE(jsModules->InstantiateModule(info));
            EXPECT_TRUE(jsModules->RunModule(info));
        }

        TEST_F(JSContextModulesTest, RunModuleJSON)
        {
            std::filesystem::path root = m_App->GetAppRoots()->GetAppRoot();
            JSContextModulesSharedPtr jsModules = m_Context->GetJSModules();

            V8Isolate::Scope iScope(m_Isolate);
            v8::HandleScope hScope(m_Isolate);
            v8::TryCatch tryCatch(m_Isolate);
            V8LocalContext context = m_Context->GetLocalContext();
            v8::Context::Scope cScope(context);

            std::filesystem::path srcPath = root / std::filesystem::path("resources/loadModule.json");
            JSModuleInfoSharedPtr info = jsModules->LoadModule(srcPath);
            ASSERT_NE(nullptr, info);
            ASSERT_TRUE(jsModules->InstantiateModule(info));
            EXPECT_TRUE(jsModules->RunModule(info));
        }
    }
}