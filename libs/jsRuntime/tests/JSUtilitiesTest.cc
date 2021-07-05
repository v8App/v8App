
// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "V8TestFixture.h"
#include "JSUtilites.h"
#include "CppBridge/V8TypeConverter.h"

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

                v8::Local<v8::String> source = CppBridge::StringToV8(m_Isolate, scriptStr);

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

                JSUtilities::ThrowV8Error(m_Isolate, JSUtilities::V8Errors::RangeError, CppBridge::StringToV8(m_Isolate, "Range Error"));
                EXPECT_TRUE(tryCatch.HasCaught());
                EXPECT_EQ("Uncaught RangeError: Range Error", CppBridge::V8ToString(m_Isolate, tryCatch.Message()->Get()));

                tryCatch.Reset();

                JSUtilities::ThrowV8Error(m_Isolate, JSUtilities::V8Errors::ReferenceError, CppBridge::StringToV8(m_Isolate, "Reference Error"));
                EXPECT_TRUE(tryCatch.HasCaught());
                EXPECT_EQ("Uncaught ReferenceError: Reference Error", CppBridge::V8ToString(m_Isolate, tryCatch.Message()->Get()));

                tryCatch.Reset();

                JSUtilities::ThrowV8Error(m_Isolate, JSUtilities::V8Errors::SyntaxError, CppBridge::StringToV8(m_Isolate, "Syntax Error"));
                EXPECT_TRUE(tryCatch.HasCaught());
                EXPECT_EQ("Uncaught SyntaxError: Syntax Error", CppBridge::V8ToString(m_Isolate, tryCatch.Message()->Get()));

                tryCatch.Reset();

                JSUtilities::ThrowV8Error(m_Isolate, JSUtilities::V8Errors::TypeError, CppBridge::StringToV8(m_Isolate, "Type Error"));
                EXPECT_TRUE(tryCatch.HasCaught());
                EXPECT_EQ("Uncaught TypeError: Type Error", CppBridge::V8ToString(m_Isolate, tryCatch.Message()->Get()));

                tryCatch.Reset();

                JSUtilities::ThrowV8Error(m_Isolate, JSUtilities::V8Errors::WasmCompileError, CppBridge::StringToV8(m_Isolate, "WASM Compile Error"));
                EXPECT_TRUE(tryCatch.HasCaught());
                EXPECT_EQ("Uncaught CompileError: WASM Compile Error", CppBridge::V8ToString(m_Isolate, tryCatch.Message()->Get()));

                tryCatch.Reset();

                JSUtilities::ThrowV8Error(m_Isolate, JSUtilities::V8Errors::WasmLinkError, CppBridge::StringToV8(m_Isolate, "WASM Link Error"));
                EXPECT_TRUE(tryCatch.HasCaught());
                EXPECT_EQ("Uncaught LinkError: WASM Link Error", CppBridge::V8ToString(m_Isolate, tryCatch.Message()->Get()));

                tryCatch.Reset();

                JSUtilities::ThrowV8Error(m_Isolate, JSUtilities::V8Errors::WasmRuntimeError, CppBridge::StringToV8(m_Isolate, "WASM Runtime Error"));
                EXPECT_TRUE(tryCatch.HasCaught());
                EXPECT_EQ("Uncaught RuntimeError: WASM Runtime Error", CppBridge::V8ToString(m_Isolate, tryCatch.Message()->Get()));

                tryCatch.Reset();

                JSUtilities::ThrowV8Error(m_Isolate, JSUtilities::V8Errors::Error, CppBridge::StringToV8(m_Isolate, "Error"));
                EXPECT_TRUE(tryCatch.HasCaught());
                EXPECT_EQ("Uncaught Error: Error", CppBridge::V8ToString(m_Isolate, tryCatch.Message()->Get()));
            }
        }
    }
}