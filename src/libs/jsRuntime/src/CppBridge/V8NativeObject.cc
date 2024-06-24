// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
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

            V8CppObjInfo *V8CppObjInfo::From(v8::Local<v8::Object> inObject)
            {
                if (inObject->InternalFieldCount() != (int)V8CppObjDataIntField::MaxInternalFields)
                {
                    return nullptr;
                }
                V8CppObjInfo *info = static_cast<V8CppObjInfo *>(
                    inObject->GetAlignedPointerFromInternalField((int)V8CppObjDataIntField::ObjInfo));
                return info;
            }

            V8NativeObjectBase::V8NativeObjectBase() = default;

            V8NativeObjectBase::~V8NativeObjectBase()
            {
                m_Object.Reset();
            }

            const char *V8NativeObjectBase::GetTypeName()
            {
                return nullptr;
            }

            void V8NativeObjectBase::FirstWeakCallback(const v8::WeakCallbackInfo<V8NativeObjectBase> &inInfo)
            {
                V8NativeObjectBase *baseObject = inInfo.GetParameter();
                baseObject->m_Dead = true;
                baseObject->m_Object.Reset();
            }

            V8LocalObject V8NativeObjectBase::CreateAndSetupJSObject(V8LocalContext inContext, V8CppObjInfo *inInfo)
            {
                JSContextSharedPtr jsContext = JSContext::GetJSContextFromV8Context(inContext);
                if (jsContext == nullptr)
                {
                    return V8LocalObject();
                }
                JSRuntimeSharedPtr runtime = jsContext->GetJSRuntime();
                if(runtime == nullptr)
                {
                    return V8LocalObject();
                }

                V8LocalObjTpl objTpl = runtime->GetObjectTemplate(inInfo);
                if (objTpl.IsEmpty())
                {
                    return V8LocalObject();
                }
                V8LocalObject jsObject;
                if (objTpl->NewInstance(inContext).ToLocal(&jsObject) == false)
                {
                    return V8LocalObject();
                }
                int indexes[] = {(int)V8CppObjDataIntField::CppHeapID, (int)V8CppObjDataIntField::ObjInfo, (int)V8CppObjDataIntField::ObjInstance};
                void *values[] = {runtime->GetCppHeapID(), inInfo, this};

                jsObject->SetAlignedPointerInInternalFields((int)V8CppObjDataIntField::MaxInternalFields, indexes, values);
                m_Object.Reset(runtime->GetIsolate(), jsObject);

                return jsObject;
            }
        }
    }
}
