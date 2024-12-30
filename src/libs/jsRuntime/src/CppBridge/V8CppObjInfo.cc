// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "CppBridge/V8CppObjInfo.h"
#include "CppBridge/V8CppObjBase.h"

namespace v8App
{
    namespace JSRuntime
    {
        namespace CppBridge
        {
            const V8CppObjInfo *V8CppObjInfo::From(V8LObject inObject)
            {
                V8CppObjectBase *base = static_cast<V8CppObjectBase *>(V8Object::Unwrap<v8::CppHeapPointerTag::kDefaultTag>(inObject->GetIsolate(), inObject));
                if (base == nullptr)
                {
                    return nullptr;
                }
                return &base->GetTypeInfo();
            }
        }
    }
}