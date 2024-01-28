// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "JSModuleInfo.h"
#include "JSUtilities.h"
#include "V8TestFixture.h"

namespace v8App
{
    namespace JSRuntime
    {
        using JSModuleInfoTest = V8TestFixture;

        TEST_F(JSModuleInfoTest, Constructor)
        {
            v8::Isolate::Scope isolateScope(m_Isolate);
            v8::HandleScope scope(m_Isolate);
            JSModuleInfo info(m_Context);

            EXPECT_EQ("", info.GetName());
            EXPECT_EQ("", info.GetModulePath().string());
            EXPECT_EQ("", info.GetVersion().GetVersionString());
            EXPECT_TRUE(info.GetLocalModule().IsEmpty());

            JSModuleInfo::AssertionInfo assertInfo = info.GetAssertionInfo();
            EXPECT_EQ(JSModuleInfo::ModuleType::kInvalid, assertInfo.m_Type);
            EXPECT_EQ("", assertInfo.m_TypeString);
            EXPECT_EQ("", assertInfo.m_Module);
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
            V8LocalModule module = v8::Module::CreateSyntheticModule(m_Isolate, moduleName, std::vector<V8LocalString>(),  v8::Module::SyntheticModuleEvaluationSteps());
            EXPECT_FALSE(module.IsEmpty());
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
            info.SetV8JSON(jsonStr);
            EXPECT_TRUE(info.GetLocalJSON()->Equals(m_Context->GetLocalContext(), jsonStr).ToChecked());
        }

        TEST_F(JSModuleInfoTest, GetSetAssertionInfo)
        {
            v8::Isolate::Scope isolateScope(m_Isolate);
            v8::HandleScope scope(m_Isolate);
            JSModuleInfo info(m_Context);

            JSModuleInfo::AssertionInfo assertInfo;
            std::string moduleName = "test";
            assertInfo.m_Type = JSModuleInfo::ModuleType::kJavascript;
            assertInfo.m_TypeString = "Javascript";
            assertInfo.m_Module = moduleName;

            info.SetAssertionInfo(assertInfo);
            JSModuleInfo::AssertionInfo assertInfo2 = info.GetAssertionInfo();

            EXPECT_EQ(JSModuleInfo::ModuleType::kJavascript, assertInfo.m_Type);
            EXPECT_EQ("Javascript", assertInfo2.m_TypeString);
            EXPECT_EQ(moduleName, assertInfo2.m_Module);
        }
    }
}