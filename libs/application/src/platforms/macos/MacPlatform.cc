// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <mach-o/dyld.h>

#include "plstforms/macos/MacPlatform.h"

namespace v8App {
    std::filesystem::path MacPlatform::GetExecutablePath()
    {
        std::string buffer;
        int bufferLen = 1024;

        buffer.reserve(bufferLen);

        if(_NSGetExecutablePath(&buffer[0], &bufferLen) == -1)
        {
            buffer.reserve(bufferLen);
            if(_NSGetExecutablePath(&buffer[0], &bufferLen) == 0)
            {
                return std::filesystem::path();
            }
        }

        return std::filesystem::path(buffer);
    }
}