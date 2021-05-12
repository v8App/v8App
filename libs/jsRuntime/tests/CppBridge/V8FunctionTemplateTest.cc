// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <iostream>
#include <string>

#include "../V8TestFixture.h"
#include "CppBridge/V8FunctionTemplate.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"

namespace v8App
{
    namespace JSRuntime
    {
        namespace CppBridge
        {
            using V8FunctionTemplateTest = V8TestFixture;

            void testCppCallback(std::string test, int test1, float test2)
            {
                EXPECT_EQ("test", test);
            }

            class TestClass
            {
            public:
                void Test1()
                {
                }

                void Test2() const
                {
                }

                static void Test3()
                {
                }
            };

            //V* Conversions for TestClass
            template <>
            struct ToReturnsMaybe<TestClass>
            {
                static const bool Value = true;
            };

            //specialization for the test type for the conversion to v8
            template <>
            struct V8TypeConverter<TestClass>
            {
                static v8::MaybeLocal<v8::Value> To(v8::Isolate *inIsolate, const TestClass &inValue)
                {
                    return v8::MaybeLocal<v8::Value>(v8::Object::New(inIsolate));
                }
                static bool From(v8::Isolate *inIsolate, v8::Local<v8::Value> inValue, TestClass *outValue)
                {
                    //no conversion for this test.
                    outValue = nullptr;
                }
            };

            TEST_F(V8FunctionTemplateTest, test)
            {
                v8::HandleScope scope(m_Isolate);

                //test creating a free function
                v8::Local<v8::FunctionTemplate> func_template = CreateFunctionTemplate(
                    m_Runtime->GetIsolate(), Utils::MakeCallback(testCppCallback));

                //test creating a memeber function callback. We'll test it's dispatching in object template test
                //test a non const member function
                v8::Local<v8::FunctionTemplate> func_template2 = CreateFunctionTemplate(
                    m_Runtime->GetIsolate(), Utils::MakeCallback(&TestClass::Test1));

                //test a const member function
                v8::Local<v8::FunctionTemplate> func_template3 = CreateFunctionTemplate(
                    m_Runtime->GetIsolate(), Utils::MakeCallback(&TestClass::Test2));

                //test a static member function
                v8::Local<v8::FunctionTemplate> func_template4 = CreateFunctionTemplate(
                    m_Runtime->GetIsolate(), Utils::MakeCallback(&TestClass::Test3));

                //Test the diapching code for a free function
                v8::Local<v8::ObjectTemplate> global = v8::ObjectTemplate::New(m_Isolate);
                global->Set(v8::String::NewFromUtf8Literal(m_Isolate, "test"), func_template);

                v8::Local<v8::Context> context = v8::Context::New(m_Isolate, nullptr, global);

                const char csource1[] = R"(
                    test('test', 1, 2.4);
                )";

                v8::TryCatch try_catch(m_Isolate);

                v8::Local<v8::String> source1 = v8::String::NewFromUtf8Literal(m_Isolate, csource1);

                v8::Local<v8::Script> script1 = v8::Script::Compile(context, source1).ToLocalChecked();

                v8::Local<v8::Value> result;
                if (script1->Run(m_Isolate->GetCurrentContext()).ToLocal(&result) == false)
                {
                    v8::String::Utf8Value error(m_Isolate, try_catch.Exception());
                    std::cout << "Script Error: " << *error << std::endl;
                }

                //normal we'd be using the pointer to the V8NativeObjectInfo to find/store the template
                int templateFinder = 5;

                //test that nothing is found
                EXPECT_EQ(true, m_Runtime->GetFunctionTemplate(&templateFinder).IsEmpty());

                //now lets set them
                m_Runtime->SetFunctionTemplate(&templateFinder, func_template);

                //should not get back a null
                EXPECT_EQ(false, m_Runtime->GetFunctionTemplate(&templateFinder).IsEmpty());
            }
        }
    }
}