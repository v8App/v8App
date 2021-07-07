// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef _JS_UTILITIES_H_
#define _JS_UTILITIES_H_

#include <string>

#include "v8.h"

namespace v8App
{
    namespace JSRuntime
    {
        namespace JSUtilities
        {
            std::string GetStackTrace(v8::Isolate *inIsolate, v8::TryCatch &inTryCatch);

            enum class V8Errors
            {
                RangeError,
                ReferenceError,
                SyntaxError,
                TypeError,
                WasmCompileError,
                WasmLinkError,
                WasmRuntimeError,
                Error
            };

            void ThrowV8Error(v8::Isolate *inIsolate, V8Errors inType, v8::Local<v8::String> inMessage);

            //Create a symbol from a std::string
            v8::Local<v8::String> CreateSymbol(v8::Isolate *inIsolate, const std::string &inString);

            //create a symbol from a std::u16string
            v8::Local<v8::String> CreateSymbol(v8::Isolate *inIsolate, const std::u16string &inString);

            std::string V8ToString(v8::Isolate *inIsolate, v8::Local<v8::Value> inValue);

            std::u16string V8ToU16String(v8::Isolate *inIsolate, v8::Local<v8::Value> inValue);

            v8::Local<v8::String> StringToV8(v8::Isolate *inIsolate, const std::string &inString);

            v8::Local<v8::String> U16StringToV8(v8::Isolate *inIsolate, const std::u16string &inString);

        }
    }
}
#endif