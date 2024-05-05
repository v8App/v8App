// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <string>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "Utils/Paths.h"

namespace v8App
{
    namespace Utils
    {
        TEST(PathsTest, ExtractWindowsUNC)
        {
            std::filesystem::path path1 = std::filesystem::path("C:\\test\\test.txt");
            std::filesystem::path path2 = std::filesystem::path("C:test\\test.txt");
            std::filesystem::path path3 = std::filesystem::path("\\test\\test.txt");
            std::filesystem::path path4 = std::filesystem::path("test\\test.txt");
            std::filesystem::path path5 = std::filesystem::path("\\\\system2\\share\\test\\test.txt");
            std::filesystem::path path6 = std::filesystem::path("\\\\.\\C:\\test\\test.txt");
            std::filesystem::path path7 = std::filesystem::path("\\\\?\\C:\\test\\test.txt");
            std::filesystem::path path8 = std::filesystem::path("\\\\.\\Volume{b75e2c83-0000-0000-0000-602f00000000}\\test\\test.txt");

            EXPECT_EQ(ExtractWindowsUNC(path1), "C:");
            EXPECT_EQ(ExtractWindowsUNC(path2), "C:");
            EXPECT_EQ(ExtractWindowsUNC(path3), "");
            EXPECT_EQ(ExtractWindowsUNC(path4), "");
            EXPECT_EQ(ExtractWindowsUNC(path5), "\\\\system2\\share");
            EXPECT_EQ(ExtractWindowsUNC(path6), "\\\\.\\C:");
            EXPECT_EQ(ExtractWindowsUNC(path7), "\\\\?\\C:");
            EXPECT_EQ(ExtractWindowsUNC(path8), "\\\\.\\Volume{b75e2c83-0000-0000-0000-602f00000000}");
        }

        TEST(PathsTest, NormalizePath)
        {
            std::filesystem::path path1 = std::filesystem::path("test/test.txt");
            std::filesystem::path path2 = std::filesystem::path("C:\\test\\test.txt");
            std::filesystem::path path3 = std::filesystem::path("\\test\\test.txt");
            std::filesystem::path path4 = std::filesystem::path("/test/test\\ test.txt");

            EXPECT_EQ(NormalizePath(path1).generic_string(), path1.generic_string());
            EXPECT_EQ(NormalizePath(path2).generic_string(), "C:/test/test.txt");
            EXPECT_EQ(NormalizePath(path3).generic_string(), "/test/test.txt");
            EXPECT_EQ(NormalizePath(path4).generic_string(), path4);
        }

        TEST(PathsTest, MakeRelativePathToRoot)
        {
            std::filesystem::path root = std::filesystem::path("/opt/V8App/app-root");
            std::filesystem::path winRoot = std::filesystem::path("C:\\opt\\V8App\\app-root");

            std::filesystem::path path1 = std::filesystem::path("/test/test2");
            std::filesystem::path path2 = std::filesystem::path("test/test2");
            std::filesystem::path path3 = root / std::filesystem::path("test/test2");
            std::filesystem::path path4 = std::filesystem::path("./test/test2");
            std::filesystem::path path5 = std::filesystem::path("test/../test2");
            std::filesystem::path path6 = std::filesystem::path("../../test");
            std::filesystem::path win1 = std::filesystem::path("test\\test2");
            std::filesystem::path win2 = std::filesystem::path("C:\\test\\test2");

            // string version
            EXPECT_EQ(MakeRelativePathToRoot(path1.generic_string(), root.generic_string()).generic_string(), "test/test2");

            EXPECT_EQ(MakeRelativePathToRoot(path1, root).generic_string(), "test/test2");
            EXPECT_EQ(MakeRelativePathToRoot(path2, root).generic_string(), "test/test2");
            EXPECT_EQ(MakeRelativePathToRoot(path3, root).generic_string(), "test/test2");
            EXPECT_EQ(MakeRelativePathToRoot(path4, root).generic_string(), "test/test2");
            EXPECT_EQ(MakeRelativePathToRoot(path5, root).generic_string(), "test2");
            EXPECT_EQ(MakeRelativePathToRoot(path6, root).generic_string(), "");
            EXPECT_EQ(MakeRelativePathToRoot(win1, root).generic_string(), "test/test2");
            EXPECT_EQ(MakeRelativePathToRoot(win2, root).generic_string(), "test/test2");

            EXPECT_EQ(MakeRelativePathToRoot(path1, winRoot).generic_string(), "test/test2");
            EXPECT_EQ(MakeRelativePathToRoot(path2, winRoot).generic_string(), "test/test2");
            EXPECT_EQ(MakeRelativePathToRoot(path3, winRoot).generic_string(), "test/test2");
            EXPECT_EQ(MakeRelativePathToRoot(path4, winRoot).generic_string(), "test/test2");
            EXPECT_EQ(MakeRelativePathToRoot(path5, winRoot).generic_string(), "test2");
            EXPECT_EQ(MakeRelativePathToRoot(path6, winRoot).generic_string(), "");
            EXPECT_EQ(MakeRelativePathToRoot(win1, winRoot).generic_string(), "test/test2");
            EXPECT_EQ(MakeRelativePathToRoot(win2, winRoot).generic_string(), "test/test2");
        }

