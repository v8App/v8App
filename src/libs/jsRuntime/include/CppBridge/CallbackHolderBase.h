// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef __CALLBACK_HOLDER_BASE__
#define __CALLBACK_HOLDER_BASE__

#include <functional>

#include "v8/v8.h"

#include "Utils/CallbackWrapper.h"

namespace v8App
{
    using Utils::CallbackWrapper;

    namespace JSRuntime
    {
        namespace CppBridge
        {

            class CallbackHolderBase
            {
            public:
                v8::Local<v8::External> GetExternalHandle(v8::Isolate *inIsolate);

            protected:
                explicit CallbackHolderBase(v8::Isolate *inIsolate);
                virtual ~CallbackHolderBase();

            private:
                static void FirstWeakCallback(const v8::WeakCallbackInfo<CallbackHolderBase> &info);
                static void SecondWeakCallback(const v8::WeakCallbackInfo<CallbackHolderBase> &info);

                v8::Global<v8::External> m_ExHolder;

                CallbackHolderBase(const CallbackHolderBase &) = delete;
                CallbackHolderBase &operator=(const CallbackHolderBase &) = delete;
            };
        }
    }
}

#endif //__CALLBACK_HOLDER_BASE__