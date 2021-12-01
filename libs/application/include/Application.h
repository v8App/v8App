// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef __APPLICATION_H__
#define __APPLICATION_H__

#include <filesystem>
#include <string>

#include "Platform.h"

namespace v8App
{
    class Application
    {
    public:
        Application() {}
        ~Application() {}

        void SetRooApplicationPath(std::string inRootPath);
        std::filesystem::path GetRootApplicationPath();

    protected:
        std::filesystem::path m_RootAppPath;
        std::unique_ptr<Platform> m_PlatformInstance;
    };
}
#endif //__APPLICATION_H__