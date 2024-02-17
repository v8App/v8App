// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "Utils/Paths.h"

namespace v8App
{
    namespace Utils
    {
        /**
         * Extracts the Windows driver letter or UNC prefix from the path and returns it.
         */
        std::string ExtractWindowsUNC(std::string inPath)
        {
            size_t start = 0;
            if (inPath.find_first_of('\\') != std::string::npos)
            {
                if (inPath[1] == ':')
                {
                    start = 2;
                }
                if (inPath.substr(0, 2) == "\\\\")
                {
                    size_t pos = inPath.find_first_of("\\", 3);
                    if (pos != std::string::npos)
                    {
                        pos = inPath.find_first_of("\\", pos + 1);
                        if (pos != std::string::npos)
                        {
                            start = pos;
                        }
                    }
                }
            }
            return start == 0 ? "" : inPath.substr(0, start);
        }

        /**
         * Converts \ to / exccpet where it escapes a space as in '\ '
         * By default will remove the drive or UNC if it exists.
         * If the drive is not removed it will preserve the \'s in it.
         */
        std::filesystem::path NormalizePath(std::filesystem::path inPath, bool removeDrive)
        {
            std::string path = inPath.string();
            std::string windowsDrive = ExtractWindowsUNC(path);
            if (windowsDrive.length())
            {
                path = path.substr(windowsDrive.length());
            }
            if (removeDrive)
            {
                windowsDrive = "";
            }

            // we need to walk the entire string cause we need to be careful of escaped spaces
            std::string converted;
            size_t pathLength = path.length();
            for (size_t x = 0; x < pathLength; x++)
            {
                char strChar = path[x];
                if (strChar == '\\')
                {
                    if (x + 1 < pathLength)
                    {
                        if (path[x + 1] != ' ')
                        {
                            strChar = '/';
                        }
                    }
                    else
                    {
                        strChar = '/';
                    }
                }
                converted += strChar;
            }

            return windowsDrive + converted;
        }

        /**
         * Make a relative path from the specified root. If the relative path escapes the specified root then an empty path is returned.
         * If an absolut path is passed in the it's consdered absolute to the passed in root path.
         * Example abs /test/test.txt, root = /opt/JSApp then the reltive path becomes test/test.txt.
         * NOTE For a windows path it will rmeove the drive or UNC from the path
         */
        std::filesystem::path MakeRelativePathToRoot(std::filesystem::path inPath, std::filesystem::path inRoot)
        {
            inPath = NormalizePath(inPath);
            if (inPath.is_absolute())
            {
                std::filesystem::path absRoot = inRoot;
                if (inPath.string().starts_with(inRoot.string()) == false)
                {
                    // we want to append the app root to the path but the / at the start will prevent it so
                    // force it relative to / to remove it.
                    absRoot = std::filesystem::path("/");
                }
                inPath = inPath.lexically_relative(absRoot);
            }
            inPath = inRoot / inPath;

            inPath = inPath.lexically_normal();
            if (inPath.string().starts_with(inRoot.string()) == false)
            {
                return std::filesystem::path();
            }
            return inPath.lexically_relative(inRoot);
        }

        /**
         * Make an absolut path from the specified root. If the relative path escapes the specified root then an empty path is returned.
         * If an absolut path is passed in the it's consdered absolute to the passed in root path.
         * Example abs /test/test.txt, root = /opt/JSApp then the reltive path becomes test/test.txt.
         * NOTE For a windows path it will rmeove the drive or UNC from the path
         */
        std::filesystem::path MakeAbsolutePathToRoot(std::filesystem::path inPath, std::filesystem::path inRoot)
        {
            inPath = NormalizePath(inPath);
            if (inPath.is_absolute())
            {
                std::filesystem::path absRoot = inRoot;
                if (inPath.string().starts_with(inRoot.string()))
                {
                    return inPath;
                }
                // we want to append the app root to the path but the / at the start will prevent it so
                // force it relative to / to remove it.
                inPath = inPath.lexically_relative(std::filesystem::path("/"));
            }
            inPath = inRoot / inPath;
            inPath = inPath.lexically_normal();
            if (inPath.string().starts_with(inRoot.string()) == false)
            {
                return std::filesystem::path();
            }

            return inPath;
        }

    }
}
