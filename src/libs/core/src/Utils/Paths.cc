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

        std::filesystem::path NormalizePath(std::filesystem::path inPath, bool inRemoveDrive)
        {
            std::string path = inPath.string();
            std::string windowsDrive = ExtractWindowsUNC(path);
            if (windowsDrive.length())
            {
                path = path.substr(windowsDrive.length());
            }
            if (inRemoveDrive)
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

        std::filesystem::path MakeRelativePathToRoot(std::filesystem::path inPath, std::filesystem::path inRoot)
        {
#ifndef V8APP_WINDOWS
            std::string winPathDrive = ExtractWindowsUNC(inPath.string());
            std::string winRootDrive = ExtractWindowsUNC(inPath.string());
            inPath = NormalizePath(inPath, true);
            inRoot = NormalizePath(inRoot, true);
#endif

            std::string gen_path = inPath.generic_string();
            std::string gen_root = inRoot.generic_string();
            std::string root_root_name = inRoot.root_name().generic_string();
            std::string path_root_name = inPath.root_name().generic_string();

            // Extract windows root names ie drive letter so we can compare posix paths and windows paths better
            // Windows root names have a size where as posix are empty
            if (path_root_name.size() != 0)
            {
                gen_path = gen_path.substr(path_root_name.size());
            }
            if (root_root_name.size() != 0)
            {
                gen_root = gen_root.substr(root_root_name.size());
            }

            if (inPath.is_absolute() || inPath.has_root_path())
            {
                if (gen_path.starts_with(gen_root))
                {
                    inPath = std::filesystem::path(gen_path.substr(gen_root.size()));
                    gen_path = inPath.generic_string();
                }
                if (inPath.has_root_path())
                {
                    std::string root = inPath.root_path().generic_string();
                    inPath = std::filesystem::path(inPath.generic_string().substr(root.size()));
                }
            }
            inPath = inRoot / inPath;
            // resolve any dots paths
            inPath = std::filesystem::weakly_canonical(inPath);
            gen_path = inPath.generic_string();

            if (gen_path.starts_with(inRoot.generic_string()) == false)
            {
                return std::filesystem::path();
            }
            return inPath.lexically_relative(inRoot);
        }

        std::filesystem::path MakeAbsolutePathToRoot(std::filesystem::path inPath, std::filesystem::path inRoot)
        {
#ifndef V8APP_WINDOWS
            std::string winPathDrive = ExtractWindowsUNC(inPath.string());
            std::string winRootDrive = ExtractWindowsUNC(inRoot.string());
            inPath = NormalizePath(inPath, true);
            inRoot = NormalizePath(inRoot, true);
#else
            std::string winRootDrive = "";
#endif

            std::string gen_path = inPath.generic_string();
            std::string gen_root = inRoot.generic_string();
            std::string root_root_name = inRoot.root_name().generic_string();
            std::string path_root_name = inPath.root_name().generic_string();

            // Extract windows root names ie drive letter so we can compare posix paths and windows paths better
            // Windows root names have a size where as posix are empty
            if (path_root_name.size() != 0)
            {
                gen_path = gen_path.substr(path_root_name.size());
            }
            if (root_root_name.size() != 0)
            {
                gen_root = gen_root.substr(root_root_name.size());
            }

            if (inPath.is_absolute() || inPath.has_root_path())
            {
                if (gen_path.starts_with(gen_root) == false)
                {

                    std::string root = inPath.root_path().generic_string();
                    inPath = std::filesystem::path(inPath.generic_string().substr(root.size()));
                    gen_path = inPath.generic_string();
                }
            }
            inPath = inRoot / inPath;
            inPath = std::filesystem::weakly_canonical(inPath);
            gen_path = inPath.generic_string();

            if (gen_path.starts_with(inRoot.generic_string()) == false)
            {
                return std::filesystem::path();
            }

            return winRootDrive + inPath.generic_string();
        }
    }
}
