// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "V8Fixture.h"

#include "JSModuleInfo.h"
#include "JSUtilities.h"

namespace v8App
{
    namespace JSRuntime
    {
        using JSModuleInfoTest = V8Fixture;

        namespace JSModuleInfoInternal
        {
            V8MaybeLocalModule UnresovledCallback(
                V8LocalContext inContet, V8LocalString inSpecifier,
                V8LocalFixedArray inImportAttributes, V8LocalModule inReferrer)
            {
                Log::LogMessage msg;
                msg.emplace(Log::MsgKey::Msg, Utils::format("Unresloved callback called"));
                LOG_ERROR(msg);
                return V8MaybeLocalModule();
            }
        }

        TEST_F(JSModuleInfoTest, Constructor)
        {
            v8::Isolate::Scope isolateScope(m_Isolate);
            v8::HandleScope scope(m_Isolate);
            JSModuleInfo info(m_Context);

            EXPECT_EQ("", info.GetName());
            EXPECT_EQ("", info.GetModulePath().string());
            EXPECT_EQ("", info.GetVersion().GetVersionString());
            EXPECT_TRUE(info.GetLocalModule().IsEmpty());

            JSModuleInfo::AttributesInfo attributesInfo = info.GetAttributesInfo();
            EXPECT_EQ(JSModuleInfo::ModuleType::kInvalid, attributesInfo.m_Type);
            EXPECT_EQ("", attributesInfo.m_TypeString);
            EXPECT_EQ("", attributesInfo.m_Module);
        }

        TEST_F(JSModuleInfoTest, GetSetPath)
        {
            v8::Isolate::Scope isolateScope(m_Isolate);
            v8::HandleScope scope(m_Isolate);
            JSModuleInfo info(m_Context);

            std::filesystem::path path("test/path");
            info.SetPath(path);
            EXPECT_EQ(path.string(), info.GetModulePath().string());
        }

        TEST_F(JSModuleInfoTest, GetSetName)
        {
            v8::Isolate::Scope isolateScope(m_Isolate);
            v8::HandleScope scope(m_Isolate);
            JSModuleInfo info(m_Context);

            std::string name("testath");
            info.SetName(name);
            EXPECT_EQ(name, info.GetName());
        }

        TEST_F(JSModuleInfoTest, GetSetVersion)
        {
            v8::Isolate::Scope isolateScope(m_Isolate);
            v8::HandleScope scope(m_Isolate);
            JSModuleInfo info(m_Context);

            std::string version("1.3");
            info.SetVersion(version);
            EXPECT_EQ(version, info.GetVersion().GetVersionString());
        }

        TEST_F(JSModuleInfoTest, GetSetModule)
        {
            v8::Isolate::Scope isolateScope(m_Isolate);
            v8::HandleScope scope(m_Isolate);
            JSModuleInfo info(m_Context);

            V8LocalString moduleName = JSUtilities::StringToV8(m_Isolate, "test");
            auto exportNames = v8::to_array<V8LocalString>(
                {v8::String::NewFromUtf8(m_Isolate, "default").ToLocalChecked()});
            V8LocalModule module = v8::Module::CreateSyntheticModule(m_Isolate, moduleName, exportNames, v8::Module::SyntheticModuleEvaluationSteps());
            EXPECT_FALSE(module.IsEmpty());
            EXPECT_TRUE(info.GetLocalModule().IsEmpty());
            info.SetV8Module(module);
            EXPECT_TRUE(info.GetLocalModule()->IsSyntheticModule());
        }

        TEST_F(JSModuleInfoTest, GetSetV8JSON)
        {
            v8::Isolate::Scope isolateScope(m_Isolate);
            v8::HandleScope scope(m_Isolate);
            JSModuleInfo info(m_Context);
            v8::Context::Scope cScope(m_Context->GetLocalContext());

            V8LocalValue jsonStr = JSUtilities::StringToV8(m_Isolate, "test");
            EXPECT_FALSE(jsonStr.IsEmpty());
            EXPECT_TRUE(info.GetLocalJSON().IsEmpty());
            info.SetV8JSON(jsonStr);
            EXPECT_TRUE(info.GetLocalJSON()->Equals(m_Context->GetLocalContext(), jsonStr).ToChecked());
        }

