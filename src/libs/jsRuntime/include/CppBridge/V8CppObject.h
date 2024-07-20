// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef __V8_NATIVE_OBJECT_H__
#define __V8_NATIVE_OBJECT_H__

#include "v8/cppgc/allocation.h"
#include "v8/cppgc/garbage-collected.h"

#include "CppBridge/V8TypeConverter.h"
#include "CppBridge/V8CppObjBase.h"
#include "CppBridge/V8CppObjHandle.h"
#include "CppBridge/V8CppObjInfo.h"
#include "JSRuntime.h"
#include "JSContext.h"

namespace v8App
{
    namespace JSRuntime
    {
        namespace CppBridge
        {
            class V8ObjectTemplateBuilder;
            class V8CppObjectBase;

            /**
             * JS Objects backed by a Cpp Class should use this class to wrap the JS Object.
             * The class requires a number of static methods to be defined that take care of registering
             * functions for callbacks registering the object template on the global template and
             * for serialization of the class for snapshots.
             *
             * The lifecycle fo the cpp class is managed by the CppGC in V8 and thus classes should be marked as Final when subclassing.
             * You shold use the NewObj() function to instantiate an instance of the class and do not call delete on it.
             *
             * Use the macros defined below to make it easier to define them
             *
             * Desrializes the class data from the startup data
             * static CppBridge::V8CppObjectBase *DeserializeCppObject(V8Isolate *inIsolate, V8LObject inObject, Serialization::ReadBuffer &inBuffer);
             *
             * Serilizes the class data ot the startup data
             * static void SerializeCppObject(Serialization::WriteBuffer &inBuffer, CppBridge::V8CppObjectBase *inNativeObject);
             *
             * Registers all the of the class's methods that are expoed to V8 with the CallbackRegistry
             * static void RegisterClassFunctions();
             *
             * Registers the class object template on the passed in isolate. This is registered with the CallbackRegistry
             * static void RegisterGlobalTemplate(JSRuntimeSharedPtr inRuntime, V8LFuncTpl &inGlobal);
             *
             *
             * To make sure that a wrapped object gets created correctly subclasses should make their construtors
             * protected and have a CreteObject method that returns a V8NativeObjHandle otherwise the c++ object
             * will leak since the weak class backs wouldn't be setup to delete it when the V8 side is disposed of.
             */
            template <typename T>
            class V8CppObject : public V8CppObjectBase, public cppgc::GarbageCollected<T>
            {
            public:
                V8CppObject() {}

                /**
                 * Takes an v8 value and ifi t's an object then attempts to fetch
                 * the cp object form and convert to the type specified
                 */
                static cppgc::Member<T> GetCppObject(V8LValue inValue)
                {
                    if (inValue->IsObject() == false)
                    {
                        return nullptr;
                    }

                    V8LObject jsObject = V8LObject::Cast(inValue);
                    if (jsObject->InternalFieldCount() < (int)V8CppObjDataIntField::MaxInternalFields)
                    {
                        return nullptr;
                    }
                    V8CppObjInfo *objInfo = V8CppObjInfo::From(jsObject);
                    if (objInfo != &T::s_V8CppObjInfo)
                    {
                        return nullptr;
                    }
                    return static_cast<T *>(static_cast<V8CppObjectBase *>(jsObject->GetAlignedPointerFromInternalField((int)V8CppObjDataIntField::ObjInstance)));
                }

                /**
                 * If you have the JSContext so you can just use it
                 */
                template <typename... Args>
                static V8CppObjHandle<T> NewObj(JSRuntimeSharedPtr inRuntime, JSContextSharedPtr inContext, Args &&...inArgs)
                {
                    if (inContext == nullptr)
                    {
                        return V8CppObjHandle<T>();
                    }
                    V8LContext context = inContext->GetLocalContext();
                    return NewObj(inRuntime, context, std::forward<Args>(inArgs)...);
                }

