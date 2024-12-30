// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef _JSRUNTIME_SNAP_DATA_H_
#define _JSRUNTIME_SNAP_DATA_H_

#include <string>

#include "Containers/NamedIndexes.h"
#include "JSContextSnapData.h"

namespace v8App
{
    namespace JSRuntime
    {
        class JSRuntimeSnapData
        {
        public:
            JSRuntimeSnapData() {};
            ~JSRuntimeSnapData() {};

            std::string m_RuntimeName;
            bool m_IdleEnabled{false};

            
            V8StartupData m_StartupData;
            std::unique_ptr<char[]> m_StartupDeleter{nullptr};

            Containers::NamedIndexes m_ContextIndexes;

            std::vector<JSContextSnapDataSharedPtr> m_ContextData;
        };
    }
}
#endif //_JSRUNTIME_SNAP_DATA_H_