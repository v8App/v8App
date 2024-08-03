// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef __ISNAHOT_OBJECT_H__
#define __ISNAHOT_OBJECT_H__

#include "Serialization/WriteBuffer.h"
#include "Serialization/ReadBuffer.h"

namespace v8App
{
    namespace JSRuntime
    {
        class ISnapshotObject
        {
        public:
            virtual bool MakeSnapshot(Serialization::WriteBuffer &inBuffer, void* inData = nullptr) = 0;
            virtual bool RestoreSnapshot(Serialization::ReadBuffer& inBufffer, void *inData = nullptr) = 0;
        };
    }
}

#endif //__ISNAHOT_OBJECT_H__