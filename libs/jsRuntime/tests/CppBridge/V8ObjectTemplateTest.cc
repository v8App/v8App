// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <iostream>
#include <string>

#include "JSUtilites.h"
#include "CppBridge/V8NativeObject.h"
#include "CppBridge/V8ObjectTemplateBuilder.h"
#include "CppBridge/V8NativeObjectHandle.h"
#include "../V8TestFixture.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"

namespace v8App
{
    namespace JSRuntime
    {
        using V8ObjectTemplateTest = V8TestFixture;

        namespace CppBridge
        {
            //used to test binding a non mmember function to the object template.
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

            class TestUnnamed* constructerCreatedObjectUnnamed = nullptr;

            class TestUnnamed : public V8NativeObject<TestUnnamed>
            {
            public:
                static V8NativeObjectInfo s_V8NativeObjectInfo;

                static V8NativeObjectHandle<TestUnnamed> CreateObject(v8::Isolate *inIsolate)
                {
                    return CreateV8NativeObjHandle(inIsolate, new TestUnnamed());
                }

                static void BuildObjectTemplate(v8::Isolate *inIsolate)
                {
                    TestUnnamed().GetOrCreateObjectTemplate(inIsolate, &s_V8NativeObjectInfo);
                }

                int GetValue() const { return m_Value; }
                void SetValue(int inValue) { m_Value = inValue; }

                void TestMethod()
                {
                    m_Value = 42;
                }
                static void TestStatic() {}

                static void Constructor(const v8::FunctionCallbackInfo<v8::Value> &inInfo)
                {
                    v8::Isolate *isolate = inInfo.GetIsolate();

                    if (inInfo.IsConstructCall() == false)
                    {
                        JSUtilities::ThrowV8Error(isolate, JSUtilities::V8Errors::TypeError, JSUtilities::StringToV8(isolate, "must be an instance call (new)"));
                        return;
                    }
                    V8NativeObjectHandle<TestUnnamed> instance = CreateV8NativeObjHandle(isolate, new TestUnnamed());
                    constructerCreatedObjectUnnamed = instance.Get();
                    inInfo.GetReturnValue().Set(ConvertToV8(isolate, instance.Get()).ToLocalChecked());
                }

            protected:
                TestUnnamed() : m_Value(0){};
                virtual ~TestUnnamed() override = default;

                V8ObjectTemplateBuilder GetObjectTemplateBuilder(v8::Isolate *inIsolate) override
                {
                    return V8NativeObject<TestUnnamed>::GetObjectTemplateBuilder(inIsolate)
                        .SetConstuctor("testUnamed", &TestUnnamed::Constructor)
                        .SetProperty("value", &TestUnnamed::GetValue, &TestUnnamed::SetValue)
                        .SetMethod("testMember", &TestUnnamed::TestMethod)
                        .SetMethod("testStaticMember", &TestUnnamed::TestStatic)
                        .SetMethod("testNonMember", &TestNonMember);
                }

            private:
                int m_Value;

                TestUnnamed(const TestUnnamed &) = delete;
                TestUnnamed &operator=(const TestUnnamed &) = delete;
            };

            V8NativeObjectInfo TestUnnamed::s_V8NativeObjectInfo;

            class TestNamed* constructerCreatedObjectNamed = nullptr;

            class TestNamed : public V8NativeObject<TestNamed>
            {
            public:
                static V8NativeObjectInfo s_V8NativeObjectInfo;

                static V8NativeObjectHandle<TestNamed> CreateObject(v8::Isolate *inIsolate)
                {
                    return CreateV8NativeObjHandle(inIsolate, new TestNamed());
                }

                static void BuildObjectTemplate(v8::Isolate *inIsolate)
                {
                    TestNamed().GetOrCreateObjectTemplate(inIsolate, &s_V8NativeObjectInfo);
                }

                void TestMethod() {}

