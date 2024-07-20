// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <iostream>
#include <string>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "test_main.h"

#include "JSContext.h"

#include "V8InitApp.h"

namespace v8App
{
    namespace JSRuntime
    {
        using JSContextTest = V8InitApp;

        class TestJSContext : public JSContext
        {
        public:
            TestJSContext(JSRuntimeSharedPtr inRuntime, std::string inName, std::string inNamespace, std::filesystem::path inEntryPoint,
                          size_t inContextIndex, std::filesystem::path inSnapEntryPoint = "", bool inSupportsSnapshot = true,
                          SnapshotMethod inSnapMethod = SnapshotMethod::kNamespaceOnly)
                : JSContext(inRuntime, inName, inNamespace, inEntryPoint, inContextIndex, inSnapEntryPoint, inSupportsSnapshot, inSnapMethod) {}
            virtual ~TestJSContext() = default;

            void SetInit(bool inValue) { m_Initialized = inValue; }
            bool ContextEmpty() { return m_Context.IsEmpty(); }
            void ClearIsolate() { m_Runtime.reset(); }

            bool TestCreateContext() { return CreateContext(); }
            void TestDisposeContext() { DisposeContext(); }
            void SetJSRuntime(JSRuntimeSharedPtr inRuntime) { m_Runtime = inRuntime; }
            void ClearSnapEntry() { m_SnapEntryPoint = ""; }

            JSContextWeakPtr *TestGetContextWeakRef() { return GetContextWeakRef(); }

            void TestMoveContext(JSContext &&inContext) { MoveContext(std::move(inContext)); }
            std::string TestGenerateShadowName() { return GenerateShadowName(); }
            void SetName(std::string inName) { m_Name = inName; }
        };

        TEST_F(JSContextTest, Constructor)
        {
            JSRuntimeSharedPtr runtime = m_App->GetMainRuntime();
            ASSERT_NE(nullptr, runtime);
            V8Isolate *isolate = runtime->GetIsolate();
            V8IsolateScope isolateScope(isolate);
            V8HandleScope handleScope(isolate);

            TestJSContext context(runtime, "test:test", "test", "testEntry", 0, "testSnapEntry", false, SnapshotMethod::kNamespaceAndEntrypoint);

            EXPECT_EQ(runtime->GetIsolate(), context.GetIsolate());
            EXPECT_EQ(runtime, context.GetJSRuntime());
            EXPECT_EQ(m_App, context.GetJSApp());
            EXPECT_EQ(nullptr, context.GetJSModules());
            EXPECT_EQ("testtest", context.GetName());
            EXPECT_EQ("test", context.GetNamespace());
            EXPECT_EQ(0, context.GetSnapshotIndex());
            EXPECT_TRUE(context.GetLocalContext().IsEmpty());
            EXPECT_FALSE(context.SupportsSnapshots());
            EXPECT_EQ(SnapshotMethod::kNamespaceAndEntrypoint, context.GetSnapshotMethod());
            EXPECT_EQ("testEntry", context.GetEntrypoint());
            EXPECT_EQ("testSnapEntry", context.GetSnapshotEntrypoint());
            EXPECT_FALSE(context.IsInitialized());
            EXPECT_TRUE(context.ContextEmpty());

            context.ClearIsolate();
            context.ClearSnapEntry();
            EXPECT_EQ(nullptr, context.GetIsolate());
            EXPECT_EQ("testEntry", context.GetSnapshotEntrypoint());
        }

        TEST_F(JSContextTest, CreateDisposeContext)
        {
            JSRuntimeSharedPtr runtime = m_App->GetMainRuntime();
            ASSERT_NE(nullptr, runtime);
            V8Isolate *isolate = runtime->GetIsolate();
            V8IsolateScope isolateScope(isolate);
            V8HandleScope handleScopr(isolate);

            std::shared_ptr<TestJSContext> context = std::make_shared<TestJSContext>(runtime, "test", "", "", 0);
            context->TestCreateContext();
            EXPECT_FALSE(context->ContextEmpty());
            EXPECT_FALSE(context->TestGetContextWeakRef()->expired());
            EXPECT_EQ(context->TestGetContextWeakRef()->lock().get(), context.get());
            EXPECT_TRUE(context->IsInitialized());

            context->TestDisposeContext();
            EXPECT_TRUE(context->ContextEmpty());
            EXPECT_FALSE(context->IsInitialized());
            EXPECT_EQ(nullptr, context->GetJSRuntime());

            context.reset();

            context = std::make_shared<TestJSContext>(runtime, "test", "", "", 0);
            context->TestCreateContext();
            EXPECT_FALSE(context->ContextEmpty());
            context->ClearIsolate();
            context->TestDisposeContext();
            EXPECT_FALSE(context->ContextEmpty());
            context->SetJSRuntime(runtime);
            context->SetInit(true);
            context->TestDisposeContext();
        }

