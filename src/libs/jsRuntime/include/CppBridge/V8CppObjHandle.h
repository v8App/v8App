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
            class V8CppObjHandle
            {
            public:
                V8CppObjHandle() : m_Object(nullptr) {}
                V8CppObjHandle(V8LValue inWrapper, T *inObject)
                    : m_Wrapper(inWrapper), m_Object(inObject) {}

                T *operator->() const { return m_Object; }
                V8LValue ToV8() const { return m_Wrapper; }
                T *Get() const { return m_Object; }

                bool IsEmpty() { return m_Object == nullptr; }
                void Clear()
                {
                    m_Wrapper.Clear();
                    m_Object = nullptr;
                }

            private:
                V8LValue m_Wrapper;
                cppgc::Member<T> m_Object;
            };

            /**
             * TypeCovneter for the handle
             */
            template <typename T>
            struct V8TypeConverter<V8CppObjHandle<T>>
            {
                static V8LValue To(V8Isolate *inIsolate, const V8CppObjHandle<T> &inValue)
                {
                    return inValue.ToV8();
                }

                static bool From(V8Isolate *inIsolate, V8LValue inValue, V8CppObjHandle<T> *outValue)
                {
                    T *object = nullptr;
                    if (V8TypeConverter<T *>::From(inIsolate, inValue, &object) == false)
                    {
                        return false;
                    }
                    *outValue = V8CppObjHandle<T>(inValue, object);
                    return true;
                }
            };
        }
    }
}
#endif