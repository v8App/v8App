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
            V8LObject V8CppObjectBase::CreateAndSetupJSObject(V8LContext inContext, V8CppObjInfo *inInfo, V8LObject inObject, bool deserializing)
            {
                V8Isolate *isolate = inContext->GetIsolate();
                if (isolate == nullptr)
                {
                    return V8LObject();
                }
                JSRuntimeSharedPtr runtime = JSRuntime::GetJSRuntimeFromV8Isolate(isolate);
                if (runtime == nullptr)
                {
                    return V8LObject();
                }

                // TODO: Look at removing this as we may be able to use the object passed in FunctionCallbackInfo.This()
                if (deserializing == false)
                {
                    V8LFuncTpl objTpl = runtime->GetClassFunctionTemplate(inInfo);
                    if (objTpl.IsEmpty())
                    {
                        return V8LObject();
                    }
                    if (objTpl->PrototypeTemplate()->NewInstance(inContext).ToLocal(&inObject) == false)
                    {
                        return V8LObject();
                    }
                }
                int indexes[] = {(int)V8CppObjDataIntField::CppHeapID, (int)V8CppObjDataIntField::ObjInfo, (int)V8CppObjDataIntField::ObjInstance};
                void *values[] = {runtime->GetCppHeapID(), inInfo, this};
                inObject->SetAlignedPointerInInternalFields((int)V8CppObjDataIntField::MaxInternalFields, indexes, values);
                m_Object.Reset(runtime->GetIsolate(), inObject);
                runtime->RegisterSnapshotHandleCloser(this);

                return inObject;
            }
        }
    }
}
