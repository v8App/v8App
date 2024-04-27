// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "Utils/Paths.h"

namespace v8App
{
    namespace Utils
    {
        std::filesystem::path MakeRelativePathToRoot(std::filesystem::path inPath, std::filesystem::path inRoot)
        {
            std::string gen_path = inPath.generic_string();
            std::string gen_root = inRoot.generic_string();
            std::string root_root_name = inRoot.root_name().generic_string();
            std::string path_root_name = inPath.root_name().generic_string();

            //Extract windows root names ie drive letter so we can compare posix paths and windows paths better
            //Windows root names have a size where as posix are empty
            if(path_root_name.size() != 0)
            {
                gen_path = gen_path.substr(path_root_name.size());
            }
            if(root_root_name.size() != 0)
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
                if(inPath.has_root_path())
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
            std::string gen_path = inPath.generic_string();
            std::string gen_root = inRoot.generic_string();
            std::string root_root_name = inRoot.root_name().generic_string();
            std::string path_root_name = inPath.root_name().generic_string();

            //Extract windows root names ie drive letter so we can compare posix paths and windows paths better
            //Windows root names have a size where as posix are empty
            if(path_root_name.size() != 0)
            {
                gen_path = gen_path.substr(path_root_name.size());
            }
            if(root_root_name.size() != 0)
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

            return inPath;
        }
    }
}
