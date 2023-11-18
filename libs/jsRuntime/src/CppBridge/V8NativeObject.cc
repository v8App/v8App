// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "CppBridge/V8NativeObject.h"
#include "CppBridge/V8ObjectTemplateBuilder.h"

namespace v8App
{
    namespace JSRuntime
    {
        namespace CppBridge
        {

            V8NativeObjectInfo *V8NativeObjectInfo::From(v8::Local<v8::Object> inObject)
            {
                if (inObject->InternalFieldCount() != kMaxReservedInternalFields)
                {
                    return nullptr;
                }
                V8NativeObjectInfo *info = static_cast<V8NativeObjectInfo *>(
                    inObject->GetAlignedPointerFromInternalField(kV8NativeObjectInfo));
                return info;
            }

            V8NativeObjectBase::V8NativeObjectBase() = default;

            V8NativeObjectBase::~V8NativeObjectBase()
            {
                m_Object.Reset();
            }

            V8ObjectTemplateBuilder V8NativeObjectBase::GetObjectTemplateBuilder(v8::Isolate *inIsolate)
            {
                return V8ObjectTemplateBuilder(inIsolate, GetTypeName());
            }

            v8::Local<v8::ObjectTemplate> V8NativeObjectBase::GetOrCreateObjectTemplate(v8::Isolate *inIsolate, V8NativeObjectInfo *inInfo)
            {
                JSRuntimeSharedPtr runtime = JSRuntime::GetJSRuntimeFromV8Isolate(inIsolate);
                v8::Local<v8::ObjectTemplate> objTemplate = runtime->GetObjectTemplate(inInfo);
                if (objTemplate.IsEmpty())
                {
                    objTemplate = GetObjectTemplateBuilder(inIsolate).Build();
                    CHECK_FALSE(objTemplate.IsEmpty());
                    runtime->SetObjectTemplate(inInfo, objTemplate);
                }

                CHECK_EQ(kMaxReservedInternalFields, objTemplate->InternalFieldCount());

                return objTemplate;
            }

            const char *V8NativeObjectBase::GetTypeName()
            {
                return nullptr;
            }

            void V8NativeObjectBase::FirstWeakCallback(const v8::WeakCallbackInfo<V8NativeObjectBase> &inInfo)
            {
                V8NativeObjectBase *baseObject = inInfo.GetParameter();
                baseObject->m_Destrying = true;
                baseObject->m_Object.Reset();
                inInfo.SetSecondPassCallback(SecondWeakCallback);
            }

            void V8NativeObjectBase::SecondWeakCallback(const v8::WeakCallbackInfo<V8NativeObjectBase> &inInfo)
            {
                V8NativeObjectBase *baseObject = inInfo.GetParameter();
                delete baseObject;
            }

            v8::MaybeLocal<v8::Object> V8NativeObjectBase::GetV8NativeObjectInternal(v8::Isolate *inIsolate, V8NativeObjectInfo *inInfo)
            {

                if (m_Object.IsEmpty() == false)
                {
                    return v8::MaybeLocal<v8::Object>(v8::Local<v8::Object>::New(inIsolate, m_Object));
                }

                if (m_Destrying)
                {
                    return v8::MaybeLocal<v8::Object>();
                }

                v8::Local<v8::ObjectTemplate> objTemplate = GetOrCreateObjectTemplate(inIsolate, inInfo);
                
                v8::Local<v8::Object> object;
                if (objTemplate->NewInstance(inIsolate->GetCurrentContext()).ToLocal(&object) == false)
                {
                    delete this;
                    return v8::MaybeLocal<v8::Object>(object);
                }

                int indexes[] = {kV8NativeObjectInfo, kV8NativeObjectInstance};
                void *values[] = {inInfo, this};

                object->SetAlignedPointerInInternalFields(2, indexes, values);
                m_Object.Reset(inIsolate, object);
                m_Object.SetWeak(this, FirstWeakCallback, v8::WeakCallbackType::kParameter);
                return v8::MaybeLocal<v8::Object>(object);
            }

            void *FromV8NativeObjectInternal(v8::Isolate *inIsolate, v8::Local<v8::Value> inValue, V8NativeObjectInfo *inInfo)
            {
                if (inValue->IsObject() == false)
                {
                    return nullptr;
                }
                v8::Local<v8::Object> object = v8::Local<v8::Object>::Cast(inValue);
                //we should at a min have kMaxReservedInternalFields fields
                if (object->InternalFieldCount() < kMaxReservedInternalFields)
                {
                    return nullptr;
                }
                V8NativeObjectInfo *info = V8NativeObjectInfo::From(object);
                if (info == nullptr)
                {
                    return nullptr;
                }

                if (info != inInfo)
                {
                    return nullptr;
                }

                return object->GetAlignedPointerFromInternalField(kV8NativeObjectInstance);
            }
        }
    }
}
