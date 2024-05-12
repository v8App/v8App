// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <fstream>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "test_main.h"
#include "TestSnapshotProvider.h"

#include "v8.h"
#include "JSRuntime.h"

namespace v8App
{
    namespace JSRuntime
    {
        TEST(Playground, BareIsolateContext)
        {
            return;
            JSSnapshotProviderSharedPtr snapProvider = std::make_shared<TestSnapshotProvider>();
            JSAppSharedPtr app = std::make_shared<JSApp>("reducedTest", snapProvider);
            JSRuntimeSharedPtr runtime = std::make_shared<JSRuntime>(app, IdleTasksSupport::kIdleTasksEnabled, "reducedTest");
            //runtime->CreateIsolate();
            V8Isolate *isolate = runtime->GetIsolate();

            v8::Isolate::CreateParams params;
            params.snapshot_blob = &s_V8StartupData;
            params.array_buffer_allocator =
                v8::ArrayBuffer::Allocator::NewDefaultAllocator();
            v8::Isolate::Initialize(isolate, params);
            {
                v8::Isolate::Scope iscope(isolate);
                v8::HandleScope hScope(isolate);
                V8LocalContext context = v8::Context::New(isolate);
                v8::Context::Scope cScope(context);
            }
        }
    }
}