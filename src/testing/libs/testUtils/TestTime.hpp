// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

// #include "TestTime.h"

namespace v8App
{
    namespace TestTime
    {
        template <class ClassName>
        bool TTestTime<ClassName>::s_Enabled = false;
        template <class ClassName>
        double TTestTime<ClassName>::s_Time = 0.0;

        template <class ClassName>
        void TTestTime<ClassName>::Enable()
        {
            s_Enabled = true;
        }

        template <class ClassName>
        void TTestTime<ClassName>::Clear()
        {
            s_Enabled = false;
        }

        template <class ClassName>
        bool TTestTime<ClassName>::IsEnabled()
        {
            return s_Enabled;
        }

        template <class ClassName>
        void TTestTime<ClassName>::Set(double inTime)
        {
            s_Time = inTime;
        }

        template <class ClassName>
        double TTestTime<ClassName>::Get()
        {
            return s_Time;
        }

    } // namespace testTime

} // namespace v8App
