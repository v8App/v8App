// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef __V8_NATIVE_OBJECT_H__
#define __V8_NATIVE_OBJECT_H__

#include "v8/cppgc/allocation.h"
#include "v8/cppgc/garbage-collected.h"

#include "CppBridge/V8TypeConverter.h"
#include "CppBridge/V8NativeObjectHandle.h"
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
            class V8NativeObjectBase;

            /**
             * Base holder class for native objects
             */
            class V8NativeObjectBase : public ISnapshotHandleCloser
            {
            protected:
                V8NativeObjectBase();
                virtual ~V8NativeObjectBase();

                virtual void CloseHandleForSnapshot() override
                {
                    m_Object.Reset();
                }

                /**
                 * Gets a Maybe local object from the global object for this cpp object
                 */
                V8MBLObject GetJSObject(V8Isolate *inIsolate)
                {
                    if (inIsolate == nullptr || m_Object.IsEmpty())
                    {
                        return V8MBLObject();
                    }
                    return V8MBLObject(V8LObject::New(inIsolate, m_Object));
                }

                /**
                 * overrided by the V8NativeObject to return the V8CppObjInfo typename
                 */
                virtual const char *GetTypeName();

            protected:
                /**
                 * Creates the jsObject and sets up it's internal fields
                 */
                V8LObject CreateAndSetupJSObject(V8LContext inContext, V8CppObjInfo *inInfo);

            private:
                /**
                 * First callback for when the object is being GC'ed resetting the global v8 object held and marking the class as destorying
                 */
                static void FirstWeakCallback(const v8::WeakCallbackInfo<V8NativeObjectBase> &inInfo);

                bool m_Dead = false;
                V8GObject m_Object;

                V8NativeObjectBase(const V8NativeObjectBase &) = delete;
                V8NativeObjectBase &operator=(const V8NativeObjectBase &) = delete;
            };

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
             * static CppBridge::V8NativeObjectBase *DeserializeCppObject(V8Isolate *inIsolate, V8LObject inObject, Serialization::ReadBuffer &inBuffer);
             *
             * Serilizes the class data ot the startup data
             * static void SerializeCppObject(Serialization::WriteBuffer &inBuffer, CppBridge::V8NativeObjectBase *inNativeObject);
             *
             * Registers all the of the class's methods that are expoed to V8 with teh CallbackRegistry
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
            class V8NativeObject : public V8NativeObjectBase, public cppgc::GarbageCollected<T>
            {
            public:
                V8NativeObject() {}
                virtual ~V8NativeObject() override {}

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
                    return static_cast<T *>(static_cast<V8NativeObjectBase *>(jsObject->GetAlignedPointerFromInternalField((int)V8CppObjDataIntField::ObjInstance)));
                }

                /**
                 * If you have the JSContext so you can just use it
                 */
                template <typename... Args>
                static V8NativeObjectHandle<T> NewObj(JSRuntimeSharedPtr inRuntime, JSContextSharedPtr inContext, Args &&...inArgs)
                {
                    if (inContext == nullptr)
                    {
                        return V8NativeObjectHandle<T>();
                    }
                    V8LContext context = inContext->GetLocalContext();
                    return NewObj(inRuntime, context, std::forward<Args>(inArgs)...);
                }

                /**
                 * Have the local context
                 */
                template <typename... Args>
                static V8NativeObjectHandle<T> NewObj(JSRuntimeSharedPtr inRuntime, V8LContext inContext, Args &&...inArgs)
                {
                    if (inRuntime == nullptr)
                    {
                        return V8NativeObjectHandle<T>();
                    }
                    V8CppHeap *heap = inRuntime->GetCppHeap();
                    if (heap == nullptr)
                    {
                        return V8NativeObjectHandle<T>();
                    }
                    T *gcObject = cppgc::MakeGarbageCollected<T>(heap->GetAllocationHandle(), std::forward<Args>(inArgs)...);
                    V8LObject jsObject = gcObject->CreateAndSetupJSObject(inContext, &T::s_V8CppObjInfo);
                    if (jsObject.IsEmpty())
                    {
                        return V8NativeObjectHandle<T>();
                    }
                    return V8NativeObjectHandle<T>(jsObject, gcObject);
                }

                /**
                 * Subclasses should override this to trace any objects they may
                 * hold that is allocated by the cpp heap
                 * Geenerally you would do something like this
                 * visitor->Trace(<some ccpgc variable);
                 * V8NativeObject::Trace(visitor);
                 */
                virtual void Trace(cppgc::Visitor *visitor) const {}

            private:
                V8NativeObject(const V8NativeObject &) = delete;
                V8NativeObject &operator=(const V8NativeObject &) = delete;
            };

            template <typename T>
            struct V8TypeConverter<T *,
                                   typename std::enable_if<std::is_convertible<T *, V8NativeObjectBase *>::value>::type>
            {
                static V8MBLValue To(V8Isolate *inIsolate, T **inValue)
                {
                    return inValue->GetJSObject(inIsolate);
                }

                static bool From(V8Isolate *inIsolate, V8LValue inValue, T **outValue)
                {
                    cppgc::Member<T> gcObj = V8NativeObject<T>::GetCppObject(inValue);
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
#define DEF_V8NATIVE_FUNCTIONS(ClassName)                                                                                                      \
    virtual const char *GetTypeName() override { return s_V8CppObjInfo.m_TypeName.c_str(); }                                                   \
    static CppBridge::V8NativeObjectBase *DeserializeCppObject(V8Isolate *inIsolate, V8LObject inObject, Serialization::ReadBuffer &inBuffer); \
    static void SerializeCppObject(Serialization::WriteBuffer &inBuffer, CppBridge::V8NativeObjectBase *inCppObject);                          \
    static inline CppBridge::V8CppObjInfo s_V8CppObjInfo{#ClassName, &ClassName::SerializeCppObject, &ClassName::DeserializeCppObject};        \
    static void RegisterClassFunctions();                                                                                                      \
    static void RegisterGlobalTemplate(JSContextSharedPtr inContext, V8LObject &inGlobal);

/**
 * Macro to implement the deserialize function
 */
#define IMPL_DESERIALIZER(ClassName) \
    CppBridge::V8NativeObjectBase *ClassName::DeserializeCppObject(V8Isolate *inIsolate, V8LObject inObject, Serialization::ReadBuffer &inBuffer)

/**
 * Macro to implement the serializer function
 */
#define IMPL_SERIALIZER(ClassName) \
    void ClassName::SerializeCppObject(Serialization::WriteBuffer &inBuffer, CppBridge::V8NativeObjectBase *inCppObject)

/**
 * Macro to implement the serializer function
 */
#define IMPL_REGISTER_CLASS_FUNCS(ClassName) \
    void ClassName::RegisterClassFunctions()

/**
 * Macro to implement the serializer function
 */
#define IMPL_REGISTER_CLASS_GLOBAL_TEMPLATE(ClassName) \
    void ClassName::RegisterGlobalTemplate(JSContextSharedPtr inContext, V8LObject &inGlobal)

#endif //__V8_NATIVE_OBJECT_H__
