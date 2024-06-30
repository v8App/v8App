// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "V8Fixture.h"

#include "CppBridge/V8TypeConverter.h"

namespace v8App
{
    namespace JSRuntime
    {
        namespace CppBridge
        {
            using ConverterTests = V8Fixture;

            TEST_F(ConverterTests, Bool)
            {
                V8IsolateScope iScope(m_Isolate);
                V8HandleScope scope(m_Isolate);
                V8ContextScope cScope(m_Context->GetLocalContext());

                EXPECT_TRUE(V8TypeConverter<bool>::To(m_Isolate, true)->StrictEquals(V8Boolean::New(m_Isolate, true)));
                EXPECT_TRUE(V8TypeConverter<bool>::To(m_Isolate, false)->StrictEquals(V8Boolean::New(m_Isolate, false)));

                bool returnedValue = false;

                EXPECT_TRUE(V8TypeConverter<bool>::From(m_Isolate, V8Boolean::New(m_Isolate, false).As<V8Value>(), &returnedValue));
                EXPECT_EQ(false, returnedValue);

                EXPECT_TRUE(V8TypeConverter<bool>::From(m_Isolate, V8Boolean::New(m_Isolate, true).As<V8Value>(), &returnedValue));
                EXPECT_EQ(true, returnedValue);

                EXPECT_TRUE(V8TypeConverter<bool>::From(m_Isolate, V8Number::New(m_Isolate, 0).As<V8Value>(), &returnedValue));
                EXPECT_EQ(false, returnedValue);

                EXPECT_TRUE(V8TypeConverter<bool>::From(m_Isolate, V8Number::New(m_Isolate, 5).As<V8Value>(), &returnedValue));
                EXPECT_EQ(true, returnedValue);

                EXPECT_TRUE(V8TypeConverter<bool>::From(m_Isolate, V8Number::New(m_Isolate, -5).As<V8Value>(), &returnedValue));
                EXPECT_EQ(true, returnedValue);

                EXPECT_TRUE(V8TypeConverter<bool>::From(m_Isolate, V8Number::New(m_Isolate, 0.1).As<V8Value>(), &returnedValue));
                EXPECT_EQ(true, returnedValue);

                EXPECT_TRUE(V8TypeConverter<bool>::From(m_Isolate, V8String::NewFromUtf8(m_Isolate, "", v8::NewStringType::kNormal).ToLocalChecked().As<V8Value>(), &returnedValue));
                EXPECT_EQ(false, returnedValue);

                EXPECT_TRUE(V8TypeConverter<bool>::From(m_Isolate, V8String::NewFromUtf8(m_Isolate, "foo", v8::NewStringType::kNormal).ToLocalChecked().As<V8Value>(), &returnedValue));
                EXPECT_EQ(true, returnedValue);

                EXPECT_TRUE(V8TypeConverter<bool>::From(m_Isolate, v8::Object::New(m_Isolate).As<V8Value>(), &returnedValue));
                EXPECT_EQ(true, returnedValue);

                EXPECT_TRUE(V8TypeConverter<bool>::From(m_Isolate, v8::Null(m_Isolate).As<V8Value>(), &returnedValue));
                EXPECT_EQ(false, returnedValue);

                EXPECT_TRUE(V8TypeConverter<bool>::From(m_Isolate, v8::Undefined(m_Isolate).As<V8Value>(), &returnedValue));
                EXPECT_EQ(false, returnedValue);
            }

