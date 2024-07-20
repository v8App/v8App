// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef __V8_CPP_OBJ_BASE_H__
#define __V8_CPP_OBJ_BASE_H__

#include "CppBridge/V8CppObjInfo.h"
#include "ISnapshotHandleCloser.h"

namespace v8App
{
    namespace JSRuntime
    {
        namespace CppBridge
        {
            /**
             * Base holder class for native objects
             */
            class V8CppObjectBase : public ISnapshotHandleCloser
            {
            protected:
                V8CppObjectBase() = default;

                virtual void CloseHandleForSnapshot() override
                {
                    m_Object.Reset();
                }

                /**
                 * Gets a Maybe local object from the global object for this cpp object
                 */
                V8MBLObject GetJSObject(V8Isolate *inIsolate)
                {
                    if (inIsolate == nullptr || m_Object.IsEmpty())
                    {
                        return V8MBLObject();
                    }
                    return V8MBLObject(V8LObject::New(inIsolate, m_Object));
                }

                /**
                 * overrided by the V8CppObject to return the V8CppObjInfo typename
                 */
                virtual std::string GetTypeName() { return std::string(); }

            protected:
                /**
                 * Creates the jsObject and sets up it's internal fields
                 */
                V8LObject CreateAndSetupJSObject(V8LContext inContext, V8CppObjInfo *inInfo);

            private:
                /**
                 * First callback for when the object is being GC'ed resetting the global v8 object held and marking the class as destorying
                 */
                static void FirstWeakCallback(const v8::WeakCallbackInfo<V8CppObjectBase> &inInfo);

                bool m_Dead = false;
                V8GObject m_Object;

                V8CppObjectBase(const V8CppObjectBase &) = delete;
                V8CppObjectBase &operator=(const V8CppObjectBase &) = delete;
            };
        }
    }
}
#endif //__V8_CPP_OBJ_BASE_H__