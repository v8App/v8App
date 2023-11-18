// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <iostream>
#include <string>

#include "../V8TestFixture.h"
#include "JSUtilities.h"
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

            void functionInfoCallback(const v8::FunctionCallbackInfo<v8::Value> &inInfo, std::string test)
            {
                EXPECT_EQ(1, inInfo.Length());
                EXPECT_FALSE(inInfo.IsConstructCall());
                EXPECT_EQ("test", test);
            }

            void functionInfoCallbackAsConstructor(const v8::FunctionCallbackInfo<v8::Value> &inInfo)
            {
                EXPECT_EQ(1, inInfo.Length());
                EXPECT_TRUE(inInfo.IsConstructCall());
            }

            TEST_F(V8FunctionTemplateTest, testFunction)
            {
                v8::HandleScope scope(m_Isolate);

                //test creating a free function
                v8::Local<v8::FunctionTemplate> func_template = CreateFunctionTemplate(
                    m_Isolate, Utils::MakeCallback(testCppCallback));

                //we test all the member callbacks in the ObjectTemplateBuilderNow

                //Test the diapching code for a free function
                v8::Local<v8::ObjectTemplate> global = v8::ObjectTemplate::New(m_Isolate);
                global->Set(JSUtilities::StringToV8(m_Isolate, "test"), func_template);

                v8::Local<v8::Context> context = v8::Context::New(m_Isolate, nullptr, global);

                const char csource1[] = R"(
                    test('test', 1, 2.4);
                )";

                v8::TryCatch try_catch(m_Isolate);

                v8::Local<v8::String> source1 = JSUtilities::StringToV8(m_Isolate, csource1);

                v8::Local<v8::Script> script1 = v8::Script::Compile(context, source1).ToLocalChecked();

                v8::Local<v8::Value> result;
                if (script1->Run(m_Isolate->GetCurrentContext()).ToLocal(&result) == false)
                {
                    v8::String::Utf8Value error(m_Isolate, try_catch.Exception());
                    std::cout << "Script Error: " << *error << std::endl;
                     EXPECT_TRUE(false);
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

            TEST_F(V8FunctionTemplateTest, testFunctionInfoCallback)
            {
                v8::HandleScope scope(m_Isolate);

                //test creating a free function
                v8::Local<v8::FunctionTemplate> func_template = CreateFunctionTemplate(
                    m_Isolate, Utils::MakeCallback(functionInfoCallback));

                //Test the diapching code for a free function
                v8::Local<v8::ObjectTemplate> global = v8::ObjectTemplate::New(m_Isolate);
                global->Set(JSUtilities::StringToV8(m_Isolate, "test"), func_template);

                v8::Local<v8::Context> context = v8::Context::New(m_Isolate, nullptr, global);

                const char csource1[] = R"(
                    test('test');
                )";

                v8::TryCatch try_catch(m_Isolate);

                v8::Local<v8::String> source1 = JSUtilities::StringToV8(m_Isolate, csource1);

                v8::Local<v8::Script> script1 = v8::Script::Compile(context, source1).ToLocalChecked();

                v8::Local<v8::Value> result;
                if (script1->Run(m_Isolate->GetCurrentContext()).ToLocal(&result) == false)
                {
                    v8::String::Utf8Value error(m_Isolate, try_catch.Exception());
                    std::cout << "Script Error: " << *error << std::endl;
                    EXPECT_TRUE(false);
                }
            }

            TEST_F(V8FunctionTemplateTest, testFunctionInfoCallbackAsConstructor)
            {
                v8::HandleScope scope(m_Isolate);

                //test creating a free function

                v8::Local<v8::FunctionTemplate> func_template = CreateFunctionTemplate(
                    m_Isolate, Utils::MakeCallback(functionInfoCallbackAsConstructor), nullptr, true);

                func_template->SetClassName(JSUtilities::StringToV8(m_Isolate, "test"));

                //Test the diapching code for a free function
                v8::Local<v8::ObjectTemplate> global = v8::ObjectTemplate::New(m_Isolate);
                global->Set(JSUtilities::StringToV8(m_Isolate, "test"), func_template);

                v8::Local<v8::Context> context = v8::Context::New(m_Isolate, nullptr, global);

                const char csource1[] = R"(
                    let obj = new test('test');
                )";

                v8::TryCatch try_catch(m_Isolate);

                v8::Local<v8::String> source1 = JSUtilities::StringToV8(m_Isolate, csource1);

                v8::Local<v8::Script> script1 = v8::Script::Compile(context, source1).ToLocalChecked();

                v8::Local<v8::Value> result;
                if (script1->Run(m_Isolate->GetCurrentContext()).ToLocal(&result) == false)
                {
                    v8::String::Utf8Value error(m_Isolate, try_catch.Exception());
                    std::cout << "Script Error: " << *error << std::endl;
                    EXPECT_TRUE(false);
                }
            }
        }
    }
}