// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef __V8_NATIVE_OBJECT_HANDLE_H__
#define __V8_NATIVE_OBJECT_HANDLE_H__

#include "v8/cppgc/member.h"

#include "CppBridge/V8TypeConverter.h"

namespace v8App
{
    namespace JSRuntime
    {
        namespace CppBridge
        {
            /**
             * Class that holds bot the cppObject and local jsObject.
             * Should only be used on the stack and not saved to the heap
             */
            template <typename T>
            class V8NativeObjectHandle
            {
            public:
                V8NativeObjectHandle() : m_Object(nullptr) {}
                V8NativeObjectHandle(v8::Local<v8::Value> inWrapper, T *inObject)
                    : m_Wrapper(inWrapper), m_Object(inObject) {}

                T *operator->() const { return m_Object; }
                v8::Local<v8::Value> ToV8() const { return m_Wrapper; }
                T *Get() const { return m_Object; }

                bool IsEmpty() { return m_Object == nullptr; }
                void Clear()
                {
                    m_Wrapper.Clear();
                    m_Object = nullptr;
                }

            private:
                v8::Local<v8::Value> m_Wrapper;
                cppgc::Member<T> m_Object;
            };

            /**
             * TypeCovneter for the handle
             */
            template <typename T>
            struct V8TypeConverter<V8NativeObjectHandle<T>>
            {
                static v8::Local<v8::Value> To(v8::Isolate *inIsolate, const V8NativeObjectHandle<T> &inValue)
                {
                    return inValue.ToV8();
                }

                static bool From(v8::Isolate *inIsolate, v8::Local<v8::Value> inValue, V8NativeObjectHandle<T> *outValue)
                {
                    T *object = nullptr;
                    if (V8TypeConverter<T *>::From(inIsolate, inValue, &object) == false)
                    {
                        return false;
                    }
                    *outValue = V8NativeObjectHandle<T>(inValue, object);
                    return true;
                }
            };
        }
    }
}
#endif