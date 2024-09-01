// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <string>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "Serialization/ReadBuffer.h"
#include "Serialization/WriteBuffer.h"
#include "Utils/VersionString.h"

namespace v8App
{
    namespace Utils
    {
        // Version taking from the Semantic Version website that are valid
        std::string v1 = "1.0.0";
        std::string v2 = "1.0.0-alpha";
        std::string v3 = "1.0.0-x.7.z.92";
        std::string v4 = "1.0.0+21AF26D3----117B344092BD";
        std::string v5 = "1.0.0-beta+exp.sha.5114f85";
        std::string v6 = "1";
        std::string v7 = "1.0";
        std::string v8 = "1.1.0";
        std::string v9 = "2.0.0";
        std::string v10 = "2.1.1";
        std::string v11 = "1.0.0-alpha.beta";
        std::string v12 = "1.0.0-beta.2";
        std::string v13 = "1.0.0-beta.11";
        std::string v14 = "1.0.0-rc.1";

        TEST(VersionStringTest, IsVersionString)
        {
            // invalid
            VersionString test1;
            EXPECT_FALSE(test1.IsVersionString());

            VersionString test2("1a");
            EXPECT_FALSE(test2.IsVersionString());

            VersionString test3("1.a");
            EXPECT_FALSE(test3.IsVersionString());

            VersionString test4("1-beta[2]");
            EXPECT_FALSE(test4.IsVersionString());

            VersionString test5("1-beta[2]");
            EXPECT_FALSE(test5.IsVersionString());

            VersionString test6("1+beta[2]");
            EXPECT_FALSE(test6.IsVersionString());

            // valid
            VersionString test7(v1);
            EXPECT_TRUE(test7.IsVersionString());

            VersionString test8(v2);
            EXPECT_TRUE(test8.IsVersionString());

            VersionString test9(v3);
            EXPECT_TRUE(test9.IsVersionString());

            VersionString test10(v4);
            EXPECT_TRUE(test10.IsVersionString());

            VersionString test11(v5);
            EXPECT_TRUE(test11.IsVersionString());

            VersionString test12(v5);
            EXPECT_TRUE(test12.IsVersionString());

            VersionString test13(v5);
            EXPECT_TRUE(test13.IsVersionString());

            VersionString test14(v5);
            EXPECT_TRUE(test14.IsVersionString());
        }

        TEST(VersionStringTest, TestAccessors)
        {
            // invalid string
            VersionString test1;
            EXPECT_EQ(test1.GetMajor(), -1);
            EXPECT_EQ(test1.GetMinor(), -1);
            EXPECT_EQ(test1.GetPatch(), -1);
            EXPECT_EQ(test1.GetPreRelease(), "");
            EXPECT_EQ(test1.GetBuild(), "");
            EXPECT_EQ(test1.GetVersionString(), "0.0.0");

            // via constrcutor
            VersionString test2(v1);
            EXPECT_EQ(test2.GetMajor(), 1);
            EXPECT_EQ(test2.GetMinor(), 0);
            EXPECT_EQ(test2.GetPatch(), 0);
            EXPECT_EQ(test2.GetPreRelease(), "");
            EXPECT_EQ(test2.GetBuild(), "");
            EXPECT_EQ(test2.GetVersionString(), v1);

            // via the set string
            VersionString test3;
            EXPECT_TRUE(test3.SetVersionString(v3));
            EXPECT_EQ(test3.GetMajor(), 1);
            EXPECT_EQ(test3.GetMinor(), 0);
            EXPECT_EQ(test3.GetPatch(), 0);
            EXPECT_EQ(test3.GetPreRelease(), "x.7.z.92");
            EXPECT_EQ(test3.GetBuild(), "");
            EXPECT_EQ(test3.GetVersionString(), v3);

            VersionString test4(v4);
            EXPECT_EQ(test4.GetMajor(), 1);
            EXPECT_EQ(test4.GetMinor(), 0);
            EXPECT_EQ(test4.GetPatch(), 0);
            EXPECT_EQ(test4.GetPreRelease(), "");
            EXPECT_EQ(test4.GetBuild(), "21AF26D3----117B344092BD");
            EXPECT_EQ(test4.GetVersionString(), v4);

            VersionString test5(v5);
            EXPECT_EQ(test5.GetMajor(), 1);
            EXPECT_EQ(test5.GetMinor(), 0);
            EXPECT_EQ(test5.GetPatch(), 0);
            EXPECT_EQ(test5.GetPreRelease(), "beta");
            EXPECT_EQ(test5.GetBuild(), "exp.sha.5114f85");
            EXPECT_EQ(test5.GetVersionString(), v5);

            // test setters
            test5.SetMajor(10);
            test5.SetMinor(10);
            test5.SetPatch(10);
            test5.SetPreRelease("preRelease");
            test5.SetBuild("build");
            EXPECT_EQ(test5.GetMajor(), 10);
            EXPECT_EQ(test5.GetMinor(), 10);
            EXPECT_EQ(test5.GetPatch(), 10);
            EXPECT_EQ(test5.GetPreRelease(), "preRelease");
            EXPECT_EQ(test5.GetBuild(), "build");
            EXPECT_EQ("10.10.10-preRelease+build", test5.GetVersionString());
        }

