// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.
#ifndef _TEST_TIME_H_
#define _TEST_TIME_H_
namespace v8App
{
    namespace TestTime
    {
        template <class ClassName>
        class TTestTime
        {
        public:
            static void Enable();
            static void Clear();
            static bool IsEnabled();
            static void Set(double inTime);
            static double Get();

        private:
            static bool s_Enabled;
            static double s_Time;
        };

        struct TestTimeSecondsStruct
        {
        };
        class TestTimeSeconds : public TTestTime<TestTimeSecondsStruct>
        {
        };
        struct TestTimeMilliSecondsStruct
        {
        };
        class TestTimeMilliSeconds : public TTestTime<TestTimeMilliSecondsStruct>
        {
        };
    } // namespace testTime
} // namespace v8App

#include "TestTime.hpp"
#endif //_TEST_TIME_H_
