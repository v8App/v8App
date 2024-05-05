// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef __TEST_FILES_H__
#define __TEST_FILES_H__

#include <filesystem>
#include <iostream>
#include <thread>

#include "Assets/AppAssetRoots.h"
#include "Utils/Paths.h"

namespace v8App
{
    namespace TestUtils
    {
        bool CreateAppDirectory(std::filesystem::path inRootPath)
        {
            // make sure the test directory is deleted or the creation fails
            std::filesystem::remove_all(inRootPath);
            std::filesystem::create_directories(inRootPath / Assets::c_RootJS);
            std::filesystem::create_directories(inRootPath / Assets::c_RootModules);
            std::filesystem::create_directories(inRootPath / Assets::c_RootResource);
            if (std::filesystem::exists(inRootPath / (Assets::c_RootJS)) == false)
            {
                std::cout << "Waiting on '" << inRootPath.string() << "' to exist ";
                for (int x = 0; x < 30; x++)
                {
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                    x++;
                    if (std::filesystem::exists(inRootPath / Assets::c_RootJS))
                    {
                        break;
                    }
                    std::cout << ".";
                }
            }
            if (std::filesystem::exists(inRootPath / Assets::c_RootJS) == false)
            {
                std::cout << std::endl << "'" << inRootPath << "' failed to exist after 30 seconds" << std::endl;
                return false;
            }
            return true;
        }
    }
}
#endif //__TEST_FILES_H__