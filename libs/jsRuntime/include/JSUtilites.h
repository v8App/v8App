// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef _JS_UTILITIES_H_
#define _JS_UTILITIES_H_

#include <string>

#include "v8.h"

namespace v8App {
    namespace JSRuntime {
        namespace JSUtilities {
            std::string GetStackTrace(v8::Isolate* inIsolate, v8::TryCatch& inTryCatch);

            enum class V8Errors {
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
        }
    }
}
#endif