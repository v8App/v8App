// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <iostream>
#include <string>

#include "../V8TestFixture.h"
#include "CppBridge/V8NativeObject.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"

namespace v8App
{
    namespace JSRuntime
    {
        namespace CppBridge
        {
            using V8NativeObjectTest = V8TestFixture;

            class V8NativeObjectNoOverrides : public V8NativeObjectBase
            {
            public:
                const char *GetTypeName() override { return V8NativeObjectBase::GetTypeName(); }
                int GetNumberOfInternalFields() override { return V8NativeObjectBase::GetNumberOfInternalFields(); }
            };

            class V8NativeObjectOverrides : public V8NativeObjectBase
            {
            public:
                const char *GetTypeName() override { return "TestOverride"; }
                int GetNumberOfInternalFields() override { return 5; }
            };

            TEST_F(V8NativeObjectTest, test)
            {
                //we only are testing these 2 methods here since the restinvolves 
                //more of a seetup with an object
                V8NativeObjectNoOverrides *test1 = new V8NativeObjectNoOverrides();
                V8NativeObjectOverrides *test2 = new V8NativeObjectOverrides();

                EXPECT_EQ(nullptr, test1->GetTypeName());
                EXPECT_EQ(kMaxReservedInternalFields, test1->GetNumberOfInternalFields());

                EXPECT_EQ("TestOverride", test2->GetTypeName());
                EXPECT_EQ(5, test2->GetNumberOfInternalFields());

                delete test1;
                delete test2;
            }
        }
    }
}