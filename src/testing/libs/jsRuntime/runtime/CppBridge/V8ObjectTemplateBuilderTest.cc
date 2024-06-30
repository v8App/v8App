// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <iostream>
#include <string>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "V8Fixture.h"

#include "JSApp.h"
#include "JSUtilities.h"
#include "CppBridge/V8NativeObject.h"
#include "CppBridge/V8ObjectTemplateBuilder.h"
#include "CppBridge/V8NativeObjectHandle.h"

namespace v8App
{
    namespace JSRuntime
    {
        using V8ObjectTemplateBuilderTest = V8Fixture;

        namespace CppBridge
        {
            // used to test binding a non mmember function to the object template.
            void TestNonMember() {}

            class TestBase
            {
            public:
                TestBase() = default;
                virtual ~TestBase() = default;

            private:
                TestBase(const TestBase &) = delete;
                TestBase &operator=(const TestBase &) = delete;
            };

            class TestUnnamed *constructerCreatedObjectUnnamed = nullptr;

            class TestUnnamed final : public V8NativeObject<TestUnnamed>
            {
            public:
                int GetValue() const { return m_Value; }
                void SetValue(int inValue) { m_Value = inValue; }

                void TestMethod()
                {
                    m_Value = 42;
                }
                static void TestStatic() {}

                static void Constructor(const V8FuncCallInfoValue &inInfo, V8Isolate *isolate)
                {

                    if (inInfo.IsConstructCall() == false)
                    {
                        JSUtilities::ThrowV8Error(isolate, JSUtilities::V8Errors::TypeError, "must be an instance call (new)");
                        return;
                    }
                    JSRuntimeSharedPtr runtime = JSRuntime::GetJSRuntimeFromV8Isolate(isolate);
                    V8LContext context = isolate->GetCurrentContext();
                    V8NativeObjectHandle<TestUnnamed> instance = TestUnnamed::NewObj(runtime, context);
                    constructerCreatedObjectUnnamed = instance.Get();
                    inInfo.GetReturnValue().Set(ConvertToV8(isolate, instance));
                }

                DEF_V8NATIVE_FUNCTIONS(TestUnnamed);

            private:
                int m_Value;;
            };

            IMPL_DESERIALIZER(TestUnnamed) {}
            IMPL_SERIALIZER(TestUnnamed) {}

            IMPL_REGISTER_CLASS_FUNCS(TestUnnamed)
            {
                CppBridge::CallbackRegistry::Register(Utils::MakeCallback(&TestUnnamed::Constructor));
                CppBridge::CallbackRegistry::Register(Utils::MakeCallback(&TestUnnamed::GetValue));
                CppBridge::CallbackRegistry::Register(Utils::MakeCallback(&TestUnnamed::SetValue));
                CppBridge::CallbackRegistry::Register(Utils::MakeCallback(&TestUnnamed::TestMethod));
                CppBridge::CallbackRegistry::Register(Utils::MakeCallback(&TestUnnamed::TestStatic));
                CppBridge::CallbackRegistry::Register(Utils::MakeCallback(&TestNonMember));
            }

            IMPL_REGISTER_CLASS_GLOBAL_TEMPLATE(TestUnnamed)
            {

                V8ObjectTemplateBuilder builder(inContext->GetIsolate(), inGlobal, "TestUnamed");
                V8LObjTpl tpl =
                    builder.SetConstuctor("testUnamed", &TestUnnamed::Constructor, inContext->GetLocalContext())
                        .SetProperty("value", &TestUnnamed::GetValue, &TestUnnamed::SetValue)
                        .SetMethod("testMember", &TestUnnamed::TestMethod)
                        .SetMethod("testStaticMember", &TestUnnamed::TestStatic)
                        .SetMethod("testNonMember", &TestNonMember)
                        .Build();
                inContext->GetJSRuntime()->SetObjectTemplate(&TestUnnamed::s_V8CppObjInfo, tpl);
            }

            REGISTER_CLASS_FUNCS_GLOBAL(TestUnnamed);

            class TestNamed *constructerCreatedObjectNamed = nullptr;

