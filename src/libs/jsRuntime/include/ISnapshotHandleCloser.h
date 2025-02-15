// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef _I_SNAPSHOT_HANLDE_CLOSER_H_
#define _I_SNAPSHOT_HANLDE_CLOSER_H_

#include <memory>

namespace v8App
{
    namespace JSRuntime
    {
        /**
         * Interface for classes that need to have their handles closed in order
         * to create a snapshot. Note if a handle is added to the context or isolate's data
         * during snapshot then they don't need to be closed.
         */
        class ISnapshotHandleCloser
        {
        public:
            ISnapshotHandleCloser() {};
            virtual ~ISnapshotHandleCloser() {};

        protected:
            virtual void CloseHandleForSnapshot() = 0;
            friend class JSRuntime;
        };
    }
}

#endif //_I_SNAPSHOT_HANLDE_CLOSER_H_