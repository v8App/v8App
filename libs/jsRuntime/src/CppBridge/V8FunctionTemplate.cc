// Copyright 2020 The v8App Authors. All rights reserved.
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
            CallbackHolderBase::CallbackHolderBase(IsolateWeakPtr inIsolate)
            {
                DCHECK_EQ(false, inIsolate.expired());
                v8::Isolate * isolate = inIsolate.lock().get();

                m_ExHolder.Reset(isolate, v8::External::New(isolate, this));
                m_ExHolder.SetWeak(this, &CallbackHolderBase::FirstWeakCallback, v8::WeakCallbackType::kParameter);
            }

            CallbackHolderBase::~CallbackHolderBase()
            {
                DCHECK_EQ(true, m_ExHolder.IsEmpty());
            }

            v8::Local<v8::External> CallbackHolderBase::GetExternalHandle(IsolateWeakPtr inIsolate)
            {
                DCHECK_EQ(false, inIsolate.expired());
                return v8::Local<v8::External>::New(inIsolate.lock().get(), m_ExHolder);
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