// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "../V8TestFixture.h"
#include "CppBridge/V8TypeConverter.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"

namespace v8App
{
    namespace JSRuntime
    {
        namespace CppBridge
        {
            using ConverterTests = V8TestFixture;

            TEST_F(ConverterTests, Bool)
            {
                v8::HandleScope scope(m_Isolate);

                EXPECT_TRUE(V8TypeConverter<bool>::To(m_Isolate, true)->StrictEquals(v8::Boolean::New(m_Isolate, true)));
                EXPECT_TRUE(V8TypeConverter<bool>::To(m_Isolate, false)->StrictEquals(v8::Boolean::New(m_Isolate, false)));

                bool returnedValue = false;

                EXPECT_TRUE(V8TypeConverter<bool>::From(m_Isolate, v8::Boolean::New(m_Isolate, false).As<v8::Value>(), &returnedValue));
                EXPECT_EQ(false, returnedValue);

                EXPECT_TRUE(V8TypeConverter<bool>::From(m_Isolate, v8::Boolean::New(m_Isolate, true).As<v8::Value>(), &returnedValue));
                EXPECT_EQ(true, returnedValue);

                EXPECT_TRUE(V8TypeConverter<bool>::From(m_Isolate, v8::Number::New(m_Isolate, 0).As<v8::Value>(), &returnedValue));
                EXPECT_EQ(false, returnedValue);

                EXPECT_TRUE(V8TypeConverter<bool>::From(m_Isolate, v8::Number::New(m_Isolate, 5).As<v8::Value>(), &returnedValue));
                EXPECT_EQ(true, returnedValue);

                EXPECT_TRUE(V8TypeConverter<bool>::From(m_Isolate, v8::Number::New(m_Isolate, -5).As<v8::Value>(), &returnedValue));
                EXPECT_EQ(true, returnedValue);

                EXPECT_TRUE(V8TypeConverter<bool>::From(m_Isolate, v8::Number::New(m_Isolate, 0.1).As<v8::Value>(), &returnedValue));
                EXPECT_EQ(true, returnedValue);

                EXPECT_TRUE(V8TypeConverter<bool>::From(m_Isolate, v8::String::NewFromUtf8(m_Isolate, "", v8::NewStringType::kNormal).ToLocalChecked().As<v8::Value>(), &returnedValue));
                EXPECT_EQ(false, returnedValue);

                EXPECT_TRUE(V8TypeConverter<bool>::From(m_Isolate, v8::String::NewFromUtf8(m_Isolate, "foo", v8::NewStringType::kNormal).ToLocalChecked().As<v8::Value>(), &returnedValue));
                EXPECT_EQ(true, returnedValue);

                EXPECT_TRUE(V8TypeConverter<bool>::From(m_Isolate, v8::Object::New(m_Isolate).As<v8::Value>(), &returnedValue));
                EXPECT_EQ(true, returnedValue);

                EXPECT_TRUE(V8TypeConverter<bool>::From(m_Isolate, v8::Null(m_Isolate).As<v8::Value>(), &returnedValue));
                EXPECT_EQ(false, returnedValue);

                EXPECT_TRUE(V8TypeConverter<bool>::From(m_Isolate, v8::Undefined(m_Isolate).As<v8::Value>(), &returnedValue));
                EXPECT_EQ(false, returnedValue);
            }

