// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <iostream>
#include <string>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "V8Fixture.h"

#include "JSApp.h"
#include "CppBridge/CallbackRegistry.h"
#include "CppBridge/V8NativeObject.h"
#include "CppBridge/V8ObjectTemplateBuilder.h"
#include "JSUtilities.h"

namespace v8App
{
    namespace JSRuntime
    {
        namespace CppBridge
        {
            using V8NativeObjectTest = V8Fixture;

            class V8NativeObjectNoOverrides : public V8NativeObjectBase
            {
            public:
                const char *GetTypeName() override { return V8NativeObjectBase::GetTypeName(); }
            };

            class V8NativeObjectOverrides : public V8NativeObjectBase
            {
            public:
                const char *GetTypeName() override { return "TestOverride"; }
            };

            class TestV8NativeObj final : public V8NativeObject<TestV8NativeObj>
            {
            public:
                virtual ~TestV8NativeObj() override 
                {
                    TestV8NativeObj::objInstance = nullptr;
                    TestV8NativeObj::secondCallback = true;
                }

                DEF_V8NATIVE_FUNCTIONS(TestV8NativeObj);

                static V8LObject Constructor(V8Isolate *isolate)
                {
                    JSRuntimeSharedPtr runtime = JSRuntime::GetJSRuntimeFromV8Isolate(isolate);
                    V8LContext context = isolate->GetCurrentContext();
                    V8NativeObjectHandle<TestV8NativeObj> instance = TestV8NativeObj::NewObj(runtime, context);
                    objInstance = instance.Get();
                    return instance.ToV8().As<v8::Object>();
                }

                void SetValue(int inValue) { m_Value = inValue; }
                int GetValue() { return m_Value; }

                static inline TestV8NativeObj *objInstance = nullptr;
                static inline bool secondCallback = false;

            protected:
                int m_Value = 5;
            };

            IMPL_DESERIALIZER(TestV8NativeObj)
            {
            }

            IMPL_SERIALIZER(TestV8NativeObj)
            {
            }

            IMPL_REGISTER_CLASS_FUNCS(TestV8NativeObj)
            {
                CallbackRegistry::Register(Utils::MakeCallback(&TestV8NativeObj::Constructor));
                CallbackRegistry::Register(Utils::MakeCallback(&TestV8NativeObj::SetValue));
                CallbackRegistry::Register(Utils::MakeCallback(&TestV8NativeObj::GetValue));
            }

            IMPL_REGISTER_CLASS_GLOBAL_TEMPLATE(TestV8NativeObj)
            {
                V8ObjectTemplateBuilder builder(inContext->GetIsolate(), inGlobal, "TestV8NativeObj");
                V8LObjTpl tpl =
                    builder.SetConstuctor(&TestV8NativeObj::Constructor, inContext->GetLocalContext())
                        .SetProperty("value", &TestV8NativeObj::GetValue, &TestV8NativeObj::SetValue)
                        .Build();
                inContext->GetJSRuntime()->SetObjectTemplate(&TestV8NativeObj::s_V8CppObjInfo, tpl);
            }

            REGISTER_CLASS_FUNCS_GLOBAL(TestV8NativeObj);

            TEST_F(V8NativeObjectTest, TestObjectInfo)
            {
                EXPECT_EQ(std::string("TestV8NativeObj"), TestV8NativeObj::s_V8CppObjInfo.m_TypeName);
                EXPECT_EQ(&TestV8NativeObj::DeserializeCppObject, TestV8NativeObj::s_V8CppObjInfo.m_Deserializer);
                EXPECT_EQ(&TestV8NativeObj::SerializeCppObject, TestV8NativeObj::s_V8CppObjInfo.m_Serializer);
            }

            TEST_F(V8NativeObjectTest, TestObjectConstructionDestructionInV8)
            {
                JSRuntimeSharedPtr runtime = m_App->GetJSRuntime();

                {
                    V8Isolate *isolate = runtime->GetIsolate();
                    V8IsolateScope iScope(isolate);
                    V8HandleScope hScope(isolate);

                    JSContextSharedPtr jsContext = m_App->CreateJSContext("default", "");
                    V8LContext context = jsContext->GetLocalContext();
                    V8ContextScope cScope(context);

                    const char csource1[] = R"(
                    let testV8NativeObj = new TestV8NativeObj();
                    testV8NativeObj.value = 100;
                )";

                    V8TryCatch try_catch(isolate);

                    V8LString source1 = JSUtilities::StringToV8(isolate, csource1);

                    V8LScript script1 = v8::Script::Compile(context, source1).ToLocalChecked();

                    V8LValue result;
                    if (script1->Run(context).ToLocal(&result) == false)
                    {
                        std::cout << "Script Error: " << JSUtilities::GetStackTrace(context, try_catch) << std::endl;
                        ASSERT_TRUE(false);
                    }

                    EXPECT_EQ(100, TestV8NativeObj::objInstance->GetValue());

                    isolate->RequestGarbageCollectionForTesting(V8Isolate::GarbageCollectionType::kFullGarbageCollection);
                    EXPECT_EQ(nullptr, TestV8NativeObj::objInstance);
                    EXPECT_TRUE(TestV8NativeObj::secondCallback);
                }
            }
        }
    }
}