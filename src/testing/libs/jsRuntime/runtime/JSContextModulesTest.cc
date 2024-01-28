// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <iostream>
#include <string>
#include <fstream>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "Assets/TextAsset.h"

#include "V8TestFixture.h"
#include "JSUtilities.h"
#include "JSContextModules.h"
#include "CppBridge/V8TypeConverter.h"
#include "CppBridge//V8NativeObjectHandle.h"
#include "CppBridge//V8NativeObject.h"
#include "CppBridge/V8ObjectTemplateBuilder.h"

namespace v8App
{
    namespace JSRuntime
    {
        using JSContextModulesTest = V8TestFixture;

        namespace JSModulesTestInternal
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

            class TestObject *constructerCreatedObjectTest = nullptr;

            class TestObject : public CppBridge::V8NativeObject<TestObject>
            {
            public:
                static CppBridge::V8NativeObjectInfo s_V8NativeObjectInfo;

                static CppBridge::V8NativeObjectHandle<TestObject> CreateObject(v8::Isolate *inIsolate)
                {
                    return CreateV8NativeObjHandle(inIsolate, new TestObject());
                }

                static void BuildObjectTemplate(v8::Isolate *inIsolate)
                {
                    TestObject().GetOrCreateObjectTemplate(inIsolate, &s_V8NativeObjectInfo);
                }

                int GetValue() const { return m_Value; }
                void SetValue(int inValue) { m_Value = inValue; }

                static void Constructor(const v8::FunctionCallbackInfo<v8::Value> &inInfo)
                {
                    v8::Isolate *isolate = inInfo.GetIsolate();

                    if (inInfo.IsConstructCall() == false)
                    {
                        JSUtilities::ThrowV8Error(isolate, JSUtilities::V8Errors::TypeError, "must be an instance call (new)");
                        return;
                    }
                    CppBridge::V8NativeObjectHandle<TestObject> instance = CreateV8NativeObjHandle(isolate, new TestObject());
                    constructerCreatedObjectTest = instance.Get();
                    inInfo.GetReturnValue().Set(CppBridge::ConvertToV8(isolate, instance.Get()).ToLocalChecked());
                }

            protected:
                TestObject() : m_Value(0){};
                virtual ~TestObject() override = default;

                CppBridge::V8ObjectTemplateBuilder GetObjectTemplateBuilder(v8::Isolate *inIsolate) override
                {
                    return V8NativeObject<TestObject>::GetObjectTemplateBuilder(inIsolate)
                        .SetConstuctor("TestObject", &TestObject::Constructor)
                        .SetProperty("value", &TestObject::GetValue, &TestObject::SetValue);
                }

            private:
                int m_Value;

                TestObject(const TestObject &) = delete;
                TestObject &operator=(const TestObject &) = delete;
            };

            CppBridge::V8NativeObjectInfo TestObject::s_V8NativeObjectInfo;
        }

        class TestJSContextModules : public JSContextModules
        {
        public:
            TestJSContextModules(JSContextSharedPtr inContext) : JSContextModules(inContext) {}

            JSContextSharedPtr GetJSContext() { return m_Context; }

            JSModuleInfo::AssertionInfo TestGetModuleAssertionInfo(V8LocalContext inContext, V8LocalFixedArray inAssertions, bool inHasPostions)
            {
                return GetModuleAssertionInfo(inContext, inAssertions, inHasPostions);
            }

            JSModuleInfoSharedPtr TestBuildModuleInfo(JSModuleInfo::AssertionInfo &inAssertionInfo, const std::filesystem::path &inImportPath, const std::filesystem::path &inCurrentModPath)
            {
                return BuildModuleInfo(inAssertionInfo, inImportPath, inCurrentModPath);
            }

            bool TestAddModule(const JSModuleInfoSharedPtr &inModule, std::string inFileName, JSModuleInfo::ModuleType inModuleType) { return AddModule(inModule, inFileName, inModuleType); }
            JSModuleInfoSharedPtr TestGetModuleInfoByModule(V8LocalModule inModule, JSModuleInfo::ModuleType inType) { return GetModuleInfoByModule(inModule, inType); }
        };

        TEST_F(JSContextModulesTest, ConstrcutorGetIsolate)
        {
            TestJSContextModules modules(m_Context);

            EXPECT_EQ(m_Context, modules.GetJSContext());
            EXPECT_EQ(m_Isolate, modules.GetIsolate());
        }

        TEST_F(JSContextModulesTest, GeModuleAssertionIndo)
        {
                //Problem is we can't call directly since we can't create FixedArray unless
                // bring in internal headers.
                //Will have to test stuff out through actual files with imports
        }

        TEST_F(JSContextModulesTest, BuildModuleInfo)
        {
            TestJSContextModules jsMOdules(m_Context);
            JSModuleInfoSharedPtr info;

            
        }

