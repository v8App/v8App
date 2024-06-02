// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef __V8_NATIVE_OBJECT_H__
#define __V8_NATIVE_OBJECT_H__

#include "V8TypeConverter.h"
#include "JSRuntime.h"

namespace v8App
{
    namespace JSRuntime
    {
        namespace CppBridge
        {
            class V8ObjectTemplateBuilder;
            class V8NativeObjectBase;

            enum V8NativeObjectInternalFields
            {
                kV8NativeObjectInfo,
                kV8NativeObjectInstance,
                kMaxReservedInternalFields
            };

            /**
             * Serialize/Deserialize function pointers for snapshots
             */
            using DeserializeNativeObject = V8NativeObjectBase *(*)(V8Isolate *inIsolate, V8LocalObject inObject, v8::StartupData inSerialized);
            using SerializeNativeObject = v8::StartupData (*)(V8NativeObjectBase *inNativeObject);

            /**
             * Provides a static type name for the NativeObject to be used to look up
             * Serializers and object templates
             */
            struct V8NativeObjectInfo
            {
                V8NativeObjectInfo(std::string inTypeName, SerializeNativeObject inSerializer, DeserializeNativeObject inDeserializer)
                    : m_TypeName(inTypeName), m_Serializer(inSerializer), m_Deserializer(inDeserializer) {}
                static V8NativeObjectInfo *From(v8::Local<v8::Object> inObject);
                std::string m_TypeName;
                SerializeNativeObject m_Serializer;
                DeserializeNativeObject m_Deserializer;
            };

            /**
             * Base holder class for native objects
             */
            class V8NativeObjectBase //: public ISnapshotHandleCloser
            {
            protected:
                V8NativeObjectBase();
                virtual ~V8NativeObjectBase();

                // virtual void CloseHandleForSnapshot() override { m_Object.Reset();}

                /**
                 * overrided by the V8NativeObject to return the V8NativeObjectInfo typename
                 */
                virtual const char *GetTypeName();

                /**
                 * Gets or create the v8 object for this class
                 */
                v8::MaybeLocal<v8::Object> GetV8NativeObjectInternal(v8::Isolate *inIsolate, V8NativeObjectInfo *inInfo);

            private:
                /**
                 * First callback for when the object is being GC'ed resetting the global v8 object held and marking the class as destorying
                 */
                static void FirstWeakCallback(const v8::WeakCallbackInfo<V8NativeObjectBase> &inInfo);
                /**
                 * Second callback for when the object is GC'ed deletes the cpp object
                 */
                static void SecondWeakCallback(const v8::WeakCallbackInfo<V8NativeObjectBase> &inInfo);

                bool m_Destrying = false;
                v8::Global<v8::Object> m_Object;

                V8NativeObjectBase(const V8NativeObjectBase &) = delete;
                V8NativeObjectBase &operator=(const V8NativeObjectBase &) = delete;
            };

            /**
             * JS Objects backed by a Cpp Class should use this class to wrap the JS Object.
             * The class requires a number of static methods to be defined that take care of registering
             * functions for callbacks registeringt he object template on the global template and
             * for serialization of the class for snapshots.
             *
             * Use the macros defined below to make it easier to define them
             *
             * Desrializes the class data from the startup data
             * static CppBridge::V8NativeObjectBase *DeserializeNativeObject(V8Isolate *inIsolate, V8LocalObject inObject, v8::StartupData inSerialized);
             *
             * Serilizes the class data ot the startup data
             * static v8::StartupData SerializeNativeObject(CppBridge::V8NativeObjectBase *inNativeObject);
             *
             * Registers all the of the class's methods that are expoed to V8 with teh CallbackRegistry
             * static void RegisterClassFunctions();
             *
             * Registers the class object template on the passed in isolate. This is registered with the CallbackRegistry
             * static void RegisterGlobalTemplate(JSRuntimeSharedPtr inRuntime, v8::Local<v8::ObjectTemplate> &inGlobal);
             *
             *
             * To make sure that a wrapped object gets created correctly subclasses should make their construtors
             * protected and have a CreteObject method that returns a V8NativeObjHandle otherwise the c++ object
             * will leak since the weak class backs wouldn't be setup to delete it when the V8 side is disposed of.
             */
            template <typename T>
            class V8NativeObject : public V8NativeObjectBase
            {
            public:
                v8::MaybeLocal<v8::Object> GetV8NativeObject(v8::Isolate *inIsolate)
                {
                    return GetV8NativeObjectInternal(inIsolate, &T::s_V8NativeObjectInfo);
                }

