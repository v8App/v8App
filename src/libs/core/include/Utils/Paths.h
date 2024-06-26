// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef __PATHS_H__
#define __PATHS_H__

#include <string>
#include <filesystem>

namespace v8App
{
    namespace Utils
    {
        /**
         * Extracts the Windows driver letter or UNC prefix from the path and returns it.
         */
        std::string ExtractWindowsUNC(std::string inPath);

        /**
         * Converts \ to / exccpet where it escapes a space as in '\ '
         * By default keeps the windows drive leter
         */
        std::filesystem::path NormalizePath(std::filesystem::path inPathe, bool inRemoveDrive = false);

        /**
         * Make a relative path from the specified root. If the relative path escapes the specified root then an empty path is returned.
         * If an absolut path is passed in the it's consdered absolute to the passed in root path.
         * Example abs /test/test.txt, root = /opt/JSApp then the relative path becomes test/test.txt.
         */
        std::filesystem::path MakeRelativePathToRoot(std::filesystem::path inPath, std::filesystem::path inRoot);

        /**
         * Make an absolut path from the specified root. If the relative path escapes the specified root then an empty path is returned.
         * If an absolut path is passed in the it's consdered absolute to the passed in root path.
         * Example abs /test/test.txt, root = /opt/JSApp then the abolute path becomes /opt/JSApp/test/test.txt.
         */
        std::filesystem::path MakeAbsolutePathToRoot(std::filesystem::path inPath, std::filesystem::path inRoot);
    }
}

#endif //__PATHS_H__