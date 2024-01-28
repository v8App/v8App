// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef __V8_NATIVE_OBJECT_HANDLE_H__
#define __V8_NATIVE_OBJECT_HANDLE_H__

#include "CppBridge/V8TypeConverter.h"

namespace v8App
{
    namespace JSRuntime
    {
        namespace CppBridge
        {
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
                T *m_Object;
            };

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

            template <typename T>
            V8NativeObjectHandle<T> CreateV8NativeObjHandle(v8::Isolate*  inIsolate, T *inObject)
            {
                v8::Local<v8::Value> wrapper;
                if (inObject->GetV8NativeObject(inIsolate).ToLocal(&wrapper) == false || wrapper.IsEmpty())
                {
                    return V8NativeObjectHandle<T>();
                }
                return V8NativeObjectHandle<T>(wrapper, inObject);
            }
        }
    }
}
#endif