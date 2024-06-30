// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef _JS_UTILITIES_H_
#define _JS_UTILITIES_H_

#include <string>
#include <filesystem>

#include "v8/v8.h"

namespace v8App
{
    namespace JSRuntime
    {
        namespace JSUtilities
        {
            std::string GetStackTrace(V8LContext inContext, V8TryCatch &inTryCatch, std::string inResourceName = "");

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

            void ThrowV8Error(V8Isolate *inIsolate, V8Errors inType, std::string inMessage);

            //Create a symbol from a std::string
            V8LString CreateSymbol(V8Isolate *inIsolate, const std::string &inString);

            //create a symbol from a std::u16string
            V8LString CreateSymbol(V8Isolate *inIsolate, const std::u16string &inString);

            std::string V8ToString(V8Isolate *inIsolate, V8LValue inValue);

            std::u16string V8ToU16String(V8Isolate *inIsolate, V8LValue inValue);

            V8LString StringToV8(V8Isolate *inIsolate, const std::string &inString, v8::NewStringType inType = v8::NewStringType::kNormal);

            V8LString U16StringToV8(V8Isolate *inIsolate, const std::u16string &inString, v8::NewStringType inType = v8::NewStringType::kNormal);

            //TODO:: this should part of a class that cam load scripts either from files or embedded in the binary
            V8LString ReadScriptFile(V8Isolate* inIsolate, std::filesystem::path inFilename);

        }
    }
}
#endif