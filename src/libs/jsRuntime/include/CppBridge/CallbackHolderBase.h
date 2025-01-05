// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef __CALLBACK_HOLDER_BASE_H__
#define __CALLBACK_HOLDER_BASE_H__

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

            /**
             * Common class for all the Callbacks to be stored together
             */
            class CallbackHolderBase
            {
                public:
                CallbackHolderBase() = default;
            protected:
                virtual ~CallbackHolderBase() = default;

            private:
                CallbackHolderBase(const CallbackHolderBase &) = delete;
                CallbackHolderBase &operator=(const CallbackHolderBase &) = delete;
            };
        }
    }
}

#endif //__CALLBACK_HOLDER_BASE_H__