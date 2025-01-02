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
                V8Object::Wrap<v8::CppHeapPointerTag::kDefaultTag>(isolate, inObject, this);
                m_Object.Reset(isolate, inObject);
                m_Object.SetWeak(this, V8CppObjectBase::WeakCallback, v8::WeakCallbackType::kParameter);
                runtime->RegisterSnapshotHandleCloser(this);

                return inObject;
            }

            void V8CppObjectBase::WeakCallback(const v8::WeakCallbackInfo<V8CppObjectBase> &inInfo)
            {
                V8CppObjectBase* baseObj = inInfo.GetParameter();
                if(baseObj != nullptr)
                {
                    baseObj->m_Object.Reset();
                }
            }

        }
    }
}
