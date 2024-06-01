// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
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

        TEST(CallWrapperTest, Function)
        {
            resetGlobals();

            // function
            auto voidFunc = MakeCallback(testFunctionVoid);
            EXPECT_FALSE(voidFunc.IsMemberFunction());
            EXPECT_FALSE(voidFunc.IsLambdaFunction());
            EXPECT_FALSE(voidFunc.IsStdFunction());
            EXPECT_TRUE(voidFunc.IsVoid());
            EXPECT_EQ((size_t)&testFunctionVoid, voidFunc.GetFuncAddress());

            voidFunc.Invoke(5, 10.0f);

            EXPECT_EQ(5, testInt);
            EXPECT_EQ(10.0f, testFloat);

            resetGlobals();

            // test function that returns vallue
            auto retFunc = MakeCallback(testFunctionReturns);
            EXPECT_EQ(20, retFunc.Invoke(5.0f, "test"));
            EXPECT_EQ(5.0f, testFloat);
            EXPECT_EQ("test", testStr);
            EXPECT_FALSE(retFunc.IsVoid());
            EXPECT_EQ((size_t)&testFunctionReturns, retFunc.GetFuncAddress());

            resetGlobals();

            // function pointer
            auto voidFunc2 = MakeCallback(&testFunctionVoid);
            EXPECT_EQ((size_t)&testFunctionVoid, voidFunc2.GetFuncAddress());
            voidFunc2.Invoke(5, 10.0f);

            EXPECT_EQ(5, testInt);
            EXPECT_EQ(10.0f, testFloat);

            resetGlobals();

            // test function pointer that returns value
            auto retFunc2 = MakeCallback(&testFunctionReturns);
            EXPECT_EQ(20, retFunc2.Invoke(5.0f, "test"));
            EXPECT_EQ(5.0f, testFloat);
            EXPECT_EQ("test", testStr);
            EXPECT_EQ((size_t)&testFunctionReturns, retFunc2.GetFuncAddress());

            resetGlobals();

            // test copy constructor
            auto newFunc(retFunc2);
            EXPECT_EQ(20, newFunc.Invoke(5.0f, "test"));
            EXPECT_EQ(5.0f, testFloat);
            EXPECT_EQ("test", testStr);
            EXPECT_EQ((size_t)&testFunctionReturns, newFunc.GetFuncAddress());

            resetGlobals();

            // test copy constructor nove
            auto newFunc2(std::move(newFunc));
            EXPECT_EQ(20, newFunc2.Invoke(5.0f, "test"));
            EXPECT_EQ(5.0f, testFloat);
            EXPECT_EQ("test", testStr);
            EXPECT_EQ((size_t)&testFunctionReturns, newFunc2.GetFuncAddress());

            resetGlobals();

            // test operator=
            newFunc = newFunc2;
            EXPECT_EQ(20, newFunc.Invoke(5.0f, "test"));
            EXPECT_EQ(5.0f, testFloat);
            EXPECT_EQ("test", testStr);
            EXPECT_EQ((size_t)&testFunctionReturns, newFunc.GetFuncAddress());

            resetGlobals();

            // test operator= move
            retFunc = std::move(newFunc);
            EXPECT_EQ(20, retFunc.Invoke(5.0f, "test"));
            EXPECT_EQ(5.0f, testFloat);
            EXPECT_EQ("test", testStr);
            EXPECT_EQ((size_t)&testFunctionReturns, retFunc.GetFuncAddress());
        }

        TEST(CallWrapperTest, MemberFunctionPtrRawObjectPtr)
        {
            resetGlobals();

            TestMemberFuncs *object = new TestMemberFuncs();

            // test static member function
            auto staticMember = MakeCallback(&TestMemberFuncs::testStatic);
            EXPECT_FALSE(staticMember.IsMemberFunction());
            EXPECT_FALSE(staticMember.IsLambdaFunction());
            EXPECT_FALSE(staticMember.IsStdFunction());
            EXPECT_TRUE(staticMember.IsVoid());
            // statics get routed as if they were a free function and thus pass the actual function address
            // rather than hash like memeber functions
            EXPECT_EQ((size_t)&TestMemberFuncs::testStatic, staticMember.GetFuncAddress());

            staticMember.Invoke(5, 5);

            EXPECT_EQ(5, testInt);
            EXPECT_EQ(0.0f, testFloat);

            resetGlobals();

            // test void memeber function
            auto voidFunc = MakeCallback(&TestMemberFuncs::Test1);
            voidFunc.Invoke(object, 5, 10.0f);
            EXPECT_TRUE(voidFunc.IsMemberFunction());
            EXPECT_FALSE(voidFunc.IsLambdaFunction());
            EXPECT_FALSE(voidFunc.IsStdFunction());
            EXPECT_TRUE(voidFunc.IsVoid());
            size_t hash = std::hash<std::string>{}(typeid(&TestMemberFuncs::Test1).name());
            EXPECT_EQ(hash, voidFunc.GetFuncAddress());

            EXPECT_EQ(5, testInt);
            EXPECT_EQ(10.0f, testFloat);

            resetGlobals();

            // test memeber function returns
            auto retFunc = MakeCallback(&TestMemberFuncs::Test2);
            EXPECT_EQ(20, retFunc.Invoke(object, 5, "test"));
            EXPECT_FALSE(retFunc.IsVoid());

            EXPECT_EQ(5.0, testFloat);
            EXPECT_EQ("test", testStr);
            hash = std::hash<std::string>{}(typeid(&TestMemberFuncs::Test2).name());
            EXPECT_EQ(hash, retFunc.GetFuncAddress());

            resetGlobals();

            // test const member function passing object
            auto constFunc = MakeCallback(&TestMemberFuncs::Test3);
            EXPECT_EQ(5, constFunc.Invoke(object));
            EXPECT_FALSE(constFunc.IsVoid());
            hash = std::hash<std::string>{}(typeid(&TestMemberFuncs::Test3).name());
            EXPECT_EQ(hash, constFunc.GetFuncAddress());

            resetGlobals();

            // test copy constrcutor

            auto newFunc(retFunc);
            EXPECT_EQ(20, newFunc.Invoke(object, 5, "test"));

            EXPECT_EQ(5.0, testFloat);
            EXPECT_EQ("test", testStr);
            hash = std::hash<std::string>{}(typeid(&TestMemberFuncs::Test2).name());
            EXPECT_EQ(hash, newFunc.GetFuncAddress());

            resetGlobals();

            // test copy move constructor
            auto newFunc2(std::move(retFunc));
            EXPECT_EQ(20, newFunc2.Invoke(object, 5, "test"));

            EXPECT_EQ(5.0, testFloat);
            EXPECT_EQ("test", testStr);
            hash = std::hash<std::string>{}(typeid(&TestMemberFuncs::Test2).name());
            EXPECT_EQ(hash, newFunc2.GetFuncAddress());

            resetGlobals();

            // test operator=
            newFunc = newFunc2;
            EXPECT_EQ(20, newFunc2.Invoke(object, 5, "test"));

            EXPECT_EQ(5.0, testFloat);
            EXPECT_EQ("test", testStr);
            hash = std::hash<std::string>{}(typeid(&TestMemberFuncs::Test2).name());
            EXPECT_EQ(hash, newFunc.GetFuncAddress());

            resetGlobals();

            // test opreator= move
            retFunc = std::move(newFunc2);
            EXPECT_EQ(20, retFunc.Invoke(object, 5, "test"));

            EXPECT_EQ(5.0, testFloat);
            EXPECT_EQ("test", testStr);
            hash = std::hash<std::string>{}(typeid(&TestMemberFuncs::Test2).name());
            EXPECT_EQ(hash, retFunc.GetFuncAddress());
        }

        TEST(CallWrapperTest, MemberFunctionPtrWeakObjectPtrInvoke)
        {
            resetGlobals();

            std::shared_ptr<TestMemberFuncs> shared = std::make_shared<TestMemberFuncs>();
            std::weak_ptr<TestMemberFuncs> weak = shared;

            // test unbounded member function passing object
            auto voidFunc = MakeCallback(&TestMemberFuncs::Test1);
            voidFunc.Invoke(weak, 5, 10.0f);

            EXPECT_EQ(5, testInt);
            EXPECT_EQ(10.0f, testFloat);

            resetGlobals();

            // test unbounded const member function passing object
            auto constFunc = MakeCallback(&TestMemberFuncs::Test3);
            EXPECT_EQ(5, constFunc.Invoke(weak));
        }

        TEST(CallWrapperTest, LambdaFunctions)
        {
            resetGlobals();

            // non capture values and void return
            auto lambda1 = [](int x, float y)
            {
                testInt = x;
                testFloat = y;
            };

            auto func = MakeCallbackForLambda(lambda1);
            EXPECT_FALSE(func.IsMemberFunction());
            EXPECT_TRUE(func.IsLambdaFunction());
            EXPECT_FALSE(func.IsStdFunction());
            EXPECT_TRUE(func.IsVoid());
            size_t hash = std::hash<std::string>{}(typeid(lambda1).name());
            EXPECT_EQ(hash, func.GetFuncAddress());

            func.Invoke(7, 12.0f);

            EXPECT_EQ(7, testInt);
            EXPECT_EQ(12.0f, testFloat);

            resetGlobals();

            int x = 30;
            float y = 20.0f;

            // caputer all by reference
            auto lambda2 = [&]()
            {
                testInt = x;
                testFloat = y;
            };

            auto func2 = MakeCallbackForLambda(lambda2);
            func2.Invoke();
            EXPECT_TRUE(func2.IsVoid());

            EXPECT_EQ(30, testInt);
            EXPECT_EQ(20.0f, testFloat);
            hash = std::hash<std::string>{}(typeid(lambda2).name());
            EXPECT_EQ(hash, func2.GetFuncAddress());

            resetGlobals();

            x = 14;
            y = 50.0f;

            // capture all by copy
            auto lambda3 = [=]()
            {
                testInt = x;
                ;
                testFloat = y;
            };

            auto func3 = MakeCallbackForLambda(lambda3);
            func3.Invoke();
            EXPECT_TRUE(func3.IsVoid());

            EXPECT_EQ(14, testInt);
            EXPECT_EQ(50.0f, testFloat);
            hash = std::hash<std::string>{}(typeid(lambda3).name());
            EXPECT_EQ(hash, func3.GetFuncAddress());

            resetGlobals();

            x = 1;
            y = 50.0f;

            // capture x pass y and return int
            auto lambda4 = [x](float y) -> int
            {
                testInt = x;
                ;
                testFloat = y;
                return 6;
            };

            auto func4 = MakeCallbackForLambda(lambda4);
            EXPECT_EQ(6, func4.Invoke(24.0f));
            EXPECT_FALSE(func4.IsVoid());

            EXPECT_EQ(1, testInt);
            EXPECT_EQ(24.0f, testFloat);
            hash = std::hash<std::string>{}(typeid(lambda4).name());
            EXPECT_EQ(hash, func4.GetFuncAddress());

            resetGlobals();

            // test copy constructor
            auto newFunc(func4);
            EXPECT_EQ(6, newFunc.Invoke(24.0f));

            EXPECT_EQ(1, testInt);
            EXPECT_EQ(24.0f, testFloat);
            hash = std::hash<std::string>{}(typeid(lambda4).name());
            EXPECT_EQ(hash, newFunc.GetFuncAddress());

            resetGlobals();

            // test copy constructor move
            auto newFunc2(std::move(func4));
            EXPECT_EQ(6, newFunc2.Invoke(24.0f));

            EXPECT_EQ(1, testInt);
            EXPECT_EQ(24.0f, testFloat);
            hash = std::hash<std::string>{}(typeid(lambda4).name());
            EXPECT_EQ(hash, newFunc2.GetFuncAddress());

            resetGlobals();
        }

        TEST(CallWrapperTest, StdFunction)
        {
            resetGlobals();

            auto func1 = std::function<void(int, float)>(testFunctionVoid);

            auto callback1 = MakeCallback(func1);
            EXPECT_FALSE(callback1.IsMemberFunction());
            EXPECT_FALSE(callback1.IsLambdaFunction());
            EXPECT_TRUE(callback1.IsStdFunction());
            EXPECT_TRUE(callback1.IsVoid());
            size_t hash = std::hash<std::string>{}(typeid(func1).name());
            EXPECT_EQ(hash, callback1.GetFuncAddress());

            callback1.Invoke(10, 20.0f);

            EXPECT_EQ(10, testInt);
            EXPECT_EQ(20.0f, testFloat);

            resetGlobals();

            auto func2 = std::function<int(float, std::string)>(testFunctionReturns);

            auto callback2 = MakeCallback(func2);
            EXPECT_EQ(20, callback2.Invoke(10, "test"));
            EXPECT_FALSE(callback2.IsVoid());

            EXPECT_EQ(10, testFloat);
            EXPECT_EQ("test", testStr);
            hash = std::hash<std::string>{}(typeid(func2).name());
            EXPECT_EQ(hash, callback2.GetFuncAddress());

            resetGlobals();

            // test copy constructor
            auto newFunc(callback2);
            EXPECT_EQ(20, newFunc.Invoke(10, "test"));

            EXPECT_EQ(10, testFloat);
            EXPECT_EQ("test", testStr);
            hash = std::hash<std::string>{}(typeid(func2).name());
            EXPECT_EQ(hash, newFunc.GetFuncAddress());

            resetGlobals();

            // test copy move construcotr
            auto newFunc2(std::move(callback2));
            EXPECT_EQ(20, newFunc2.Invoke(10, "test"));

            EXPECT_EQ(10, testFloat);
            EXPECT_EQ("test", testStr);
            hash = std::hash<std::string>{}(typeid(func2).name());
            EXPECT_EQ(hash, newFunc2.GetFuncAddress());

            resetGlobals();

            // test operator =
            newFunc = newFunc2;
            EXPECT_EQ(20, newFunc.Invoke(10, "test"));

            EXPECT_EQ(10, testFloat);
            EXPECT_EQ("test", testStr);
            hash = std::hash<std::string>{}(typeid(func2).name());
            EXPECT_EQ(hash, newFunc.GetFuncAddress());

            resetGlobals();

            // test operator= move
            callback2 = std::move(newFunc2);
            EXPECT_EQ(20, callback2.Invoke(10, "test"));

            EXPECT_EQ(10, testFloat);
            EXPECT_EQ("test", testStr);
            hash = std::hash<std::string>{}(typeid(func2).name());
            EXPECT_EQ(hash, callback2.GetFuncAddress());
        }

    }
}