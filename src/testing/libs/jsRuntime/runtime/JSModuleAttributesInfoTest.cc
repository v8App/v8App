// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "V8Fixture.h"

#include "Serialization/ReadBuffer.h"
#include "Serialization/WriteBuffer.h"

#include "Utils/Format.h"

#include "JSApp.h"
#include "JSModuleAttributesInfo.h"
#include "JSUtilities.h"

namespace v8App
{
    namespace JSRuntime
    {
        using JSModuleAttributesInfoTest = V8Fixture;

        TEST_F(JSModuleAttributesInfoTest, Constructor)
        {
            V8IsolateScope isolateScope(m_Isolate);
            V8HandleScope scope(m_Isolate);
            JSModuleAttributesInfo info;

            EXPECT_EQ(JSModuleType::kNoAttribute, info.m_Type);
            EXPECT_EQ("", info.m_Module);
            EXPECT_EQ("", info.m_Version.GetVersionString());
        }

        TEST_F(JSModuleAttributesInfoTest, DoesExtensionMatchType)
        {
            JSModuleAttributesInfo info;

            info.m_Type = JSModuleType::kJavascript;
            EXPECT_TRUE(info.DoesExtensionMatchType(JSModuleAttributesInfo::kExtJS));
            EXPECT_TRUE(info.DoesExtensionMatchType(JSModuleAttributesInfo::kExtModuleJS));
            EXPECT_FALSE(info.DoesExtensionMatchType(".txt"));

            info.m_Type = JSModuleType::kJSON;
            EXPECT_TRUE(info.DoesExtensionMatchType(JSModuleAttributesInfo::kExtJSON));
            EXPECT_FALSE(info.DoesExtensionMatchType(".txt"));

            info.m_Type = JSModuleType::kNative;
            EXPECT_TRUE(info.DoesExtensionMatchType(JSModuleAttributesInfo::kExtNative));
            EXPECT_FALSE(info.DoesExtensionMatchType(".txt"));
        }

        TEST_F(JSModuleAttributesInfoTest, Serialization)
        {
            JSModuleAttributesInfo info;
            info.m_Module = "test";
            info.m_Version.SetVersionString("1.1.1");
            info.m_Type = JSModuleType::kJavascript;

            Serialization::WriteBuffer wBuffer;
            wBuffer << info;
            ASSERT_FALSE(wBuffer.HasErrored());

            JSModuleAttributesInfo info2;
            Serialization::ReadBuffer rBuffer(wBuffer.GetData(), wBuffer.BufferSize());

            rBuffer >> info2;
            ASSERT_FALSE(rBuffer.HasErrored());
            EXPECT_EQ("test", info2.m_Module);
            EXPECT_EQ(JSModuleType::kJavascript, info2.m_Type);
            EXPECT_EQ("1.1.1", info2.m_Version.GetVersionString());
        }

        TEST_F(JSModuleAttributesInfoTest, GetModuleAttributesInfo)
        {
            //TODO: Write a test to test it. Will have to craft a number of modules with import with statements
            //to do it.
        }
    }
}