        TEST_F(JSContextTest, MoveContext)
        {
            JSRuntimeSharedPtr runtime = m_App->GetMainRuntime();
            ASSERT_NE(nullptr, runtime);
            V8Isolate *isolate = runtime->GetIsolate();
            V8IsolateScope isolateScope(isolate);
            V8HandleScope handleScope(isolate);

            std::shared_ptr<TestJSContext> context = std::make_shared<TestJSContext>(runtime, "test", "test", "test", 0, "testSnap", false, SnapshotMethod::kNamespaceAndEntrypoint);
            context->TestCreateContext();
            std::shared_ptr<TestJSContext> context2 = std::make_shared<TestJSContext>(runtime, "", "", "", 0);

            context2->TestMoveContext(std::move(*context.get()));

            EXPECT_EQ(context->GetJSRuntime(), nullptr);
            EXPECT_TRUE(context->ContextEmpty());
            EXPECT_FALSE(context->IsInitialized());
            EXPECT_EQ(nullptr, context->TestGetContextWeakRef());

            EXPECT_EQ(context2->GetJSRuntime().get(), runtime.get());
            EXPECT_FALSE(context2->ContextEmpty());
            EXPECT_TRUE(context2->IsInitialized());
            EXPECT_EQ(context2.get(), context2->TestGetContextWeakRef()->lock().get());
            EXPECT_EQ("test", context2->GetName());
            EXPECT_EQ("test", context2->GetNamespace());
            EXPECT_EQ("test", context2->GetEntrypoint());
            EXPECT_EQ("testSnap", context2->GetSnapshotEntrypoint().string());
            EXPECT_FALSE(context2->SupportsSnapshots());
            EXPECT_EQ(0, context->GetSnapshotIndex());
            EXPECT_EQ(SnapshotMethod::kNamespaceAndEntrypoint, context2->GetSnapshotMethod());

            std::shared_ptr<TestJSContext> context3 = std::make_shared<TestJSContext>(runtime, "", "", "", 0);

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
            JSRuntimeSharedPtr runtime = m_App->GetMainRuntime();
            ASSERT_NE(nullptr, runtime);
            V8Isolate *isolate = runtime->GetIsolate();
            V8IsolateScope isolateScope(isolate);
            V8HandleScope handleScopr(isolate);

            JSContextSharedPtr context = runtime->CreateContext("test", "", "");
            EXPECT_NE(context, nullptr);

            V8LContext local = context->GetLocalContext();
            EXPECT_FALSE(local.IsEmpty());
        }

        TEST_F(JSContextTest, GenerateShadowName)
        {
            JSRuntimeSharedPtr runtime = m_App->GetMainRuntime();
            ASSERT_NE(nullptr, runtime);
            V8Isolate *isolate = runtime->GetIsolate();
            V8IsolateScope isolateScope(isolate);
            V8HandleScope handleScope(isolate);

            std::shared_ptr<TestJSContext> context = std::make_shared<TestJSContext>(runtime, "test", "", "", 0);
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

        /**
         * RunModule is tested in the runtime set of tests with JSCOntextModulesTest
         */

        TEST_F(JSContextTest, RunScript)
        {
            JSRuntimeSharedPtr runtime = m_App->GetMainRuntime();

            JSContextSharedPtr jsContext = runtime->CreateContext("runModules", "", "", "", false, SnapshotMethod::kNamespaceOnly);
            ASSERT_NE(nullptr, jsContext);

            const char* testSource1 = R"(
                var test log
            )";

            V8Isolate::Scope iScope(runtime->GetIsolate());
            V8HandleScope hScope(runtime->GetIsolate());
            V8Context::Scope cScope(jsContext->GetLocalContext());

            V8LValue testValue = jsContext->RunScript(testSource1);
            EXPECT_TRUE(testValue.IsEmpty());

            const char* testSource2 = R"(
                42;
            )";

            testValue = jsContext->RunScript(testSource2);
            ASSERT_FALSE(testValue.IsEmpty());
            EXPECT_EQ(42, testValue->IntegerValue(jsContext->GetLocalContext()).FromJust());
        }

        TEST_F(JSContextTest, CreateShadowRealm)
        {
        }
    }
}
