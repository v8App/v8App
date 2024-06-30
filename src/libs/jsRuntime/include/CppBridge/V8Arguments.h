// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef __V8_ARGUMENTS_H__
#define __V8_ARGUMENTS_H__

#include "V8TypeConverter.h"

namespace v8App
{
    namespace JSRuntime
    {
        namespace CppBridge
        {
            class V8Arguments
            {
            public:
                explicit V8Arguments(const V8FuncCallInfoValue &inInfo);
                explicit V8Arguments(const V8PropCallInfoValeu &inInfo);
                ~V8Arguments();

                template <typename T>
                bool GetHolder(T **outHolder)
                {
                    V8LObject holder = m_IsProperty ? m_PropertyInfo->Holder()
                                                                : m_FunctionInfo->Holder();

                    if (ConvertFromV8(m_Isolate, holder, outHolder))
                    {
                        return true;
                    }
                    m_AllConverted = false;
                    return false;
                }

                template <typename T>
                bool GetData(T *outData)
                {
                    V8LValue data = m_IsProperty ? m_PropertyInfo->Data()
                                                             : m_FunctionInfo->Data();

                    if (ConvertFromV8(m_Isolate, data, outData))
                    {
                        return true;
                    }
                    m_AllConverted = false;
                    return false;
                }

                template <typename T>
                bool GetNextArg(T *outType)
                {
                    if (InsufficientArgs())
                    {
                        return false;
                    }

                    V8LValue v8Value;
                    if (m_CppArgsReveresed)
                    {
                        v8Value = (*m_FunctionInfo)[m_NextArg--];
                    }
                    else
                    {
                        v8Value = (*m_FunctionInfo)[m_NextArg++];
                    }
                    if (ConvertFromV8(m_Isolate, v8Value, outType))
                    {
                        return true;
                    }
                    // cpp arguments may be in reverse order from the js arguments
                    if (m_CppArgsReveresed == false)
                    {
                        size_t tempNext = Length() - 1;
                        v8Value = (*m_FunctionInfo)[tempNext];

                        if (ConvertFromV8(m_Isolate, v8Value, outType))
                        {
                            m_CppArgsReveresed = true;
                            m_NextArg = tempNext - 1;
                            return true;
                        }
                    }
                    m_AllConverted = false;
                    return false;
                }

                int Length() const
                {
                    return m_IsProperty ? 0 : m_FunctionInfo->Length();
                }

                template <typename T>
                bool Return(T inValue)
                {
                    V8LValue v8Value;
                    if (TryConvertToV8(m_Isolate, inValue, &v8Value) == false)
                    {
                        return false;
                    }
                    (m_IsProperty ? m_PropertyInfo->GetReturnValue() : m_FunctionInfo->GetReturnValue()).Set(v8Value);
                    return true;
                }

                V8Isolate *GetIsolate()
                {
                    return m_Isolate;
                }

                bool InsufficientArgs() const
                {
                    if (m_CppArgsReveresed)
                    {
                        return m_NextArg < 0;
                    }
                    else
                    {
                        return m_NextArg >= Length();
                    }
                }

                bool NoConversionErrors() const
                {
                    return m_AllConverted;
                }

                size_t GetNextArgIndex() const
                {
                    return m_NextArg;
                }

                void ThrowError() const;
                void ThrowTypeError(const std::string inError) const;

                // help determine if whic of the 2 below are safe to call.
                bool IsPropertyCallback() const { return m_IsProperty; }

                // When using these 2 make sure that the you use the right one as the types are
                // are a union and you if you request the wrong type you'll end up with at least a crash
                // or worse a hard to track down bug.
                const V8FuncCallInfoValue &GetFunctionInfo() { return *m_FunctionInfo; }
                const V8PropCallInfoValeu &GetPropertyInfo() { return *m_PropertyInfo; }

            private:
                V8Isolate *m_Isolate;
                bool m_IsProperty = false;
                bool m_AllConverted = true;
                bool m_CppArgsReveresed = false;

                union
                {
                    const V8FuncCallInfoValue *m_FunctionInfo;
                    const V8PropCallInfoValeu *m_PropertyInfo;
                };
                size_t m_NextArg = 0;
            };

            std::string V8TypeAsString(V8Isolate *inIsolate, V8LValue inValue);

            void ThrowConversionError(V8Arguments *inArgs, size_t inIndex, bool isMemberFunction, const char *inTypeNmae = nullptr);
        } // namespace CppBridge
    } // namespace JSRuntime
} // namespace v8App
#endif