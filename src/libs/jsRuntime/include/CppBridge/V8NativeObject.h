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

            enum V8NativeObjectInternalFields
            {
                kV8NativeObjectInfo,
                kV8NativeObjectInstance,
                kMaxReservedInternalFields
            };

            struct V8NativeObjectInfo
            {
                static V8NativeObjectInfo *From(v8::Local<v8::Object> inObject);
                int dummy;
            };

            class V8NativeObjectBase
            {
            protected:
                V8NativeObjectBase();
                virtual ~V8NativeObjectBase();

                //subclasses should override to return a name for the objects for logging
                virtual const char *GetTypeName();

                virtual V8ObjectTemplateBuilder GetObjectTemplateBuilder(v8::Isolate*  inIsolate);

                v8::Local<v8::ObjectTemplate> GetOrCreateObjectTemplate(v8::Isolate* inIsolate, V8NativeObjectInfo *inInfo);

                v8::MaybeLocal<v8::Object> GetV8NativeObjectInternal(v8::Isolate* inIsolate, V8NativeObjectInfo *inInfo);

            private:
                static void FirstWeakCallback(const v8::WeakCallbackInfo<V8NativeObjectBase> &inInfo);
                static void SecondWeakCallback(const v8::WeakCallbackInfo<V8NativeObjectBase> &inInfo);

                bool m_Destrying = false;
                v8::Global<v8::Object> m_Object;

                V8NativeObjectBase(const V8NativeObjectBase &) = delete;
                V8NativeObjectBase &operator=(const V8NativeObjectBase &) = delete;
            };

            /**
             * Subclasses of V8NativeObject should have a staitc variable s_V8NativeObjectInfo where the pointer
             * to this variable is used to cache the object template on the runtime.
             * 
             * To make sure that a wrapped object gets created correctly subclasses should make their construtors
             * protected and have a CreteObject method that returns a V8NativeObjHandle otherwise the c++ object
             * will leak since the weakclass backs wouldn't be setup to delete it when the V8 side is disposed of.
             */
            template <typename T>
            class V8NativeObject : public V8NativeObjectBase
            {
            public:
                v8::MaybeLocal<v8::Object> GetV8NativeObject(v8::Isolate*  inIsolate)
                {
                    return GetV8NativeObjectInternal(inIsolate, &T::s_V8NativeObjectInfo);
                }

             protected:
                V8NativeObject() {}
                ~V8NativeObject() override {}

            private:
                V8NativeObject(const V8NativeObject &) = delete;
                V8NativeObject &operator=(const V8NativeObject &) = delete;
            };

            template <typename T>
            struct ToReturnsMaybe<
                T*,
                typename std::enable_if<
                    std::is_convertible<T *, V8NativeObjectBase *>::value>::type>
            {
                static const bool Value = true;
            };

            template <typename T>
            struct V8TypeConverter<T*,
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

            //Converts the value to the native object info and if it matches then returns the instance pointer
            void *FromV8NativeObjectInternal(v8::Isolate *inIsolate, v8::Local<v8::Value> inValue, V8NativeObjectInfo *inInfo);
        }
    }
}

#endif