        TEST(VersionStringTest, TestSerializers)
        {
            Serialization::WriteBuffer wBuffer;
            VersionString test1(v1);
            VersionString test2;

            wBuffer << test1;

            Serialization::ReadBuffer rBuffer(wBuffer.GetData(), wBuffer.BufferSize());
            rBuffer >> test2;

            EXPECT_EQ(1, test2.GetMajor());
            EXPECT_EQ(0, test2.GetMinor());
            EXPECT_EQ(0, test2.GetPatch());
            EXPECT_EQ("", test2.GetPreRelease());
            EXPECT_EQ("", test2.GetBuild());
        }

        TEST(VersionStringTest, TestCompareVersion)
        {
            VersionString vs1(v1);
            VersionString vs2(v2);
            VersionString vs9(v9);
            VersionString vs8(v8);
            VersionString vs10(v10);
            VersionString vs11(v11);
            VersionString vs12(v12);
            VersionString vs13(v13);
            VersionString vs14(v14);

            // Equal
            EXPECT_EQ(vs1.CompareVersions(vs1), 0);
            EXPECT_EQ(vs11.CompareVersions(vs11), 0);
            EXPECT_EQ(vs14.CompareVersions(vs14), 0);

            // <
            EXPECT_EQ(vs1.CompareVersions(vs9), -1);
            EXPECT_EQ(vs9.CompareVersions(vs10), -1);
            EXPECT_EQ(vs11.CompareVersions(vs12), -1);
            EXPECT_EQ(vs12.CompareVersions(vs13), -1);
            EXPECT_EQ(vs13.CompareVersions(vs14), -1);
            EXPECT_EQ(vs14.CompareVersions(vs1), -1);
            EXPECT_EQ(vs1.CompareVersions(vs9), -1);
            EXPECT_EQ(vs2.CompareVersions(vs11), -1);

            // >
            EXPECT_EQ(vs10.CompareVersions(vs9), 1);
            EXPECT_EQ(vs1.CompareVersions(vs14), 1);
            EXPECT_EQ(vs14.CompareVersions(vs13), 1);
            EXPECT_EQ(vs13.CompareVersions(vs12), 1);
            EXPECT_EQ(vs12.CompareVersions(vs11), 1);
            EXPECT_EQ(vs9.CompareVersions(vs8), 1);
            EXPECT_EQ(vs8.CompareVersions(vs1), 1);
            EXPECT_EQ(vs10.CompareVersions(vs9), 1);

            // implicit conversion from string
            EXPECT_EQ(vs1.CompareVersions(v1), 0);
            EXPECT_EQ(vs1.CompareVersions(v9), -1);
            EXPECT_EQ(vs10.CompareVersions(v9), 1);
        }

        TEST(VersionStringTest, ComparisonOperatos)
        {
            const VersionString vs1(v1);
            VersionString vs9(v9);
            VersionString vs10(v10);

            EXPECT_TRUE(vs1 == vs1);
            EXPECT_TRUE(vs1 < vs9);
            EXPECT_TRUE(vs9 > vs1);

            // compare against version strings
            EXPECT_TRUE(vs1 == v1);
            EXPECT_TRUE(vs1 < v9);
            EXPECT_TRUE(vs9 > v1);
        }
    }
}