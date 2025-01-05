// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "V8Fixture.h"

#include "Serialization/ReadBuffer.h"
#include "Serialization/WriteBuffer.h"
#include "Utils/Format.h"

#include "JSApp.h"
#include "JSModuleInfo.h"
#include "JSUtilities.h"

namespace v8App
{
    namespace JSRuntime
    {
        using JSModuleInfoTest = V8Fixture;

        namespace JSModuleInfoInternal
        {
            V8MBLModule UnresovledCallback(
                V8LContext inContet, V8LString inSpecifier,
                V8LFixedArray inImportAttributes, V8LModule inReferrer)
            {
                LOG_ERROR(Utils::format("Unresloved callback called"));
                return V8MBLModule();
            }
        }

        TEST_F(JSModuleInfoTest, ModuleTypeToString)
        {
            EXPECT_EQ("Invalid", JSModuleInfo::ModuleTypeToString(JSModuleType::kInvalid));
            EXPECT_EQ("Javascript", JSModuleInfo::ModuleTypeToString(JSModuleType::kJavascript));
            EXPECT_EQ("JSON", JSModuleInfo::ModuleTypeToString(JSModuleType::kJSON));
            EXPECT_EQ("Native", JSModuleInfo::ModuleTypeToString(JSModuleType::kNative));
            EXPECT_EQ("", JSModuleInfo::ModuleTypeToString(JSModuleType::kNoAttribute));
            EXPECT_EQ("", JSModuleInfo::ModuleTypeToString(JSModuleType::kMaxModType));
            EXPECT_EQ("", JSModuleInfo::ModuleTypeToString(static_cast<JSModuleType>(10)));
        }

        TEST_F(JSModuleInfoTest, StringToModuleType)
        {
            EXPECT_EQ(JSModuleType::kInvalid, JSModuleInfo::StringToModuleType("Invalid"));
            EXPECT_EQ(JSModuleType::kJavascript, JSModuleInfo::StringToModuleType("Javascript"));
            EXPECT_EQ(JSModuleType::kJSON, JSModuleInfo::StringToModuleType("JSON"));
            EXPECT_EQ(JSModuleType::kNative, JSModuleInfo::StringToModuleType("Native"));
            EXPECT_EQ(JSModuleType::kInvalid, JSModuleInfo::StringToModuleType(""));
        }

        TEST_F(JSModuleInfoTest, Constructor)
        {
            V8IsolateScope isolateScope(m_Isolate);
            V8HandleScope scope(m_Isolate);
            JSModuleInfo info(m_Context);

            EXPECT_EQ(JSModuleType::kInvalid, info.GetType());
            EXPECT_EQ("", info.GetName());
            EXPECT_EQ("", info.GetModulePath().string());
            EXPECT_EQ("", info.GetVersion().GetVersionString());
            EXPECT_TRUE(info.GetLocalModule().IsEmpty());
            EXPECT_TRUE(info.GetLocalJSON().IsEmpty());
            EXPECT_TRUE(info.GetUnboundScript().IsEmpty());

            JSModuleAttributesInfo attributesInfo = info.GetAttributesInfo();
            EXPECT_EQ(JSModuleType::kNoAttribute, attributesInfo.m_Type);
            EXPECT_EQ("", attributesInfo.m_Module);
            EXPECT_EQ("", attributesInfo.m_Version.GetVersionString());
        }

        TEST_F(JSModuleInfoTest, GetSetPath)
        {
            V8IsolateScope isolateScope(m_Isolate);
            V8HandleScope scope(m_Isolate);
            JSModuleInfo info(m_Context);

            std::filesystem::path path("test/path");
            info.SetPath(path);
            EXPECT_EQ(path.string(), info.GetModulePath().string());
        }

        TEST_F(JSModuleInfoTest, GetSetType)
        {
            V8IsolateScope isolateScope(m_Isolate);
            V8HandleScope scope(m_Isolate);
            JSModuleInfo info(m_Context);

            info.SetType(JSModuleType::kJavascript);
            EXPECT_EQ(JSModuleType::kJavascript, info.GetType());
        }

        TEST_F(JSModuleInfoTest, GetSetName)
        {
            V8IsolateScope isolateScope(m_Isolate);
            V8HandleScope scope(m_Isolate);
            JSModuleInfo info(m_Context);

            std::string name("testath");
            info.SetName(name);
            EXPECT_EQ(name, info.GetName());
        }

        TEST_F(JSModuleInfoTest, GetSetVersion)
        {
            V8IsolateScope isolateScope(m_Isolate);
            V8HandleScope scope(m_Isolate);
            JSModuleInfo info(m_Context);

            std::string version("1.3");
            info.SetVersion(version);
            EXPECT_EQ(version, info.GetVersion().GetVersionString());
        }

        TEST_F(JSModuleInfoTest, GetSetModule)
        {
            V8IsolateScope isolateScope(m_Isolate);
            V8HandleScope scope(m_Isolate);
            JSModuleInfo info(m_Context);

            V8LString moduleName = JSUtilities::StringToV8(m_Isolate, "test");
            auto exportNames = v8::to_array<V8LString>(
                {V8String::NewFromUtf8(m_Isolate, "default").ToLocalChecked()});
            V8LModule module = V8Module::CreateSyntheticModule(m_Isolate, moduleName, exportNames, V8Module::SyntheticModuleEvaluationSteps());
            EXPECT_FALSE(module.IsEmpty());
            EXPECT_TRUE(info.GetLocalModule().IsEmpty());
            info.SetV8Module(module);
            EXPECT_TRUE(info.GetLocalModule()->IsSyntheticModule());
        }

