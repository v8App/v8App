// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "V8TestFixture.h"
#include "JSUtilites.h"

namespace v8App
{
    namespace JSRuntime
    {
        namespace JSUtilities
        {
            using V8UtilitiesTest = V8TestFixture;

            TEST_F(V8UtilitiesTest, TestGetStackTrace)
            {
                const char scriptStr[] = R"script(
                        function test(
                        {
                            let x = 5;
                        }
                )script";

                v8::HandleScope scope(m_Isolate);
                v8::TryCatch tryCatch(m_Isolate);
                v8::Local<v8::ObjectTemplate> global = v8::ObjectTemplate::New(m_Isolate);
                v8::Local<v8::Context> context = v8::Context::New(m_Isolate, nullptr, global);

                v8::Local<v8::String> source = StringToV8(m_Isolate, scriptStr);

                v8::Local<v8::Script> script;
                EXPECT_FALSE(v8::Script::Compile(context, source).ToLocal(&script));

                EXPECT_TRUE(tryCatch.HasCaught());

                std::string error(R"error(Uncaught SyntaxError: Unexpected identifier
                            let x = 5;
)error");

                EXPECT_EQ(error, JSUtilities::GetStackTrace(m_Isolate, tryCatch));
            }

            TEST_F(V8UtilitiesTest, TestThrowError)
            {
                v8::HandleScope scope(m_Isolate);
                v8::TryCatch tryCatch(m_Isolate);

                JSUtilities::ThrowV8Error(m_Isolate, JSUtilities::V8Errors::RangeError, "Range Error");
                EXPECT_TRUE(tryCatch.HasCaught());
                EXPECT_EQ("Uncaught RangeError: Range Error", V8ToString(m_Isolate, tryCatch.Message()->Get()));

                tryCatch.Reset();

                JSUtilities::ThrowV8Error(m_Isolate, JSUtilities::V8Errors::ReferenceError, "Reference Error");
                EXPECT_TRUE(tryCatch.HasCaught());
                EXPECT_EQ("Uncaught ReferenceError: Reference Error", V8ToString(m_Isolate, tryCatch.Message()->Get()));

                tryCatch.Reset();

                JSUtilities::ThrowV8Error(m_Isolate, JSUtilities::V8Errors::SyntaxError, "Syntax Error");
                EXPECT_TRUE(tryCatch.HasCaught());
                EXPECT_EQ("Uncaught SyntaxError: Syntax Error", V8ToString(m_Isolate, tryCatch.Message()->Get()));

                tryCatch.Reset();

                JSUtilities::ThrowV8Error(m_Isolate, JSUtilities::V8Errors::TypeError, "Type Error");
                EXPECT_TRUE(tryCatch.HasCaught());
                EXPECT_EQ("Uncaught TypeError: Type Error", V8ToString(m_Isolate, tryCatch.Message()->Get()));

                tryCatch.Reset();

                JSUtilities::ThrowV8Error(m_Isolate, JSUtilities::V8Errors::WasmCompileError, "WASM Compile Error");
                EXPECT_TRUE(tryCatch.HasCaught());
                EXPECT_EQ("Uncaught CompileError: WASM Compile Error", V8ToString(m_Isolate, tryCatch.Message()->Get()));

                tryCatch.Reset();

                JSUtilities::ThrowV8Error(m_Isolate, JSUtilities::V8Errors::WasmLinkError, "WASM Link Error");
                EXPECT_TRUE(tryCatch.HasCaught());
                EXPECT_EQ("Uncaught LinkError: WASM Link Error", V8ToString(m_Isolate, tryCatch.Message()->Get()));

                tryCatch.Reset();

                JSUtilities::ThrowV8Error(m_Isolate, JSUtilities::V8Errors::WasmRuntimeError, "WASM Runtime Error");
                EXPECT_TRUE(tryCatch.HasCaught());
                EXPECT_EQ("Uncaught RuntimeError: WASM Runtime Error", V8ToString(m_Isolate, tryCatch.Message()->Get()));

                tryCatch.Reset();

                JSUtilities::ThrowV8Error(m_Isolate, JSUtilities::V8Errors::Error, "Error");
                EXPECT_TRUE(tryCatch.HasCaught());
                EXPECT_EQ("Uncaught Error: Error", V8ToString(m_Isolate, tryCatch.Message()->Get()));
            }

            TEST_F(V8UtilitiesTest, TestStdStringConversion)
            {
                v8::HandleScope scope(m_Isolate);

                std::string str = "";
                std::string returnedValue = "";

                //test the create symbol
                str = "TestSymbol";
                v8::Local<v8::String> symbol = CreateSymbol(m_Isolate, str);
                returnedValue = V8ToString(m_Isolate, symbol.As<v8::Value>());
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

            TEST_F(V8UtilitiesTest, TestStdU16StringConversion)
            {

                v8::HandleScope scope(m_Isolate);

                std::u16string str = u"";
                std::u16string returnedValue = u"";

                //test the create symbol
                str = u"TestSymbol";
                v8::Local<v8::String> symbol = CreateSymbol(m_Isolate, str);
                returnedValue = V8ToU16String(m_Isolate, symbol.As<v8::Value>());
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

        }
    }
}