        TEST_F(JSContextModulesTest, LoadScriptNoImports)
        {
            std::filesystem::path root = m_App->GetAppRoots()->GetAppRoot();
            TestJSContextModules jsModules(m_Context);

            std::filesystem::path srcPath = root / std::filesystem::path("js/jsModulesTest.js");

            Assets::TextAsset srcFile(srcPath);
            srcFile.SetContent("let x = 5;");
            ASSERT_TRUE(srcFile.WriteAsset());

            V8LocalModule module = jsModules.LoadModule(srcPath).ToLocalChecked();
            JSModuleInfoSharedPtr info = jsModules.GetModuleBySpecifier(srcPath);
            ASSERT_NE(nullptr, info);
            EXPECT_EQ(srcPath, info->GetModulePath());
            EXPECT_EQ(srcPath.stem().string(), info->GetName());
            EXPECT_EQ(module->GetIdentityHash(), info->GetLocalModule()->GetIdentityHash());

            EXPECT_EQ(srcPath.string(), jsModules.GetSpecifierByModule(module));

            info.reset();
            info = jsModules.TestGetModuleInfoByModule(module, JSModuleInfo::ModuleType::kInvalid);
            EXPECT_EQ(module->GetIdentityHash(), info->GetLocalModule()->GetIdentityHash());

            info.reset();
            info = jsModules.TestGetModuleInfoByModule(module, JSModuleInfo::ModuleType::kJavascript);
            EXPECT_EQ(module->GetIdentityHash(), info->GetLocalModule()->GetIdentityHash());

            info.reset();
            info = jsModules.TestGetModuleInfoByModule(module, JSModuleInfo::ModuleType::kJSON);
            EXPECT_EQ(nullptr, info);

            jsModules.ResetModules();
            info = jsModules.TestGetModuleInfoByModule(module, JSModuleInfo::ModuleType::kJavascript);
            EXPECT_EQ(nullptr, info);
        }

        TEST_F(JSContextModulesTest, LoadJSON)
        {
            std::filesystem::path root = m_App->GetAppRoots()->GetAppRoot();
            TestJSContextModules jsModules(m_Context);

            std::filesystem::path srcPath = root / std::filesystem::path("js/jsModulesTest.json");

            Assets::TextAsset srcFile(srcPath);
            std::string jsonStr = "{\"test\":\"test\"}";
            srcFile.SetContent(jsonStr);
            ASSERT_TRUE(srcFile.WriteAsset());

            V8LocalModule module = jsModules.LoadModule(srcPath).ToLocalChecked();
            JSModuleInfoSharedPtr info = jsModules.GetModuleBySpecifier(srcPath);
            ASSERT_NE(nullptr, info);
            EXPECT_EQ(srcPath, info->GetModulePath());
            EXPECT_EQ(srcPath.stem().string(), info->GetName());
            EXPECT_EQ(module->GetIdentityHash(), info->GetLocalModule()->GetIdentityHash());
            V8LocalValue jsonValue = info->GetLocalJSON();
            EXPECT_FALSE(jsonValue.IsEmpty());
            EXPECT_TRUE(jsonValue->IsObject());

            V8Isolate::Scope iScope(m_Isolate);
            v8::HandleScope hScope(m_Isolate);
            V8LocalContext context = m_Context->GetLocalContext();
            v8::Context::Scope cScope(context);

            V8LocalObject jsonObject = jsonValue->ToObject(context).ToLocalChecked();
            v8::Local<v8::Array> keys = jsonObject->GetOwnPropertyNames(context).ToLocalChecked();

            EXPECT_EQ(keys->Length(), 1);
            V8LocalValue key = keys->Get(context, 0).ToLocalChecked();
            V8LocalValue value = jsonObject->Get(context, key).ToLocalChecked();
            EXPECT_EQ("test", JSUtilities::V8ToString(m_Isolate, key));
            EXPECT_EQ("test", JSUtilities::V8ToString(m_Isolate, value));

            EXPECT_EQ(srcPath.string(), jsModules.GetSpecifierByModule(module));

            info.reset();
            info = jsModules.TestGetModuleInfoByModule(module, JSModuleInfo::ModuleType::kInvalid);
            EXPECT_EQ(module->GetIdentityHash(), info->GetLocalModule()->GetIdentityHash());

            info.reset();
            info = jsModules.TestGetModuleInfoByModule(module, JSModuleInfo::ModuleType::kJSON);
            EXPECT_EQ(module->GetIdentityHash(), info->GetLocalModule()->GetIdentityHash());

            info.reset();
            info = jsModules.TestGetModuleInfoByModule(module, JSModuleInfo::ModuleType::kJavascript);
            EXPECT_EQ(nullptr, info);
        }
    }
}