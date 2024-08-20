// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "CppBridge/V8CppObjBase.h"
#include "JSRuntime.h"
#include "JSContext.h"

namespace v8App
{
    namespace JSRuntime
    {
        namespace CppBridge
        {
            void V8CppObjectBase::FirstWeakCallback(const v8::WeakCallbackInfo<V8CppObjectBase> &inInfo)
            {
                JSRuntimeSharedPtr runtime = JSRuntime::GetJSRuntimeFromV8Isolate(inInfo.GetIsolate());
                V8CppObjectBase *baseObject = inInfo.GetParameter();
                baseObject->m_Dead = true;
                baseObject->m_Object.Reset();
                if(runtime != nullptr)
                {
                    runtime->UnregisterSnapshotHandlerCloser(baseObject);
                }
            }

            V8LObject V8CppObjectBase::CreateAndSetupJSObject(V8LContext inContext, V8CppObjInfo *inInfo)
            {
                JSContextSharedPtr jsContext = JSContext::GetJSContextFromV8Context(inContext);
                if (jsContext == nullptr)
                {
                    return V8LObject();
                }
                JSRuntimeSharedPtr runtime = jsContext->GetJSRuntime();
                if(runtime == nullptr)
                {
                    return V8LObject();
                }

                //TODO: Look at removing this as we may be able to us ethe object passed in FunctionCallbackInfo.This()
                V8LObjTpl objTpl = runtime->GetObjectTemplate(inInfo);
                if (objTpl.IsEmpty())
                {
                    return V8LObject();
                }
                V8LObject jsObject;
                if (objTpl->NewInstance(inContext).ToLocal(&jsObject) == false)
                {
                    return V8LObject();
                }
                int indexes[] = {(int)V8CppObjDataIntField::CppHeapID, (int)V8CppObjDataIntField::ObjInfo, (int)V8CppObjDataIntField::ObjInstance};
                void *values[] = {runtime->GetCppHeapID(), inInfo, this};

                jsObject->SetAlignedPointerInInternalFields((int)V8CppObjDataIntField::MaxInternalFields, indexes, values);
                m_Object.Reset(runtime->GetIsolate(), jsObject);
                runtime->RegisterSnapshotHandleCloser(shared_from_this());

                return jsObject;
            }
        }
    }
}
