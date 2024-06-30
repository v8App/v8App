// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "V8Fixture.h"
#include "JSUtilities.h"

namespace v8App
{
    namespace JSRuntime
    {
        namespace JSUtilities
        {
            using V8UtilitiesTest = V8Fixture;

            TEST_F(V8UtilitiesTest, TestGetStackTrace)
            {
                //DOn't add the missing ) or it'll be come valid js.
                const char scriptStr[] = R"script(
                        function test(
                        {
                            let x = 5;
                        }
                )script";

                V8Isolate::Scope iScope(m_Isolate);
                V8HandleScope scope(m_Isolate);
                V8TryCatch tryCatch(m_Isolate);
                V8LObjTpl global = V8ObjTpl::New(m_Isolate);
                V8LContext context = V8Context::New(m_Isolate, nullptr, global);
                V8ContextScope cScope(context);

                V8LString v8SourceStr = StringToV8(m_Isolate, scriptStr);
                V8LString origin_name = StringToV8(m_Isolate, "test.js");
                V8ScriptOrigin origin(m_Isolate, origin_name);
                V8ScriptSource source(v8SourceStr, origin);

                V8LScript script;
                EXPECT_FALSE(V8ScriptCompiler::Compile(context, &source).ToLocal(&script));

                EXPECT_TRUE(tryCatch.HasCaught());

                std::string error(R"error(Uncaught SyntaxError: Unexpected identifier 'x'
test.js:4:SyntaxError: Unexpected identifier 'x'
)error");

                EXPECT_EQ(error, JSUtilities::GetStackTrace(context, tryCatch));
            }

            TEST_F(V8UtilitiesTest, TestThrowError)
            {
                V8Isolate::Scope iScope(m_Isolate);
                V8HandleScope scope(m_Isolate);
                V8TryCatch tryCatch(m_Isolate);
                V8LContext context = V8Context::New(m_Isolate);
                V8ContextScope cScope(context);

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
                V8Isolate::Scope iScope(m_Isolate);
                V8HandleScope scope(m_Isolate);

                std::string str = "";
                std::string returnedValue = "";

                // test the create symbol
                str = "TestSymbol";
                V8LString symbol = CreateSymbol(m_Isolate, str);
                returnedValue = V8ToString(m_Isolate, symbol.As<V8Value>());
                EXPECT_EQ(str.c_str(), returnedValue);

                // test the to string function
                str = "TestString";
                std::string emptyString;
                V8LValue emptyValue;
                V8LValue testValue = V8String::NewFromUtf8(m_Isolate, "TestString", v8::NewStringType::kNormal).ToLocalChecked().As<V8Value>();
                EXPECT_EQ(emptyString, V8ToString(m_Isolate, emptyValue));
                EXPECT_EQ(emptyString, V8ToString(m_Isolate, v8::Undefined(m_Isolate).As<V8Value>()));
                EXPECT_EQ(str, V8ToString(m_Isolate, testValue));

                // test StringToV8
                V8LString v8String = StringToV8(m_Isolate, str);
                // resue the converted value above to test on the V* side
                EXPECT_TRUE(v8String->StrictEquals(testValue));
            }

            TEST_F(V8UtilitiesTest, TestStdU16StringConversion)
            {
                V8Isolate::Scope iScope(m_Isolate);
                V8HandleScope scope(m_Isolate);

                std::u16string str = u"";
                std::u16string returnedValue = u"";

                // test the create symbol
                str = u"TestSymbol";
                V8LString symbol = CreateSymbol(m_Isolate, str);
                returnedValue = V8ToU16String(m_Isolate, symbol.As<V8Value>());
                EXPECT_EQ(str, returnedValue);

                // test the to string function
                str = u"TestString";
                std::u16string emptyString;
                V8LValue emptyValue;
                V8LValue testValue = V8String::NewFromTwoByte(m_Isolate, reinterpret_cast<const uint16_t *>(u"TestString")).ToLocalChecked().As<V8Value>();
                EXPECT_EQ(emptyString, V8ToU16String(m_Isolate, emptyValue));
                EXPECT_EQ(emptyString, V8ToU16String(m_Isolate, v8::Undefined(m_Isolate).As<V8Value>()));
                EXPECT_EQ(str, V8ToU16String(m_Isolate, testValue));

                // test StringToV8
                V8LString v8String = U16StringToV8(m_Isolate, str);
                // resue the converted value above to test on the V* side
                EXPECT_TRUE(v8String->StrictEquals(testValue));
            }

        }
    }
}