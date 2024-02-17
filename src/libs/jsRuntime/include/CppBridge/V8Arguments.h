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
                explicit V8Arguments(const v8::FunctionCallbackInfo<v8::Value> &inInfo);
                explicit V8Arguments(const v8::PropertyCallbackInfo<v8::Value> &inInfo);
                ~V8Arguments();

                template <typename T>
                bool GetHolder(T **outHolder)
                {
                    v8::Local<v8::Object> holder = m_IsProperty ? m_PropertyInfo->Holder()
                                                                : m_FunctionInfo->Holder();

                    if(ConvertFromV8(m_Isolate, holder, outHolder))
                    {
                        return true;
                    }
                    m_AllConverted = false;
                    return false;
                }

                template <typename T>
                bool GetData(T *outData)
                {
                    v8::Local<v8::Value> data = m_IsProperty ? m_PropertyInfo->Data()
                                                             : m_FunctionInfo->Data();

                    if(ConvertFromV8(m_Isolate, data, outData))
                    {
                        return true;
                    }
                    m_AllConverted = false;
                    return false;
                }

                template<typename T>
                bool GetNextArg(T* outType)
                {
                    if(InsufficientArgs()) {
                        return false;
                    }
                    v8::Local<v8::Value> v8Value = (*m_FunctionInfo)[m_NextArg++];
                    if(ConvertFromV8(m_Isolate, v8Value, outType))
                    {
                        return true;
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
                    v8::Local<v8::Value> v8Value;
                    if (TryConvertToV8(m_Isolate, inValue, &v8Value) == false)
                    {
                        return false;
                    }
                    (m_IsProperty ? m_PropertyInfo->GetReturnValue() : m_FunctionInfo->GetReturnValue()).Set(v8Value);
                    return true;
                }

                v8::Isolate* GetIsolate()
                {
                    return m_Isolate;
                }

                bool InsufficientArgs() const{
                    return m_NextArg >= Length();
                }

                bool NoConversionErrors() const{
                    return m_AllConverted;
                }

                size_t GetNextArgIndex() const
                {
                    return m_NextArg;
                }
                
                void ThrowError() const;
                void ThrowTypeError(const std::string inError) const;

                //help determine if whic of the 2 below are safe to call.
                bool IsPropertyCallback() const { return m_IsProperty; }

                // When using these 2 make sure that the you use the right one as the types are
                // are a union and you if you request the wrong type you'll end up with at least a crash
                //or worse a hard to track down bug.
                const v8::FunctionCallbackInfo<v8::Value>& GetFunctionInfo() { return *m_FunctionInfo; }
                const v8::PropertyCallbackInfo<v8::Value>& GetPropertyInfo() { return *m_PropertyInfo; }

            private:
                v8::Isolate* m_Isolate;
                bool m_IsProperty = false;
                bool m_AllConverted = true;

                union
                {
                    const v8::FunctionCallbackInfo<v8::Value> *m_FunctionInfo;
                    const v8::PropertyCallbackInfo<v8::Value> *m_PropertyInfo;
                };
                size_t m_NextArg = 0;
            };

            std::string V8TypeAsString(v8::Isolate* inIsolate, v8::Local<v8::Value> inValue);

            void ThrowConversionError(V8Arguments* inArgs, size_t inIndex, bool isMemberFunction, const char* inTypeNmae = nullptr);
        } // namespace CppBridge
    }     // namespace JSRuntime
} // namespace v8App
#endif