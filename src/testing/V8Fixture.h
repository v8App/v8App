// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifdef USE_JSRUNTIME

#include "gtest/gtest.h"
#include "test_main.h"

#include "TestLogSink.h"

#include "V8Platform.h"
#include "JSContext.h"

namespace v8App
{
    namespace JSRuntime
    {
        class V8TestFixture : public testing::Test
        {
        public:
            V8TestFixture() = default;
            ~V8TestFixture() = default;

            void SetUp() override;
            void TearDown() override;
            v8::Local<v8::Context> GetContextAndEnter();

        protected:
            JSRuntimeSharedPtr m_Runtime;
            JSAppSharedPtr m_App;
            V8Isolate *m_Isolate = nullptr;
            JSContextSharedPtr m_Context;

            TestUtils::IgnoreMsgKeys m_IgnoreKeys = {
                Log::MsgKey::AppName,
                Log::MsgKey::TimeStamp,
                Log::MsgKey::File,
                Log::MsgKey::Function,
                Log::MsgKey::Line};

        private:
            V8TestFixture(const V8TestFixture &) = delete;
            V8TestFixture &operator=(const V8TestFixture &) = delete;
        };
    } // namespace JSRuntime
}

#endif
