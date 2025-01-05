// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef __TEST_FILES_H__
#define __TEST_FILES_H__

#include <filesystem>

namespace v8App
{
    namespace TestUtils
    {
        bool CreateAppDirectory(std::filesystem::path inRootPath);
    }
}
#endif //__TEST_FILES_H__