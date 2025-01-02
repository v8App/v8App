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
            struct FuncTplSnapshotData
            {
                size_t m_IsolateDataIndex{0};
                std::string m_ClassName;
                std::string m_FunctionName;
                std::string m_Namespace;
            };

        public:
            JSRuntimeSnapData() {};
            ~JSRuntimeSnapData() {};

            std::string m_RuntimeName;
            bool m_IdleEnabled{false};

            V8StartupData m_StartupData;
            std::unique_ptr<char[]> m_StartupDeleter{nullptr};

            Containers::NamedIndexes m_ContextIndexes;

            std::vector<JSContextSnapDataSharedPtr> m_ContextData;
            std::vector<FuncTplSnapshotData> m_FunctionTemplates;
        };
    }

    template <>
    struct Serialization::TypeSerializer<v8App::JSRuntime::JSRuntimeSnapData::FuncTplSnapshotData>
    {
        static bool SerializeRead(ReadBuffer &inBuffer, v8App::JSRuntime::JSRuntimeSnapData::FuncTplSnapshotData &inValue)
        {
            inBuffer >> inValue.m_Namespace;
            inBuffer >> inValue.m_FunctionName;
            inBuffer >> inValue.m_ClassName;
            inBuffer >> inValue.m_IsolateDataIndex;
            return inBuffer.HasErrored() == false;
        }
        static bool SerializeWrite(WriteBuffer &inBuffer, const v8App::JSRuntime::JSRuntimeSnapData::FuncTplSnapshotData &inValue)
        {
            inBuffer << inValue.m_Namespace;
            inBuffer << inValue.m_FunctionName;
            inBuffer << inValue.m_ClassName;
            inBuffer << inValue.m_IsolateDataIndex;
            return inBuffer.HasErrored() == false;
        }
    };

}
#endif //_JSRUNTIME_SNAP_DATA_H_