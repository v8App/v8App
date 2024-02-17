// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <iostream>
#include <string>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "V8Fixture.h"

#include "CppBridge/V8NativeObject.h"

namespace v8App
{
    namespace JSRuntime
    {
        namespace CppBridge
        {
            using V8NativeObjectTest = V8Fixture;

            class V8NativeObjectNoOverrides : public V8NativeObjectBase
            {
            public:
                const char *GetTypeName() override { return V8NativeObjectBase::GetTypeName(); }
            };

            class V8NativeObjectOverrides : public V8NativeObjectBase
            {
            public:
                const char *GetTypeName() override { return "TestOverride"; }
            };

            TEST_F(V8NativeObjectTest, test)
            {
                //we only are testing these 2 methods here since the restinvolves 
                //more of a seetup with an object
                V8NativeObjectNoOverrides *test1 = new V8NativeObjectNoOverrides();
                V8NativeObjectOverrides *test2 = new V8NativeObjectOverrides();

                EXPECT_EQ(nullptr, test1->GetTypeName());

                EXPECT_EQ("TestOverride", test2->GetTypeName());

                delete test1;
                delete test2;
            }
        }
    }
}