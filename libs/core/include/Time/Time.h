// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

//because _TIME_H_ is going to be already defined in lib header
#ifndef _V8APP_TIME_H_
#define _V8APP_TIME_H_

#include <chrono>

namespace v8App
{
    namespace Time
    {
        inline double MonotonicallyIncreasingTimeSeconds()
        {
            return std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch()).count();
        }
    } // namespace Time
} // namespace v8App
#endif