        TEST_F(JSModuleInfoTest, GetSetV8JSON)
        {
            V8IsolateScope isolateScope(m_Isolate);
            V8HandleScope scope(m_Isolate);
            JSModuleInfo info(m_Context);
            V8ContextScope cScope(m_Context->GetLocalContext());

            V8LValue jsonStr = JSUtilities::StringToV8(m_Isolate, "test");
            EXPECT_FALSE(jsonStr.IsEmpty());
            EXPECT_TRUE(info.GetLocalJSON().IsEmpty());
            info.SetV8JSON(jsonStr);
            EXPECT_TRUE(info.GetLocalJSON()->Equals(m_Context->GetLocalContext(), jsonStr).ToChecked());
        }

        TEST_F(JSModuleInfoTest, GetSetClearUnboundScript)
        {
            V8IsolateScope isolateScope(m_Isolate);
            V8HandleScope scope(m_Isolate);
            JSModuleInfo info(m_Context);
            V8ContextScope cScope(m_Context->GetLocalContext());

            std::filesystem::path filePath = m_App->GetAppRoot()->GetAppRoot() / std::filesystem::path("js/UnboundScript.js");
            V8ScriptSourceUniquePtr source = m_App->GetCodeCache()->LoadScriptFile(filePath, m_Isolate);
            V8LModule module = V8ScriptCompiler::CompileModule(m_Isolate, source.get()).ToLocalChecked();
            module->InstantiateModule(m_Context->GetLocalContext(), JSModuleInfoInternal::UnresovledCallback);

            V8LUnboundModScript unbound = module->GetUnboundModuleScript();
            EXPECT_TRUE(info.GetUnboundScript().IsEmpty());
            info.SetUnboundScript(unbound);
            EXPECT_FALSE(info.GetUnboundScript().IsEmpty());
            info.ClearUnboundScript();
            EXPECT_TRUE(info.GetUnboundScript().IsEmpty());
        }

        TEST_F(JSModuleInfoTest, GetSetAttributesInfo)
        {
            V8IsolateScope isolateScope(m_Isolate);
            V8HandleScope scope(m_Isolate);
            JSModuleInfo info(m_Context);

            JSModuleAttributesInfo attributesInfo;
            //we only set a couple of fields for this test
            std::string moduleName = "test";
            attributesInfo.m_Type = JSModuleType::kJavascript;
            attributesInfo.m_Module = moduleName;

            info.SetAttributesInfo(attributesInfo);
            JSModuleAttributesInfo attributesInfo2 = info.GetAttributesInfo();

            EXPECT_EQ(JSModuleType::kJavascript, attributesInfo.m_Type);
            EXPECT_EQ(moduleName, attributesInfo2.m_Module);
        }

        TEST_F(JSModuleInfoTest, Serialization)
        {
            JSModuleInfo info(m_Context);
            JSModuleAttributesInfo aInfo;
            //just set a couple of the fields
            aInfo.m_Module = "testModule";
            aInfo.m_Type = JSModuleType::kJavascript;

            // We don't set any of the v8 values since we need a SnapshotCreator
            //  in order for it to work they'll be tested during snapshot creation
            info.SetType(JSModuleType::kJSON);
            info.SetName("test");
            info.SetPath("testPath");
            info.SetVersion("1.1.1");
            info.SetAttributesInfo(aInfo);

            JSModuleInfo::SnapshotData snapData = info.CreateSnapshotData(V8SnapshotCreatorSharedPtr());
            ASSERT_EQ(JSModuleType::kJSON, snapData.m_Type);
            ASSERT_EQ("test", snapData.m_ModuleName);
            ASSERT_EQ("testPath", snapData.m_Path);
            ASSERT_EQ("1.1.1", snapData.m_Version);
            ASSERT_EQ("testModule", snapData.m_AtrribInfo.m_Module);
            ASSERT_EQ(JSModuleType::kJavascript, snapData.m_AtrribInfo.m_Type);
            //These get fully tested when we test snapshotting
            ASSERT_EQ(false, snapData.m_SaveModule);
            ASSERT_EQ(false, snapData.m_SavedJSON);
            ASSERT_EQ(0, snapData.m_ModuleDataIndex);
            ASSERT_EQ(0, snapData.m_JSONModuleDataIndex);

            Serialization::WriteBuffer wBuffer;
            wBuffer << snapData;
            ASSERT_FALSE(wBuffer.HasErrored());

            JSModuleInfo::SnapshotData restored;
            Serialization::ReadBuffer rBuffer(wBuffer.GetData(), wBuffer.BufferSize());

            rBuffer >> restored;
            ASSERT_FALSE(rBuffer.HasErrored());

            ASSERT_EQ(JSModuleType::kJSON, restored.m_Type);
            ASSERT_EQ("test", restored.m_ModuleName);
            ASSERT_EQ("testPath", restored.m_Path);
            ASSERT_EQ("1.1.1", restored.m_Version);
            ASSERT_EQ("testModule", restored.m_AtrribInfo.m_Module);
            ASSERT_EQ(JSModuleType::kJavascript, restored.m_AtrribInfo.m_Type);
            //These get fully tested when we test snapshotting
            ASSERT_EQ(false, restored.m_SaveModule);
            ASSERT_EQ(false, restored.m_SavedJSON);
            ASSERT_EQ(0, restored.m_ModuleDataIndex);
            ASSERT_EQ(0, restored.m_JSONModuleDataIndex);
        }
    }
}