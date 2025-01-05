// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <iostream>
#include <string>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "V8Fixture.h"

#include "JSApp.h"
#include "JSUtilities.h"
#include "CppBridge/V8CppObject.h"
#include "CppBridge/V8ObjectTemplateBuilder.h"
#include "CppBridge/V8CppObjHandle.h"

namespace v8App
{
    namespace JSRuntime
    {
        using V8ObjectTemplateBuilderDeathTest = V8Fixture;

        void Constructor(const V8FuncCallInfoValue &inInfo, V8Isolate *isolate)
        {
            //do nothing
        }

        namespace CppBridge
        {
            TEST_F(V8ObjectTemplateBuilderDeathTest, cosntructorAlreadySet)
            {
                GTEST_FLAG_SET(death_test_style, "threadsafe");
                ASSERT_DEATH({
                    V8ObjectTemplateBuilder builder(m_Isolate, "TestUnamed");
                    builder.SetConstuctor("testUnamed", &Constructor);
                    builder.SetConstuctor("testUnamed", &Constructor);
                std::exit(0); }, "");
            }

            TEST_F(V8ObjectTemplateBuilderDeathTest, constructorNotSetValue)
            {
                GTEST_FLAG_SET(death_test_style, "threadsafe");
                ASSERT_DEATH({
                    V8ObjectTemplateBuilder builder(m_Isolate, "TestUnamed");
                    builder.SetValue("testUnamed", &Constructor);
                std::exit(0); }, "");
            }

            TEST_F(V8ObjectTemplateBuilderDeathTest, constructorNotSetMethod)
            {
                GTEST_FLAG_SET(death_test_style, "threadsafe");
                ASSERT_DEATH({
                    V8ObjectTemplateBuilder builder(m_Isolate, "TestUnamed");
                    builder.SetMethod("testUnamed", &Constructor);
                std::exit(0); }, "");
            }
            TEST_F(V8ObjectTemplateBuilderDeathTest, constructorNotSetProperty)
            {
                GTEST_FLAG_SET(death_test_style, "threadsafe");
                ASSERT_DEATH({
                    V8ObjectTemplateBuilder builder(m_Isolate, "TestUnamed");
                    builder.SetReadOnlyProperty("testUnamed", &Constructor);
                std::exit(0); }, "");
            }
        }
    }
}