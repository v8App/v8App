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

#include "JSApp.h"
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

            JSModuleInfoSharedPtr TestBuildModuleInfo(JSModuleInfo::AttributesInfo &inAttributesInfo, const std::filesystem::path &inImportPath, const std::filesystem::path &inCurrentModPath)
            {
                return BuildModuleInfo(inAttributesInfo, inImportPath, inCurrentModPath);
            }

            JSModuleInfoSharedPtr TestLoadModuleTree(JSContextSharedPtr inContext, JSModuleInfoSharedPtr inModInfo) { return LoadModuleTree(inContext, inModInfo); }

            bool TestAddModule(const JSModuleInfoSharedPtr &inModule, std::string inFileName, JSModuleInfo::ModuleType inModuleType) { return AddModule(inModule, inFileName, inModuleType); }
            JSModuleInfoSharedPtr TestGetModuleInfoByModule(V8LModule inModule, JSModuleInfo::ModuleType inType) { return GetModuleInfoByModule(inModule, inType); }
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
            JSModuleInfo::AttributesInfo attributesInfo;
            std::filesystem::path rootPath = m_App->GetAppRoot()->GetAppRoot();

            V8Isolate::Scope iScope(m_Isolate);
            V8HandleScope hScope(m_Isolate);
            V8TryCatch tryCatch(m_Isolate);
            V8ContextScope cScope(m_Context->GetLocalContext());

            std::filesystem::path testPath("testModule.js");
            // test no module for attribute
            attributesInfo.m_Module = "NoModuleTest";
            info = jsModules.TestBuildModuleInfo(attributesInfo, testPath, rootPath);
            EXPECT_EQ(info, nullptr);
            EXPECT_TRUE(tryCatch.HasCaught());
            EXPECT_EQ(JSUtilities::V8ToString(m_Isolate, tryCatch.Message()->Get()),
                      Utils::format("Uncaught SyntaxError: Failed to find attributed module: {}, ImportPath: {}", attributesInfo.m_Module, testPath));

            // JS Folder
            tryCatch.Reset();
            // module attributes in js path
            testPath = std::filesystem::path("%JS%/testModule.js");
            info = jsModules.TestBuildModuleInfo(attributesInfo, testPath, rootPath);
            EXPECT_EQ(info, nullptr);
            EXPECT_TRUE(tryCatch.HasCaught());
            EXPECT_EQ(JSUtilities::V8ToString(m_Isolate, tryCatch.Message()->Get()),
                      Utils::format("Uncaught SyntaxError: Can not use a module assetion in the {} root, ImportPath: {}", Assets::c_RootJS, testPath));

            tryCatch.Reset();
            // extension doesn't match allowed type
            testPath = std::filesystem::path("%JS%/testModule.txt");
            attributesInfo.m_Module = "";
            attributesInfo.m_Type = JSModuleInfo::ModuleType::kJavascript;
            attributesInfo.m_TypeString = "js";
            info = jsModules.TestBuildModuleInfo(attributesInfo, testPath, rootPath);
            EXPECT_EQ(info, nullptr);
            EXPECT_TRUE(tryCatch.HasCaught());
            EXPECT_EQ(JSUtilities::V8ToString(m_Isolate, tryCatch.Message()->Get()),
                      Utils::format("Uncaught TypeError: File type doesn't match specified type {}. Importpath: {}", attributesInfo.m_TypeString, testPath));

            tryCatch.Reset();
            // non module info returned
            testPath = std::filesystem::path("%JS%/testModule.js");
            info = jsModules.TestBuildModuleInfo(attributesInfo, testPath, rootPath);
            ASSERT_NE(info, nullptr);
            EXPECT_FALSE(tryCatch.HasCaught());
            EXPECT_EQ(info->GetName(), "testModule");
            EXPECT_EQ(info->GetModulePath().generic_string(), (rootPath / std::filesystem::path("js/testModule.js")).generic_string());

            tryCatch.Reset();
            // non module info returned with abs path
            testPath = std::filesystem::path("/js/testModule.js");
            info = jsModules.TestBuildModuleInfo(attributesInfo, testPath, rootPath);
            ASSERT_NE(info, nullptr);
            EXPECT_FALSE(tryCatch.HasCaught());
            EXPECT_EQ(info->GetName(), "testModule");
            EXPECT_EQ(info->GetModulePath().generic_string(), (rootPath / std::filesystem::path("js/testModule.js")).generic_string());

            // Resources Folder
            tryCatch.Reset();
            // module attribute in resources path
            testPath = std::filesystem::path("%RESOURCES%/testModule.txt");
            attributesInfo.m_Module = "tetsMofule";
            info = jsModules.TestBuildModuleInfo(attributesInfo, testPath, rootPath);
            EXPECT_EQ(info, nullptr);
            EXPECT_TRUE(tryCatch.HasCaught());
            EXPECT_EQ(JSUtilities::V8ToString(m_Isolate, tryCatch.Message()->Get()),
                      Utils::format("Uncaught SyntaxError: Can not use a module assetion in the {} root, ImportPath: {}", Assets::c_RootResource, testPath));

            // js or mjs in resources
            testPath = std::filesystem::path("%RESOURCES%/testModule.js");
            attributesInfo.m_Module = "";
            info = jsModules.TestBuildModuleInfo(attributesInfo, testPath, rootPath);
            EXPECT_EQ(info, nullptr);
            EXPECT_TRUE(tryCatch.HasCaught());
            EXPECT_EQ(JSUtilities::V8ToString(m_Isolate, tryCatch.Message()->Get()),
                      Utils::format("Uncaught SyntaxError: Files ending in .js or .mjs can not be in resources, ImportPath: {}", testPath));

            testPath = std::filesystem::path("%RESOURCES%/testModule.mjs");
            info = jsModules.TestBuildModuleInfo(attributesInfo, testPath, rootPath);
            EXPECT_EQ(info, nullptr);
            EXPECT_TRUE(tryCatch.HasCaught());
            EXPECT_EQ(JSUtilities::V8ToString(m_Isolate, tryCatch.Message()->Get()),
                      Utils::format("Uncaught SyntaxError: Files ending in .js or .mjs can not be in resources, ImportPath: {}", testPath));

            tryCatch.Reset();
            // extension doesn't match allowed type
            testPath = std::filesystem::path("%RESOURCES%/testModule.txt");
            attributesInfo.m_Module = "";
            attributesInfo.m_Type = JSModuleInfo::ModuleType::kJSON;
            attributesInfo.m_TypeString = "json";
            info = jsModules.TestBuildModuleInfo(attributesInfo, testPath, rootPath);
            EXPECT_EQ(info, nullptr);
            EXPECT_TRUE(tryCatch.HasCaught());
            EXPECT_EQ(JSUtilities::V8ToString(m_Isolate, tryCatch.Message()->Get()),
                      Utils::format("Uncaught TypeError: File type doesn't match specified type json. Importpath: {}", testPath));

            tryCatch.Reset();
            // non module info returned
            testPath = std::filesystem::path("%RESOURCES%/testModule.json");
            info = jsModules.TestBuildModuleInfo(attributesInfo, testPath, rootPath);
            ASSERT_NE(info, nullptr);
            EXPECT_FALSE(tryCatch.HasCaught());
            EXPECT_EQ(info->GetName(), "testModule");
            EXPECT_EQ(info->GetModulePath().generic_string(), (rootPath / std::filesystem::path("resources/testModule.json")).generic_string());

            // Modules Folder
            tryCatch.Reset();
            // extension doesn't match allowed type
            testPath = std::filesystem::path("%MODULES%/builModInfo/test.json");
            attributesInfo.m_Module = "";
            attributesInfo.m_Type = JSModuleInfo::ModuleType::kJavascript;
            attributesInfo.m_TypeString = "js";
            info = jsModules.TestBuildModuleInfo(attributesInfo, testPath, rootPath);
            EXPECT_EQ(info, nullptr);
            EXPECT_TRUE(tryCatch.HasCaught());
            EXPECT_EQ(JSUtilities::V8ToString(m_Isolate, tryCatch.Message()->Get()),
                      Utils::format("Uncaught TypeError: File type doesn't match specified type {}. Importpath: {}", attributesInfo.m_TypeString, testPath));

            // module attributed but module not the module
            tryCatch.Reset();
            testPath = std::filesystem::path("%MODULES%/NotBuilModInfo/test.js");
            attributesInfo.m_Module = "buildModInfo";
            info = jsModules.TestBuildModuleInfo(attributesInfo, testPath, rootPath);
            EXPECT_EQ(info, nullptr);
            EXPECT_TRUE(tryCatch.HasCaught());
            EXPECT_EQ(JSUtilities::V8ToString(m_Isolate, tryCatch.Message()->Get()),
                      Utils::format("Uncaught SyntaxError: Import path was not in attributed module's path. Module:{}, ImportPath: {}", attributesInfo.m_Module, testPath));

            // No module version
            tryCatch.Reset();
            testPath = std::filesystem::path("%MODULES%/NoModVersion/test.js");
            attributesInfo.m_Module = "";
            info = jsModules.TestBuildModuleInfo(attributesInfo, testPath, rootPath);
            EXPECT_EQ(info, nullptr);
            EXPECT_TRUE(tryCatch.HasCaught());
            EXPECT_EQ(JSUtilities::V8ToString(m_Isolate, tryCatch.Message()->Get()),
                      Utils::format("Uncaught SyntaxError: Failed to find module's version: NoModVersion, ImportPath: {}", testPath));

            // test relative module path to specified root
            std::filesystem::path modPath = m_App->GetAppRoot()->FindModuleVersionRootPath("buildModInfo/1.0.0");
            ASSERT_NE("", modPath.string());

            tryCatch.Reset();
            testPath = std::filesystem::path("test.js");
            attributesInfo.m_Module = "";
            info = jsModules.TestBuildModuleInfo(attributesInfo, testPath, modPath);
            ASSERT_NE(info, nullptr);
            EXPECT_FALSE(tryCatch.HasCaught());
            EXPECT_EQ(info->GetName(), "buildModInfo");
            EXPECT_EQ(info->GetVersion(), "1.0.0");
            EXPECT_EQ(info->GetModulePath().generic_string(), (rootPath / std::filesystem::path("modules/buildModInfo/1.0.0/test.js")).generic_string());

            // test relative module to attributed module
            tryCatch.Reset();
            testPath = std::filesystem::path("test.js");
            attributesInfo.m_Module = "buildModInfo";
            info = jsModules.TestBuildModuleInfo(attributesInfo, testPath, modPath);
            ASSERT_NE(info, nullptr);
            EXPECT_FALSE(tryCatch.HasCaught());
            EXPECT_EQ(info->GetName(), "buildModInfo");
            EXPECT_EQ(info->GetVersion(), "1.0.0");
            EXPECT_EQ(info->GetModulePath().generic_string(), (rootPath / std::filesystem::path("modules/buildModInfo/1.0.0/test.js")).generic_string());

            // test absolute path to module
            tryCatch.Reset();
            testPath = std::filesystem::path("/modules/buildModInfo/test.js");
            info = jsModules.TestBuildModuleInfo(attributesInfo, testPath, rootPath);
            ASSERT_NE(info, nullptr);
            EXPECT_FALSE(tryCatch.HasCaught());
            EXPECT_EQ(info->GetName(), "buildModInfo");
            EXPECT_EQ(info->GetVersion(), "1.0.0");
            EXPECT_EQ(info->GetModulePath().generic_string(), (rootPath / std::filesystem::path("modules/buildModInfo/1.0.0/test.js")).generic_string());
        }

        TEST_F(JSContextModulesTest, LoadModuleTreeJSNoImports)
        {
            TestUtils::TestLogSink *logSink = TestUtils::TestLogSink::GetGlobalSink();
            Log::Log::SetLogLevel(Log::LogLevel::Error);

            TestJSContextModules jsModules(m_Context);

            V8Isolate::Scope iScope(m_Isolate);
            V8HandleScope hScope(m_Isolate);
            V8TryCatch tryCatch(m_Isolate);
            V8LContext context = m_Context->GetLocalContext();
            V8ContextScope cScope(context);

            std::filesystem::path appRoot = m_App->GetAppRoot()->GetAppRoot();
            JSModuleInfoSharedPtr info = std::make_shared<JSModuleInfo>(m_Context);
            JSModuleInfo::AttributesInfo attributesInfo;
            attributesInfo.m_Type = JSModuleInfo::ModuleType::kJavascript;
            attributesInfo.m_TypeString = "js";
            info->SetAttributesInfo(attributesInfo);

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
                {Log::MsgKey::Msg, Utils::format("Uncaught SyntaxError: Unexpected token '-'\n{}:1:SyntaxError: Unexpected token '-'\n", info->GetModulePath().generic_string())},
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
            JSModuleInfoSharedPtr addInfo = m_Context->GetJSModules()->GetModuleBySpecifier(info->GetModulePath().generic_string());
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
            V8HandleScope hScope(m_Isolate);
            V8TryCatch tryCatch(m_Isolate);
            V8LContext context = m_Context->GetLocalContext();
            V8ContextScope cScope(context);

            std::filesystem::path appRoot = m_App->GetAppRoot()->GetAppRoot();
            JSModuleInfoSharedPtr info = std::make_shared<JSModuleInfo>(m_Context);
            JSModuleInfo::AttributesInfo attributesInfo;
            attributesInfo.m_Type = JSModuleInfo::ModuleType::kJSON;
            attributesInfo.m_TypeString = "json";
            info->SetAttributesInfo(attributesInfo);

            // failed to load the json
            info->SetPath(appRoot / std::filesystem::path("js/NotExists.json"));

            JSModuleInfoSharedPtr moduleInfo = jsModules.TestLoadModuleTree(m_Context, info);
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

            JSModuleInfoSharedPtr addInfo = m_Context->GetJSModules()->GetModuleBySpecifier(info->GetModulePath().generic_string());
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
            V8HandleScope hScope(m_Isolate);
            V8TryCatch tryCatch(m_Isolate);
            V8LContext context = m_Context->GetLocalContext();
            V8ContextScope cScope(context);

            std::filesystem::path appRoot = m_App->GetAppRoot()->GetAppRoot();
            JSModuleInfoSharedPtr info = std::make_shared<JSModuleInfo>(m_Context);
            JSModuleInfo::AttributesInfo attributesInfo;
            attributesInfo.m_Type = JSModuleInfo::ModuleType::kJavascript;
            attributesInfo.m_TypeString = "js";
            info->SetAttributesInfo(attributesInfo);

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
            V8HandleScope hScope(m_Isolate);
            V8TryCatch tryCatch(m_Isolate);
            V8LContext context = m_Context->GetLocalContext();
            V8ContextScope cScope(context);

            std::filesystem::path appRoot = m_App->GetAppRoot()->GetAppRoot();
            JSModuleInfoSharedPtr info = std::make_shared<JSModuleInfo>(m_Context);
            JSModuleInfo::AttributesInfo attributesInfo;
            attributesInfo.m_Type = JSModuleInfo::ModuleType::kJavascript;
            attributesInfo.m_TypeString = "js";
            info->SetAttributesInfo(attributesInfo);

            info->SetPath(appRoot / std::filesystem::path("js/importModuleDynamic.js"));

            info = jsModules.TestLoadModuleTree(m_Context, info);
            ASSERT_NE(nullptr, info);
        }

        TEST_F(JSContextModulesTest, ImportAttributes)
        {
            TestUtils::TestLogSink *logSink = TestUtils::TestLogSink::GetGlobalSink();
            Log::Log::SetLogLevel(Log::LogLevel::Warn);

            TestJSContextModules jsModules(m_Context);

            V8Isolate::Scope iScope(m_Isolate);
            V8HandleScope hScope(m_Isolate);
            V8TryCatch tryCatch(m_Isolate);
            V8LContext context = m_Context->GetLocalContext();
            V8ContextScope cScope(context);

            std::filesystem::path appRoot = m_App->GetAppRoot()->GetAppRoot();
            JSModuleInfoSharedPtr info = std::make_shared<JSModuleInfo>(m_Context);
            JSModuleInfo::AttributesInfo attributesInfo;
            attributesInfo.m_Type = JSModuleInfo::ModuleType::kJavascript;
            attributesInfo.m_TypeString = "js";
            info->SetAttributesInfo(attributesInfo);

            // unknown asssertion
            info->SetPath(appRoot / std::filesystem::path("js/importAttributes/attributeUnknown.js"));
            JSModuleInfoSharedPtr moduleInfo = jsModules.TestLoadModuleTree(m_Context, info);
            ASSERT_NE(nullptr, moduleInfo);
            EXPECT_FALSE(moduleInfo->GetLocalModule().IsEmpty());

            Log::LogMessage expected = {
                {Log::MsgKey::Msg, "Unknown attribute: unknown"},
                {Log::MsgKey::LogLevel, "Warn"},
            };
            EXPECT_TRUE(logSink->ValidateMessage(expected, m_IgnoreKeys));

            // modules attributes
            info->SetPath(appRoot / std::filesystem::path("js/importAttributes/attributeModule.js"));
            moduleInfo = jsModules.TestLoadModuleTree(m_Context, info);
            ASSERT_EQ(nullptr, moduleInfo);
            EXPECT_TRUE(tryCatch.HasCaught());
            EXPECT_EQ(JSUtilities::V8ToString(m_Isolate, tryCatch.Message()->Get()),
                      "Uncaught SyntaxError: Failed to find attributed module: test, ImportPath: \"./importFile.js\"");

            // type json attributes
            tryCatch.Reset();
            info->SetPath(appRoot / std::filesystem::path("js/importAttributes/attributeJSONType.js"));
            moduleInfo = jsModules.TestLoadModuleTree(m_Context, info);
            ASSERT_EQ(nullptr, moduleInfo);
            EXPECT_TRUE(tryCatch.HasCaught());
            EXPECT_EQ(JSUtilities::V8ToString(m_Isolate, tryCatch.Message()->Get()),
                      "Uncaught TypeError: File type doesn't match specified type json. Importpath: \"./importFile.js\"");

            // type js attributes
            tryCatch.Reset();
            info->SetPath(appRoot / std::filesystem::path("js/importAttributes/attributeTypeJS.js"));
            moduleInfo = jsModules.TestLoadModuleTree(m_Context, info);
            ASSERT_NE(nullptr, moduleInfo);
            EXPECT_FALSE(moduleInfo->GetLocalModule().IsEmpty());
            EXPECT_FALSE(tryCatch.HasCaught());

            // type native attributes
            tryCatch.Reset();
            info->SetPath(appRoot / std::filesystem::path("js/importAttributes/attributeTypeNative.js"));
            moduleInfo = jsModules.TestLoadModuleTree(m_Context, info);
            ASSERT_EQ(nullptr, moduleInfo);
            EXPECT_TRUE(tryCatch.HasCaught());
            EXPECT_EQ(JSUtilities::V8ToString(m_Isolate, tryCatch.Message()->Get()),
                      "Uncaught TypeError: File type doesn't match specified type native. Importpath: \"./importFile.js\"");

            // type unknown attributes
            tryCatch.Reset();
            info->SetPath(appRoot / std::filesystem::path("js/importAttributes/attributeTypeInvalid.js"));
            moduleInfo = jsModules.TestLoadModuleTree(m_Context, info);
            ASSERT_EQ(nullptr, moduleInfo);
            EXPECT_FALSE(tryCatch.HasCaught());
            expected = {
                {Log::MsgKey::Msg, "Unknown type attribute: unknown"},
                {Log::MsgKey::LogLevel, "Warn"},
            };
            EXPECT_TRUE(logSink->ValidateMessage(expected, m_IgnoreKeys));
        }

        TEST_F(JSContextModulesTest, LoadModule)
        {
            std::filesystem::path root = m_App->GetAppRoot()->GetAppRoot();
            JSContextModulesSharedPtr jsModules = m_Context->GetJSModules();

            V8MBLModule maybeModule;
            V8Isolate::Scope iScope(m_Isolate);
            V8HandleScope hScope(m_Isolate);
            V8TryCatch tryCatch(m_Isolate);
            V8LContext context = m_Context->GetLocalContext();
            V8ContextScope cScope(context);

            std::filesystem::path srcPath = root / std::filesystem::path("js/compileError.js");

            JSModuleInfoSharedPtr moduleInfo = jsModules->LoadModule(srcPath);
            ASSERT_EQ(moduleInfo, nullptr);

            srcPath = root / std::filesystem::path("js/loadModule.js");
            moduleInfo = jsModules->LoadModule(srcPath);
            V8LModule module = moduleInfo->GetLocalModule();
            ASSERT_FALSE(module.IsEmpty());
            EXPECT_NE(nullptr, jsModules->GetModuleBySpecifier(srcPath.generic_string()));
        }

        TEST_F(JSContextModulesTest, LoadJSON)
        {
            std::filesystem::path root = m_App->GetAppRoot()->GetAppRoot();
            JSContextModulesSharedPtr jsModules = m_Context->GetJSModules();

            V8Isolate::Scope iScope(m_Isolate);
            V8HandleScope hScope(m_Isolate);
            V8TryCatch tryCatch(m_Isolate);
            V8LContext context = m_Context->GetLocalContext();
            V8ContextScope cScope(context);

            std::filesystem::path srcPath = root / std::filesystem::path("resources/loadModule.json");

            JSModuleInfoSharedPtr moduleInfo = jsModules->LoadModule(srcPath);
            ASSERT_NE(moduleInfo, nullptr);
            JSModuleInfoSharedPtr info = jsModules->GetModuleBySpecifier(srcPath.generic_string());
            V8LModule module = moduleInfo->GetLocalModule();
            ASSERT_NE(nullptr, info);
            V8LValue jsonValue = info->GetLocalJSON();
            EXPECT_FALSE(jsonValue.IsEmpty());
            EXPECT_TRUE(jsonValue->IsObject());
        }

        TEST_F(JSContextModulesTest, InstantiateModuleJS)
        {
            TestUtils::TestLogSink *logSink = TestUtils::TestLogSink::GetGlobalSink();
            Log::Log::SetLogLevel(Log::LogLevel::Error);

            std::filesystem::path root = m_App->GetAppRoot()->GetAppRoot();
            JSContextModulesSharedPtr jsModules = m_Context->GetJSModules();

            V8Isolate::Scope iScope(m_Isolate);
            V8HandleScope hScope(m_Isolate);
            V8TryCatch tryCatch(m_Isolate);
            V8LContext context = m_Context->GetLocalContext();
            V8ContextScope cScope(context);

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
            info = jsModules->LoadModule(srcPath);
            ASSERT_NE(nullptr, info);

            EXPECT_TRUE(jsModules->InstantiateModule(info));
        }

        TEST_F(JSContextModulesTest, InstantiateModuleJSDynamic)
        {
            std::filesystem::path root = m_App->GetAppRoot()->GetAppRoot();
            JSContextModulesSharedPtr jsModules = m_Context->GetJSModules();

            V8Isolate::Scope iScope(m_Isolate);
            V8HandleScope hScope(m_Isolate);
            V8TryCatch tryCatch(m_Isolate);
            V8LContext context = m_Context->GetLocalContext();
            V8ContextScope cScope(context);

            std::filesystem::path srcPath = root / std::filesystem::path("js/importModuleDynamic.js");
            JSModuleInfoSharedPtr info = jsModules->LoadModule(srcPath);
            ASSERT_NE(nullptr, info);
            EXPECT_TRUE(jsModules->InstantiateModule(info));
        }

        TEST_F(JSContextModulesTest, InstantiateModuleJSON)
        {
            std::filesystem::path root = m_App->GetAppRoot()->GetAppRoot();
            JSContextModulesSharedPtr jsModules = m_Context->GetJSModules();

            V8Isolate::Scope iScope(m_Isolate);
            V8HandleScope hScope(m_Isolate);
            V8TryCatch tryCatch(m_Isolate);
            V8LContext context = m_Context->GetLocalContext();
            V8ContextScope cScope(context);

            std::filesystem::path srcPath = root / std::filesystem::path("resources/loadModule.json");
            JSModuleInfoSharedPtr info = jsModules->LoadModule(srcPath);
            ASSERT_NE(nullptr, info);
            EXPECT_TRUE(jsModules->InstantiateModule(info));
        }

        TEST_F(JSContextModulesTest, RunModuleJS)
        {
            TestUtils::TestLogSink *logSink = TestUtils::TestLogSink::GetGlobalSink();
            Log::Log::SetLogLevel(Log::LogLevel::Error);

            std::filesystem::path root = m_App->GetAppRoot()->GetAppRoot();
            JSContextModulesSharedPtr jsModules = m_Context->GetJSModules();

            V8Isolate::Scope iScope(m_Isolate);
            V8HandleScope hScope(m_Isolate);
            V8TryCatch tryCatch(m_Isolate);
            V8LContext context = m_Context->GetLocalContext();
            V8ContextScope cScope(context);

            // passed nullptr
            JSModuleInfoSharedPtr info;
            EXPECT_TRUE(jsModules->RunModule(info).IsEmpty());
            Log::LogMessage expected = {
                {Log::MsgKey::Msg, "RunModule passed a null module ptr"},
                {Log::MsgKey::LogLevel, "Error"}};
            EXPECT_TRUE(logSink->ValidateMessage(expected, m_IgnoreKeys));

            // info hasn't had module loaded
            info = std::make_shared<JSModuleInfo>(m_Context);
            EXPECT_TRUE(jsModules->RunModule(info).IsEmpty());
            expected = {
                {Log::MsgKey::Msg, "RunModule passed module info's module is empty"},
                {Log::MsgKey::LogLevel, "Error"}};
            EXPECT_TRUE(logSink->ValidateMessage(expected, m_IgnoreKeys));

            std::filesystem::path srcPath = root / std::filesystem::path("js/loadModuleImport.mjs");
            info = jsModules->LoadModule(srcPath);
            ASSERT_NE(nullptr, info);
            ASSERT_TRUE(jsModules->InstantiateModule(info));
            EXPECT_FALSE(jsModules->RunModule(info).IsEmpty());
        }

        TEST_F(JSContextModulesTest, RunModuleJSDynamic)
        {
            std::filesystem::path root = m_App->GetAppRoot()->GetAppRoot();
            JSContextModulesSharedPtr jsModules = m_Context->GetJSModules();

            V8Isolate::Scope iScope(m_Isolate);
            V8HandleScope hScope(m_Isolate);
            V8TryCatch tryCatch(m_Isolate);
            V8LContext context = m_Context->GetLocalContext();
            V8ContextScope cScope(context);

            std::filesystem::path srcPath = root / std::filesystem::path("js/importModuleDynamic.js");
            JSModuleInfoSharedPtr moduleInfo = jsModules->LoadModule(srcPath);
            V8LModule module = moduleInfo->GetLocalModule();
            JSModuleInfoSharedPtr info = jsModules->GetModuleBySpecifier(srcPath.generic_string());
            ASSERT_NE(nullptr, info);
            ASSERT_TRUE(jsModules->InstantiateModule(info));
            EXPECT_FALSE(jsModules->RunModule(info).IsEmpty());
        }

        TEST_F(JSContextModulesTest, RunModuleJSON)
        {
            std::filesystem::path root = m_App->GetAppRoot()->GetAppRoot();
            JSContextModulesSharedPtr jsModules = m_Context->GetJSModules();

            V8Isolate::Scope iScope(m_Isolate);
            V8HandleScope hScope(m_Isolate);
            V8TryCatch tryCatch(m_Isolate);
            V8LContext context = m_Context->GetLocalContext();
            V8ContextScope cScope(context);

            std::filesystem::path srcPath = root / std::filesystem::path("resources/loadModule.json");
            JSModuleInfoSharedPtr info = jsModules->LoadModule(srcPath);
            ASSERT_NE(nullptr, info);
            ASSERT_TRUE(jsModules->InstantiateModule(info));
            EXPECT_FALSE(jsModules->RunModule(info).IsEmpty());
        }

        TEST_F(JSContextModulesTest, GenerateCodeCache)
        {
            std::filesystem::path root = m_App->GetAppRoot()->GetAppRoot();
            JSContextModulesSharedPtr jsModules = m_Context->GetJSModules();

            V8Isolate::Scope iScope(m_Isolate);
            V8HandleScope hScope(m_Isolate);
            V8TryCatch tryCatch(m_Isolate);
            V8LContext context = m_Context->GetLocalContext();
            V8ContextScope cScope(context);

            std::filesystem::path srcPath = root / std::filesystem::path("js/loadModuleImport.mjs");
            CodeCacheSharedPtr codeCache = m_App->GetCodeCache();
            JSModuleInfoSharedPtr moduleInfo = jsModules->LoadModule(srcPath);
            V8LModule module = moduleInfo->GetLocalModule();
            JSModuleInfoSharedPtr info = jsModules->GetModuleBySpecifier(srcPath.generic_string());
            ASSERT_NE(nullptr, info);
            ASSERT_TRUE(jsModules->InstantiateModule(info));
            jsModules->GenerateCodeCache();
            EXPECT_TRUE(codeCache->HasCodeCache(srcPath));
            EXPECT_TRUE(codeCache->HasCodeCache(root /std::filesystem::path("js/loadModuleImported.js")));
        }
    }
}