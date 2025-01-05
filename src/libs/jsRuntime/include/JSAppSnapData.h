// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef _JSAPP_SNAP_DATA_H_
#define _JSAPP_SNAP_DATA_H_

#include <string>
#include <vector>

#include "V8Types.h"
#include "JSRuntimeSnapData.h"

namespace v8App
{
    namespace JSRuntime
    {
        class JSAppSnapData
        {
        public:
            JSAppSnapData() {};
            ~JSAppSnapData() {};

            // name of the app
            std::string m_Name;
            /** Version of the app as a string, it's a Version class in the JSApp Class */
            std::string m_AppVersion;

            /** Runtimes snap data. Main is 0 */
            std::vector<JSRuntimeSnapDataSharedPtr> m_RuntimesSnapData;
            //holds a map of indexes to runtime snapshot names
            Containers::NamedIndexes m_RuntimesSnapIndexes;
        };
    }
}
#endif //_JSAPP_SNAP_DATA_H_