            TEST_F(ConverterTests, Int32)
            {
                V8IsolateScope iScope(m_Isolate);
                V8HandleScope scope(m_Isolate);
                V8ContextScope cScope(m_Context->GetLocalContext());

                EXPECT_TRUE(V8TypeConverter<int32_t>::To(m_Isolate, -5)->StrictEquals(V8Integer::New(m_Isolate, -5)));
                EXPECT_TRUE(V8TypeConverter<int32_t>::To(m_Isolate, 0)->StrictEquals(V8Integer::New(m_Isolate, 0)));
                EXPECT_TRUE(V8TypeConverter<int32_t>::To(m_Isolate, 5)->StrictEquals(V8Integer::New(m_Isolate, 5)));

                int32_t returnedValue = 0;

                EXPECT_FALSE(V8TypeConverter<int32_t>::From(m_Isolate, V8Boolean::New(m_Isolate, false).As<V8Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue);

                returnedValue = 0;
                EXPECT_FALSE(V8TypeConverter<int32_t>::From(m_Isolate, V8Boolean::New(m_Isolate, true).As<V8Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue);

                returnedValue = 0;
                EXPECT_TRUE(V8TypeConverter<int32_t>::From(m_Isolate, V8Integer::New(m_Isolate, 0).As<V8Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue);

                returnedValue = 0;
                EXPECT_TRUE(V8TypeConverter<int32_t>::From(m_Isolate, V8Integer::New(m_Isolate, 5).As<V8Value>(), &returnedValue));
                EXPECT_EQ(5, returnedValue);

                returnedValue = 0;
                EXPECT_TRUE(V8TypeConverter<int32_t>::From(m_Isolate, V8Integer::New(m_Isolate, -5).As<V8Value>(), &returnedValue));
                EXPECT_EQ(-5, returnedValue);

                returnedValue = 0;
                //this works cause it's a whole integer
                EXPECT_TRUE(V8TypeConverter<int32_t>::From(m_Isolate, V8Number::New(m_Isolate, -5).As<V8Value>(), &returnedValue));
                EXPECT_EQ(-5, returnedValue);

                returnedValue = 0;
                EXPECT_FALSE(V8TypeConverter<int32_t>::From(m_Isolate, V8Number::New(m_Isolate, 0.1).As<V8Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue);

                returnedValue = 0;
                EXPECT_FALSE(V8TypeConverter<int32_t>::From(m_Isolate, V8String::NewFromUtf8(m_Isolate, "", v8::NewStringType::kNormal).ToLocalChecked().As<V8Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue);

                returnedValue = 0;
                EXPECT_FALSE(V8TypeConverter<int32_t>::From(m_Isolate, V8String::NewFromUtf8(m_Isolate, "foo", v8::NewStringType::kNormal).ToLocalChecked().As<V8Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue);

                returnedValue = 0;
                EXPECT_FALSE(V8TypeConverter<int32_t>::From(m_Isolate, v8::Object::New(m_Isolate).As<V8Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue);

                returnedValue = 0;
                EXPECT_FALSE(V8TypeConverter<int32_t>::From(m_Isolate, v8::Null(m_Isolate).As<V8Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue);

                returnedValue = 0;
                EXPECT_FALSE(V8TypeConverter<int32_t>::From(m_Isolate, v8::Undefined(m_Isolate).As<V8Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue);
            }

            TEST_F(ConverterTests, UInt32)
            {
                V8IsolateScope iScope(m_Isolate);
                V8HandleScope scope(m_Isolate);
                V8ContextScope cScope(m_Context->GetLocalContext());

                EXPECT_TRUE(V8TypeConverter<uint32_t>::To(m_Isolate, 0)->StrictEquals(V8Integer::New(m_Isolate, 0)));
                EXPECT_TRUE(V8TypeConverter<uint32_t>::To(m_Isolate, 5)->StrictEquals(V8Integer::New(m_Isolate, 5)));

                uint32_t returnedValue = 0;

                EXPECT_FALSE(V8TypeConverter<uint32_t>::From(m_Isolate, V8Boolean::New(m_Isolate, false).As<V8Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue);

                returnedValue = 0;
                EXPECT_FALSE(V8TypeConverter<uint32_t>::From(m_Isolate, V8Boolean::New(m_Isolate, true).As<V8Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue);

                returnedValue = 0;
                EXPECT_TRUE(V8TypeConverter<uint32_t>::From(m_Isolate, V8Integer::New(m_Isolate, 0).As<V8Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue);

                returnedValue = 0;
                EXPECT_TRUE(V8TypeConverter<uint32_t>::From(m_Isolate, V8Integer::New(m_Isolate, 5).As<V8Value>(), &returnedValue));
                EXPECT_EQ(5, returnedValue);

                returnedValue = 0;
                EXPECT_FALSE(V8TypeConverter<uint32_t>::From(m_Isolate, V8Integer::New(m_Isolate, -5).As<V8Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue);

                returnedValue = 0;
                EXPECT_TRUE(V8TypeConverter<uint32_t>::From(m_Isolate, V8Number::New(m_Isolate, 5).As<V8Value>(), &returnedValue));
                EXPECT_EQ(5, returnedValue);

                returnedValue = 0;
                EXPECT_FALSE(V8TypeConverter<uint32_t>::From(m_Isolate, V8Number::New(m_Isolate, -5).As<V8Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue);

                returnedValue = 0;
                EXPECT_FALSE(V8TypeConverter<uint32_t>::From(m_Isolate, V8Number::New(m_Isolate, 0.1).As<V8Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue);

                returnedValue = 0;
                EXPECT_FALSE(V8TypeConverter<uint32_t>::From(m_Isolate, V8String::NewFromUtf8(m_Isolate, "", v8::NewStringType::kNormal).ToLocalChecked().As<V8Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue);

                returnedValue = 0;
                EXPECT_FALSE(V8TypeConverter<uint32_t>::From(m_Isolate, V8String::NewFromUtf8(m_Isolate, "foo", v8::NewStringType::kNormal).ToLocalChecked().As<V8Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue);

                returnedValue = 0;
                EXPECT_FALSE(V8TypeConverter<uint32_t>::From(m_Isolate, v8::Object::New(m_Isolate).As<V8Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue);

                returnedValue = 0;
                EXPECT_FALSE(V8TypeConverter<uint32_t>::From(m_Isolate, v8::Null(m_Isolate).As<V8Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue);

                returnedValue = 0;
                EXPECT_FALSE(V8TypeConverter<uint32_t>::From(m_Isolate, v8::Undefined(m_Isolate).As<V8Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue);
            }

            TEST_F(ConverterTests, Int64)
            {
                V8IsolateScope iScope(m_Isolate);
                V8HandleScope scope(m_Isolate);
                V8ContextScope cScope(m_Context->GetLocalContext());

                EXPECT_TRUE(V8TypeConverter<int64_t>::To(m_Isolate, -5)->StrictEquals(V8Integer::New(m_Isolate, -5)));
                EXPECT_TRUE(V8TypeConverter<int64_t>::To(m_Isolate, 0)->StrictEquals(V8Integer::New(m_Isolate, 0)));
                EXPECT_TRUE(V8TypeConverter<int64_t>::To(m_Isolate, 5)->StrictEquals(V8Integer::New(m_Isolate, 5)));

                int64_t returnedValue = 0;

                EXPECT_FALSE(V8TypeConverter<int64_t>::From(m_Isolate, V8Boolean::New(m_Isolate, false).As<V8Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue);

                returnedValue = 0;
                EXPECT_FALSE(V8TypeConverter<int64_t>::From(m_Isolate, V8Boolean::New(m_Isolate, true).As<V8Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue);

                returnedValue = 0;
                EXPECT_TRUE(V8TypeConverter<int64_t>::From(m_Isolate, V8Integer::New(m_Isolate, 0).As<V8Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue);

                returnedValue = 0;
                EXPECT_TRUE(V8TypeConverter<int64_t>::From(m_Isolate, V8Number::New(m_Isolate, 4294967297).As<V8Value>(), &returnedValue));
                EXPECT_EQ(4294967297, returnedValue);

                returnedValue = 0;
                EXPECT_TRUE(V8TypeConverter<int64_t>::From(m_Isolate, V8Number::New(m_Isolate, -4294967297).As<V8Value>(), &returnedValue));
                EXPECT_EQ(-4294967297, returnedValue);

                returnedValue = 0;
                //because of how the value is conveted from a number it'll work and just return the int portion
                EXPECT_TRUE(V8TypeConverter<int64_t>::From(m_Isolate, V8Number::New(m_Isolate, 20345.1).As<V8Value>(), &returnedValue));
                EXPECT_EQ(20345, returnedValue);

                returnedValue = 0;
                EXPECT_FALSE(V8TypeConverter<int64_t>::From(m_Isolate, V8String::NewFromUtf8(m_Isolate, "", v8::NewStringType::kNormal).ToLocalChecked().As<V8Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue);

                returnedValue = 0;
                EXPECT_FALSE(V8TypeConverter<int64_t>::From(m_Isolate, V8String::NewFromUtf8(m_Isolate, "foo", v8::NewStringType::kNormal).ToLocalChecked().As<V8Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue);

                returnedValue = 0;
                EXPECT_FALSE(V8TypeConverter<int64_t>::From(m_Isolate, v8::Object::New(m_Isolate).As<V8Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue);

                returnedValue = 0;
                EXPECT_FALSE(V8TypeConverter<int64_t>::From(m_Isolate, v8::Null(m_Isolate).As<V8Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue);

                returnedValue = 0;
                EXPECT_FALSE(V8TypeConverter<int64_t>::From(m_Isolate, v8::Undefined(m_Isolate).As<V8Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue);
            }

            TEST_F(ConverterTests, UInt64)
            {
                V8IsolateScope iScope(m_Isolate);
                V8HandleScope scope(m_Isolate);
                V8ContextScope cScope(m_Context->GetLocalContext());

                EXPECT_TRUE(V8TypeConverter<uint64_t>::To(m_Isolate, 0)->StrictEquals(V8Integer::New(m_Isolate, 0)));
                EXPECT_TRUE(V8TypeConverter<uint64_t>::To(m_Isolate, 5)->StrictEquals(V8Integer::New(m_Isolate, 5)));

                uint64_t returnedValue = 0;

                EXPECT_FALSE(V8TypeConverter<uint64_t>::From(m_Isolate, V8Boolean::New(m_Isolate, false).As<V8Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue);

                returnedValue = 0;
                EXPECT_FALSE(V8TypeConverter<uint64_t>::From(m_Isolate, V8Boolean::New(m_Isolate, true).As<V8Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue);

                returnedValue = 0;
                EXPECT_TRUE(V8TypeConverter<uint64_t>::From(m_Isolate, V8Number::New(m_Isolate, 0).As<V8Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue);

                returnedValue = 0;
                EXPECT_TRUE(V8TypeConverter<uint64_t>::From(m_Isolate, V8Number::New(m_Isolate, 4294967297).As<V8Value>(), &returnedValue));
                EXPECT_EQ(4294967297, returnedValue);

                returnedValue = 0;
                //because of how the value is converted from a number it'll work but won't return the expected result
                EXPECT_TRUE(V8TypeConverter<uint64_t>::From(m_Isolate, V8Number::New(m_Isolate, -5).As<V8Value>(), &returnedValue));
                //-5 in uint54
                EXPECT_EQ(18446744073709551611ull, returnedValue);

                returnedValue = 0;
                //because of how the value is conveted from a number it'll work and just return the int portion
                EXPECT_TRUE(V8TypeConverter<uint64_t>::From(m_Isolate, V8Number::New(m_Isolate, 0.1).As<V8Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue);

                returnedValue = 0;
                EXPECT_FALSE(V8TypeConverter<uint64_t>::From(m_Isolate, V8String::NewFromUtf8(m_Isolate, "", v8::NewStringType::kNormal).ToLocalChecked().As<V8Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue);

                returnedValue = 0;
                EXPECT_FALSE(V8TypeConverter<uint64_t>::From(m_Isolate, V8String::NewFromUtf8(m_Isolate, "foo", v8::NewStringType::kNormal).ToLocalChecked().As<V8Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue);

                returnedValue = 0;
                EXPECT_FALSE(V8TypeConverter<uint64_t>::From(m_Isolate, v8::Object::New(m_Isolate).As<V8Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue);

                returnedValue = 0;
                EXPECT_FALSE(V8TypeConverter<uint64_t>::From(m_Isolate, v8::Null(m_Isolate).As<V8Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue);

                returnedValue = 0;
                EXPECT_FALSE(V8TypeConverter<uint64_t>::From(m_Isolate, v8::Undefined(m_Isolate).As<V8Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue);
            }

            TEST_F(ConverterTests, Float)
            {
                V8IsolateScope iScope(m_Isolate);
                V8HandleScope scope(m_Isolate);
                V8ContextScope cScope(m_Context->GetLocalContext());

                EXPECT_TRUE(V8TypeConverter<float>::To(m_Isolate, -5.0f)->StrictEquals(V8Number::New(m_Isolate, -5.0f)));
                EXPECT_TRUE(V8TypeConverter<float>::To(m_Isolate, 0.0f)->StrictEquals(V8Number::New(m_Isolate, 0.0f)));
                EXPECT_TRUE(V8TypeConverter<float>::To(m_Isolate, 5.0f)->StrictEquals(V8Number::New(m_Isolate, 5.0f)));

                float returnedValue = 0;

                EXPECT_FALSE(V8TypeConverter<float>::From(m_Isolate, V8Boolean::New(m_Isolate, false).As<V8Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue);

                returnedValue = 0;
                EXPECT_FALSE(V8TypeConverter<float>::From(m_Isolate, V8Boolean::New(m_Isolate, true).As<V8Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue);

                returnedValue = 0;
                EXPECT_TRUE(V8TypeConverter<float>::From(m_Isolate, V8Number::New(m_Isolate, 0.0f).As<V8Value>(), &returnedValue));
                EXPECT_EQ(0.0f, returnedValue);

                returnedValue = 0;
                EXPECT_TRUE(V8TypeConverter<float>::From(m_Isolate, V8Number::New(m_Isolate, -5.0f).As<V8Value>(), &returnedValue));
                EXPECT_EQ(-5.0f, returnedValue);

                returnedValue = 0;
                EXPECT_TRUE(V8TypeConverter<float>::From(m_Isolate, V8Integer::New(m_Isolate, -5).As<V8Value>(), &returnedValue));
                //-5 in uint54
                EXPECT_EQ(-5.0f, returnedValue);

                returnedValue = 0;
                EXPECT_TRUE(V8TypeConverter<float>::From(m_Isolate, V8Integer::New(m_Isolate, 5).As<V8Value>(), &returnedValue));
                EXPECT_EQ(5.0f, returnedValue);

                returnedValue = 0;
                EXPECT_FALSE(V8TypeConverter<float>::From(m_Isolate, V8String::NewFromUtf8(m_Isolate, "", v8::NewStringType::kNormal).ToLocalChecked().As<V8Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue);

                returnedValue = 0;
                EXPECT_FALSE(V8TypeConverter<float>::From(m_Isolate, V8String::NewFromUtf8(m_Isolate, "foo", v8::NewStringType::kNormal).ToLocalChecked().As<V8Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue);

                returnedValue = 0;
                EXPECT_FALSE(V8TypeConverter<float>::From(m_Isolate, v8::Object::New(m_Isolate).As<V8Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue);

                returnedValue = 0;
                EXPECT_FALSE(V8TypeConverter<float>::From(m_Isolate, v8::Null(m_Isolate).As<V8Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue);

                returnedValue = 0;
                EXPECT_FALSE(V8TypeConverter<float>::From(m_Isolate, v8::Undefined(m_Isolate).As<V8Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue);
            }

            TEST_F(ConverterTests, Double)
            {
                V8IsolateScope iScope(m_Isolate);
                V8HandleScope scope(m_Isolate);
                V8ContextScope cScope(m_Context->GetLocalContext());

                EXPECT_TRUE(V8TypeConverter<double>::To(m_Isolate, -5.0)->StrictEquals(V8Number::New(m_Isolate, -5.0)));
                EXPECT_TRUE(V8TypeConverter<double>::To(m_Isolate, 0.0)->StrictEquals(V8Number::New(m_Isolate, 0.0)));
                EXPECT_TRUE(V8TypeConverter<double>::To(m_Isolate, 5.0)->StrictEquals(V8Number::New(m_Isolate, 5.0)));

                double returnedValue = 0;

                EXPECT_FALSE(V8TypeConverter<double>::From(m_Isolate, V8Boolean::New(m_Isolate, false).As<V8Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue);

                returnedValue = 0;
                EXPECT_FALSE(V8TypeConverter<double>::From(m_Isolate, V8Boolean::New(m_Isolate, true).As<V8Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue);

                returnedValue = 0;
                EXPECT_TRUE(V8TypeConverter<double>::From(m_Isolate, V8Number::New(m_Isolate, 0.0).As<V8Value>(), &returnedValue));
                EXPECT_EQ(0.0, returnedValue);

                returnedValue = 0;
                EXPECT_TRUE(V8TypeConverter<double>::From(m_Isolate, V8Number::New(m_Isolate, -5.0).As<V8Value>(), &returnedValue));
                EXPECT_EQ(-5.0, returnedValue);

                returnedValue = 0;
                EXPECT_TRUE(V8TypeConverter<double>::From(m_Isolate, V8Integer::New(m_Isolate, -5).As<V8Value>(), &returnedValue));
                //-5 in uint54
                EXPECT_EQ(-5.0, returnedValue);

                returnedValue = 0;
                EXPECT_TRUE(V8TypeConverter<double>::From(m_Isolate, V8Integer::New(m_Isolate, 5).As<V8Value>(), &returnedValue));
                EXPECT_EQ(5.0, returnedValue);

                returnedValue = 0;
                EXPECT_FALSE(V8TypeConverter<double>::From(m_Isolate, V8String::NewFromUtf8(m_Isolate, "", v8::NewStringType::kNormal).ToLocalChecked().As<V8Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue);

                returnedValue = 0;
                EXPECT_FALSE(V8TypeConverter<double>::From(m_Isolate, V8String::NewFromUtf8(m_Isolate, "foo", v8::NewStringType::kNormal).ToLocalChecked().As<V8Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue);

                returnedValue = 0;
                EXPECT_FALSE(V8TypeConverter<double>::From(m_Isolate, v8::Object::New(m_Isolate).As<V8Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue);

                returnedValue = 0;
                EXPECT_FALSE(V8TypeConverter<double>::From(m_Isolate, v8::Null(m_Isolate).As<V8Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue);

                returnedValue = 0;
                EXPECT_FALSE(V8TypeConverter<double>::From(m_Isolate, v8::Undefined(m_Isolate).As<V8Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue);
            }

            TEST_F(ConverterTests, StdString)
            {
                V8IsolateScope iScope(m_Isolate);
                V8HandleScope scope(m_Isolate);
                V8ContextScope cScope(m_Context->GetLocalContext());

                std::string str = "";
                EXPECT_TRUE(V8TypeConverter<std::string>::To(m_Isolate, str)->StrictEquals(V8String::NewFromUtf8(m_Isolate, "").ToLocalChecked()));
                str = "Test";
                EXPECT_TRUE(V8TypeConverter<std::string>::To(m_Isolate, str)->StrictEquals(V8String::NewFromUtf8(m_Isolate, "Test").ToLocalChecked()));

                std::string returnedValue = "";

                EXPECT_FALSE(V8TypeConverter<std::string>::From(m_Isolate, V8Boolean::New(m_Isolate, false).As<V8Value>(), &returnedValue));
                EXPECT_EQ("", returnedValue);

                returnedValue = "";
                EXPECT_FALSE(V8TypeConverter<std::string>::From(m_Isolate, V8Boolean::New(m_Isolate, true).As<V8Value>(), &returnedValue));
                EXPECT_EQ("", returnedValue);

                returnedValue = "";
                EXPECT_FALSE(V8TypeConverter<std::string>::From(m_Isolate, V8Number::New(m_Isolate, 0.0).As<V8Value>(), &returnedValue));
                EXPECT_EQ("", returnedValue);

                returnedValue = "";
                EXPECT_FALSE(V8TypeConverter<std::string>::From(m_Isolate, V8Number::New(m_Isolate, -5.0).As<V8Value>(), &returnedValue));
                EXPECT_EQ("", returnedValue);

                returnedValue = "";
                EXPECT_FALSE(V8TypeConverter<std::string>::From(m_Isolate, V8Integer::New(m_Isolate, -5).As<V8Value>(), &returnedValue));
                EXPECT_EQ("", returnedValue);

                returnedValue = "";
                EXPECT_FALSE(V8TypeConverter<std::string>::From(m_Isolate, V8Integer::New(m_Isolate, 5).As<V8Value>(), &returnedValue));
                EXPECT_EQ("", returnedValue);

                returnedValue = "";
                EXPECT_TRUE(V8TypeConverter<std::string>::From(m_Isolate, V8String::NewFromUtf8(m_Isolate, "", v8::NewStringType::kNormal).ToLocalChecked().As<V8Value>(), &returnedValue));
                EXPECT_EQ("", returnedValue);

                returnedValue = "";
                EXPECT_TRUE(V8TypeConverter<std::string>::From(m_Isolate, V8String::NewFromUtf8(m_Isolate, "foo", v8::NewStringType::kNormal).ToLocalChecked().As<V8Value>(), &returnedValue));
                EXPECT_EQ("foo", returnedValue);

                returnedValue = "";
                EXPECT_FALSE(V8TypeConverter<std::string>::From(m_Isolate, v8::Object::New(m_Isolate).As<V8Value>(), &returnedValue));
                EXPECT_EQ("", returnedValue);

                returnedValue = "";
                EXPECT_FALSE(V8TypeConverter<std::string>::From(m_Isolate, v8::Null(m_Isolate).As<V8Value>(), &returnedValue));
                EXPECT_EQ("", returnedValue);

                returnedValue = "";
                EXPECT_FALSE(V8TypeConverter<std::string>::From(m_Isolate, v8::Undefined(m_Isolate).As<V8Value>(), &returnedValue));
                EXPECT_EQ("", returnedValue);
            }

            TEST_F(ConverterTests, StdU16String)
            {
                V8IsolateScope iScope(m_Isolate);
                V8HandleScope scope(m_Isolate);
                V8ContextScope cScope(m_Context->GetLocalContext());

                std::u16string str = u"";
                EXPECT_TRUE(V8TypeConverter<std::u16string>::To(m_Isolate, str)->StrictEquals(V8String::NewFromTwoByte(m_Isolate, reinterpret_cast<const uint16_t *>(u"")).ToLocalChecked()));
                str = u"Test";
                EXPECT_TRUE(V8TypeConverter<std::u16string>::To(m_Isolate, str)->StrictEquals(V8String::NewFromTwoByte(m_Isolate, reinterpret_cast<const uint16_t *>(u"Test"), v8::NewStringType::kNormal, 4).ToLocalChecked()));

                std::u16string returnedValue = u"";

                EXPECT_FALSE(V8TypeConverter<std::u16string>::From(m_Isolate, V8Boolean::New(m_Isolate, false).As<V8Value>(), &returnedValue));
                EXPECT_EQ(u"", returnedValue);

                returnedValue = u"";
                EXPECT_FALSE(V8TypeConverter<std::u16string>::From(m_Isolate, V8Boolean::New(m_Isolate, true).As<V8Value>(), &returnedValue));
                EXPECT_EQ(u"", returnedValue);

                returnedValue = u"";
                EXPECT_FALSE(V8TypeConverter<std::u16string>::From(m_Isolate, V8Number::New(m_Isolate, 0.0).As<V8Value>(), &returnedValue));
                EXPECT_EQ(u"", returnedValue);

                returnedValue = u"";
                EXPECT_FALSE(V8TypeConverter<std::u16string>::From(m_Isolate, V8Number::New(m_Isolate, -5.0).As<V8Value>(), &returnedValue));
                EXPECT_EQ(u"", returnedValue);

                returnedValue = u"";
                EXPECT_FALSE(V8TypeConverter<std::u16string>::From(m_Isolate, V8Integer::New(m_Isolate, -5).As<V8Value>(), &returnedValue));
                EXPECT_EQ(u"", returnedValue);

                returnedValue = u"";
                EXPECT_FALSE(V8TypeConverter<std::u16string>::From(m_Isolate, V8Integer::New(m_Isolate, 5).As<V8Value>(), &returnedValue));
                EXPECT_EQ(u"", returnedValue);

                returnedValue = u"";
                EXPECT_TRUE(V8TypeConverter<std::u16string>::From(m_Isolate, V8String::NewFromUtf8(m_Isolate, "", v8::NewStringType::kNormal).ToLocalChecked().As<V8Value>(), &returnedValue));
                EXPECT_EQ(u"", returnedValue);

                returnedValue = u"";
                EXPECT_TRUE(V8TypeConverter<std::u16string>::From(m_Isolate, V8String::NewFromUtf8(m_Isolate, "foo", v8::NewStringType::kNormal).ToLocalChecked().As<V8Value>(), &returnedValue));
                EXPECT_EQ(u"foo", returnedValue);

                returnedValue = u"";
                EXPECT_FALSE(V8TypeConverter<std::u16string>::From(m_Isolate, v8::Object::New(m_Isolate).As<V8Value>(), &returnedValue));
                EXPECT_EQ(u"", returnedValue);

                returnedValue = u"";
                EXPECT_FALSE(V8TypeConverter<std::u16string>::From(m_Isolate, v8::Null(m_Isolate).As<V8Value>(), &returnedValue));
                EXPECT_EQ(u"", returnedValue);

                returnedValue = u"";
                EXPECT_FALSE(V8TypeConverter<std::u16string>::From(m_Isolate, v8::Undefined(m_Isolate).As<V8Value>(), &returnedValue));
                EXPECT_EQ(u"", returnedValue);
            }

            static void testFunc(V8FuncCallInfoValue const &args)
            {
                //do nothign it's just a test and won't be called
                return;
            }

            TEST_F(ConverterTests, V8Function)
            {
                V8IsolateScope iScope(m_Isolate);
                V8HandleScope scope(m_Isolate);
                V8ContextScope cScope(m_Context->GetLocalContext());

                V8LFunction func = V8Function::New(m_Isolate->GetCurrentContext(), testFunc).ToLocalChecked();
                EXPECT_TRUE(V8TypeConverter<V8LFunction>::To(m_Isolate, func)->StrictEquals(func));

                V8LFunction returnedValue;

                EXPECT_FALSE(V8TypeConverter<V8LFunction>::From(m_Isolate, V8Boolean::New(m_Isolate, false).As<V8Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_FALSE(V8TypeConverter<V8LFunction>::From(m_Isolate, V8Boolean::New(m_Isolate, true).As<V8Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_FALSE(V8TypeConverter<V8LFunction>::From(m_Isolate, V8Number::New(m_Isolate, 0.0).As<V8Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_FALSE(V8TypeConverter<V8LFunction>::From(m_Isolate, V8Number::New(m_Isolate, -5.0).As<V8Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_FALSE(V8TypeConverter<V8LFunction>::From(m_Isolate, V8Integer::New(m_Isolate, -5).As<V8Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_FALSE(V8TypeConverter<V8LFunction>::From(m_Isolate, V8Integer::New(m_Isolate, 5).As<V8Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_FALSE(V8TypeConverter<V8LFunction>::From(m_Isolate, V8String::NewFromUtf8(m_Isolate, "", v8::NewStringType::kNormal).ToLocalChecked().As<V8Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_FALSE(V8TypeConverter<V8LFunction>::From(m_Isolate, V8String::NewFromUtf8(m_Isolate, "foo", v8::NewStringType::kNormal).ToLocalChecked().As<V8Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_FALSE(V8TypeConverter<V8LFunction>::From(m_Isolate, v8::Object::New(m_Isolate).As<V8Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_FALSE(V8TypeConverter<V8LFunction>::From(m_Isolate, v8::Null(m_Isolate).As<V8Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_FALSE(V8TypeConverter<V8LFunction>::From(m_Isolate, v8::Undefined(m_Isolate).As<V8Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_TRUE(V8TypeConverter<V8LFunction>::From(m_Isolate, V8Function::New(m_Isolate->GetCurrentContext(), testFunc).ToLocalChecked(), &returnedValue));
                EXPECT_FALSE(returnedValue.IsEmpty());
            }

            TEST_F(ConverterTests, V8Object)
            {
                V8IsolateScope iScope(m_Isolate);
                V8HandleScope scope(m_Isolate);
                V8ContextScope cScope(m_Context->GetLocalContext());

                V8LObject obj = v8::Object::New(m_Isolate);
                EXPECT_TRUE(V8TypeConverter<V8LObject>::To(m_Isolate, obj)->StrictEquals(obj));

                V8LObject returnedValue;

                EXPECT_FALSE(V8TypeConverter<V8LObject>::From(m_Isolate, V8Boolean::New(m_Isolate, false).As<V8Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_FALSE(V8TypeConverter<V8LObject>::From(m_Isolate, V8Boolean::New(m_Isolate, true).As<V8Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_FALSE(V8TypeConverter<V8LObject>::From(m_Isolate, V8Number::New(m_Isolate, 0.0).As<V8Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_FALSE(V8TypeConverter<V8LObject>::From(m_Isolate, V8Number::New(m_Isolate, -5.0).As<V8Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_FALSE(V8TypeConverter<V8LObject>::From(m_Isolate, V8Integer::New(m_Isolate, -5).As<V8Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_FALSE(V8TypeConverter<V8LObject>::From(m_Isolate, V8Integer::New(m_Isolate, 5).As<V8Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_FALSE(V8TypeConverter<V8LObject>::From(m_Isolate, V8String::NewFromUtf8(m_Isolate, "", v8::NewStringType::kNormal).ToLocalChecked().As<V8Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_FALSE(V8TypeConverter<V8LObject>::From(m_Isolate, V8String::NewFromUtf8(m_Isolate, "foo", v8::NewStringType::kNormal).ToLocalChecked().As<V8Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_TRUE(V8TypeConverter<V8LObject>::From(m_Isolate, v8::Object::New(m_Isolate).As<V8Value>(), &returnedValue));
                EXPECT_FALSE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_FALSE(V8TypeConverter<V8LObject>::From(m_Isolate, v8::Null(m_Isolate).As<V8Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_FALSE(V8TypeConverter<V8LObject>::From(m_Isolate, v8::Undefined(m_Isolate).As<V8Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());
            }

            TEST_F(ConverterTests, V8Promise)
            {
                V8IsolateScope iScope(m_Isolate);
                V8HandleScope scope(m_Isolate);
                V8ContextScope cScope(m_Context->GetLocalContext());

                auto resolver = V8PromiseResolver::New(m_Isolate->GetCurrentContext());
                ASSERT_FALSE(resolver.IsEmpty());

                V8LPromise promise = resolver.ToLocalChecked()->GetPromise();
                EXPECT_TRUE(V8TypeConverter<V8LPromise>::To(m_Isolate, promise)->StrictEquals(promise));

                V8LPromise returnedValue;

                EXPECT_FALSE(V8TypeConverter<V8LPromise>::From(m_Isolate, V8Boolean::New(m_Isolate, false).As<V8Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_FALSE(V8TypeConverter<V8LPromise>::From(m_Isolate, V8Boolean::New(m_Isolate, true).As<V8Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_FALSE(V8TypeConverter<V8LPromise>::From(m_Isolate, V8Number::New(m_Isolate, 0.0).As<V8Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_FALSE(V8TypeConverter<V8LPromise>::From(m_Isolate, V8Number::New(m_Isolate, -5.0).As<V8Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_FALSE(V8TypeConverter<V8LPromise>::From(m_Isolate, V8Integer::New(m_Isolate, -5).As<V8Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_FALSE(V8TypeConverter<V8LPromise>::From(m_Isolate, V8Integer::New(m_Isolate, 5).As<V8Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_FALSE(V8TypeConverter<V8LPromise>::From(m_Isolate, V8String::NewFromUtf8(m_Isolate, "", v8::NewStringType::kNormal).ToLocalChecked().As<V8Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_FALSE(V8TypeConverter<V8LPromise>::From(m_Isolate, V8String::NewFromUtf8(m_Isolate, "foo", v8::NewStringType::kNormal).ToLocalChecked().As<V8Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_FALSE(V8TypeConverter<V8LPromise>::From(m_Isolate, v8::Object::New(m_Isolate).As<V8Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_FALSE(V8TypeConverter<V8LPromise>::From(m_Isolate, v8::Null(m_Isolate).As<V8Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_FALSE(V8TypeConverter<V8LPromise>::From(m_Isolate, v8::Undefined(m_Isolate).As<V8Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_TRUE(V8TypeConverter<V8LPromise>::From(m_Isolate, resolver.ToLocalChecked()->GetPromise(), &returnedValue));
                EXPECT_FALSE(returnedValue.IsEmpty());
            }

            TEST_F(ConverterTests, V8ArrayBuffer)
            {
                V8IsolateScope iScope(m_Isolate);
                V8HandleScope scope(m_Isolate);
                V8ContextScope cScope(m_Context->GetLocalContext());

                V8LArrayBuffer buffer = V8ArrayBuffer::New(m_Isolate, 1);
                EXPECT_TRUE(V8TypeConverter<V8LArrayBuffer>::To(m_Isolate, buffer)->StrictEquals(buffer));

                V8LArrayBuffer returnedValue;

                EXPECT_FALSE(V8TypeConverter<V8LArrayBuffer>::From(m_Isolate, V8Boolean::New(m_Isolate, false).As<V8Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_FALSE(V8TypeConverter<V8LArrayBuffer>::From(m_Isolate, V8Boolean::New(m_Isolate, true).As<V8Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_FALSE(V8TypeConverter<V8LArrayBuffer>::From(m_Isolate, V8Number::New(m_Isolate, 0.0).As<V8Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_FALSE(V8TypeConverter<V8LArrayBuffer>::From(m_Isolate, V8Number::New(m_Isolate, -5.0).As<V8Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_FALSE(V8TypeConverter<V8LArrayBuffer>::From(m_Isolate, V8Integer::New(m_Isolate, -5).As<V8Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_FALSE(V8TypeConverter<V8LArrayBuffer>::From(m_Isolate, V8Integer::New(m_Isolate, 5).As<V8Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_FALSE(V8TypeConverter<V8LArrayBuffer>::From(m_Isolate, V8String::NewFromUtf8(m_Isolate, "", v8::NewStringType::kNormal).ToLocalChecked().As<V8Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_FALSE(V8TypeConverter<V8LArrayBuffer>::From(m_Isolate, V8String::NewFromUtf8(m_Isolate, "foo", v8::NewStringType::kNormal).ToLocalChecked().As<V8Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_FALSE(V8TypeConverter<V8LArrayBuffer>::From(m_Isolate, v8::Object::New(m_Isolate).As<V8Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_FALSE(V8TypeConverter<V8LArrayBuffer>::From(m_Isolate, v8::Null(m_Isolate).As<V8Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_FALSE(V8TypeConverter<V8LArrayBuffer>::From(m_Isolate, v8::Undefined(m_Isolate).As<V8Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_TRUE(V8TypeConverter<V8LArrayBuffer>::From(m_Isolate, V8ArrayBuffer::New(m_Isolate, 1).As<V8Value>(), &returnedValue));
                EXPECT_FALSE(returnedValue.IsEmpty());
            }

            TEST_F(ConverterTests, V8External)
            {
                V8IsolateScope iScope(m_Isolate);
                V8HandleScope scope(m_Isolate);
                V8ContextScope cScope(m_Context->GetLocalContext());

                V8LExternal external = V8LExternal::New(m_Isolate, V8External::New(m_Isolate, (void *)testFunc));
                EXPECT_TRUE(V8TypeConverter<V8LExternal>::To(m_Isolate, external)->StrictEquals(external));

                V8LExternal returnedValue;

                EXPECT_FALSE(V8TypeConverter<V8LExternal>::From(m_Isolate, V8Boolean::New(m_Isolate, false).As<V8Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_FALSE(V8TypeConverter<V8LExternal>::From(m_Isolate, V8Boolean::New(m_Isolate, true).As<V8Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_FALSE(V8TypeConverter<V8LExternal>::From(m_Isolate, V8Number::New(m_Isolate, 0.0).As<V8Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_FALSE(V8TypeConverter<V8LExternal>::From(m_Isolate, V8Number::New(m_Isolate, -5.0).As<V8Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_FALSE(V8TypeConverter<V8LExternal>::From(m_Isolate, V8Integer::New(m_Isolate, -5).As<V8Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_FALSE(V8TypeConverter<V8LExternal>::From(m_Isolate, V8Integer::New(m_Isolate, 5).As<V8Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_FALSE(V8TypeConverter<V8LExternal>::From(m_Isolate, V8String::NewFromUtf8(m_Isolate, "", v8::NewStringType::kNormal).ToLocalChecked().As<V8Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_FALSE(V8TypeConverter<V8LExternal>::From(m_Isolate, V8String::NewFromUtf8(m_Isolate, "foo", v8::NewStringType::kNormal).ToLocalChecked().As<V8Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_FALSE(V8TypeConverter<V8LExternal>::From(m_Isolate, v8::Object::New(m_Isolate).As<V8Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_FALSE(V8TypeConverter<V8LExternal>::From(m_Isolate, v8::Null(m_Isolate).As<V8Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_FALSE(V8TypeConverter<V8LExternal>::From(m_Isolate, v8::Undefined(m_Isolate).As<V8Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_TRUE(V8TypeConverter<V8LExternal>::From(m_Isolate, V8External::New(m_Isolate, (void *)testFunc).As<V8Value>(), &returnedValue));
                EXPECT_FALSE(returnedValue.IsEmpty());
            }

            TEST_F(ConverterTests, V8Value)
            {
                V8IsolateScope iScope(m_Isolate);
                V8HandleScope scope(m_Isolate);
                V8ContextScope cScope(m_Context->GetLocalContext());

                V8LValue value = v8::Object::New(m_Isolate).As<V8Value>();
                EXPECT_TRUE(V8TypeConverter<V8LValue>::To(m_Isolate, value)->StrictEquals(value));

                V8LValue returnedValue;

                // vaue is the base of everything so these will call convert
                EXPECT_TRUE(V8TypeConverter<V8LValue>::From(m_Isolate, V8Boolean::New(m_Isolate, false).As<V8Value>(), &returnedValue));
                EXPECT_FALSE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_TRUE(V8TypeConverter<V8LValue>::From(m_Isolate, V8Boolean::New(m_Isolate, true).As<V8Value>(), &returnedValue));
                EXPECT_FALSE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_TRUE(V8TypeConverter<V8LValue>::From(m_Isolate, V8Number::New(m_Isolate, 0.0).As<V8Value>(), &returnedValue));
                EXPECT_FALSE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_TRUE(V8TypeConverter<V8LValue>::From(m_Isolate, V8Number::New(m_Isolate, -5.0).As<V8Value>(), &returnedValue));
                EXPECT_FALSE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_TRUE(V8TypeConverter<V8LValue>::From(m_Isolate, V8Integer::New(m_Isolate, -5).As<V8Value>(), &returnedValue));
                EXPECT_FALSE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_TRUE(V8TypeConverter<V8LValue>::From(m_Isolate, V8Integer::New(m_Isolate, 5).As<V8Value>(), &returnedValue));
                EXPECT_FALSE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_TRUE(V8TypeConverter<V8LValue>::From(m_Isolate, V8String::NewFromUtf8(m_Isolate, "", v8::NewStringType::kNormal).ToLocalChecked().As<V8Value>(), &returnedValue));
                EXPECT_FALSE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_TRUE(V8TypeConverter<V8LValue>::From(m_Isolate, V8String::NewFromUtf8(m_Isolate, "foo", v8::NewStringType::kNormal).ToLocalChecked().As<V8Value>(), &returnedValue));
                EXPECT_FALSE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_TRUE(V8TypeConverter<V8LValue>::From(m_Isolate, v8::Object::New(m_Isolate).As<V8Value>(), &returnedValue));
                EXPECT_FALSE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_TRUE(V8TypeConverter<V8LValue>::From(m_Isolate, v8::Null(m_Isolate).As<V8Value>(), &returnedValue));
                EXPECT_FALSE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_TRUE(V8TypeConverter<V8LValue>::From(m_Isolate, v8::Undefined(m_Isolate).As<V8Value>(), &returnedValue));
                EXPECT_FALSE(returnedValue.IsEmpty());
            }

           TEST_F(ConverterTests, V8Number)
            {
                V8IsolateScope iScope(m_Isolate);
                V8HandleScope scope(m_Isolate);
                V8ContextScope cScope(m_Context->GetLocalContext());

                V8LNumber number = V8LNumber::New(m_Isolate, V8Number::New(m_Isolate, 10));
                EXPECT_TRUE(V8TypeConverter<V8LNumber>::To(m_Isolate, number)->StrictEquals(number));

                V8LNumber returnedValue;

                EXPECT_FALSE(V8TypeConverter<V8LNumber>::From(m_Isolate, V8Boolean::New(m_Isolate, false).As<V8Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_FALSE(V8TypeConverter<V8LNumber>::From(m_Isolate, V8Boolean::New(m_Isolate, true).As<V8Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_FALSE(V8TypeConverter<V8LNumber>::From(m_Isolate, V8String::NewFromUtf8(m_Isolate, "", v8::NewStringType::kNormal).ToLocalChecked().As<V8Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_FALSE(V8TypeConverter<V8LNumber>::From(m_Isolate, V8String::NewFromUtf8(m_Isolate, "foo", v8::NewStringType::kNormal).ToLocalChecked().As<V8Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_FALSE(V8TypeConverter<V8LNumber>::From(m_Isolate, v8::Object::New(m_Isolate).As<V8Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_FALSE(V8TypeConverter<V8LNumber>::From(m_Isolate, v8::Null(m_Isolate).As<V8Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_FALSE(V8TypeConverter<V8LNumber>::From(m_Isolate, v8::Undefined(m_Isolate).As<V8Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_FALSE(V8TypeConverter<V8LNumber>::From(m_Isolate, V8External::New(m_Isolate, (void *)testFunc).As<V8Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());
            }

            TEST_F(ConverterTests, StdVector)
            {
                V8IsolateScope iScope(m_Isolate);
                V8HandleScope scope(m_Isolate);
                V8ContextScope cScope(m_Context->GetLocalContext());

                std::vector<int> expected = {-1, 0, 1};

                V8LArray array = V8TypeConverter<std::vector<int>>::To(m_Isolate, expected).As<V8Array>();
                EXPECT_EQ(3, array->Length());
                EXPECT_TRUE(V8Integer::New(m_Isolate, expected[0])->StrictEquals(array->Get(m_Isolate->GetCurrentContext(), 0).ToLocalChecked()));
                EXPECT_TRUE(V8Integer::New(m_Isolate, expected[1])->StrictEquals(array->Get(m_Isolate->GetCurrentContext(), 1).ToLocalChecked()));
                EXPECT_TRUE(V8Integer::New(m_Isolate, expected[2])->StrictEquals(array->Get(m_Isolate->GetCurrentContext(), 2).ToLocalChecked()));

                std::vector<int> returnedValue;

                EXPECT_FALSE(V8TypeConverter<std::vector<int>>::From(m_Isolate, V8Boolean::New(m_Isolate, false).As<V8Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue.size());

                returnedValue.clear();
                EXPECT_FALSE(V8TypeConverter<std::vector<int>>::From(m_Isolate, V8Boolean::New(m_Isolate, true).As<V8Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue.size());

                returnedValue.clear();
                EXPECT_FALSE(V8TypeConverter<std::vector<int>>::From(m_Isolate, V8Number::New(m_Isolate, 0.0).As<V8Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue.size());

                returnedValue.clear();
                EXPECT_FALSE(V8TypeConverter<std::vector<int>>::From(m_Isolate, V8Number::New(m_Isolate, -5.0).As<V8Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue.size());

                returnedValue.clear();
                EXPECT_FALSE(V8TypeConverter<std::vector<int>>::From(m_Isolate, V8Integer::New(m_Isolate, -5).As<V8Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue.size());

                returnedValue.clear();
                EXPECT_FALSE(V8TypeConverter<std::vector<int>>::From(m_Isolate, V8Integer::New(m_Isolate, 5).As<V8Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue.size());

                returnedValue.clear();
                EXPECT_FALSE(V8TypeConverter<std::vector<int>>::From(m_Isolate, V8String::NewFromUtf8(m_Isolate, "", v8::NewStringType::kNormal).ToLocalChecked().As<V8Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue.size());

                returnedValue.clear();
                EXPECT_FALSE(V8TypeConverter<std::vector<int>>::From(m_Isolate, V8String::NewFromUtf8(m_Isolate, "foo", v8::NewStringType::kNormal).ToLocalChecked().As<V8Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue.size());

                returnedValue.clear();
                EXPECT_FALSE(V8TypeConverter<std::vector<int>>::From(m_Isolate, v8::Object::New(m_Isolate).As<V8Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue.size());

                returnedValue.clear();
                EXPECT_FALSE(V8TypeConverter<std::vector<int>>::From(m_Isolate, v8::Null(m_Isolate).As<V8Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue.size());

                returnedValue.clear();
                EXPECT_FALSE(V8TypeConverter<std::vector<int>>::From(m_Isolate, v8::Undefined(m_Isolate).As<V8Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue.size());

                returnedValue.clear();
                EXPECT_TRUE(V8TypeConverter<std::vector<int>>::From(m_Isolate, array, &returnedValue));
                EXPECT_EQ(3, returnedValue.size());
                EXPECT_EQ(-1, returnedValue[0]);
                EXPECT_EQ(0, returnedValue[1]);
                EXPECT_EQ(1, returnedValue[2]);
            }

            TEST_F(ConverterTests, StdVectorOfVectors)
            {
                V8IsolateScope iScope(m_Isolate);
                V8HandleScope scope(m_Isolate);
                V8ContextScope cScope(m_Context->GetLocalContext());
                
                using vvInt = std::vector<std::vector<int>>;
                vvInt expected = {{-1, 0, 1}, {3, 4, 5}};

                V8LArray array = V8TypeConverter<vvInt>::To(m_Isolate, expected).As<V8Array>();
                EXPECT_EQ(2, array->Length());

                vvInt returnedValue;

                EXPECT_TRUE(V8TypeConverter<vvInt>::From(m_Isolate, array, &returnedValue));
                EXPECT_TRUE(expected == returnedValue);
            }

            TEST_F(ConverterTests, ConvertToFromV8)
            {
                V8IsolateScope iScope(m_Isolate);
                V8HandleScope scope(m_Isolate);
                V8ContextScope cScope(m_Context->GetLocalContext());

                std::string strVal("Test");
                V8LExternal external = V8LExternal::New(m_Isolate, V8External::New(m_Isolate, (void *)testFunc));

                V8LValue boolean = ConvertToV8(m_Isolate, true);
                V8LValue integer = ConvertToV8(m_Isolate, 10);
                V8LValue string = ConvertToV8(m_Isolate, strVal);
                V8LValue external2 = ConvertToV8(m_Isolate, external);

                bool outBool = false;
                int32_t outInt = 0;
                std::string outString;
                V8LValue outValue;

                EXPECT_TRUE(ConvertFromV8(m_Isolate, boolean, &outBool));
                EXPECT_TRUE(ConvertFromV8(m_Isolate, integer, &outInt));
                EXPECT_TRUE(ConvertFromV8(m_Isolate, string, &outString));
                EXPECT_TRUE(ConvertFromV8(m_Isolate, external2, &outValue));

                EXPECT_EQ(true, outBool);
                EXPECT_EQ(10, outInt);
                EXPECT_EQ("Test", outString);
                EXPECT_TRUE(outValue->IsExternal());
            }

            TEST_F(ConverterTests, TryConvertToV8)
            {
                V8IsolateScope iScope(m_Isolate);
                V8HandleScope scope(m_Isolate);
                V8ContextScope cScope(m_Context->GetLocalContext());

                V8LValue value;
                std::string str = "Test";
                bool result = TryConvertToV8(m_Isolate, str, &value);
                EXPECT_TRUE(result);
                EXPECT_TRUE(value->StrictEquals(V8String::NewFromUtf8(m_Isolate, str.c_str()).ToLocalChecked()));
            }
        } // namespace Bridge
    }     // namespace JSRuntime
} // namespace v8App