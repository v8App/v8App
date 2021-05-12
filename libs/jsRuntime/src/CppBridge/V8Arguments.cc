// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <sstream>

#include "CppBridge/V8Arguments.h"

namespace v8App
{
    namespace JSRuntime
    {
        namespace CppBridge
        {
            V8Arguments::V8Arguments(const v8::FunctionCallbackInfo<v8::Value> &inInfo)
                : m_Isolate(inInfo.GetIsolate()), m_IsProperty(false), m_FunctionInfo(&inInfo) {}

            V8Arguments::V8Arguments(const v8::PropertyCallbackInfo<v8::Value> &inInfo)
                : m_Isolate(inInfo.GetIsolate()), m_IsProperty(true), m_PropertyInfo(&inInfo) {}

            V8Arguments::~V8Arguments() = default;

            void V8Arguments::ThrowError() const
            {
                if (m_IsProperty)
                {
                    ThrowTypeError("Error converting property arguments.");
                    return;
                }

                if (InsufficientArgs())
                {
                    ThrowTypeError("Insufficient number of arguments for function");
                    return;
                }

                v8::Local<v8::Value> value = (*m_FunctionInfo)[m_NextArg - 1];
                std::ostringstream error;
                error << "Failed to convert argument at index" << m_NextArg - 1
                      << " from " << V8TypeAsString(m_Isolate, value);
                ThrowTypeError(error.str());
            }

            void V8Arguments::ThrowTypeError(const std::string inError) const
            {
                m_Isolate->ThrowException(v8::Exception::TypeError(ConvertToV8(m_Isolate, inError).As<v8::String>()));
            }

            std::string V8TypeAsString(v8::Isolate *inIsolate, v8::Local<v8::Value> inValue)
            {
                if (inValue.IsEmpty())
                {
                    return "<empty handle>";
                }
                if (inValue->IsUndefined())
                {
                    return "undefined";
                }
                std::string type;
                if (ConvertFromV8(inIsolate, inValue, &type))
                {
                    return std::string();
                }
                return type;
            }

            void ThrowConversionError(V8Arguments *inArgs, size_t inIndex, bool isMemberFunction, const char *inTypeName)
            {
                if (inIndex == 0 && isMemberFunction)
                {
                    std::ostringstream error;
                    if (inTypeName != nullptr)
                    {
                        error << "Failed invocation of function: Function must be called with type " << inTypeName;
                    }
                    else
                    {
                        error << "Failed invocation of function";
                    }
                    inArgs->ThrowTypeError(error.str());
                }
                else
                {
                    inArgs->ThrowError();
                }
            }

        } // namespace CppBridge

    } // namespace JSRuntime

} // namespace v8App
