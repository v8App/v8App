// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef _JSCONTEXT_SNAP_DATA_H_
#define _JSCONTEXT_SNAP_DATA_H_

#include <string>
#include <vector>

#include "V8Types.h"
#include "JSModuleInfo.h"

namespace v8App
{
    namespace JSRuntime
    {
        class JSContextSnapData
        {
        public:
            JSContextSnapData() {};
            ~JSContextSnapData() {};

            // name of the app
            std::string m_Name;
            std::string m_Namespace;
            std::string m_EntryPoint;
            std::vector<JSModuleInfo::SnapshotData> m_Modules;
        };
    }
}
#endif //_JSCONTEXT_SNAP_DATA_H_