                static void Constructor(const v8::FunctionCallbackInfo<v8::Value> &inInfo)
                {
                    v8::Isolate *isolate = inInfo.GetIsolate();

                    if (inInfo.IsConstructCall() == false)
                    {
                        JSUtilities::ThrowV8Error(isolate, JSUtilities::V8Errors::TypeError, JSUtilities::StringToV8(isolate, "must be an instance call (new)"));
                        return;
                    }
                    V8NativeObjectHandle<TestNamed> instance = CreateV8NativeObjHandle(isolate, new TestNamed());
                    constructerCreatedObjectNamed = instance.Get();
                    inInfo.GetReturnValue().Set(ConvertToV8(isolate, instance.Get()).ToLocalChecked());
                }

            protected:
                TestNamed() = default;
                ~TestNamed() = default;

                V8ObjectTemplateBuilder GetObjectTemplateBuilder(v8::Isolate *inIsolate) override
                {
                    return V8NativeObject<TestNamed>::GetObjectTemplateBuilder(inIsolate)
                        .SetConstuctor(&TestNamed::Constructor)
                        .SetMethod("testMember", &TestNamed::TestMethod)
                        .SetMethod("testNonMember", &TestNonMember);
                }

                const char *GetTypeName() override { return "TestNamed"; }
            };

            V8NativeObjectInfo TestNamed::s_V8NativeObjectInfo;

            class TestMismatch : public V8NativeObject<TestMismatch>
            {
            public:
                static V8NativeObjectInfo s_V8NativeObjectInfo;
            };

            V8NativeObjectInfo TestMismatch::s_V8NativeObjectInfo;

            TEST_F(V8ObjectTemplateTest, testV8ToFromConversion)
            {
                v8::HandleScope handleScope(m_Isolate);

                V8NativeObjectHandle<TestUnnamed> object = TestUnnamed::CreateObject(m_Isolate);

                //test conerting to v8
                v8::Local<v8::Value> wrapper = ConvertToV8(m_Isolate, object.Get()).ToLocalChecked();
                EXPECT_FALSE(wrapper.IsEmpty());

                //test converting from v8
                TestUnnamed *unwrapped = nullptr;
                EXPECT_TRUE(ConvertFromV8(m_Isolate, wrapper, &unwrapped));
                EXPECT_EQ(object.Get(), unwrapped);

                //test conversion that isn't an object
                v8::Local<v8::Value> notObject = v8::Number::New(m_Isolate, 10);
                unwrapped = nullptr;
                EXPECT_FALSE(ConvertFromV8(m_Isolate, notObject, &unwrapped));
                EXPECT_EQ(nullptr, unwrapped);

                //test empty object
                unwrapped = nullptr;
                v8::Local<v8::Value> emptyObject = v8::Object::New(m_Isolate);
                EXPECT_FALSE(ConvertFromV8(m_Isolate, emptyObject, &unwrapped));
                EXPECT_EQ(nullptr, unwrapped);

                //test wrong native object class
                v8::Local<v8::Value> wrongType = ConvertToV8(m_Isolate, new TestMismatch()).ToLocalChecked();
                EXPECT_FALSE(wrapper.IsEmpty());
                EXPECT_FALSE(ConvertFromV8(m_Isolate, wrongType, &unwrapped));
                EXPECT_EQ(nullptr, unwrapped);
            }