            protected:
                V8NativeObject() {}
                virtual ~V8NativeObject() override {}

            private:
                V8NativeObject(const V8NativeObject &) = delete;
                V8NativeObject &operator=(const V8NativeObject &) = delete;
            };

            template <typename T>
            struct ToReturnsMaybe<
                T *,
                typename std::enable_if<std::is_convertible<T *, V8NativeObjectBase *>::value>::type>
            {
                static const bool Value = true;
            };

            template <typename T>
            struct V8TypeConverter<T *,
                                   typename std::enable_if<std::is_convertible<T *, V8NativeObjectBase *>::value>::type>
            {
                static v8::MaybeLocal<v8::Value> To(v8::Isolate *inIsolate, T *inValue)
                {
                    v8::Local<v8::Object> object;
                    if (inValue->GetV8NativeObject(inIsolate).ToLocal(&object) == false)
                    {
                        return v8::MaybeLocal<v8::Value>();
                    }
                    return v8::MaybeLocal<v8::Value>(object);
                }

                static bool From(v8::Isolate *inIsolate, v8::Local<v8::Value> inValue, T **outValue)
                {
                    *outValue = static_cast<T *>(static_cast<V8NativeObjectBase *>(FromV8NativeObjectInternal(inIsolate, inValue, &T::s_V8NativeObjectInfo)));
                    return *outValue != nullptr;
                }
            };

            // Converts the value to the native object info and if it matches then returns the instance pointer
            void *FromV8NativeObjectInternal(v8::Isolate *inIsolate, v8::Local<v8::Value> inValue, V8NativeObjectInfo *inInfo);
        }
    }
}

/**
 * Creates the static class info variable and initializes it
 */
#define DEF_V8NATIVE_FUNCTIONS(ClassName)                                                                                                                 \
    virtual const char *GetTypeName() override { return s_V8NativeObjectInfo.m_TypeName.c_str(); }                                                        \
    static CppBridge::V8NativeObjectBase *DeserializeNativeObject(V8Isolate *inIsolate, V8LocalObject inObject, v8::StartupData inSerialized);            \
    static v8::StartupData SerializeNativeObject(CppBridge::V8NativeObjectBase *inNativeObject);                                                          \
    static inline CppBridge::V8NativeObjectInfo s_V8NativeObjectInfo{#ClassName, &ClassName::SerializeNativeObject, &ClassName::DeserializeNativeObject}; \
    static void RegisterClassFunctions();                                                                                                                 \
    static void RegisterGlobalTemplate(JSRuntimeSharedPtr inRuntime, v8::Local<v8::ObjectTemplate> &inGlobal);

/**
 * Macro to implement the deserialize function
 */
#define IMPL_DESERIALIZER(ClassName) \
    CppBridge::V8NativeObjectBase *ClassName::DeserializeNativeObject(V8Isolate *inIsolate, V8LocalObject inObject, v8::StartupData inSerialized)

/**
 * Macro to implement the serializer function
 */
#define IMPL_SERIALIZER(ClassName) \
    v8::StartupData ClassName::SerializeNativeObject(CppBridge::V8NativeObjectBase *inNativeObject)

/**
 * Macro to implement the serializer function
 */
#define IMPL_REGISTER_CLASS_FUNCS(ClassName) \
    void ClassName::RegisterClassFunctions()

/**
 * Macro to implement the serializer function
 */
#define IMPL_REGISTER_CLASS_GLOBAL_TEMPLATE(ClassName) \
    void ClassName::RegisterGlobalTemplate(JSRuntimeSharedPtr inRuntime, v8::Local<v8::ObjectTemplate> &inGlobal)

#endif //__V8_NATIVE_OBJECT_H__
