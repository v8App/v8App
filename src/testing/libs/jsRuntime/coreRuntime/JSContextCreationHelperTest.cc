
// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <iostream>
#include <string>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "JSContextCreationHelper.h"
#include "JSRuntime.h"

namespace v8App
{
    namespace JSRuntime
    {
        class TestJSContextCreation : public JSContextCreationHelper
        {
        public:
            virtual JSContextSharedPtr CreateContext(JSRuntimeSharedPtr inRuntime, std::string inName, std::string inNamespce,
                                                     std::filesystem::path inEntryPoint, std::filesystem::path inSnapEntryPoint, bool inSupportsSnapshot,
                                                     SnapshotMethod inSnapMethod) override { return JSContextSharedPtr(); };

            virtual void DisposeContext(JSContextSharedPtr inContext) override {};
            virtual void RegisterSnapshotCloser(JSContextSharedPtr) override{};
            virtual void UnregisterSnapshotCloser(JSContextSharedPtr inContext) override {};

            bool TestSetContextNamespaces(V8StartupData *inData) { return SetContextNamespaces(inData); }

            V8StartupData TestSerializeContextNamespaces() { return SerializeContextNamespaces(); }

            bool TestAddSnapIndexNamespace(size_t inSnapIndex, std::string inNamespace) { return AddSnapIndexNamespace(inSnapIndex, inNamespace); }
        };

        TEST(JSContextCreationHelper, AddGetIndexNamespace)
        {
            std::unique_ptr<TestJSContextCreation> helper = std::make_unique<TestJSContextCreation>();

            EXPECT_TRUE(helper->TestAddSnapIndexNamespace(0, ""));
            EXPECT_TRUE(helper->TestAddSnapIndexNamespace(0, "test"));
            EXPECT_FALSE(helper->TestAddSnapIndexNamespace(0, "test2"));
            EXPECT_FALSE(helper->TestAddSnapIndexNamespace(1, "test"));

            EXPECT_EQ(0, helper->GetSnapIndexForNamespace("test"));
            EXPECT_EQ(JSRuntime::kMaxContextNamespaces, helper->GetSnapIndexForNamespace("test2"));
            EXPECT_EQ("test", helper->GetNamespaceForSnapIndex(0));
            EXPECT_EQ("", helper->GetNamespaceForSnapIndex(5));
        }

        TEST(JSContextCreationHelper, SerializeDeserilaizeNamespaces)
        {
            std::unique_ptr<TestJSContextCreation> helper = std::make_unique<TestJSContextCreation>();
            std::unique_ptr<TestJSContextCreation> helper2 = std::make_unique<TestJSContextCreation>();

            EXPECT_TRUE(helper->TestAddSnapIndexNamespace(0, "test1"));
            EXPECT_TRUE(helper->TestAddSnapIndexNamespace(1, "test2"));
            EXPECT_TRUE(helper->TestAddSnapIndexNamespace(2, "test3"));

            V8StartupData data{nullptr, 0};
            EXPECT_FALSE(helper2->TestSetContextNamespaces(&data));

            data = helper->TestSerializeContextNamespaces();
            EXPECT_TRUE(helper2->TestSetContextNamespaces(&data));

            EXPECT_EQ("test1", helper->GetNamespaceForSnapIndex(0));
            EXPECT_EQ("test2", helper->GetNamespaceForSnapIndex(1));
            EXPECT_EQ("test3", helper->GetNamespaceForSnapIndex(2));
        }

    }
}