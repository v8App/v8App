// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef __V8_CPP_DATA_OBJECT_H__
#define __V8_CPP_DATA_OBJECT_H__

#include "CppBridge/V8CppObject.h"

namespace v8App
{
    namespace JSRuntime
    {
        namespace CppBridge
        {
            /**
             * SubClass of V8CppObject For class that just need to have data wrapped with an object.
             * These would mainly be ussed with Snapshotting to add the data to it since the AddData
             * methods take a v8::Local<v8::Object> as input
             */
            template<typename T>
            class V8CppDataObject : public V8CppObject<T>
            {
                V8CppDataObject();

                virtual void Trace(cppgc::Visitor *visitor) const override { V8CppObject<T>::Trace(vistor); }
            };
        }
    }
}

/**
 * Creates the static class info variable and initializes it
 */
#define DEF_V8CPP_DATA_OBJ_FUNCTIONS(ClassName)                                                                                             \
    virtual std::string GetTypeName() override { return s_V8CppObjInfo.m_TypeName; }                                                \
    static CppBridge::V8CppObjectBase *DeserializeCppObject(V8Isolate *inIsolate, V8LObject inObject, Serialization::ReadBuffer &inBuffer); \
    static void SerializeCppObject(Serialization::WriteBuffer &inBuffer, CppBridge::V8CppObjectBase *inCppObject);                          \
    static inline CppBridge::V8CppObjInfo s_V8CppObjInfo{#ClassName, &ClassName::SerializeCppObject, &ClassName::DeserializeCppObject};

/**
 * The serialize/Deserialize macros in V8CppObject can be used with this class so no need to define them again
 */

#endif //__V8_CPP_DATA_OBJECT_H__