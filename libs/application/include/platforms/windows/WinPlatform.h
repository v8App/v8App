// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef __WIN_PLATFORM_H__
#define __WIN_PLATFORM_H__

#include "BasePlatform.h"

namespace v8App {
    class WinPlatform : public BasePlatform
    {
        public:
        WinPlatform();
        virtual ~WinPlatform();

        virtual std::filesystem::path GetExecutablePath() override;
    };
}
#endif //__WIN_PLATFORM_H__