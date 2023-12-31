// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <iostream>
#include <string>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "JSContext.h"

#include "V8PlatformInitFixture.h"

namespace v8App
{
    namespace JSRuntime
    {
        using JSContextTest = V8PlatformInitFixture;

        class TestJSContext : public JSContext
        {
        public:
            TestJSContext(JSRuntimeSharedPtr inRuntime, std::string inName) : JSContext(inRuntime, inName) {}
            ~TestJSContext() = default;

            bool Initialized() { return m_Initialized; }
            void SetInit(bool inValue) { m_Initialized = inValue; }
            bool ContextEmpty() { return m_Context.IsEmpty(); }
            void ClearIsolate() { m_Runtime.reset(); }

            void TestCreateContext() { CreateContext(); }
            void testDisposeContext() { DisposeContext(); }
            void SetJSRuntime(JSRuntimeSharedPtr inRUntime) { m_Runtime = inRUntime; }

            JSContextWeakPtr *TestGetContextWeakRef() { return GetContextWeakRef(); }

            void TestMoveContext(JSContext &&inContext) { MoveContext(std::move(inContext)); }
            std::string TestGenerateShadowName() { return GenerateShadowName(); }
            void SetName(std::string inName) { m_Name = inName; }
        };

        TEST_F(JSContextTest, Constructor)
        {
            JSRuntimeSharedPtr runtime = m_App->CreateJSRuntime("ContextConstructor");
            ASSERT_NE(nullptr, runtime);
            v8::Isolate *isolate = runtime->GetIsolate().get();
            v8::Isolate::Scope isolateScope(isolate);
            v8::HandleScope handleScopr(isolate);

            TestJSContext context(runtime, "test:test");

            EXPECT_EQ(runtime, context.GetJSRuntime());
            EXPECT_EQ(runtime->GetIsolate().get(), context.GetIsolate());
            EXPECT_FALSE(context.Initialized());
            EXPECT_TRUE(context.ContextEmpty());
            EXPECT_EQ("testtest", context.GetName());

            context.ClearIsolate();
            EXPECT_EQ(nullptr, context.GetIsolate());
        }

        TEST_F(JSContextTest, CreateDisposeContext)
        {
            JSRuntimeSharedPtr runtime = m_App->CreateJSRuntime("CreateDisposeContext");
            ASSERT_NE(nullptr, runtime);
            v8::Isolate *isolate = runtime->GetIsolate().get();
            v8::Isolate::Scope isolateScope(isolate);
            v8::HandleScope handleScopr(isolate);

            std::shared_ptr<TestJSContext> context = std::make_shared<TestJSContext>(runtime, "test");
            context->TestCreateContext();
            EXPECT_FALSE(context->ContextEmpty());
            EXPECT_FALSE(context->TestGetContextWeakRef()->expired());
            EXPECT_EQ(context->TestGetContextWeakRef()->lock().get(), context.get());
            EXPECT_TRUE(context->Initialized());

            context->testDisposeContext();
            EXPECT_TRUE(context->ContextEmpty());
            EXPECT_FALSE(context->Initialized());
            EXPECT_EQ(nullptr, context->GetJSRuntime());

            context.reset();

            context = std::make_shared<TestJSContext>(runtime, "test");
            context->TestCreateContext();
            EXPECT_FALSE(context->ContextEmpty());
            context->ClearIsolate();
            context->testDisposeContext();
            EXPECT_FALSE(context->ContextEmpty());
            context->SetJSRuntime(runtime);
            context->SetInit(true);
            context->testDisposeContext();
        }

        TEST_F(JSContextTest, MoveContext)
        {
            JSRuntimeSharedPtr runtime = m_App->CreateJSRuntime("MoveContext");
            ASSERT_NE(nullptr, runtime);
            v8::Isolate *isolate = runtime->GetIsolate().get();
            v8::Isolate::Scope isolateScope(isolate);
            v8::HandleScope handleScope(isolate);

            std::shared_ptr<TestJSContext> context = std::make_shared<TestJSContext>(runtime, "test");
            context->TestCreateContext();
            // need a second runtime or the null check will aboirt the tests.
            JSRuntimeSharedPtr runtime2 = m_App->CreateJSRuntime("MoveContext2");
            ASSERT_NE(nullptr, runtime2);
            std::shared_ptr<TestJSContext> context2 = std::make_shared<TestJSContext>(runtime2, "");

            context2->TestMoveContext(std::move(*context.get()));

            EXPECT_EQ(context->GetJSRuntime(), nullptr);
            EXPECT_TRUE(context->ContextEmpty());
            EXPECT_FALSE(context->Initialized());
            EXPECT_EQ(nullptr, context->TestGetContextWeakRef());

            EXPECT_EQ(context2->GetJSRuntime().get(), runtime.get());
            EXPECT_FALSE(context2->ContextEmpty());
            EXPECT_TRUE(context2->Initialized());
            EXPECT_EQ(context2.get(), context2->TestGetContextWeakRef()->lock().get());

            std::shared_ptr<TestJSContext> context3 = std::make_shared<TestJSContext>(runtime2, "");

            context3 = std::move(context2);
            EXPECT_EQ(context2, nullptr);
            EXPECT_EQ(context3->GetJSRuntime(), runtime);
            EXPECT_EQ(context3.get(), context3->TestGetContextWeakRef()->lock().get());

            std::shared_ptr<JSContext> context4(std::move(context3));
            EXPECT_EQ(context3, nullptr);
            EXPECT_EQ(context4->GetJSRuntime(), runtime);
        }

        TEST_F(JSContextTest, GetLocalContext)
        {
            JSRuntimeSharedPtr runtime = m_App->CreateJSRuntime("GetLocalContext");
            ASSERT_NE(nullptr, runtime);
            v8::Isolate *isolate = runtime->GetIsolate().get();
            v8::Isolate::Scope isolateScope(isolate);
            v8::HandleScope handleScopr(isolate);

            JSContextWeakPtr context = runtime->CreateContext("test");
            EXPECT_FALSE(context.expired());

            V8LocalContext local = context.lock()->GetLocalContext();
            EXPECT_FALSE(local.IsEmpty());
        }

        TEST_F(JSContextTest, GenerateShadowName)
        {
            JSRuntimeSharedPtr runtime = m_App->CreateJSRuntime("GenerateShadowName");
            ASSERT_NE(nullptr, runtime);
            v8::Isolate *isolate = runtime->GetIsolate().get();
            v8::Isolate::Scope isolateScope(isolate);
            v8::HandleScope handleScope(isolate);

            std::shared_ptr<TestJSContext> context = std::make_shared<TestJSContext>(runtime, "test");
            context->TestCreateContext();

            EXPECT_EQ("test:shadow:1", context->TestGenerateShadowName());
            context->SetName("test:test:1");
            EXPECT_EQ("test:shadow:2", context->TestGenerateShadowName());
            context->SetName("test:shadow:0");
            EXPECT_EQ("test:shadow:1", context->TestGenerateShadowName());
            context->SetName("test:shadow");
            EXPECT_EQ("test:shadow:1", context->TestGenerateShadowName());
            context->SetName("test:test:100");
            EXPECT_EQ("test:shadow:101", context->TestGenerateShadowName());
        }

        TEST_F(JSContextTest, CreateShadowRealm)
        {
        }
    }
}
