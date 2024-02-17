// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "V8Fixture.h"

#include "CppBridge/V8TypeConverter.h"
#include "CppBridge//V8NativeObjectHandle.h"
#include "CppBridge//V8NativeObject.h"
#include "CppBridge/V8ObjectTemplateBuilder.h"
#include "JSUtilities.h"

namespace v8App
{
    namespace JSRuntime
    {
        using BridgeTest = V8Fixture;

        namespace JSModulesTestInternal
        {

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

        TEST_F(BridgeTest, JSConstructCPPDefinedClass)
        {
            V8Isolate::Scope iScope(m_Isolate);
            v8::HandleScope hScope(m_Isolate);
            v8::TryCatch tryCatch(m_Isolate);
            v8::Context::Scope cScope(m_Context->GetLocalContext());

            JSContextModulesSharedPtr jsModules = m_Context->GetJSModules();
            std::filesystem::path rootPath = m_App->GetAppRoots()->GetAppRoot();

            JSModulesTestInternal::TestObject::BuildObjectTemplate(m_Isolate);

            std::filesystem::path jsFile= rootPath / std::filesystem::path("js/createCppObject.js");
            JSModuleInfoSharedPtr module = jsModules->LoadModule(jsFile);
            ASSERT_NE(nullptr, module);

            ASSERT_TRUE(jsModules->InstantiateModule(module));
            EXPECT_TRUE(jsModules->RunModule(module));

            ASSERT_NE(nullptr, JSModulesTestInternal::constructerCreatedObjectTest);
            EXPECT_EQ(JSModulesTestInternal::constructerCreatedObjectTest->GetValue(), 5);
        }
    }
}