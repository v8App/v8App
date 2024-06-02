// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <iostream>
#include <string>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "V8Fixture.h"

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

            class TestV8NativeObj : public V8NativeObject<TestV8NativeObj>
            {
            public:
                TestV8NativeObj() {}
                virtual ~TestV8NativeObj()
                {
                    objInstance = nullptr;
                    secondCallback = true;
                };
                DEF_V8NATIVE_FUNCTIONS(TestV8NativeObj);

                static V8LocalObject Constructor(V8Isolate *isolate)
                {
                    objInstance = new TestV8NativeObj();
                    return objInstance->GetV8NativeObjectInternal(isolate, &TestV8NativeObj::s_V8NativeObjectInfo).ToLocalChecked();
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
                V8ObjectTemplateBuilder builder(inRuntime->GetIsolate(), inGlobal, "TestV8NativeObj");
                v8::Local<v8::ObjectTemplate> tpl =
                    builder.SetConstuctor(&TestV8NativeObj::Constructor)
                        .SetProperty("value", &TestV8NativeObj::GetValue, &TestV8NativeObj::SetValue)
                        .Build();
                inRuntime->SetObjectTemplate(&TestV8NativeObj::s_V8NativeObjectInfo, tpl);
            }

            REGISTER_CLASS_FUNCS(TestV8NativeObj);

            TEST_F(V8NativeObjectTest, TestObjectInfo)
            {
                std::unique_ptr<TestV8NativeObj> test = std::make_unique<TestV8NativeObj>();
                EXPECT_EQ(std::string("TestV8NativeObj"), test->GetTypeName());

                EXPECT_EQ(std::string("TestV8NativeObj"), TestV8NativeObj::s_V8NativeObjectInfo.m_TypeName);
                EXPECT_EQ(&TestV8NativeObj::DeserializeNativeObject, TestV8NativeObj::s_V8NativeObjectInfo.m_Deserializer);
                EXPECT_EQ(&TestV8NativeObj::SerializeNativeObject, TestV8NativeObj::s_V8NativeObjectInfo.m_Serializer);
            }

            TEST_F(V8NativeObjectTest, TestObjectConstructionDestructionInV8)
            {
                JSRuntimeSharedPtr runtime = m_App->GetJSRuntime();
                runtime->CreateGlobalTemplate(true);

                {
                    v8::Isolate *isolate = runtime->GetIsolate();
                    v8::Isolate::Scope iScope(isolate);
                    v8::HandleScope hScope(isolate);

                    JSContextSharedPtr jsContext = m_App->CreateJSContext("default");
                    V8LocalContext context = jsContext->GetLocalContext();
                    v8::Context::Scope cScope(context);

                    const char csource1[] = R"(
                    let testV8NativeObj = new TestV8NativeObj();
                    testV8NativeObj.value = 100;
                )";

                    v8::TryCatch try_catch(isolate);

                    v8::Local<v8::String> source1 = JSUtilities::StringToV8(isolate, csource1);

                    v8::Local<v8::Script> script1 = v8::Script::Compile(context, source1).ToLocalChecked();

                    v8::Local<v8::Value> result;
                    if (script1->Run(context).ToLocal(&result) == false)
                    {
                        std::cout << "Script Error: " << JSUtilities::GetStackTrace(context, try_catch) << std::endl;
                        ASSERT_TRUE(false);
                    }

                    EXPECT_EQ(100, TestV8NativeObj::objInstance->GetValue());

                    const char csource2[] = R"(
                    testV8NativeObj = undefined;
                )";

                    try_catch.Reset();
                    v8::Local<v8::String> source2 = JSUtilities::StringToV8(isolate, csource2);

                    v8::Local<v8::Script> script2 = v8::Script::Compile(context, source2).ToLocalChecked();

                    if (script2->Run(context).ToLocal(&result) == false)
                    {
                        std::cout << "Script Error: " << JSUtilities::GetStackTrace(context, try_catch) << std::endl;
                        EXPECT_TRUE(false);
                    }

                    isolate->RequestGarbageCollectionForTesting(v8::Isolate::GarbageCollectionType::kFullGarbageCollection);
                    EXPECT_EQ(nullptr, TestV8NativeObj::objInstance);
                    EXPECT_TRUE(TestV8NativeObj::secondCallback);
                }
            }
        }
    }
}