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
            void *FromV8NativeObjectInternal(v8::Isolate *inIsolate, v8::Local<v8::Value> inValue, V8NativeObjectInfo *inInfo)
            {
                if (inValue->IsObject() == false)
                {
                    return nullptr;
                }
                v8::Local<v8::Object> object = v8::Local<v8::Object>::Cast(inValue);
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

            V8NativeObjectInfo *V8NativeObjectInfo::From(v8::Local<v8::Object> inObject)
            {
                V8NativeObjectInfo *info = static_cast<V8NativeObjectInfo *>(
                    inObject->GetAlignedPointerFromInternalField(kV8NativeObjectInfo));
                if (inObject->InternalFieldCount() != info->m_NumObjectFields)
                {
                    return nullptr;
                }
                return info;
            }

            V8NativeObjectBase::V8NativeObjectBase() = default;

            V8NativeObjectBase::~V8NativeObjectBase()
            {
                m_Object.Reset();
            }

            V8ObjectTemplateBuilder V8NativeObjectBase::GetObjectTemplateBuilder(IsolateWeakPtr inIsolate)
            {
                return V8ObjectTemplateBuilder(inIsolate, GetNumberOfInternalFields(), GetTypeName());
            }

            const char *V8NativeObjectBase::GetTypeName()
            {
                return nullptr;
            }

            int V8NativeObjectBase::GetNumberOfInternalFields()
            {
                return kMaxReservedInternalFields;
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

            v8::MaybeLocal<v8::Object> V8NativeObjectBase::GetNativeObjectInternal(IsolateWeakPtr inIsolate, V8NativeObjectInfo *inInfo)
            {
                CHECK_EQ(false, inIsolate.expired());
                v8::Isolate *isolate = inIsolate.lock().get();

                if (m_Object.IsEmpty() == false)
                {
                    return v8::MaybeLocal<v8::Object>(v8::Local<v8::Object>::New(isolate, m_Object));
                }

                if (m_Destrying)
                {
                    return v8::MaybeLocal<v8::Object>();
                }

                JSRuntime *runtime = JSRuntime::GetRuntime(isolate);
                v8::Local<v8::ObjectTemplate> objTemplate = runtime->GetObjectTemplate(inInfo);
                if (objTemplate.IsEmpty())
                {
                    objTemplate = GetObjectTemplateBuilder(inIsolate).Build();
                    CHECK_EQ(false, objTemplate.IsEmpty());
                    runtime->SetObjectTemplate(inInfo, objTemplate);
                }

                CHECK_EQ(inInfo->m_NumObjectFields, objTemplate->InternalFieldCount());
                v8::Local<v8::Object> object;
                if (objTemplate->NewInstance(isolate->GetCurrentContext()).ToLocal(&object) == false)
                {
                    delete this;
                    return v8::MaybeLocal<v8::Object>(object);
                }

                int indexes[] = {kV8NativeObjectInfo, kV8NativeObjectInstance};
                void *values[] = {inInfo, this};
                object->SetAlignedPointerInInternalFields(2, indexes, values);
                m_Object.Reset(isolate, object);
                m_Object.SetWeak(this, FirstWeakCallback, v8::WeakCallbackType::kParameter);
                return v8::MaybeLocal<v8::Object>(object);
            }
        }
    }
}