            class TestNamed final : public V8NativeObject<TestNamed>
            {
            public:
                void TestMethod() {}

                static void Constructor(const V8FuncCallInfoValue &inInfo)
                {
                    V8Isolate *isolate = inInfo.GetIsolate();
                    if (inInfo.IsConstructCall() == false)
                    {
                        JSUtilities::ThrowV8Error(isolate, JSUtilities::V8Errors::TypeError, "must be an instance call (new)");
                        return;
                    }

                    JSRuntimeSharedPtr runtime = JSRuntime::GetJSRuntimeFromV8Isolate(isolate);
                    V8LContext context = isolate->GetCurrentContext();
                    V8NativeObjectHandle<TestNamed> instance = TestNamed::NewObj(runtime, context);
                    constructerCreatedObjectNamed = instance.Get();
                    inInfo.GetReturnValue().Set(ConvertToV8(isolate, instance));
                }

                DEF_V8NATIVE_FUNCTIONS(TestNamed);
            };

            IMPL_DESERIALIZER(TestNamed) {}
            IMPL_SERIALIZER(TestNamed) {}

            IMPL_REGISTER_CLASS_FUNCS(TestNamed)
            {
                CppBridge::CallbackRegistry::Register(Utils::MakeCallback(&TestNamed::Constructor));
                CppBridge::CallbackRegistry::Register(Utils::MakeCallback(&TestNamed::TestMethod));
                CppBridge::CallbackRegistry::Register(Utils::MakeCallback(&TestNonMember));
            }

            IMPL_REGISTER_CLASS_GLOBAL_TEMPLATE(TestNamed)
            {
                V8ObjectTemplateBuilder builder(inContext->GetIsolate(), inGlobal, "TestNamed");
                V8LObjTpl tpl =
                    builder.SetConstuctor("testNamed", &TestNamed::Constructor, inContext->GetLocalContext())
                        .SetMethod("testMember", &TestNamed::TestMethod)
                        .SetMethod("testNonMember", &TestNonMember)
                        .Build();
                inContext->GetJSRuntime()->SetObjectTemplate(&TestNamed::s_V8CppObjInfo, tpl);
            }

            REGISTER_CLASS_FUNCS_GLOBAL(TestNamed);

            class TestMismatch : public V8NativeObject<TestMismatch>
            {
            public:
                DEF_V8NATIVE_FUNCTIONS(TestMismatch);

                static void Constructor(const V8FuncCallInfoValue &inInfo)
                {
                    V8Isolate *isolate = inInfo.GetIsolate();
                    if (inInfo.IsConstructCall() == false)
                    {
                        JSUtilities::ThrowV8Error(isolate, JSUtilities::V8Errors::TypeError, "must be an instance call (new)");
                        return;
                    }

                    JSRuntimeSharedPtr runtime = JSRuntime::GetJSRuntimeFromV8Isolate(isolate);
                    V8LContext context = isolate->GetCurrentContext();
                    V8NativeObjectHandle<TestMismatch> instance = TestMismatch::NewObj(runtime, context);
                    inInfo.GetReturnValue().Set(ConvertToV8(isolate, instance));
                }
            };

            IMPL_DESERIALIZER(TestMismatch) {}
            IMPL_SERIALIZER(TestMismatch) {}

            IMPL_REGISTER_CLASS_GLOBAL_TEMPLATE(TestMismatch)
            {
                V8ObjectTemplateBuilder builder(inContext->GetIsolate(), inGlobal, "TestMismatch");
                V8LObjTpl tpl =
                    builder.SetConstuctor(&TestMismatch::Constructor, inContext->GetLocalContext())
                        .Build();
                inContext->GetJSRuntime()->SetObjectTemplate(&TestMismatch::s_V8CppObjInfo, tpl);
            }

            IMPL_REGISTER_CLASS_FUNCS(TestMismatch)
            {
                CppBridge::CallbackRegistry::Register(Utils::MakeCallback(&TestMismatch::Constructor));
            }

            REGISTER_CLASS_FUNCS_GLOBAL(TestMismatch);

