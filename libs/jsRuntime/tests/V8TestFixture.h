// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef __V8_TEST_SUITE_H__
#define __V8_TEST_SUITE_H__

#include "v8.h"
#include "V8Platform.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"

namespace v8App
{
    namespace JSRuntime
    {
        class V8TestFixture : public testing::Test
        {
        public:
            V8TestFixture();
            ~V8TestFixture();

            void SetUp() override;
            void TearDown() override;

        protected:
            std::unique_ptr<JSRuntime> m_Runtime;
            v8::Isolate* m_Isolate = nullptr;
            v8::Persistent<v8::Context> m_Context;

        private:
            V8TestFixture(const V8TestFixture &) = delete;
            V8TestFixture &operator=(const V8TestFixture &) = delete;
        };
    } // namespace JSRuntime
} // namespace v8App

#endif