        TEST_F(JSModuleInfoTest, GetSetClearUnboundScript)
        {
            v8::Isolate::Scope isolateScope(m_Isolate);
            v8::HandleScope scope(m_Isolate);
            JSModuleInfo info(m_Context);
            v8::Context::Scope cScope(m_Context->GetLocalContext());

            std::filesystem::path filePath = m_App->GetAppRoots()->GetAppRoot() / std::filesystem::path("js/UnboundScript.js");
            V8ScriptSourceUniquePtr source = m_App->GetCodeCache()->LoadScriptFile(filePath, m_Isolate);
            V8LocalModule module = v8::ScriptCompiler::CompileModule(m_Isolate, source.get()).ToLocalChecked();
            module->InstantiateModule(m_Context->GetLocalContext(), JSModuleInfoInternal::UnresovledCallback);

            V8LocalUnboundModuleScript unbound = module->GetUnboundModuleScript();
            EXPECT_TRUE(info.GetUnboundScript().IsEmpty());
            info.SetUnboundScript(unbound);
            EXPECT_FALSE(info.GetUnboundScript().IsEmpty());
            info.ClearUnboundScript();
            EXPECT_TRUE(info.GetUnboundScript().IsEmpty());
        }

        TEST_F(JSModuleInfoTest, GetSetAttributesInfo)
        {
            v8::Isolate::Scope isolateScope(m_Isolate);
            v8::HandleScope scope(m_Isolate);
            JSModuleInfo info(m_Context);

            JSModuleInfo::AttributesInfo attributesInfo;
            std::string moduleName = "test";
            attributesInfo.m_Type = JSModuleInfo::ModuleType::kJavascript;
            attributesInfo.m_TypeString = "Javascript";
            attributesInfo.m_Module = moduleName;

            info.SetAttributesInfo(attributesInfo);
            JSModuleInfo::AttributesInfo attributesInfo2 = info.GetAttributesInfo();

            EXPECT_EQ(JSModuleInfo::ModuleType::kJavascript, attributesInfo.m_Type);
            EXPECT_EQ("Javascript", attributesInfo2.m_TypeString);
            EXPECT_EQ(moduleName, attributesInfo2.m_Module);
        }

        TEST_F(JSModuleInfoTest, ModuleTypeToString)
        {
            EXPECT_EQ("Invalid", JSModuleInfo::ModuleTypeToString(JSModuleInfo::ModuleType::kInvalid));
            EXPECT_EQ("Javascript", JSModuleInfo::ModuleTypeToString(JSModuleInfo::ModuleType::kJavascript));
            EXPECT_EQ("JSON", JSModuleInfo::ModuleTypeToString(JSModuleInfo::ModuleType::kJSON));
            EXPECT_EQ("Native", JSModuleInfo::ModuleTypeToString(JSModuleInfo::ModuleType::kNative));
            EXPECT_EQ("Unknown ModuleType enum 10, perhaps need to delcare it's macro in ModuleTypeToString", JSModuleInfo::ModuleTypeToString(static_cast<JSModuleInfo::ModuleType>(10)));
        }

        TEST_F(JSModuleInfoTest, AttributeInfoDoesExtensionMatchType)
        {
            JSModuleInfo::AttributesInfo info;

            info.m_Type = JSModuleInfo::ModuleType::kJavascript;
            EXPECT_TRUE(info.DoesExtensionMatchType(".js"));
            EXPECT_TRUE(info.DoesExtensionMatchType(".mjs"));
            EXPECT_FALSE(info.DoesExtensionMatchType(".txt"));

            info.m_Type = JSModuleInfo::ModuleType::kJSON;
            EXPECT_TRUE(info.DoesExtensionMatchType(".json"));
            EXPECT_FALSE(info.DoesExtensionMatchType(".txt"));
        }
    }
}