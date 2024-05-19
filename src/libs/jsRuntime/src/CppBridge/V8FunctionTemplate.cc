// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "Logging/LogMacros.h"
#include "CppBridge/V8FunctionTemplate.h"

namespace v8App
{
    namespace JSRuntime
    {
        namespace CppBridge
        {
            CallbackHolderBase::CallbackHolderBase(v8::Isolate* inIsolate)
            {
                //m_ExHolder.Reset(inIsolate, v8::External::New(inIsolate, this));
                //m_ExHolder.SetWeak(this, &CallbackHolderBase::FirstWeakCallback, v8::WeakCallbackType::kParameter);
            }

            CallbackHolderBase::~CallbackHolderBase()
            {
                DCHECK_TRUE(m_ExHolder.IsEmpty());
            }

            v8::Local<v8::External> CallbackHolderBase::GetExternalHandle(v8::Isolate* inIsolate)
            {
                return v8::Local<v8::External>::New(inIsolate, m_ExHolder);
            }

            void CallbackHolderBase::FirstWeakCallback(const v8::WeakCallbackInfo<CallbackHolderBase>& info)
            {
                info.GetParameter()->m_ExHolder.Reset();
                info.SetSecondPassCallback(SecondWeakCallback);
            }

            void CallbackHolderBase::SecondWeakCallback(const v8::WeakCallbackInfo<CallbackHolderBase>& info)
            {
                delete info.GetParameter();
            }
        }
    } // namespace JSRuntume
} // namespace v8App