        TEST(PathsTest, MakeAbsolutePath)
        {
            std::filesystem::path root = std::filesystem::path("/opt/V8App/app-root");
            std::filesystem::path winRoot = std::filesystem::path("C:\\opt\\V8App\\app-root");

            std::filesystem::path normWinRoot = NormalizePath(winRoot);

            std::filesystem::path path1 = std::filesystem::path("/test/test2");
            std::filesystem::path path2 = std::filesystem::path("test/test2");
            std::filesystem::path path3 = std::filesystem::path("test/test2");
            std::filesystem::path path4 = std::filesystem::path("./test/test2");
            std::filesystem::path path5 = std::filesystem::path("test/../test2");
            std::filesystem::path path6 = std::filesystem::path("../../test");
            std::filesystem::path win1 = std::filesystem::path("test\\test2");
            std::filesystem::path win2 = std::filesystem::path("C:\\test\\test2");

            // string version
            EXPECT_EQ(MakeAbsolutePathToRoot(path1.string(), root.string()).generic_string(), (root / std::filesystem::path("test/test2")).generic_string());

            EXPECT_EQ(MakeAbsolutePathToRoot(path1, root).generic_string(), (root / std::filesystem::path("test/test2")).generic_string());
            EXPECT_EQ(MakeAbsolutePathToRoot(path2, root).generic_string(), (root / std::filesystem::path("test/test2")).generic_string());
            EXPECT_EQ(MakeAbsolutePathToRoot(path3, root).generic_string(), (root / std::filesystem::path("test/test2")).generic_string());
            EXPECT_EQ(MakeAbsolutePathToRoot(path4, root).generic_string(), (root / std::filesystem::path("test/test2")).generic_string());
            EXPECT_EQ(MakeAbsolutePathToRoot(path5, root).generic_string(), (root / std::filesystem::path("test2")).generic_string());
            EXPECT_EQ(MakeAbsolutePathToRoot(path6, root).generic_string(), "");
            EXPECT_EQ(MakeAbsolutePathToRoot(win1, root).generic_string(), (root / std::filesystem::path("test/test2")).generic_string());
            EXPECT_EQ(MakeAbsolutePathToRoot(win2, root).generic_string(), (root / std::filesystem::path("test/test2")).generic_string());

            EXPECT_EQ(MakeAbsolutePathToRoot(path1, winRoot).generic_string(), (normWinRoot / std::filesystem::path("test/test2")).generic_string());
            EXPECT_EQ(MakeAbsolutePathToRoot(path2, winRoot).generic_string(), (normWinRoot / std::filesystem::path("test/test2")).generic_string());
            EXPECT_EQ(MakeAbsolutePathToRoot(path3, winRoot).generic_string(), (normWinRoot / std::filesystem::path("test/test2")).generic_string());
            EXPECT_EQ(MakeAbsolutePathToRoot(path4, winRoot).generic_string(), (normWinRoot / std::filesystem::path("test/test2")).generic_string());
            EXPECT_EQ(MakeAbsolutePathToRoot(path5, winRoot).generic_string(), (normWinRoot / std::filesystem::path("test2")).generic_string());
            EXPECT_EQ(MakeAbsolutePathToRoot(path6, winRoot).generic_string(), "");
            EXPECT_EQ(MakeAbsolutePathToRoot(win1, winRoot).generic_string(), (normWinRoot / std::filesystem::path("test/test2")).generic_string());
            EXPECT_EQ(MakeAbsolutePathToRoot(win2, winRoot).generic_string(), (normWinRoot / std::filesystem::path("test/test2")).generic_string());
        }
    }
}