            TEST_F(V8ObjectTemplateBuilderTest, testV8ToFromConversion)
            {
                V8IsolateScope iScope(m_Isolate);
                V8HandleScope scope(m_Isolate);
                V8ContextScope cScope(m_Context->GetLocalContext());

                V8NativeObjectHandle<TestUnnamed> object = TestUnnamed::NewObj(m_Runtime, m_Context->GetLocalContext());

                // test conerting to v8
                V8LValue wrapper = ConvertToV8(m_Isolate, object);
                EXPECT_FALSE(wrapper.IsEmpty());

                // test converting from v8
                TestUnnamed *unwrapped = nullptr;
                EXPECT_TRUE(ConvertFromV8(m_Isolate, wrapper, &unwrapped));
                EXPECT_EQ(object.Get(), unwrapped);

                // test conversion that isn't an object
                V8LValue notObject = V8Number::New(m_Isolate, 10);
                unwrapped = nullptr;
                EXPECT_FALSE(ConvertFromV8(m_Isolate, notObject, &unwrapped));
                EXPECT_EQ(nullptr, unwrapped);

                // test empty object
                unwrapped = nullptr;
                V8LValue emptyObject = v8::Object::New(m_Isolate);
                EXPECT_FALSE(ConvertFromV8(m_Isolate, emptyObject, &unwrapped));
                EXPECT_EQ(nullptr, unwrapped);

                // test wrong native object class
                V8NativeObjectHandle<TestMismatch> instance = TestMismatch::NewObj(m_Runtime, m_Context->GetLocalContext());
                V8LValue wrongType = ConvertToV8(m_Isolate, instance);
                EXPECT_FALSE(wrapper.IsEmpty());
                EXPECT_FALSE(ConvertFromV8(m_Isolate, wrongType, &unwrapped));
                EXPECT_EQ(nullptr, unwrapped);
            }

            TEST_F(V8ObjectTemplateBuilderTest, testProperty)
            {
                V8IsolateScope iScope(m_Isolate);
                V8HandleScope scope(m_Isolate);
                V8ContextScope cScope(m_Context->GetLocalContext());
                V8TryCatch tryCatch(m_Isolate);

                V8NativeObjectHandle<TestUnnamed> object = TestUnnamed::NewObj(m_Runtime, m_Context->GetLocalContext());
                object->SetValue(100);
                EXPECT_EQ(100, object->GetValue());

                V8LString source = JSUtilities::StringToV8(m_Isolate, R"script(
                        (
                            function(obj) {
                                if(obj.value !== 100) {
                                    throw 'Failed';
                                } else {
                                    obj.value = 200;
                                }
                            }
                        )
                    )script");

                EXPECT_FALSE(source.IsEmpty());
                EXPECT_FALSE(tryCatch.HasCaught());

                V8LScript script = v8::Script::Compile(m_Context->GetLocalContext(), source).ToLocalChecked();
                V8LValue value = script->Run(m_Context->GetLocalContext()).ToLocalChecked();
                V8LFunction function;
                EXPECT_TRUE(ConvertFromV8(m_Isolate, value, &function));
                EXPECT_FALSE(function.IsEmpty());

                V8LValue argv[] = {object.ToV8()};

                function->Call(m_Context->GetLocalContext(), v8::Undefined(m_Isolate), 1, argv).ToLocalChecked();
                EXPECT_FALSE(tryCatch.HasCaught());
                EXPECT_EQ("", JSUtilities::GetStackTrace(m_Context->GetLocalContext(), tryCatch));

                EXPECT_EQ(200, object->GetValue());
            }

            std::string getError(V8Isolate *inIsolate, V8LContext inContext, V8LValue inFunction, V8LValue inObject)
            {
                constexpr char source[] = R"script(
                            (
                                function(runFunc, contextObject) {
                                    runFunc.apply(contextObject, []);
                                }
                            )
                    )script";

                V8LString v8Source = JSUtilities::StringToV8(inIsolate, source);
                EXPECT_FALSE(v8Source.IsEmpty());

                V8TryCatch tryCatch(inIsolate);