                /**
                 * Have the local context
                 */
                template <typename... Args>
                static V8CppObjHandle<T> NewObj(JSRuntimeSharedPtr inRuntime, V8LContext inContext, Args &&...inArgs)
                {
                    if (inRuntime == nullptr)
                    {
                        return V8CppObjHandle<T>();
                    }
                    V8CppHeap *heap = inRuntime->GetCppHeap();
                    if (heap == nullptr)
                    {
                        return V8CppObjHandle<T>();
                    }
                    T *gcObject = cppgc::MakeGarbageCollected<T>(heap->GetAllocationHandle(), std::forward<Args>(inArgs)...);
                    V8LObject jsObject = gcObject->CreateAndSetupJSObject(inContext, &T::s_V8CppObjInfo);
                    if (jsObject.IsEmpty())
                    {
                        return V8CppObjHandle<T>();
                    }
                    return V8CppObjHandle<T>(jsObject, gcObject);
                }

                /**
                 * Subclasses should override this to trace any objects they may
                 * hold that is allocated by the cpp heap
                 * Geenerally you would do something like this
                 * visitor->Trace(<some ccpgc variable);
                 * V8CppObject::Trace(visitor);
                 */
                virtual void Trace(cppgc::Visitor *visitor) const {}

            private:
                V8CppObject(const V8CppObject &) = delete;
                V8CppObject &operator=(const V8CppObject &) = delete;
            };

            template <typename T>
            struct V8TypeConverter<T *,
                                   typename std::enable_if<std::is_convertible<T *, V8CppObjectBase *>::value>::type>
            {
                static V8MBLValue To(V8Isolate *inIsolate, T **inValue)
                {
                    return inValue->GetJSObject(inIsolate);
                }

                static bool From(V8Isolate *inIsolate, V8LValue inValue, T **outValue)
                {
                    cppgc::Member<T> gcObj = V8CppObject<T>::GetCppObject(inValue);
                    *outValue = gcObj.Get();
                    return *outValue != nullptr;
                }
            };
        }
    }
}

/**
 * Creates the static class info variable and initializes it
 */
#define DEF_V8CPP_OBJ_FUNCTIONS(ClassName)                                                                                                  \
    virtual std::string GetTypeName() override { return s_V8CppObjInfo.m_TypeName; }                                                        \
    static CppBridge::V8CppObjectBase *DeserializeCppObject(V8Isolate *inIsolate, V8LObject inObject, Serialization::ReadBuffer &inBuffer); \
    static void SerializeCppObject(Serialization::WriteBuffer &inBuffer, CppBridge::V8CppObjectBase *inCppObject);                          \
    static inline CppBridge::V8CppObjInfo s_V8CppObjInfo{#ClassName, &ClassName::SerializeCppObject, &ClassName::DeserializeCppObject};     \
    static void RegisterClassFunctions();                                                                                                   \
    static void RegisterGlobalTemplate(JSContextSharedPtr inContext, V8LObject &inGlobal);

/**
 * Macro to implement the deserialize function
 */
#define IMPL_V8CPPOBJ_DESERIALIZER(ClassName) \
    CppBridge::V8CppObjectBase *ClassName::DeserializeCppObject(V8Isolate *inIsolate, V8LObject inObject, Serialization::ReadBuffer &inBuffer)

/**
 * Macro to implement the serializer function
 */
#define IMPL_V8CPPOBJ_SERIALIZER(ClassName) \
    void ClassName::SerializeCppObject(Serialization::WriteBuffer &inBuffer, CppBridge::V8CppObjectBase *inCppObject)

/**
 * Macro to implement the serializer function
 */
#define IMPL_V8CPPOBJ_REGISTER_CLASS_FUNCS(ClassName) \
    void ClassName::RegisterClassFunctions()

/**
 * Macro to implement the serializer function
 */
#define IMPL_V8CPPOBJ_REGISTER_CLASS_GLOBAL_TEMPLATE(ClassName) \
    void ClassName::RegisterGlobalTemplate(JSContextSharedPtr inContext, V8LObject &inGlobal)

#endif //__V8_NATIVE_OBJECT_H__
