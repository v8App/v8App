// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef __V8_CPP_OBJ_INFO_H__
#define __V8_CPP_OBJ_INFO_H__

#include "V8Types.h"

namespace v8App
{
    namespace Serialization
    {
        class ReadBuffer;
        class WriteBuffer;
    }

    namespace JSRuntime
    {
        namespace CppBridge
        {
            class V8CppObjectBase;

            /**
             * Serialize/Deserialize function pointers for snapshots
             */
            using DeserializeCppObject = V8CppObjectBase *(*)(V8Isolate *inIsolate, V8LObject inObject, Serialization::ReadBuffer &inBuffer);
            using SerializeCppObject = void (*)(Serialization::WriteBuffer &inBuffer, V8CppObjectBase *inCppObject);

            /**
             * Provides a static type name for the NativeObject to be used to look up
             * Serializers and object templates
             */
            struct V8CppObjInfo
            {
                V8CppObjInfo(std::string inTypeName, SerializeCppObject inSerializer, DeserializeCppObject inDeserializer)
                    : m_TypeName(inTypeName), m_Serializer(inSerializer), m_Deserializer(inDeserializer) {}
                /**
                 * Gets the Object info form the Object
                 */
                static V8CppObjInfo *From(V8LObject inObject)
                {
                    if (inObject->InternalFieldCount() != (int)V8CppObjDataIntField::MaxInternalFields)
                    {
                        return nullptr;
                    }
                    V8CppObjInfo *info = static_cast<V8CppObjInfo *>(
                        inObject->GetAlignedPointerFromInternalField((int)V8CppObjDataIntField::ObjInfo));
                    return info;
                }

                /**
                 * The type name of the native obj usually the class name
                 */
                std::string m_TypeName;
                /**
                 * Serializer to serilize the object during snapshot
                 */
                SerializeCppObject m_Serializer;
                /**
                 * Deserializer to reset the object when loading from a snapshot
                 */
                DeserializeCppObject m_Deserializer;
            };
        }
    }
}

#endif //__V8_CPP_OBJ_INFO_H__