            TEST_F(V8ObjectTemplateTest, testProperty)
            {
                v8::HandleScope handleScope(m_Isolate);
                v8::TryCatch tryCatch(m_Isolate);

                V8NativeObjectHandle<TestUnnamed> object = TestUnnamed::CreateObject(m_Isolate);
                object->SetValue(100);
                EXPECT_EQ(100, object->GetValue());

                v8::Local<v8::String> source = JSUtilities::StringToV8(m_Isolate, R"script(
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

                v8::Local<v8::Script> script = v8::Script::Compile(m_Context.Get(m_Isolate), source).ToLocalChecked();
                v8::Local<v8::Value> value = script->Run(m_Context.Get(m_Isolate)).ToLocalChecked();
                v8::Local<v8::Function> function;
                EXPECT_TRUE(ConvertFromV8(m_Isolate, value, &function));
                EXPECT_FALSE(function.IsEmpty());

                v8::Local<v8::Value> argv[] = {ConvertToV8(m_Isolate, object.Get()).ToLocalChecked()};

                function->Call(m_Context.Get(m_Isolate), v8::Undefined(m_Isolate), 1, argv).ToLocalChecked();
                EXPECT_FALSE(tryCatch.HasCaught());
                EXPECT_EQ("", JSUtilities::GetStackTrace(m_Isolate, tryCatch));

                EXPECT_EQ(200, object->GetValue());
            }

            std::string getError(v8::Isolate *inIsolate, v8::Local<v8::Context> inContext, v8::Local<v8::Value> inFunction, v8::Local<v8::Value> inObject)
            {
                constexpr char source[] = R"script(
                            (
                                function(runFunc, contextObject) {
                                    runFunc.apply(contextObject, []);
                                }
                            )
                    )script";

                v8::Local<v8::String> v8Source = JSUtilities::StringToV8(inIsolate, source);
                EXPECT_FALSE(v8Source.IsEmpty());

                v8::TryCatch tryCatch(inIsolate);

                v8::Local<v8::Script> script = v8::Script::Compile(inContext, v8Source).ToLocalChecked();
                v8::Local<v8::Function> func = script->Run(inContext).ToLocalChecked().As<v8::Function>();
                EXPECT_FALSE(func.IsEmpty());

                v8::Local<v8::Value> argv[] = {inFunction, inObject};
                func->Call(inContext, v8::Undefined(inIsolate), 2, argv).FromMaybe(v8::Local<v8::Value>());
                if (tryCatch.HasCaught() == false)
                {
                    return std::string();
                }

                return JSUtilities::V8ToString(inIsolate, tryCatch.Message()->Get());
            }

            TEST_F(V8ObjectTemplateTest, InvocationErrorOnUnnamedObjectMethods)
            {
                v8::HandleScope handleScope(m_Isolate);
                v8::Local<v8::Context> context = m_Context.Get(m_Isolate);

                V8NativeObjectHandle<TestUnnamed> object = TestUnnamed::CreateObject(m_Isolate);

                v8::Local<v8::Object> v8Object = ConvertToV8(m_Isolate, object.Get()).ToLocalChecked().As<v8::Object>();
                v8::Local<v8::Value> memberMethod = v8Object->Get(context, JSUtilities::StringToV8(m_Isolate, "testMember")).ToLocalChecked();
                v8::Local<v8::Value> staticMemberMethod = v8Object->Get(context, JSUtilities::StringToV8(m_Isolate, "testStaticMember")).ToLocalChecked();
                v8::Local<v8::Value> nonMemberMethod = v8Object->Get(context, JSUtilities::StringToV8(m_Isolate, "testNonMember")).ToLocalChecked();

                EXPECT_TRUE(memberMethod->IsFunction());
                EXPECT_TRUE(nonMemberMethod->IsFunction());

                EXPECT_EQ(std::string(), getError(m_Isolate, context, memberMethod, v8Object));
                EXPECT_EQ(42, object->GetValue());
                EXPECT_EQ(std::string(), getError(m_Isolate, context, staticMemberMethod, v8Object));
                EXPECT_EQ(std::string(), getError(m_Isolate, context, nonMemberMethod, v8Object));

                //pass null object casuing invocation error
                EXPECT_EQ("Uncaught TypeError", getError(m_Isolate, context, memberMethod, v8::Null(m_Isolate)));
                //since the static doesn't require the object should not throw
                EXPECT_EQ(std::string(), getError(m_Isolate, context, staticMemberMethod, v8::Null(m_Isolate)));

                //no invocation error since the method isn't a member method
                EXPECT_EQ(std::string(), getError(m_Isolate, context, nonMemberMethod, v8::Null(m_Isolate)));

                //test calling on the wrong object
                v8::Local<v8::Object> wrongObject = v8::Object::New(m_Isolate);
                EXPECT_EQ("Uncaught TypeError", getError(m_Isolate, context, memberMethod, wrongObject));
                //since the static doesn't require the object should not throw
                EXPECT_EQ(std::string(), getError(m_Isolate, context, staticMemberMethod, wrongObject));
                //but non memeber won't throw
                EXPECT_EQ(std::string(), getError(m_Isolate, context, nonMemberMethod, wrongObject));
            }

