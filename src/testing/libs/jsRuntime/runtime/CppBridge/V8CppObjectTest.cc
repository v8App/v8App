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
#include "CppBridge/V8CppObject.h"
#include "CppBridge/V8ObjectTemplateBuilder.h"
#include "JSUtilities.h"

namespace v8App
{
    namespace JSRuntime
    {
        namespace CppBridge
        {
            using V8CppObjectTest = V8Fixture;

            class V8CppObjectNoOverrides : public V8CppObjectBase
            {
            public:
                virtual std::string GetTypeName() override { return V8CppObjectBase::GetTypeName(); }
            };

            class V8CppObjectOverrides : public V8CppObjectBase
            {
            public:
                virtual std::string GetTypeName() override { return "TestOverride"; }
            };

            class TestV8CppObject final : public V8CppObject<TestV8CppObject>
            {
            public:
                TestV8CppObject() {}
                virtual ~TestV8CppObject()
                {
                    TestV8CppObject::objInstance = nullptr;
                }

                DEF_V8CPP_OBJ_FUNCTIONS(TestV8CppObject);

                static V8LObject Constructor(V8Isolate *isolate)
                {
                    JSRuntimeSharedPtr runtime = JSRuntime::GetJSRuntimeFromV8Isolate(isolate);
                    V8LContext context = isolate->GetCurrentContext();
                    V8LObject testObject = V8Object::New(isolate);

                    V8CppObjHandle<TestV8CppObject> instance = TestV8CppObject::NewObj(runtime, context, testObject, false);
                    objInstance = instance.Get();
                    return instance.ToV8().As<v8::Object>();
                }

                void SetValue(int inValue) { m_Value = inValue; }
                int GetValue() { return m_Value; }

                void ClearPersistent() { m_CppHolder.Clear(); }

                static inline TestV8CppObject *objInstance = nullptr;

            protected:
                int m_Value = 5;
            };

            IMPL_V8CPPOBJ_DESERIALIZER(TestV8CppObject)
            {
            }

            IMPL_V8CPPOBJ_SERIALIZER(TestV8CppObject)
            {
            }

            IMPL_V8CPPOBJ_REGISTER_CLASS_FUNCS(TestV8CppObject)
            {
                CallbackRegistry::Register(Utils::MakeCallback(&TestV8CppObject::Constructor));
                CallbackRegistry::Register(Utils::MakeCallback(&TestV8CppObject::SetValue));
                CallbackRegistry::Register(Utils::MakeCallback(&TestV8CppObject::GetValue));
            }

            IMPL_V8CPPOBJ_REGISTER_CLASS_GLOBAL_TEMPLATE(TestV8CppObject)
            {
                V8ObjectTemplateBuilder builder(inRuntime->GetIsolate(), "TestV8CppObject");
                V8LFuncTpl tpl =
                    builder.SetConstuctor(&TestV8CppObject::Constructor)
                        .SetProperty("value", &TestV8CppObject::GetValue, &TestV8CppObject::SetValue)
                        .Build();
                inRuntime->SetClassFunctionTemplate("", &TestV8CppObject::s_V8CppObjInfo, tpl);
            }

            REGISTER_CLASS_FUNCS_GLOBAL(TestV8CppObject);

            TEST_F(V8CppObjectTest, TestObjectInfo)
            {
                EXPECT_EQ(std::string("TestV8CppObject"), TestV8CppObject::s_V8CppObjInfo.m_TypeName);
                EXPECT_EQ(&TestV8CppObject::DeserializeCppObject, TestV8CppObject::s_V8CppObjInfo.m_Deserializer);
                EXPECT_EQ(&TestV8CppObject::SerializeCppObject, TestV8CppObject::s_V8CppObjInfo.m_Serializer);
            }

            TEST_F(V8CppObjectTest, TestObjectConstructionDestructionInV8)
            {
                JSRuntimeSharedPtr runtime = m_App->GetMainRuntime();

                {
                    V8Isolate *isolate = runtime->GetIsolate();
                    V8IsolateScope iScope(isolate);
                    V8HandleScope hScope(isolate);

                    JSContextSharedPtr jsContext = m_App->GetMainRuntime()->CreateContext("default", "");
                    V8LContext context = jsContext->GetLocalContext();
                    V8ContextScope cScope(context);

                    const char csource1[] = R"(
                    let testV8CppObject = new TestV8CppObject();
                    testV8CppObject.value = 100;
                )";

                    const char csource2[] = R"(
                    testV8CppObject = undefined;
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

                    EXPECT_EQ(100, TestV8CppObject::objInstance->GetValue());

                    V8LString source2 = JSUtilities::StringToV8(isolate, csource2);

                    V8LScript script2 = v8::Script::Compile(context, source2).ToLocalChecked();

                    if (script2->Run(context).ToLocal(&result) == false)
                    {
                        std::cout << "Script Error: " << JSUtilities::GetStackTrace(context, try_catch) << std::endl;
                        ASSERT_TRUE(false);
                    }
                     TestV8CppObject::objInstance->ClearPersistent();

                    isolate->RequestGarbageCollectionForTesting(V8Isolate::GarbageCollectionType::kFullGarbageCollection);
                    EXPECT_EQ(nullptr, TestV8CppObject::objInstance);
                }
            }
        }
    }
}