                V8LScript script = v8::Script::Compile(inContext, v8Source).ToLocalChecked();
                V8LFunction func = script->Run(inContext).ToLocalChecked().As<V8Function>();
                EXPECT_FALSE(func.IsEmpty());

                V8LValue argv[] = {inFunction, inObject};
                func->Call(inContext, v8::Undefined(inIsolate), 2, argv).FromMaybe(V8LValue());
                if (tryCatch.HasCaught() == false)
                {
                    return std::string();
                }

                return JSUtilities::V8ToString(inIsolate, tryCatch.Message()->Get());
            }

            TEST_F(V8ObjectTemplateBuilderTest, InvocationErrorOnUnnamedObjectMethods)
            {
                V8IsolateScope iScope(m_Isolate);
                V8HandleScope scope(m_Isolate);
                JSContextSharedPtr jsContext = m_App->CreateJSContext("InvocationErrorOnUnnamedObjectMethods", "");
                V8LContext context = jsContext->GetLocalContext();
                V8ContextScope cScope(context);

                V8NativeObjectHandle<TestUnnamed> object = TestUnnamed::NewObj(m_Runtime, context);

                V8LObject v8Object = object.ToV8().As<v8::Object>();
                V8LValue memberMethod = v8Object->Get(context, JSUtilities::StringToV8(m_Isolate, "testMember")).ToLocalChecked();
                V8LValue staticMemberMethod = v8Object->Get(context, JSUtilities::StringToV8(m_Isolate, "testStaticMember")).ToLocalChecked();
                V8LValue nonMemberMethod = v8Object->Get(context, JSUtilities::StringToV8(m_Isolate, "testNonMember")).ToLocalChecked();

                EXPECT_TRUE(memberMethod->IsFunction());
                EXPECT_TRUE(nonMemberMethod->IsFunction());

                EXPECT_EQ(std::string(), getError(m_Isolate, context, memberMethod, v8Object));
                EXPECT_EQ(42, object->GetValue());
                EXPECT_EQ(std::string(), getError(m_Isolate, context, staticMemberMethod, v8Object));
                EXPECT_EQ(std::string(), getError(m_Isolate, context, nonMemberMethod, v8Object));

                // pass null object casuing invocation error
                EXPECT_EQ("Uncaught TypeError", getError(m_Isolate, context, memberMethod, v8::Null(m_Isolate)));
                // since the static doesn't require the object should not throw
                EXPECT_EQ(std::string(), getError(m_Isolate, context, staticMemberMethod, v8::Null(m_Isolate)));

                // no invocation error since the method isn't a member method
                EXPECT_EQ(std::string(), getError(m_Isolate, context, nonMemberMethod, v8::Null(m_Isolate)));

                // test calling on the wrong object
                V8LObject wrongObject = v8::Object::New(m_Isolate);
                EXPECT_EQ("Uncaught TypeError", getError(m_Isolate, context, memberMethod, wrongObject));
                // since the static doesn't require the object should not throw
                EXPECT_EQ(std::string(), getError(m_Isolate, context, staticMemberMethod, wrongObject));
                // but non memeber won't throw
                EXPECT_EQ(std::string(), getError(m_Isolate, context, nonMemberMethod, wrongObject));
            }

