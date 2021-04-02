// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <string>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "Utils/CallbackWrapper.h"

namespace v8App
{
    namespace Utils
    {
        int testInt = 0;
        float testFloat = 0;
        std::string testStr;

        void testFunctionVoid(int x, float y)
        {
            testInt = x;
            testFloat = y;
        }

        int testFunctionReturns(float x, std::string str)
        {
            testFloat = x;
            testStr = str;

            return 20;
        }

        class TestMemberFuncs
        {
        public:
            TestMemberFuncs() = default;

            void Test1(int x, float y)
            {
                testInt = x;
                testFloat = y;
            }

            int Test2(float x, std::string str)
            {
                testFloat = x;
                testStr = str;

                return 20;
            }

            int Test3() const
            {
                return 5;
            }

            static void testStatic(int x, int y)
            {
                testInt = y;
                testFloat = 0;
            }
        };

        void resetGlobals()
        {
            testFloat = 0;
            testInt = 0;
            testStr = "";
        }

        template<typename Signature>
        auto TestMoveConstructor(Signature&& inWrapper)
        {
            return new CallbackWrapper<Signature>(std::move(inWrapper));
        }
        TEST(CallWrapperTest, Function)
        {
            resetGlobals();

            //function
            auto voidFunc = MakeCallback(testFunctionVoid);
            voidFunc->invoke(5, 10.0f);

            EXPECT_EQ(5, testInt);
            EXPECT_EQ(10.0f, testFloat);

            resetGlobals();

            //test function that returns vallue
            auto retFunc = MakeCallback(testFunctionReturns);
            EXPECT_EQ(20, retFunc->invoke(5.0f, "test"));
            EXPECT_EQ(5.0f, testFloat);
            EXPECT_EQ("test", testStr);

            resetGlobals();

            //function pointer
            auto voidFunc2 = MakeCallback(&testFunctionVoid);
            voidFunc2->invoke(5, 10.0f);

            EXPECT_EQ(5, testInt);
            EXPECT_EQ(10.0f, testFloat);

            resetGlobals();

            //test function pointer that returns value
            auto retFunc2 = MakeCallback(&testFunctionReturns);
            EXPECT_EQ(20, retFunc2->invoke(5.0f, "test"));
            EXPECT_EQ(5.0f, testFloat);
            EXPECT_EQ("test", testStr);

            resetGlobals();

            auto newFunc2 = TestMoveConstructor(std::move(*retFunc2));
            EXPECT_EQ(20, newFunc2->invoke(5.0f, "test"));
            EXPECT_EQ(5.0f, testFloat);
            EXPECT_EQ("test", testStr);
        }

        TEST(CallWrapperTest, MemberFunctionPtrRawObjectPtr)
        {
            resetGlobals();

            TestMemberFuncs *object = new TestMemberFuncs();

            //test static member function
            auto staticMember = MakeStaticMemberCallback(&TestMemberFuncs::testStatic);
            staticMember->invoke(5, 5);

            EXPECT_EQ(5, testInt);
            EXPECT_EQ(0.0f, testFloat);

            resetGlobals();

            //test unbounded member function passing object
            auto unbounded = MakeMemberCallback(&TestMemberFuncs::Test1);
            unbounded->invoke(object, 5, 10.0f);

            EXPECT_EQ(5, testInt);
            EXPECT_EQ(10.0f, testFloat);

            resetGlobals();

            //test unbounded const member function passing object
            auto unbounded2 = MakeMemberCallback(&TestMemberFuncs::Test3);
            EXPECT_EQ(5, unbounded2->invoke(object));
        }

        TEST(CallWrapperTest, MemberFunctionPtrWeakObjectPtr)
        {
            resetGlobals();

            std::shared_ptr<TestMemberFuncs> shared = std::make_shared<TestMemberFuncs>();
            std::weak_ptr<TestMemberFuncs> weak = shared;

            //test unbounded member function passing object
            auto unbounded = MakeMemberCallback(&TestMemberFuncs::Test1);
            unbounded->invoke(weak, 5, 10.0f);

            EXPECT_EQ(5, testInt);
            EXPECT_EQ(10.0f, testFloat);

            resetGlobals();
            //reset weak since it's was moved
            weak = shared;


            //test unbounded const member function passing object
            auto unbounded2 = MakeMemberCallback(&TestMemberFuncs::Test3);
            EXPECT_EQ(5, unbounded2->invoke(weak));
        }

        TEST(CallWrapperTest, LambdaFunctions)
        {
            resetGlobals();

            //non capture values and void return
            auto lambda1 = [](int x, float y) {
                testInt = x;
                testFloat = y;
            };

            auto func = MakeLambdaCallback(lambda1);
            func->invoke(7, 12.0f);

            EXPECT_EQ(7, testInt);
            EXPECT_EQ(12.0f, testFloat);

            resetGlobals();

            int x = 30;
            float y = 20.0f;

        //caputer all by reference
            auto lambda2 = [&](){
                testInt = x;;
                testFloat = y;
            };

            auto func2 = MakeLambdaCallback(lambda2);
            func2->invoke();

            EXPECT_EQ(30, testInt);
            EXPECT_EQ(20.0f, testFloat);

            resetGlobals();

            x = 14;
            y = 50.0f;

            //capture all by copy
            auto lambda3 = [=](){
                testInt = x;;
                testFloat = y;
            };

            auto func3 = MakeLambdaCallback(lambda3);
            func3->invoke();

            EXPECT_EQ(14, testInt);
            EXPECT_EQ(50.0f, testFloat);

            resetGlobals();

            x = 1;
            y = 50.0f;

            //capture x pass y and return int
           auto lambda4 = [x](float y)->int{
                testInt = x;;
                testFloat = y;
                return 6;
            };


            auto func4 = MakeLambdaCallback(lambda4);
            EXPECT_EQ(6, func4->invoke(24.0f));

            EXPECT_EQ(1, testInt);
            EXPECT_EQ(24.0f, testFloat);
        }

        TEST(CallWrapperTest, StdFunction)
        {
            resetGlobals();

            auto func1 = std::function<void(int,float)>(testFunctionVoid);

            auto callback1 = MakeCallback(func1);
            callback1->invoke(10, 20.0f);

            EXPECT_EQ(10, testInt);
            EXPECT_EQ(20.0f, testFloat);

            resetGlobals();

            auto func2 = std::function<int(int,std::string)>(testFunctionReturns);

            auto callback2 = MakeCallback(func2);
            EXPECT_EQ(20, callback2->invoke(10, "test"));

            EXPECT_EQ(10, testFloat);
            EXPECT_EQ("test", testStr);
        }
    }
}