// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <iostream>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "V8Fixture.h"

#include "CppBridge/V8Arguments.h"
#include "JSUtilities.h"

namespace v8App
{
    namespace JSRuntime
    {
        namespace CppBridge
        {
            using V8ArgumentsTest = V8Fixture;

            V8Isolate *testIsolate = nullptr;
            struct testData
            {
            };

            testData *testDataPtr = nullptr;

            void testFunctionCallback(const V8FuncCallInfoValue &info)
            {
                //check initial construction
                V8Arguments test(info);
                ASSERT_EQ(3, test.Length());
                EXPECT_EQ(testIsolate, test.GetIsolate());
                ASSERT_FALSE(test.InsufficientArgs());

                //test the data that was passed to the function template
                V8LExternal data;
                EXPECT_TRUE(test.GetData(&data));
                testData *dataPtr = reinterpret_cast<testData *>(data->Value());
                EXPECT_EQ(testDataPtr, dataPtr);

                std::string arg1;
                int arg2;
                //arg 3 is a float we want to trigger the conversion error
                std::string arg3;
                float arg4;

                //test conversion
                EXPECT_EQ(0, test.GetNextArgIndex());
                EXPECT_TRUE(test.GetNextArg(&arg1));
                EXPECT_EQ("test", arg1);

                EXPECT_EQ(1, test.GetNextArgIndex());
                EXPECT_TRUE(test.GetNextArg(&arg2));
                EXPECT_EQ(1, arg2);
                //up to this point we should have had no converrsion errors
                EXPECT_TRUE(test.NoConversionErrors());

                //now we'll trugger a converion error
                EXPECT_FALSE(test.GetNextArg(&arg3));
                EXPECT_FALSE(test.NoConversionErrors());

                //insuffcient should be true since we've goten all the args that were passed
                EXPECT_FALSE(test.GetNextArg(&arg4));
                EXPECT_TRUE(test.InsufficientArgs());

                EXPECT_TRUE(test.Return(arg1));
                //add a test where the converesion fails and return false
            }

            void testFunctionCallback2(const V8FuncCallInfoValue &info)
            {
                V8Arguments test(info);
                ASSERT_EQ(3, test.Length());
                std::string arg1;
                int arg2;
                float arg3;

                EXPECT_FALSE(test.GetNextArg(&arg1));
                EXPECT_TRUE(test.GetNextArg(&arg2));
                EXPECT_TRUE(test.GetNextArg(&arg3));
                EXPECT_EQ(2.4f, arg3);
            }

            void testReturnValue(const V8FuncCallInfoValue &info)
            {
                V8Arguments test(info);
                ASSERT_EQ(1, test.Length());
                std::string arg1;

                EXPECT_TRUE(test.GetNextArg(&arg1));
                EXPECT_EQ("test", arg1);
            }

            void testProperyCallback(const V8PropCallInfoValeu &info)
            {
                //                capturedArguments = new CppArguments(info);
            }

            TEST_F(V8ArgumentsTest, TestFunction)
            {
                //TODO: add test for an class instance and property callbacks
                V8IsolateScope iScope(m_Isolate);
                V8HandleScope scope(m_Isolate);
                V8ContextScope cScope(m_Context->GetLocalContext());

                testIsolate = m_Isolate;

                std::unique_ptr<testData> testUnique = std::make_unique<testData>();
                testDataPtr = testUnique.get();
                V8LExternal external = V8External::New(m_Isolate, testDataPtr);

                V8LObjTpl global = V8ObjTpl::New(m_Isolate);
                global->Set(JSUtilities::StringToV8(m_Isolate, "test"),
                            V8FuncTpl::New(m_Isolate, &testFunctionCallback, external));
                global->Set(JSUtilities::StringToV8(m_Isolate, "test2"),
                            V8FuncTpl::New(m_Isolate, &testFunctionCallback2));
                global->Set(JSUtilities::StringToV8(m_Isolate, "testReturn"),
                            V8FuncTpl::New(m_Isolate, &testReturnValue));

                V8LContext context = V8Context::New(m_Isolate, nullptr, global);

                const char csource1[] = R"(
                    x = test('test', 1, 2.0);
                    testReturn(x);
                )";
                const char csource2[] = R"(
                    test2([], 1, 2.4);
                )";

                V8TryCatch try_catch(m_Isolate);

                V8LString source1 = JSUtilities::StringToV8(m_Isolate, csource1);
                V8LString source2 = JSUtilities::StringToV8(m_Isolate, csource2);

                V8LScript script1 = v8::Script::Compile(context, source1).ToLocalChecked();
                V8LScript script2 = v8::Script::Compile(context, source2).ToLocalChecked();

                V8LValue result;
                if (script1->Run(m_Isolate->GetCurrentContext()).ToLocal(&result) == false)
                {
                    V8String::Utf8Value error(m_Isolate, try_catch.Exception());
                    std::cout << "Script Error: " << *error << std::endl;
                }
                if (script2->Run(m_Isolate->GetCurrentContext()).ToLocal(&result) == false)
                {
                    V8String::Utf8Value error(m_Isolate, try_catch.Exception());
                    std::cout << "Script Error: " << *error << std::endl;
                }
            }
        } // namespace CppBridge
    }     // namespace JSRuntime
} // namespace v8App