            TEST_F(V8ObjectTemplateBuilderTest, InvocationErrorsOnNamedObjectMethods)
            {
                V8IsolateScope iScope(m_Isolate);
                V8HandleScope handleScope(m_Isolate);
                JSContextSharedPtr jsContext = m_App->CreateJSContext("InvocationErrorsOnNamedObjectMethods", "");
                V8LContext context = jsContext->GetLocalContext();
                V8ContextScope cScope(context);

                V8NativeObjectHandle<TestNamed> object = TestNamed::NewObj(m_Runtime, m_Context->GetLocalContext());

                V8LObject v8Object = object.ToV8().As<v8::Object>();
                V8LValue memberMethod = v8Object->Get(context, JSUtilities::StringToV8(m_Isolate, "testMember")).ToLocalChecked();
                V8LValue nonMemberMethod = v8Object->Get(context, JSUtilities::StringToV8(m_Isolate, "testNonMember")).ToLocalChecked();

                EXPECT_TRUE(memberMethod->IsFunction());
                EXPECT_TRUE(nonMemberMethod->IsFunction());

                EXPECT_EQ(std::string(), getError(m_Isolate, context, memberMethod, v8Object));
                EXPECT_EQ(std::string(), getError(m_Isolate, context, nonMemberMethod, v8Object));

                // pass null object casuing invocation error
                EXPECT_EQ("Uncaught TypeError", getError(m_Isolate, context, memberMethod, v8::Null(m_Isolate)));

                // no invocation error since the method isn't a member method
                EXPECT_EQ(std::string(), getError(m_Isolate, context, nonMemberMethod, v8::Null(m_Isolate)));

                // test calling on the wrong object
                V8LObject wrongObject = v8::Object::New(m_Isolate);
                EXPECT_EQ("Uncaught TypeError", getError(m_Isolate, context, memberMethod, wrongObject));
                // but non memeber won't throw
                EXPECT_EQ(std::string(), getError(m_Isolate, context, nonMemberMethod, wrongObject));
            }

            TEST_F(V8ObjectTemplateBuilderTest, TestObjectConstructionInJSUnnamed)
            {
                V8IsolateScope iScope(m_Isolate);
                V8HandleScope handleScope(m_Isolate);
                JSContextSharedPtr jsContext = m_App->CreateJSContext("TestObjectConstructionInJSUnnamed", "");
                V8LContext context = jsContext->GetLocalContext();
                V8ContextScope cScope(context);

                const char source[] = R"script(
                        let obj = new testUnamed();
                )script";
                V8LString v8Source = JSUtilities::StringToV8(m_Isolate, source);
                EXPECT_FALSE(v8Source.IsEmpty());

                V8TryCatch tryCatch(m_Isolate);

                V8LScript script = v8::Script::Compile(context, v8Source).ToLocalChecked();
                script->Run(context);
                if (tryCatch.HasCaught())
                {
                    std::string error = JSUtilities::GetStackTrace(m_Context->GetLocalContext(), tryCatch);
                    std::cout << "Script Error: " << error << std::endl;
                    ASSERT_TRUE(false);
                }

                constructerCreatedObjectUnnamed->SetValue(500);

                const char source2[] = R"script(
                        if(obj.value != 500) {
                            throw 'Failed';
                        }
                )script";
                v8Source = JSUtilities::StringToV8(m_Isolate, source2);
                EXPECT_FALSE(v8Source.IsEmpty());

                tryCatch.Reset();

                script = v8::Script::Compile(context, v8Source).ToLocalChecked();
                script->Run(context);
                if (tryCatch.HasCaught())
                {
                    std::string error = JSUtilities::GetStackTrace(m_Context->GetLocalContext(), tryCatch);
                    std::cout << "Script Error: " << error << std::endl;
                    ASSERT_TRUE(false);
                }
            }

            TEST_F(V8ObjectTemplateBuilderTest, TestObjectConstructionInJSNamed)
            {
                V8IsolateScope iScope(m_Isolate);
                V8HandleScope handleScope(m_Isolate);
                JSContextSharedPtr jsContext = m_App->CreateJSContext("TestObjectConstructionInJSNamed", "");
                V8LContext context = jsContext->GetLocalContext();
                V8ContextScope cScopt(context);

                const char source[] = R"script(
                        let obj = new testNamed();
                )script";
                V8LString v8Source = JSUtilities::StringToV8(m_Isolate, source);
                EXPECT_FALSE(v8Source.IsEmpty());

                V8TryCatch tryCatch(m_Isolate);

                V8LScript script = v8::Script::Compile(context, v8Source).ToLocalChecked();
                script->Run(context); //.ToLocalChecked();
                if (tryCatch.HasCaught())
                {
                    std::string error = JSUtilities::GetStackTrace(m_Context->GetLocalContext(), tryCatch);
                    std::cout << "Script Error: " << error << std::endl;
                    ASSERT_TRUE(false);
                }

                EXPECT_NE(nullptr, constructerCreatedObjectUnnamed);
            }
        }
    }
}