            TEST_F(ConverterTests, Int32)
            {
                v8::HandleScope scope(m_Isolate);

                EXPECT_TRUE(V8TypeConverter<int32_t>::To(m_Isolate, -5)->StrictEquals(v8::Integer::New(m_Isolate, -5)));
                EXPECT_TRUE(V8TypeConverter<int32_t>::To(m_Isolate, 0)->StrictEquals(v8::Integer::New(m_Isolate, 0)));
                EXPECT_TRUE(V8TypeConverter<int32_t>::To(m_Isolate, 5)->StrictEquals(v8::Integer::New(m_Isolate, 5)));

                int32_t returnedValue = 0;

                EXPECT_FALSE(V8TypeConverter<int32_t>::From(m_Isolate, v8::Boolean::New(m_Isolate, false).As<v8::Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue);

                returnedValue = 0;
                EXPECT_FALSE(V8TypeConverter<int32_t>::From(m_Isolate, v8::Boolean::New(m_Isolate, true).As<v8::Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue);

                returnedValue = 0;
                EXPECT_TRUE(V8TypeConverter<int32_t>::From(m_Isolate, v8::Integer::New(m_Isolate, 0).As<v8::Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue);

                returnedValue = 0;
                EXPECT_TRUE(V8TypeConverter<int32_t>::From(m_Isolate, v8::Integer::New(m_Isolate, 5).As<v8::Value>(), &returnedValue));
                EXPECT_EQ(5, returnedValue);

                returnedValue = 0;
                EXPECT_TRUE(V8TypeConverter<int32_t>::From(m_Isolate, v8::Integer::New(m_Isolate, -5).As<v8::Value>(), &returnedValue));
                EXPECT_EQ(-5, returnedValue);

                returnedValue = 0;
                //this works cause it's a whole integer
                EXPECT_TRUE(V8TypeConverter<int32_t>::From(m_Isolate, v8::Number::New(m_Isolate, -5).As<v8::Value>(), &returnedValue));
                EXPECT_EQ(-5, returnedValue);

                returnedValue = 0;
                EXPECT_FALSE(V8TypeConverter<int32_t>::From(m_Isolate, v8::Number::New(m_Isolate, 0.1).As<v8::Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue);

                returnedValue = 0;
                EXPECT_FALSE(V8TypeConverter<int32_t>::From(m_Isolate, v8::String::NewFromUtf8(m_Isolate, "", v8::NewStringType::kNormal).ToLocalChecked().As<v8::Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue);

                returnedValue = 0;
                EXPECT_FALSE(V8TypeConverter<int32_t>::From(m_Isolate, v8::String::NewFromUtf8(m_Isolate, "foo", v8::NewStringType::kNormal).ToLocalChecked().As<v8::Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue);

                returnedValue = 0;
                EXPECT_FALSE(V8TypeConverter<int32_t>::From(m_Isolate, v8::Object::New(m_Isolate).As<v8::Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue);

                returnedValue = 0;
                EXPECT_FALSE(V8TypeConverter<int32_t>::From(m_Isolate, v8::Null(m_Isolate).As<v8::Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue);

                returnedValue = 0;
                EXPECT_FALSE(V8TypeConverter<int32_t>::From(m_Isolate, v8::Undefined(m_Isolate).As<v8::Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue);
            }

            TEST_F(ConverterTests, UInt32)
            {
                v8::HandleScope scope(m_Isolate);

                EXPECT_TRUE(V8TypeConverter<uint32_t>::To(m_Isolate, 0)->StrictEquals(v8::Integer::New(m_Isolate, 0)));
                EXPECT_TRUE(V8TypeConverter<uint32_t>::To(m_Isolate, 5)->StrictEquals(v8::Integer::New(m_Isolate, 5)));

                uint32_t returnedValue = 0;

                EXPECT_FALSE(V8TypeConverter<uint32_t>::From(m_Isolate, v8::Boolean::New(m_Isolate, false).As<v8::Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue);

                returnedValue = 0;
                EXPECT_FALSE(V8TypeConverter<uint32_t>::From(m_Isolate, v8::Boolean::New(m_Isolate, true).As<v8::Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue);

                returnedValue = 0;
                EXPECT_TRUE(V8TypeConverter<uint32_t>::From(m_Isolate, v8::Integer::New(m_Isolate, 0).As<v8::Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue);

                returnedValue = 0;
                EXPECT_TRUE(V8TypeConverter<uint32_t>::From(m_Isolate, v8::Integer::New(m_Isolate, 5).As<v8::Value>(), &returnedValue));
                EXPECT_EQ(5, returnedValue);

                returnedValue = 0;
                EXPECT_FALSE(V8TypeConverter<uint32_t>::From(m_Isolate, v8::Integer::New(m_Isolate, -5).As<v8::Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue);

                returnedValue = 0;
                EXPECT_TRUE(V8TypeConverter<uint32_t>::From(m_Isolate, v8::Number::New(m_Isolate, 5).As<v8::Value>(), &returnedValue));
                EXPECT_EQ(5, returnedValue);

                returnedValue = 0;
                EXPECT_FALSE(V8TypeConverter<uint32_t>::From(m_Isolate, v8::Number::New(m_Isolate, -5).As<v8::Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue);

                returnedValue = 0;
                EXPECT_FALSE(V8TypeConverter<uint32_t>::From(m_Isolate, v8::Number::New(m_Isolate, 0.1).As<v8::Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue);

                returnedValue = 0;
                EXPECT_FALSE(V8TypeConverter<uint32_t>::From(m_Isolate, v8::String::NewFromUtf8(m_Isolate, "", v8::NewStringType::kNormal).ToLocalChecked().As<v8::Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue);

                returnedValue = 0;
                EXPECT_FALSE(V8TypeConverter<uint32_t>::From(m_Isolate, v8::String::NewFromUtf8(m_Isolate, "foo", v8::NewStringType::kNormal).ToLocalChecked().As<v8::Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue);

                returnedValue = 0;
                EXPECT_FALSE(V8TypeConverter<uint32_t>::From(m_Isolate, v8::Object::New(m_Isolate).As<v8::Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue);

                returnedValue = 0;
                EXPECT_FALSE(V8TypeConverter<uint32_t>::From(m_Isolate, v8::Null(m_Isolate).As<v8::Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue);

                returnedValue = 0;
                EXPECT_FALSE(V8TypeConverter<uint32_t>::From(m_Isolate, v8::Undefined(m_Isolate).As<v8::Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue);
            }

            TEST_F(ConverterTests, Int64)
            {
                v8::HandleScope scope(m_Isolate);

                EXPECT_TRUE(V8TypeConverter<int64_t>::To(m_Isolate, -5)->StrictEquals(v8::Integer::New(m_Isolate, -5)));
                EXPECT_TRUE(V8TypeConverter<int64_t>::To(m_Isolate, 0)->StrictEquals(v8::Integer::New(m_Isolate, 0)));
                EXPECT_TRUE(V8TypeConverter<int64_t>::To(m_Isolate, 5)->StrictEquals(v8::Integer::New(m_Isolate, 5)));

                int64_t returnedValue = 0;

                EXPECT_FALSE(V8TypeConverter<int64_t>::From(m_Isolate, v8::Boolean::New(m_Isolate, false).As<v8::Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue);

                returnedValue = 0;
                EXPECT_FALSE(V8TypeConverter<int64_t>::From(m_Isolate, v8::Boolean::New(m_Isolate, true).As<v8::Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue);

                returnedValue = 0;
                EXPECT_TRUE(V8TypeConverter<int64_t>::From(m_Isolate, v8::Integer::New(m_Isolate, 0).As<v8::Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue);

                returnedValue = 0;
                EXPECT_TRUE(V8TypeConverter<int64_t>::From(m_Isolate, v8::Number::New(m_Isolate, 4294967297).As<v8::Value>(), &returnedValue));
                EXPECT_EQ(4294967297, returnedValue);

                returnedValue = 0;
                EXPECT_TRUE(V8TypeConverter<int64_t>::From(m_Isolate, v8::Number::New(m_Isolate, -4294967297).As<v8::Value>(), &returnedValue));
                EXPECT_EQ(-4294967297, returnedValue);

                returnedValue = 0;
                //because of how the value is conveted from a number it'll work and just return the int portion
                EXPECT_TRUE(V8TypeConverter<int64_t>::From(m_Isolate, v8::Number::New(m_Isolate, 20345.1).As<v8::Value>(), &returnedValue));
                EXPECT_EQ(20345, returnedValue);

                returnedValue = 0;
                EXPECT_FALSE(V8TypeConverter<int64_t>::From(m_Isolate, v8::String::NewFromUtf8(m_Isolate, "", v8::NewStringType::kNormal).ToLocalChecked().As<v8::Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue);

                returnedValue = 0;
                EXPECT_FALSE(V8TypeConverter<int64_t>::From(m_Isolate, v8::String::NewFromUtf8(m_Isolate, "foo", v8::NewStringType::kNormal).ToLocalChecked().As<v8::Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue);

                returnedValue = 0;
                EXPECT_FALSE(V8TypeConverter<int64_t>::From(m_Isolate, v8::Object::New(m_Isolate).As<v8::Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue);

                returnedValue = 0;
                EXPECT_FALSE(V8TypeConverter<int64_t>::From(m_Isolate, v8::Null(m_Isolate).As<v8::Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue);

                returnedValue = 0;
                EXPECT_FALSE(V8TypeConverter<int64_t>::From(m_Isolate, v8::Undefined(m_Isolate).As<v8::Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue);
            }

            TEST_F(ConverterTests, UInt64)
            {
                v8::HandleScope scope(m_Isolate);

                EXPECT_TRUE(V8TypeConverter<uint64_t>::To(m_Isolate, 0)->StrictEquals(v8::Integer::New(m_Isolate, 0)));
                EXPECT_TRUE(V8TypeConverter<uint64_t>::To(m_Isolate, 5)->StrictEquals(v8::Integer::New(m_Isolate, 5)));

                uint64_t returnedValue = 0;

                EXPECT_FALSE(V8TypeConverter<uint64_t>::From(m_Isolate, v8::Boolean::New(m_Isolate, false).As<v8::Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue);

                returnedValue = 0;
                EXPECT_FALSE(V8TypeConverter<uint64_t>::From(m_Isolate, v8::Boolean::New(m_Isolate, true).As<v8::Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue);

                returnedValue = 0;
                EXPECT_TRUE(V8TypeConverter<uint64_t>::From(m_Isolate, v8::Number::New(m_Isolate, 0).As<v8::Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue);

                returnedValue = 0;
                EXPECT_TRUE(V8TypeConverter<uint64_t>::From(m_Isolate, v8::Number::New(m_Isolate, 4294967297).As<v8::Value>(), &returnedValue));
                EXPECT_EQ(4294967297, returnedValue);

                returnedValue = 0;
                //because of how the value is converted from a number it'll work but won't return the expected result
                EXPECT_TRUE(V8TypeConverter<uint64_t>::From(m_Isolate, v8::Number::New(m_Isolate, -5).As<v8::Value>(), &returnedValue));
                //-5 in uint54
                EXPECT_EQ(18446744073709551611ull, returnedValue);

                returnedValue = 0;
                //because of how the value is conveted from a number it'll work and just return the int portion
                EXPECT_TRUE(V8TypeConverter<uint64_t>::From(m_Isolate, v8::Number::New(m_Isolate, 0.1).As<v8::Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue);

                returnedValue = 0;
                EXPECT_FALSE(V8TypeConverter<uint64_t>::From(m_Isolate, v8::String::NewFromUtf8(m_Isolate, "", v8::NewStringType::kNormal).ToLocalChecked().As<v8::Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue);

                returnedValue = 0;
                EXPECT_FALSE(V8TypeConverter<uint64_t>::From(m_Isolate, v8::String::NewFromUtf8(m_Isolate, "foo", v8::NewStringType::kNormal).ToLocalChecked().As<v8::Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue);

                returnedValue = 0;
                EXPECT_FALSE(V8TypeConverter<uint64_t>::From(m_Isolate, v8::Object::New(m_Isolate).As<v8::Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue);

                returnedValue = 0;
                EXPECT_FALSE(V8TypeConverter<uint64_t>::From(m_Isolate, v8::Null(m_Isolate).As<v8::Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue);

                returnedValue = 0;
                EXPECT_FALSE(V8TypeConverter<uint64_t>::From(m_Isolate, v8::Undefined(m_Isolate).As<v8::Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue);
            }

            TEST_F(ConverterTests, Float)
            {
                v8::HandleScope scope(m_Isolate);

                EXPECT_TRUE(V8TypeConverter<float>::To(m_Isolate, -5.0f)->StrictEquals(v8::Number::New(m_Isolate, -5.0f)));
                EXPECT_TRUE(V8TypeConverter<float>::To(m_Isolate, 0.0f)->StrictEquals(v8::Number::New(m_Isolate, 0.0f)));
                EXPECT_TRUE(V8TypeConverter<float>::To(m_Isolate, 5.0f)->StrictEquals(v8::Number::New(m_Isolate, 5.0f)));

                float returnedValue = 0;

                EXPECT_FALSE(V8TypeConverter<float>::From(m_Isolate, v8::Boolean::New(m_Isolate, false).As<v8::Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue);

                returnedValue = 0;
                EXPECT_FALSE(V8TypeConverter<float>::From(m_Isolate, v8::Boolean::New(m_Isolate, true).As<v8::Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue);

                returnedValue = 0;
                EXPECT_TRUE(V8TypeConverter<float>::From(m_Isolate, v8::Number::New(m_Isolate, 0.0f).As<v8::Value>(), &returnedValue));
                EXPECT_EQ(0.0f, returnedValue);

                returnedValue = 0;
                EXPECT_TRUE(V8TypeConverter<float>::From(m_Isolate, v8::Number::New(m_Isolate, -5.0f).As<v8::Value>(), &returnedValue));
                EXPECT_EQ(-5.0f, returnedValue);

                returnedValue = 0;
                EXPECT_TRUE(V8TypeConverter<float>::From(m_Isolate, v8::Integer::New(m_Isolate, -5).As<v8::Value>(), &returnedValue));
                //-5 in uint54
                EXPECT_EQ(-5.0f, returnedValue);

                returnedValue = 0;
                EXPECT_TRUE(V8TypeConverter<float>::From(m_Isolate, v8::Integer::New(m_Isolate, 5).As<v8::Value>(), &returnedValue));
                EXPECT_EQ(5.0f, returnedValue);

                returnedValue = 0;
                EXPECT_FALSE(V8TypeConverter<float>::From(m_Isolate, v8::String::NewFromUtf8(m_Isolate, "", v8::NewStringType::kNormal).ToLocalChecked().As<v8::Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue);

                returnedValue = 0;
                EXPECT_FALSE(V8TypeConverter<float>::From(m_Isolate, v8::String::NewFromUtf8(m_Isolate, "foo", v8::NewStringType::kNormal).ToLocalChecked().As<v8::Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue);

                returnedValue = 0;
                EXPECT_FALSE(V8TypeConverter<float>::From(m_Isolate, v8::Object::New(m_Isolate).As<v8::Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue);

                returnedValue = 0;
                EXPECT_FALSE(V8TypeConverter<float>::From(m_Isolate, v8::Null(m_Isolate).As<v8::Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue);

                returnedValue = 0;
                EXPECT_FALSE(V8TypeConverter<float>::From(m_Isolate, v8::Undefined(m_Isolate).As<v8::Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue);
            }

            TEST_F(ConverterTests, Double)
            {
                v8::HandleScope scope(m_Isolate);

                EXPECT_TRUE(V8TypeConverter<double>::To(m_Isolate, -5.0)->StrictEquals(v8::Number::New(m_Isolate, -5.0)));
                EXPECT_TRUE(V8TypeConverter<double>::To(m_Isolate, 0.0)->StrictEquals(v8::Number::New(m_Isolate, 0.0)));
                EXPECT_TRUE(V8TypeConverter<double>::To(m_Isolate, 5.0)->StrictEquals(v8::Number::New(m_Isolate, 5.0)));

                double returnedValue = 0;

                EXPECT_FALSE(V8TypeConverter<double>::From(m_Isolate, v8::Boolean::New(m_Isolate, false).As<v8::Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue);

                returnedValue = 0;
                EXPECT_FALSE(V8TypeConverter<double>::From(m_Isolate, v8::Boolean::New(m_Isolate, true).As<v8::Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue);

                returnedValue = 0;
                EXPECT_TRUE(V8TypeConverter<double>::From(m_Isolate, v8::Number::New(m_Isolate, 0.0).As<v8::Value>(), &returnedValue));
                EXPECT_EQ(0.0, returnedValue);

                returnedValue = 0;
                EXPECT_TRUE(V8TypeConverter<double>::From(m_Isolate, v8::Number::New(m_Isolate, -5.0).As<v8::Value>(), &returnedValue));
                EXPECT_EQ(-5.0, returnedValue);

                returnedValue = 0;
                EXPECT_TRUE(V8TypeConverter<double>::From(m_Isolate, v8::Integer::New(m_Isolate, -5).As<v8::Value>(), &returnedValue));
                //-5 in uint54
                EXPECT_EQ(-5.0, returnedValue);

                returnedValue = 0;
                EXPECT_TRUE(V8TypeConverter<double>::From(m_Isolate, v8::Integer::New(m_Isolate, 5).As<v8::Value>(), &returnedValue));
                EXPECT_EQ(5.0, returnedValue);

                returnedValue = 0;
                EXPECT_FALSE(V8TypeConverter<double>::From(m_Isolate, v8::String::NewFromUtf8(m_Isolate, "", v8::NewStringType::kNormal).ToLocalChecked().As<v8::Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue);

                returnedValue = 0;
                EXPECT_FALSE(V8TypeConverter<double>::From(m_Isolate, v8::String::NewFromUtf8(m_Isolate, "foo", v8::NewStringType::kNormal).ToLocalChecked().As<v8::Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue);

                returnedValue = 0;
                EXPECT_FALSE(V8TypeConverter<double>::From(m_Isolate, v8::Object::New(m_Isolate).As<v8::Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue);

                returnedValue = 0;
                EXPECT_FALSE(V8TypeConverter<double>::From(m_Isolate, v8::Null(m_Isolate).As<v8::Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue);

                returnedValue = 0;
                EXPECT_FALSE(V8TypeConverter<double>::From(m_Isolate, v8::Undefined(m_Isolate).As<v8::Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue);
            }

            TEST_F(ConverterTests, StdString)
            {
                v8::HandleScope scope(m_Isolate);

                std::string str = "";
                EXPECT_TRUE(V8TypeConverter<std::string>::To(m_Isolate, str)->StrictEquals(v8::String::NewFromUtf8(m_Isolate, "").ToLocalChecked()));
                str = "Test";
                EXPECT_TRUE(V8TypeConverter<std::string>::To(m_Isolate, str)->StrictEquals(v8::String::NewFromUtf8(m_Isolate, "Test").ToLocalChecked()));

                std::string returnedValue = "";

                EXPECT_FALSE(V8TypeConverter<std::string>::From(m_Isolate, v8::Boolean::New(m_Isolate, false).As<v8::Value>(), &returnedValue));
                EXPECT_EQ("", returnedValue);

                returnedValue = "";
                EXPECT_FALSE(V8TypeConverter<std::string>::From(m_Isolate, v8::Boolean::New(m_Isolate, true).As<v8::Value>(), &returnedValue));
                EXPECT_EQ("", returnedValue);

                returnedValue = "";
                EXPECT_FALSE(V8TypeConverter<std::string>::From(m_Isolate, v8::Number::New(m_Isolate, 0.0).As<v8::Value>(), &returnedValue));
                EXPECT_EQ("", returnedValue);

                returnedValue = "";
                EXPECT_FALSE(V8TypeConverter<std::string>::From(m_Isolate, v8::Number::New(m_Isolate, -5.0).As<v8::Value>(), &returnedValue));
                EXPECT_EQ("", returnedValue);

                returnedValue = "";
                EXPECT_FALSE(V8TypeConverter<std::string>::From(m_Isolate, v8::Integer::New(m_Isolate, -5).As<v8::Value>(), &returnedValue));
                EXPECT_EQ("", returnedValue);

                returnedValue = "";
                EXPECT_FALSE(V8TypeConverter<std::string>::From(m_Isolate, v8::Integer::New(m_Isolate, 5).As<v8::Value>(), &returnedValue));
                EXPECT_EQ("", returnedValue);

                returnedValue = "";
                EXPECT_TRUE(V8TypeConverter<std::string>::From(m_Isolate, v8::String::NewFromUtf8(m_Isolate, "", v8::NewStringType::kNormal).ToLocalChecked().As<v8::Value>(), &returnedValue));
                EXPECT_EQ("", returnedValue);

                returnedValue = "";
                EXPECT_TRUE(V8TypeConverter<std::string>::From(m_Isolate, v8::String::NewFromUtf8(m_Isolate, "foo", v8::NewStringType::kNormal).ToLocalChecked().As<v8::Value>(), &returnedValue));
                EXPECT_EQ("foo", returnedValue);

                returnedValue = "";
                EXPECT_FALSE(V8TypeConverter<std::string>::From(m_Isolate, v8::Object::New(m_Isolate).As<v8::Value>(), &returnedValue));
                EXPECT_EQ("", returnedValue);

                returnedValue = "";
                EXPECT_FALSE(V8TypeConverter<std::string>::From(m_Isolate, v8::Null(m_Isolate).As<v8::Value>(), &returnedValue));
                EXPECT_EQ("", returnedValue);

                returnedValue = "";
                EXPECT_FALSE(V8TypeConverter<std::string>::From(m_Isolate, v8::Undefined(m_Isolate).As<v8::Value>(), &returnedValue));
                EXPECT_EQ("", returnedValue);

                //test the create symbol
                str = "TestSymbol";
                v8::Local<v8::String> symbol = CreateSymbol(m_Isolate, str);
                EXPECT_TRUE(V8TypeConverter<std::string>::From(m_Isolate, symbol.As<v8::Value>(), &returnedValue));
                EXPECT_EQ(str.c_str(), returnedValue);

                //test the to string function
                str = "TestString";
                std::string emptyString;
                v8::Local<v8::Value> emptyValue;
                v8::Local<v8::Value> testValue = v8::String::NewFromUtf8(m_Isolate, "TestString", v8::NewStringType::kNormal).ToLocalChecked().As<v8::Value>();
                EXPECT_EQ(emptyString, V8ToString(m_Isolate, emptyValue));
                EXPECT_EQ(emptyString, V8ToString(m_Isolate, v8::Undefined(m_Isolate).As<v8::Value>()));
                EXPECT_EQ(str, V8ToString(m_Isolate, testValue));

                //test StringToV8
                v8::Local<v8::String> v8String = StringToV8(m_Isolate, str);
                //resue the converted value above to test on the V* side
                EXPECT_TRUE(v8String->StrictEquals(testValue));
            }

            TEST_F(ConverterTests, StdU16String)
            {
                v8::HandleScope scope(m_Isolate);

                std::u16string str = u"";
                EXPECT_TRUE(V8TypeConverter<std::u16string>::To(m_Isolate, str)->StrictEquals(v8::String::NewFromTwoByte(m_Isolate, reinterpret_cast<const uint16_t *>(u"")).ToLocalChecked()));
                str = u"Test";
                EXPECT_TRUE(V8TypeConverter<std::u16string>::To(m_Isolate, str)->StrictEquals(v8::String::NewFromTwoByte(m_Isolate, reinterpret_cast<const uint16_t *>(u"Test"), v8::NewStringType::kNormal, 4).ToLocalChecked()));

                std::u16string returnedValue = u"";

                EXPECT_FALSE(V8TypeConverter<std::u16string>::From(m_Isolate, v8::Boolean::New(m_Isolate, false).As<v8::Value>(), &returnedValue));
                EXPECT_EQ(u"", returnedValue);

                returnedValue = u"";
                EXPECT_FALSE(V8TypeConverter<std::u16string>::From(m_Isolate, v8::Boolean::New(m_Isolate, true).As<v8::Value>(), &returnedValue));
                EXPECT_EQ(u"", returnedValue);

                returnedValue = u"";
                EXPECT_FALSE(V8TypeConverter<std::u16string>::From(m_Isolate, v8::Number::New(m_Isolate, 0.0).As<v8::Value>(), &returnedValue));
                EXPECT_EQ(u"", returnedValue);

                returnedValue = u"";
                EXPECT_FALSE(V8TypeConverter<std::u16string>::From(m_Isolate, v8::Number::New(m_Isolate, -5.0).As<v8::Value>(), &returnedValue));
                EXPECT_EQ(u"", returnedValue);

                returnedValue = u"";
                EXPECT_FALSE(V8TypeConverter<std::u16string>::From(m_Isolate, v8::Integer::New(m_Isolate, -5).As<v8::Value>(), &returnedValue));
                EXPECT_EQ(u"", returnedValue);

                returnedValue = u"";
                EXPECT_FALSE(V8TypeConverter<std::u16string>::From(m_Isolate, v8::Integer::New(m_Isolate, 5).As<v8::Value>(), &returnedValue));
                EXPECT_EQ(u"", returnedValue);

                returnedValue = u"";
                EXPECT_TRUE(V8TypeConverter<std::u16string>::From(m_Isolate, v8::String::NewFromUtf8(m_Isolate, "", v8::NewStringType::kNormal).ToLocalChecked().As<v8::Value>(), &returnedValue));
                EXPECT_EQ(u"", returnedValue);

                returnedValue = u"";
                EXPECT_TRUE(V8TypeConverter<std::u16string>::From(m_Isolate, v8::String::NewFromUtf8(m_Isolate, "foo", v8::NewStringType::kNormal).ToLocalChecked().As<v8::Value>(), &returnedValue));
                EXPECT_EQ(u"foo", returnedValue);

                returnedValue = u"";
                EXPECT_FALSE(V8TypeConverter<std::u16string>::From(m_Isolate, v8::Object::New(m_Isolate).As<v8::Value>(), &returnedValue));
                EXPECT_EQ(u"", returnedValue);

                returnedValue = u"";
                EXPECT_FALSE(V8TypeConverter<std::u16string>::From(m_Isolate, v8::Null(m_Isolate).As<v8::Value>(), &returnedValue));
                EXPECT_EQ(u"", returnedValue);

                returnedValue = u"";
                EXPECT_FALSE(V8TypeConverter<std::u16string>::From(m_Isolate, v8::Undefined(m_Isolate).As<v8::Value>(), &returnedValue));
                EXPECT_EQ(u"", returnedValue);

                //test the create symbol
                str = u"TestSymbol";
                v8::Local<v8::String> symbol = CreateSymbol(m_Isolate, str);
                EXPECT_TRUE(V8TypeConverter<std::u16string>::From(m_Isolate, symbol.As<v8::Value>(), &returnedValue));
                EXPECT_EQ(str, returnedValue);

                //test the to string function
                str = u"TestString";
                std::u16string emptyString;
                v8::Local<v8::Value> emptyValue;
                v8::Local<v8::Value> testValue = v8::String::NewFromTwoByte(m_Isolate, reinterpret_cast<const uint16_t *>(u"TestString")).ToLocalChecked().As<v8::Value>();
                EXPECT_EQ(emptyString, V8ToU16String(m_Isolate, emptyValue));
                EXPECT_EQ(emptyString, V8ToU16String(m_Isolate, v8::Undefined(m_Isolate).As<v8::Value>()));
                EXPECT_EQ(str, V8ToU16String(m_Isolate, testValue));

                //test StringToV8
                v8::Local<v8::String> v8String = U16StringToV8(m_Isolate, str);
                //resue the converted value above to test on the V* side
                EXPECT_TRUE(v8String->StrictEquals(testValue));
            }

            static void testFunc(v8::FunctionCallbackInfo<v8::Value> const &args)
            {
                //do nothign it's just a test and won't be called
                return;
            }

            TEST_F(ConverterTests, V8Function)
            {
                v8::HandleScope scope(m_Isolate);

                v8::Local<v8::Function> func = v8::Function::New(m_Isolate->GetCurrentContext(), testFunc).ToLocalChecked();
                EXPECT_TRUE(V8TypeConverter<v8::Local<v8::Function>>::To(m_Isolate, func)->StrictEquals(func));

                v8::Local<v8::Function> returnedValue;

                EXPECT_FALSE(V8TypeConverter<v8::Local<v8::Function>>::From(m_Isolate, v8::Boolean::New(m_Isolate, false).As<v8::Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_FALSE(V8TypeConverter<v8::Local<v8::Function>>::From(m_Isolate, v8::Boolean::New(m_Isolate, true).As<v8::Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_FALSE(V8TypeConverter<v8::Local<v8::Function>>::From(m_Isolate, v8::Number::New(m_Isolate, 0.0).As<v8::Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_FALSE(V8TypeConverter<v8::Local<v8::Function>>::From(m_Isolate, v8::Number::New(m_Isolate, -5.0).As<v8::Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_FALSE(V8TypeConverter<v8::Local<v8::Function>>::From(m_Isolate, v8::Integer::New(m_Isolate, -5).As<v8::Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_FALSE(V8TypeConverter<v8::Local<v8::Function>>::From(m_Isolate, v8::Integer::New(m_Isolate, 5).As<v8::Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_FALSE(V8TypeConverter<v8::Local<v8::Function>>::From(m_Isolate, v8::String::NewFromUtf8(m_Isolate, "", v8::NewStringType::kNormal).ToLocalChecked().As<v8::Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_FALSE(V8TypeConverter<v8::Local<v8::Function>>::From(m_Isolate, v8::String::NewFromUtf8(m_Isolate, "foo", v8::NewStringType::kNormal).ToLocalChecked().As<v8::Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_FALSE(V8TypeConverter<v8::Local<v8::Function>>::From(m_Isolate, v8::Object::New(m_Isolate).As<v8::Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_FALSE(V8TypeConverter<v8::Local<v8::Function>>::From(m_Isolate, v8::Null(m_Isolate).As<v8::Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_FALSE(V8TypeConverter<v8::Local<v8::Function>>::From(m_Isolate, v8::Undefined(m_Isolate).As<v8::Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_TRUE(V8TypeConverter<v8::Local<v8::Function>>::From(m_Isolate, v8::Function::New(m_Isolate->GetCurrentContext(), testFunc).ToLocalChecked(), &returnedValue));
                EXPECT_FALSE(returnedValue.IsEmpty());
            }

            TEST_F(ConverterTests, V8Object)
            {
                v8::HandleScope scope(m_Isolate);

                v8::Local<v8::Object> obj = v8::Object::New(m_Isolate);
                EXPECT_TRUE(V8TypeConverter<v8::Local<v8::Object>>::To(m_Isolate, obj)->StrictEquals(obj));

                v8::Local<v8::Object> returnedValue;

                EXPECT_FALSE(V8TypeConverter<v8::Local<v8::Object>>::From(m_Isolate, v8::Boolean::New(m_Isolate, false).As<v8::Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_FALSE(V8TypeConverter<v8::Local<v8::Object>>::From(m_Isolate, v8::Boolean::New(m_Isolate, true).As<v8::Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_FALSE(V8TypeConverter<v8::Local<v8::Object>>::From(m_Isolate, v8::Number::New(m_Isolate, 0.0).As<v8::Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_FALSE(V8TypeConverter<v8::Local<v8::Object>>::From(m_Isolate, v8::Number::New(m_Isolate, -5.0).As<v8::Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_FALSE(V8TypeConverter<v8::Local<v8::Object>>::From(m_Isolate, v8::Integer::New(m_Isolate, -5).As<v8::Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_FALSE(V8TypeConverter<v8::Local<v8::Object>>::From(m_Isolate, v8::Integer::New(m_Isolate, 5).As<v8::Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_FALSE(V8TypeConverter<v8::Local<v8::Object>>::From(m_Isolate, v8::String::NewFromUtf8(m_Isolate, "", v8::NewStringType::kNormal).ToLocalChecked().As<v8::Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_FALSE(V8TypeConverter<v8::Local<v8::Object>>::From(m_Isolate, v8::String::NewFromUtf8(m_Isolate, "foo", v8::NewStringType::kNormal).ToLocalChecked().As<v8::Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_TRUE(V8TypeConverter<v8::Local<v8::Object>>::From(m_Isolate, v8::Object::New(m_Isolate).As<v8::Value>(), &returnedValue));
                EXPECT_FALSE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_FALSE(V8TypeConverter<v8::Local<v8::Object>>::From(m_Isolate, v8::Null(m_Isolate).As<v8::Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_FALSE(V8TypeConverter<v8::Local<v8::Object>>::From(m_Isolate, v8::Undefined(m_Isolate).As<v8::Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());
            }

            TEST_F(ConverterTests, V8Promise)
            {
                v8::HandleScope scope(m_Isolate);

                auto resolver = v8::Promise::Resolver::New(m_Isolate->GetCurrentContext());
                ASSERT_FALSE(resolver.IsEmpty());

                v8::Local<v8::Promise> promise = resolver.ToLocalChecked()->GetPromise();
                EXPECT_TRUE(V8TypeConverter<v8::Local<v8::Promise>>::To(m_Isolate, promise)->StrictEquals(promise));

                v8::Local<v8::Promise> returnedValue;

                EXPECT_FALSE(V8TypeConverter<v8::Local<v8::Promise>>::From(m_Isolate, v8::Boolean::New(m_Isolate, false).As<v8::Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_FALSE(V8TypeConverter<v8::Local<v8::Promise>>::From(m_Isolate, v8::Boolean::New(m_Isolate, true).As<v8::Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_FALSE(V8TypeConverter<v8::Local<v8::Promise>>::From(m_Isolate, v8::Number::New(m_Isolate, 0.0).As<v8::Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_FALSE(V8TypeConverter<v8::Local<v8::Promise>>::From(m_Isolate, v8::Number::New(m_Isolate, -5.0).As<v8::Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_FALSE(V8TypeConverter<v8::Local<v8::Promise>>::From(m_Isolate, v8::Integer::New(m_Isolate, -5).As<v8::Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_FALSE(V8TypeConverter<v8::Local<v8::Promise>>::From(m_Isolate, v8::Integer::New(m_Isolate, 5).As<v8::Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_FALSE(V8TypeConverter<v8::Local<v8::Promise>>::From(m_Isolate, v8::String::NewFromUtf8(m_Isolate, "", v8::NewStringType::kNormal).ToLocalChecked().As<v8::Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_FALSE(V8TypeConverter<v8::Local<v8::Promise>>::From(m_Isolate, v8::String::NewFromUtf8(m_Isolate, "foo", v8::NewStringType::kNormal).ToLocalChecked().As<v8::Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_FALSE(V8TypeConverter<v8::Local<v8::Promise>>::From(m_Isolate, v8::Object::New(m_Isolate).As<v8::Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_FALSE(V8TypeConverter<v8::Local<v8::Promise>>::From(m_Isolate, v8::Null(m_Isolate).As<v8::Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_FALSE(V8TypeConverter<v8::Local<v8::Promise>>::From(m_Isolate, v8::Undefined(m_Isolate).As<v8::Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_TRUE(V8TypeConverter<v8::Local<v8::Promise>>::From(m_Isolate, resolver.ToLocalChecked()->GetPromise(), &returnedValue));
                EXPECT_FALSE(returnedValue.IsEmpty());
            }

            TEST_F(ConverterTests, V8ArrayBuffer)
            {
                v8::HandleScope scope(m_Isolate);

                v8::Local<v8::ArrayBuffer> buffer = v8::ArrayBuffer::New(m_Isolate, 1);
                EXPECT_TRUE(V8TypeConverter<v8::Local<v8::ArrayBuffer>>::To(m_Isolate, buffer)->StrictEquals(buffer));

                v8::Local<v8::ArrayBuffer> returnedValue;

                EXPECT_FALSE(V8TypeConverter<v8::Local<v8::ArrayBuffer>>::From(m_Isolate, v8::Boolean::New(m_Isolate, false).As<v8::Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_FALSE(V8TypeConverter<v8::Local<v8::ArrayBuffer>>::From(m_Isolate, v8::Boolean::New(m_Isolate, true).As<v8::Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_FALSE(V8TypeConverter<v8::Local<v8::ArrayBuffer>>::From(m_Isolate, v8::Number::New(m_Isolate, 0.0).As<v8::Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_FALSE(V8TypeConverter<v8::Local<v8::ArrayBuffer>>::From(m_Isolate, v8::Number::New(m_Isolate, -5.0).As<v8::Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_FALSE(V8TypeConverter<v8::Local<v8::ArrayBuffer>>::From(m_Isolate, v8::Integer::New(m_Isolate, -5).As<v8::Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_FALSE(V8TypeConverter<v8::Local<v8::ArrayBuffer>>::From(m_Isolate, v8::Integer::New(m_Isolate, 5).As<v8::Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_FALSE(V8TypeConverter<v8::Local<v8::ArrayBuffer>>::From(m_Isolate, v8::String::NewFromUtf8(m_Isolate, "", v8::NewStringType::kNormal).ToLocalChecked().As<v8::Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_FALSE(V8TypeConverter<v8::Local<v8::ArrayBuffer>>::From(m_Isolate, v8::String::NewFromUtf8(m_Isolate, "foo", v8::NewStringType::kNormal).ToLocalChecked().As<v8::Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_FALSE(V8TypeConverter<v8::Local<v8::ArrayBuffer>>::From(m_Isolate, v8::Object::New(m_Isolate).As<v8::Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_FALSE(V8TypeConverter<v8::Local<v8::ArrayBuffer>>::From(m_Isolate, v8::Null(m_Isolate).As<v8::Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_FALSE(V8TypeConverter<v8::Local<v8::ArrayBuffer>>::From(m_Isolate, v8::Undefined(m_Isolate).As<v8::Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_TRUE(V8TypeConverter<v8::Local<v8::ArrayBuffer>>::From(m_Isolate, v8::ArrayBuffer::New(m_Isolate, 1).As<v8::Value>(), &returnedValue));
                EXPECT_FALSE(returnedValue.IsEmpty());
            }

            TEST_F(ConverterTests, V8External)
            {
                v8::HandleScope scope(m_Isolate);

                v8::Local<v8::External> external = v8::Local<v8::External>::New(m_Isolate, v8::External::New(m_Isolate, (void *)testFunc));
                EXPECT_TRUE(V8TypeConverter<v8::Local<v8::External>>::To(m_Isolate, external)->StrictEquals(external));

                v8::Local<v8::External> returnedValue;

                EXPECT_FALSE(V8TypeConverter<v8::Local<v8::External>>::From(m_Isolate, v8::Boolean::New(m_Isolate, false).As<v8::Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_FALSE(V8TypeConverter<v8::Local<v8::External>>::From(m_Isolate, v8::Boolean::New(m_Isolate, true).As<v8::Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_FALSE(V8TypeConverter<v8::Local<v8::External>>::From(m_Isolate, v8::Number::New(m_Isolate, 0.0).As<v8::Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_FALSE(V8TypeConverter<v8::Local<v8::External>>::From(m_Isolate, v8::Number::New(m_Isolate, -5.0).As<v8::Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_FALSE(V8TypeConverter<v8::Local<v8::External>>::From(m_Isolate, v8::Integer::New(m_Isolate, -5).As<v8::Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_FALSE(V8TypeConverter<v8::Local<v8::External>>::From(m_Isolate, v8::Integer::New(m_Isolate, 5).As<v8::Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_FALSE(V8TypeConverter<v8::Local<v8::External>>::From(m_Isolate, v8::String::NewFromUtf8(m_Isolate, "", v8::NewStringType::kNormal).ToLocalChecked().As<v8::Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_FALSE(V8TypeConverter<v8::Local<v8::External>>::From(m_Isolate, v8::String::NewFromUtf8(m_Isolate, "foo", v8::NewStringType::kNormal).ToLocalChecked().As<v8::Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_FALSE(V8TypeConverter<v8::Local<v8::External>>::From(m_Isolate, v8::Object::New(m_Isolate).As<v8::Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_FALSE(V8TypeConverter<v8::Local<v8::External>>::From(m_Isolate, v8::Null(m_Isolate).As<v8::Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_FALSE(V8TypeConverter<v8::Local<v8::External>>::From(m_Isolate, v8::Undefined(m_Isolate).As<v8::Value>(), &returnedValue));
                EXPECT_TRUE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_TRUE(V8TypeConverter<v8::Local<v8::External>>::From(m_Isolate, v8::External::New(m_Isolate, (void *)testFunc).As<v8::Value>(), &returnedValue));
                EXPECT_FALSE(returnedValue.IsEmpty());
            }

            TEST_F(ConverterTests, V8Value)
            {
                v8::HandleScope scope(m_Isolate);

                v8::Local<v8::Value> value = v8::Object::New(m_Isolate).As<v8::Value>();
                EXPECT_TRUE(V8TypeConverter<v8::Local<v8::Value>>::To(m_Isolate, value)->StrictEquals(value));

                v8::Local<v8::Value> returnedValue;

                // vaue is the base of everything so these will call convert
                EXPECT_TRUE(V8TypeConverter<v8::Local<v8::Value>>::From(m_Isolate, v8::Boolean::New(m_Isolate, false).As<v8::Value>(), &returnedValue));
                EXPECT_FALSE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_TRUE(V8TypeConverter<v8::Local<v8::Value>>::From(m_Isolate, v8::Boolean::New(m_Isolate, true).As<v8::Value>(), &returnedValue));
                EXPECT_FALSE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_TRUE(V8TypeConverter<v8::Local<v8::Value>>::From(m_Isolate, v8::Number::New(m_Isolate, 0.0).As<v8::Value>(), &returnedValue));
                EXPECT_FALSE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_TRUE(V8TypeConverter<v8::Local<v8::Value>>::From(m_Isolate, v8::Number::New(m_Isolate, -5.0).As<v8::Value>(), &returnedValue));
                EXPECT_FALSE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_TRUE(V8TypeConverter<v8::Local<v8::Value>>::From(m_Isolate, v8::Integer::New(m_Isolate, -5).As<v8::Value>(), &returnedValue));
                EXPECT_FALSE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_TRUE(V8TypeConverter<v8::Local<v8::Value>>::From(m_Isolate, v8::Integer::New(m_Isolate, 5).As<v8::Value>(), &returnedValue));
                EXPECT_FALSE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_TRUE(V8TypeConverter<v8::Local<v8::Value>>::From(m_Isolate, v8::String::NewFromUtf8(m_Isolate, "", v8::NewStringType::kNormal).ToLocalChecked().As<v8::Value>(), &returnedValue));
                EXPECT_FALSE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_TRUE(V8TypeConverter<v8::Local<v8::Value>>::From(m_Isolate, v8::String::NewFromUtf8(m_Isolate, "foo", v8::NewStringType::kNormal).ToLocalChecked().As<v8::Value>(), &returnedValue));
                EXPECT_FALSE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_TRUE(V8TypeConverter<v8::Local<v8::Value>>::From(m_Isolate, v8::Object::New(m_Isolate).As<v8::Value>(), &returnedValue));
                EXPECT_FALSE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_TRUE(V8TypeConverter<v8::Local<v8::Value>>::From(m_Isolate, v8::Null(m_Isolate).As<v8::Value>(), &returnedValue));
                EXPECT_FALSE(returnedValue.IsEmpty());

                returnedValue.Clear();
                EXPECT_TRUE(V8TypeConverter<v8::Local<v8::Value>>::From(m_Isolate, v8::Undefined(m_Isolate).As<v8::Value>(), &returnedValue));
                EXPECT_FALSE(returnedValue.IsEmpty());
            }

            TEST_F(ConverterTests, StdVector)
            {
                v8::HandleScope scope(m_Isolate);

                std::vector<int> expected = {-1, 0, 1};

                v8::Local<v8::Array> array = V8TypeConverter<std::vector<int>>::To(m_Isolate, expected).As<v8::Array>();
                EXPECT_EQ(3, array->Length());
                EXPECT_TRUE(v8::Integer::New(m_Isolate, expected[0])->StrictEquals(array->Get(m_Isolate->GetCurrentContext(), 0).ToLocalChecked()));
                EXPECT_TRUE(v8::Integer::New(m_Isolate, expected[1])->StrictEquals(array->Get(m_Isolate->GetCurrentContext(), 1).ToLocalChecked()));
                EXPECT_TRUE(v8::Integer::New(m_Isolate, expected[2])->StrictEquals(array->Get(m_Isolate->GetCurrentContext(), 2).ToLocalChecked()));

                std::vector<int> returnedValue;

                EXPECT_FALSE(V8TypeConverter<std::vector<int>>::From(m_Isolate, v8::Boolean::New(m_Isolate, false).As<v8::Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue.size());

                returnedValue.clear();
                EXPECT_FALSE(V8TypeConverter<std::vector<int>>::From(m_Isolate, v8::Boolean::New(m_Isolate, true).As<v8::Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue.size());

                returnedValue.clear();
                EXPECT_FALSE(V8TypeConverter<std::vector<int>>::From(m_Isolate, v8::Number::New(m_Isolate, 0.0).As<v8::Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue.size());

                returnedValue.clear();
                EXPECT_FALSE(V8TypeConverter<std::vector<int>>::From(m_Isolate, v8::Number::New(m_Isolate, -5.0).As<v8::Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue.size());

                returnedValue.clear();
                EXPECT_FALSE(V8TypeConverter<std::vector<int>>::From(m_Isolate, v8::Integer::New(m_Isolate, -5).As<v8::Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue.size());

                returnedValue.clear();
                EXPECT_FALSE(V8TypeConverter<std::vector<int>>::From(m_Isolate, v8::Integer::New(m_Isolate, 5).As<v8::Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue.size());

                returnedValue.clear();
                EXPECT_FALSE(V8TypeConverter<std::vector<int>>::From(m_Isolate, v8::String::NewFromUtf8(m_Isolate, "", v8::NewStringType::kNormal).ToLocalChecked().As<v8::Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue.size());

                returnedValue.clear();
                EXPECT_FALSE(V8TypeConverter<std::vector<int>>::From(m_Isolate, v8::String::NewFromUtf8(m_Isolate, "foo", v8::NewStringType::kNormal).ToLocalChecked().As<v8::Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue.size());

                returnedValue.clear();
                EXPECT_FALSE(V8TypeConverter<std::vector<int>>::From(m_Isolate, v8::Object::New(m_Isolate).As<v8::Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue.size());

                returnedValue.clear();
                EXPECT_FALSE(V8TypeConverter<std::vector<int>>::From(m_Isolate, v8::Null(m_Isolate).As<v8::Value>(), &returnedValue));
                EXPECT_EQ(0, returnedValue.size());

                returnedValue.clear();
                EXPECT_FALSE(V8TypeConverter<std::vector<int>>::From(m_Isolate, v8::Undefined(m_Isolate).As<v8::Value>(), &returnedValue));
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
                v8::HandleScope scope(m_Isolate);
                using vvInt = std::vector<std::vector<int>>;
                vvInt expected = {{-1, 0, 1}, {3, 4, 5}};

                v8::Local<v8::Array> array = V8TypeConverter<vvInt>::To(m_Isolate, expected).As<v8::Array>();
                EXPECT_EQ(2, array->Length());

                vvInt returnedValue;

                EXPECT_TRUE(V8TypeConverter<vvInt>::From(m_Isolate, array, &returnedValue));
                EXPECT_TRUE(expected == returnedValue);
            }

            TEST_F(ConverterTests, ConvertToFromV8)
            {
                v8::HandleScope scope(m_Isolate);

                std::string strVal("Test");
                v8::Local<v8::External> external = v8::Local<v8::External>::New(m_Isolate, v8::External::New(m_Isolate, (void *)testFunc));

                v8::Local<v8::Value> boolean = ConvertToV8(m_Isolate, true);
                v8::Local<v8::Value> integer = ConvertToV8(m_Isolate, 10);
                v8::Local<v8::Value> string = ConvertToV8(m_Isolate, strVal);
                v8::Local<v8::Value> external2 = ConvertToV8(m_Isolate, external);

                bool outBool = false;
                int32_t outInt = 0;
                std::string outString;
                v8::Local<v8::Value> outValue;

                EXPECT_TRUE(ConvertFromV8(m_Isolate, boolean, &outBool));
                EXPECT_TRUE(ConvertFromV8(m_Isolate, integer, &outInt));
                EXPECT_TRUE(ConvertFromV8(m_Isolate, string, &outString));
                EXPECT_TRUE(ConvertFromV8(m_Isolate, external2, &outValue));

                EXPECT_EQ(true, outBool);
                EXPECT_EQ(10, outInt);
                EXPECT_EQ("Test", outString);
                EXPECT_TRUE(outValue->IsExternal());
            }

            //create a test type for the returns maybe
            struct testStruct
            {
            };

            //specialization to say the type returns a maybe
            template <>
            struct ToReturnsMaybe<testStruct>
            {
                static const bool Value = true;
            };

            //specialization for the test type for the conversion to v8
            template <>
            struct V8TypeConverter<testStruct>
            {
                static v8::MaybeLocal<v8::Value> To(v8::Isolate *inIsolate, const testStruct &inValue)
                {
                    return v8::MaybeLocal<v8::Value>(v8::Object::New(inIsolate));
                }
            };

            TEST_F(ConverterTests, TryConvertToV8)
            {
                v8::HandleScope scope(m_Isolate);

                v8::Local<v8::Value> value;
                std::string str = "Test";
                bool result = TryConvertToV8(m_Isolate, str, &value);
                EXPECT_TRUE(result);
                EXPECT_TRUE(value->StrictEquals(v8::String::NewFromUtf8(m_Isolate, str.c_str()).ToLocalChecked()));

                testStruct test_struct;
                result = TryConvertToV8(m_Isolate, test_struct, &value);
                EXPECT_TRUE(result);
            }
        } // namespace Bridge
    }     // namespace JSRuntime
} // namespace v8App