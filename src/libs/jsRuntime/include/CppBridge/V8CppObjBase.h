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
             * TODO: Create a tag range to use with the class rather using any tag or default tag.
             * Potentially as part fo the registry can assign a int to the
             * class to be able to type check it. We'd only have 28671 avaialble though
             * before hitting the default tag value
             */
            /**
             * Base holder class for native objects
             */
            class V8CppObjectBase : public std::enable_shared_from_this<V8CppObjectBase>, public ISnapshotHandleCloser
            {
            public:
                /**
                 * overrided by the V8CppObject to return the V8CppObjInfo typename
                 */
                virtual std::string GetTypeName() { return s_BaseInfo.m_TypeName; }
                /**
                 * overrided by the V8CppObject to return the V8CppObjInfo type info
                 */
                virtual const V8CppObjInfo &GetTypeInfo() { return s_BaseInfo; }

            protected:
                V8CppObjectBase() {};
                virtual ~V8CppObjectBase() {
                    m_Object.Reset();
                }

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

            protected:
                /**
                 * Creates the jsObject and sets up it's internal fields
                 */
                V8LObject CreateAndSetupJSObject(V8LContext inContext, V8CppObjInfo *inInfo, V8LObject inObject, bool deserializing);
                /**
                 * Hides the trace method and allows us to keep the object private
                 */
                void TraceBase(cppgc::Visitor *visitor) const { }//visitor->Trace(m_Object); }

            private:
                /** Resets the global when the js object is gced. CppGC will reclaim the cpp object */
                static void WeakCallback(const v8::WeakCallbackInfo<V8CppObjectBase> &inInfo);

                static inline V8CppObjInfo s_BaseInfo{"CppBaseObject", nullptr, nullptr};

                bool m_Dead{false};
                V8GObject m_Object;

                V8CppObjectBase(const V8CppObjectBase &) = delete;
                V8CppObjectBase &operator=(const V8CppObjectBase &) = delete;
            };
        }
    }
}
#endif //__V8_CPP_OBJ_BASE_H__