            TEST_F(V8ObjectTemplateTest, InvocationErrorsOnNamedObjectMethods)
            {
                v8::HandleScope handleScope(m_Isolate);
                v8::Local<v8::Context> context = m_Context.Get(m_Isolate);

                V8NativeObjectHandle<TestNamed> object = TestNamed::CreateObject(m_Isolate);

                v8::Local<v8::Object> v8Object = ConvertToV8(m_Isolate, object.Get()).ToLocalChecked().As<v8::Object>();
                v8::Local<v8::Value> memberMethod = v8Object->Get(context, JSUtilities::StringToV8(m_Isolate, "testMember")).ToLocalChecked();
                v8::Local<v8::Value> nonMemberMethod = v8Object->Get(context, JSUtilities::StringToV8(m_Isolate, "testNonMember")).ToLocalChecked();

                EXPECT_TRUE(memberMethod->IsFunction());
                EXPECT_TRUE(nonMemberMethod->IsFunction());

                EXPECT_EQ(std::string(), getError(m_Isolate, context, memberMethod, v8Object));
                EXPECT_EQ(std::string(), getError(m_Isolate, context, nonMemberMethod, v8Object));

                //pass null object casuing invocation error
                EXPECT_EQ("Uncaught TypeError", getError(m_Isolate, context, memberMethod, v8::Null(m_Isolate)));

                //no invocation error since the method isn't a member method
                EXPECT_EQ(std::string(), getError(m_Isolate, context, nonMemberMethod, v8::Null(m_Isolate)));

                //test calling on the wrong object
                v8::Local<v8::Object> wrongObject = v8::Object::New(m_Isolate);
                EXPECT_EQ("Uncaught TypeError", getError(m_Isolate, context, memberMethod, wrongObject));
                //but non memeber won't throw
                EXPECT_EQ(std::string(), getError(m_Isolate, context, nonMemberMethod, wrongObject));
            }

            TEST_F(V8ObjectTemplateTest, TestObjectConstructionInJSUnnamed)
            {
                v8::HandleScope handleScope(m_Isolate);
                v8::Local<v8::Context> context = m_Context.Get(m_Isolate);
                TestUnnamed::BuildObjectTemplate(m_Isolate);

                const char source[] = R"script(
                        let obj = new testUnamed();
                )script";
                v8::Local<v8::String> v8Source = JSUtilities::StringToV8(m_Isolate, source);
                EXPECT_FALSE(v8Source.IsEmpty());

                v8::TryCatch tryCatch(m_Isolate);

                v8::Local<v8::Script> script = v8::Script::Compile(context, v8Source).ToLocalChecked();
                script->Run(context);
                if (tryCatch.HasCaught())
                {
                    std::string error = JSUtilities::GetStackTrace(m_Isolate, tryCatch);
                    std::cout << "Script Error: " << error << std::endl;
                    ASSERT_TRUE(false);
                }

                EXPECT_NE(nullptr, constructerCreatedObjectUnnamed);

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
                    std::string error = JSUtilities::GetStackTrace(m_Isolate, tryCatch);
                    std::cout << "Script Error: " << error << std::endl;
                    ASSERT_TRUE(false);
                }
            }

            TEST_F(V8ObjectTemplateTest, TestObjectConstructionInJSNamed)
            {
                v8::HandleScope handleScope(m_Isolate);
                v8::Local<v8::Context> context = m_Context.Get(m_Isolate);

                //V8NativeObjectHandle<TestNamed> handle = TestNamed::CreateObject(m_Isolate);
                TestNamed::BuildObjectTemplate(m_Isolate);

                const char source[] = R"script(
                        let obj = new TestNamed();
                )script";
                v8::Local<v8::String> v8Source = JSUtilities::StringToV8(m_Isolate, source);
                EXPECT_FALSE(v8Source.IsEmpty());

                v8::TryCatch tryCatch(m_Isolate);

                v8::Local<v8::Script> script = v8::Script::Compile(context, v8Source).ToLocalChecked();
                script->Run(context); //.ToLocalChecked();
                if (tryCatch.HasCaught())
                {
                    std::string error = JSUtilities::GetStackTrace(m_Isolate, tryCatch);
                    std::cout << "Script Error: " << error << std::endl;
                    ASSERT_TRUE(false);
                }

                EXPECT_NE(nullptr, constructerCreatedObjectUnnamed);
            }
        }
    }
}