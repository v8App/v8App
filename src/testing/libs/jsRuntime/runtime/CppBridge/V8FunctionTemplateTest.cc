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

            void functionInfoCallback(const V8FuncCallInfoValue &inInfo, std::string test)
            {
                EXPECT_EQ(1, inInfo.Length());
                EXPECT_FALSE(inInfo.IsConstructCall());
                EXPECT_EQ("test", test);
            }

            void functionInfoCallbackAsConstructor(const V8FuncCallInfoValue &inInfo)
            {
                EXPECT_EQ(1, inInfo.Length());
                EXPECT_TRUE(inInfo.IsConstructCall());
            }

            TEST_F(V8FunctionTemplateTest, testFunction)
            {
                V8IsolateScope iScope(m_Isolate);
                V8HandleScope scope(m_Isolate);

                //test creating a free function
                V8LFuncTpl func_template = CreateFunctionTemplate(
                    m_Isolate, Utils::MakeCallback(testCppCallback));

                //Test the diapching code for a free function
                V8LObjTpl global = V8ObjTpl::New(m_Isolate);
                global->Set(JSUtilities::StringToV8(m_Isolate, "test"), func_template);

                V8LContext context = V8Context::New(m_Isolate, nullptr, global);
                V8ContextScope cScope(context);

                const char csource1[] = R"(
                    test('test', 1, 2.4);
                )";

                V8TryCatch try_catch(m_Isolate);

                V8LString source1 = JSUtilities::StringToV8(m_Isolate, csource1);

                V8LScript script1 = v8::Script::Compile(context, source1).ToLocalChecked();

                V8LValue result;
                if (script1->Run(m_Isolate->GetCurrentContext()).ToLocal(&result) == false)
                {
                    std::cout << "Script Error: " << JSUtilities::GetStackTrace(m_Isolate, try_catch) << std::endl;
                     EXPECT_TRUE(false);
               }
            }

            TEST_F(V8FunctionTemplateTest, testFunctionInfoCallback)
            {
                V8IsolateScope iScope(m_Isolate);
                V8HandleScope scope(m_Isolate);
                V8ContextScope cScope(m_Context->GetLocalContext());

                //test creating a free function
                V8LFuncTpl func_template = CreateFunctionTemplate(
                    m_Isolate, Utils::MakeCallback(functionInfoCallback));

                //Test the diapching code for a free function
                V8LObjTpl global = V8ObjTpl::New(m_Isolate);
                global->Set(JSUtilities::StringToV8(m_Isolate, "test"), func_template);

                V8LContext context = V8Context::New(m_Isolate, nullptr, global);

                const char csource1[] = R"(
                    test('test');
                )";

                V8TryCatch try_catch(m_Isolate);

                V8LString source1 = JSUtilities::StringToV8(m_Isolate, csource1);

                V8LScript script1 = v8::Script::Compile(context, source1).ToLocalChecked();

                V8LValue result;
                if (script1->Run(m_Isolate->GetCurrentContext()).ToLocal(&result) == false)
                {
                    V8String::Utf8Value error(m_Isolate, try_catch.Exception());
                    std::cout << "Script Error: " << *error << std::endl;
                    EXPECT_TRUE(false);
                }
            }

            TEST_F(V8FunctionTemplateTest, testFunctionInfoCallbackAsConstructor)
            {
                V8IsolateScope iScope(m_Isolate);
                V8HandleScope scope(m_Isolate);
                V8ContextScope cScope(m_Context->GetLocalContext());

                //test creating a free function

                V8LFuncTpl func_template = CreateFunctionTemplate(
                    m_Isolate, Utils::MakeCallback(functionInfoCallbackAsConstructor), nullptr, true);

                func_template->SetClassName(JSUtilities::StringToV8(m_Isolate, "test"));

                //Test the diapching code for a free function
                V8LObjTpl global = V8ObjTpl::New(m_Isolate);
                global->Set(JSUtilities::StringToV8(m_Isolate, "test"), func_template);

                V8LContext context = V8Context::New(m_Isolate, nullptr, global);

                const char csource1[] = R"(
                    let obj = new test('test');
                )";

                V8TryCatch try_catch(m_Isolate);

                V8LString source1 = JSUtilities::StringToV8(m_Isolate, csource1);

                V8LScript script1 = v8::Script::Compile(context, source1).ToLocalChecked();

                V8LValue result;
                if (script1->Run(m_Isolate->GetCurrentContext()).ToLocal(&result) == false)
                {
                    V8String::Utf8Value error(m_Isolate, try_catch.Exception());
                    std::cout << "Script Error: " << *error << std::endl;
                    EXPECT_TRUE(false);
                }
            }
        }
    }
}