// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifdef USE_JSRUNTIME

#include "gtest/gtest.h"
#include "test_main.h"

#include "TestLogSink.h"

#include "V8AppPlatform.h"
#include "JSContext.h"

namespace v8App
{
    namespace JSRuntime
    {
        class V8Fixture : public testing::Test
        {
        public:
            V8Fixture() = default;
            ~V8Fixture() = default;

            void SetUp() override;
            void TearDown() override;

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
            V8Fixture(const V8Fixture &) = delete;
            V8Fixture &operator=(const V8Fixture &) = delete;
        };
    } // namespace JSRuntime
}

#endif
