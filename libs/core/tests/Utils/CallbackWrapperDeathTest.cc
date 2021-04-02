// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "Utils/CallbackWrapper.h"

namespace v8App
{
    namespace Utils
    {
        int testInt = 0;
        float testFloat = 0.0f;
        std::string testStr;

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

        TEST(CallbackWrapperDeathTest, MemberFunctionRawObjct)
        {
            ASSERT_DEATH({
                TestMemberFuncs *object = NULL;
                auto func = MakeMemberCallback(&TestMemberFuncs::Test1, object);
            },
                         "v8App Log {");

            ASSERT_DEATH({
                TestMemberFuncs *object = NULL;
                auto func = MakeUnboundedMemberCallback(&TestMemberFuncs::Test1);
                func->invoke(object, 10, 10.0f);
            },
                         "v8App Log {");

            ASSERT_DEATH({
                TestMemberFuncs *object = NULL;
                auto func = MakeMemberCallback(&TestMemberFuncs::Test3, object);
                func->invoke(object);
            },
                         "v8App Log {");

         }

        TEST(CallbackWrapperDeathTest, MemberFunctionWeakPtr)
        {
            ASSERT_DEATH({
                std::weak_ptr<TestMemberFuncs> weak_ptr;
                auto func = MakeMemberCallback(&TestMemberFuncs::Test1, std::move(weak_ptr));
                func->invoke(5, 10.0f);
            },
                         "v8App Log {");

            ASSERT_DEATH({
                std::weak_ptr<TestMemberFuncs> weak_ptr;
                auto func = MakeUnboundedMemberCallback(&TestMemberFuncs::Test1);
                func->invoke(weak_ptr, 5, 10.0f);
            },
                         "v8App Log {");
  
            ASSERT_DEATH({
                std::weak_ptr<TestMemberFuncs> weak_ptr;
                auto func = MakeMemberCallback(&TestMemberFuncs::Test3, std::move(weak_ptr));
                func->invoke();
            },
                         "v8App Log {");

        }
    }
}