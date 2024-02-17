// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef __BASE_PLATFORM_H__
#define __BASE_PLATFORM_H__

#include <filesystem>
#include <string>

namespace v8App
{
    class BasePlatform
    {
    public:
        BasePlatform() {}
        virtual ~BasePlatform() {}

        virtual std::filesystem::path GetExecutablePath() = 0;
    };
}
#endif //__BASE_PLATFORM_H__