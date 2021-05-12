// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "CppBridge/V8TypeConverter.h"

namespace v8App
{
    namespace JSRuntime
    {
        namespace CppBridge
        {
            v8::Local<v8::String> CreateSymbol(v8::Isolate *inIsolate, const std::string &inString)
            {
                return v8::String::NewFromUtf8(inIsolate, inString.c_str(), v8::NewStringType::kInternalized, inString.length()).ToLocalChecked();
            }

            v8::Local<v8::String> CreateSymbol(v8::Isolate *inIsolate, const std::wstring &inString)
            {
                //we use the converter becuae of the extra handling it does fort he wstring
                return v8::Local<v8::String>::Cast(V8TypeConverter<std::wstring>::To(inIsolate, inString, v8::NewStringType::kInternalized));
            }
        }
    }
}