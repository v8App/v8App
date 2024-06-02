// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <iostream>
#include <string>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "V8Fixture.h"

#include "JSUtilities.h"
#include "CppBridge/V8FunctionTemplate.h"

namespace v8App
{
    namespace JSRuntime
    {
        namespace CppBridge
        {
            using V8FunctionTemplateTest = V8Fixture;

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
                v8::Isolate::Scope iScope(m_Isolate);
                v8::HandleScope scope(m_Isolate);

                //test creating a free function
                v8::Local<v8::FunctionTemplate> func_template = CreateFunctionTemplate(
                    m_Isolate, Utils::MakeCallback(testCppCallback));

                //Test the diapching code for a free function
                v8::Local<v8::ObjectTemplate> global = v8::ObjectTemplate::New(m_Isolate);
                global->Set(JSUtilities::StringToV8(m_Isolate, "test"), func_template);

                v8::Local<v8::Context> context = v8::Context::New(m_Isolate, nullptr, global);
                v8::Context::Scope cScope(context);

                const char csource1[] = R"(
                    test('test', 1, 2.4);
                )";

                v8::TryCatch try_catch(m_Isolate);

                v8::Local<v8::String> source1 = JSUtilities::StringToV8(m_Isolate, csource1);

                v8::Local<v8::Script> script1 = v8::Script::Compile(context, source1).ToLocalChecked();

                v8::Local<v8::Value> result;
                if (script1->Run(m_Isolate->GetCurrentContext()).ToLocal(&result) == false)
                {
                    std::cout << "Script Error: " << JSUtilities::GetStackTrace(context, try_catch) << std::endl;
                     EXPECT_TRUE(false);
               }
            }

            TEST_F(V8FunctionTemplateTest, testFunctionInfoCallback)
            {
                v8::Isolate::Scope iScope(m_Isolate);
                v8::HandleScope scope(m_Isolate);
                v8::Context::Scope cScope(m_Context->GetLocalContext());

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
                v8::Isolate::Scope iScope(m_Isolate);
                v8::HandleScope scope(m_Isolate);
                v8::Context::Scope cScope(m_Context->GetLocalContext());

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