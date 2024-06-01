// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

// because _TIME_H_ is going to be already defined in lib header
#ifndef _V8APP_TIME_H_
#define _V8APP_TIME_H_

#include <chrono>

#ifdef UNIT_TESTING
#include "TestTime.h"
#endif
namespace v8App
{
    namespace Time
    {
        /**
         * Gets the time in an increasing in in seconds since the empoch
         */
        inline double MonotonicallyIncreasingTimeSeconds()
        {
#ifdef UNIT_TESTING
            if (TestTime::TestTimeSeconds::IsEnabled())
            {
                return TestTime::TestTimeSeconds::Get();
            }
#endif
            uint64_t now = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
            return now;
        }

        /**
         * Gets the time in an increasing in in milliseconds since the empoch
         */
        inline double MonotonicallyIncreasingTimeMilliSeconds()
        {
#ifdef UNIT_TESTING
            if (TestTime::TestTimeMilliSeconds::IsEnabled())
            {
                return TestTime::TestTimeMilliSeconds::Get();
            }
#endif
            uint64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
            return now;
        }
    } // namespace Time
} // namespace v8App
#endif