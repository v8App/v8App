// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef _LOG_MACROS_H_
#define _LOG_MACROS_H_

#include "Logging/Log.h"

#ifdef _WIN32
#define ABORT() _exit(134);
#else
#define ABORT() abort()
#endif

#define CHECK_FULL(expr, file, function, line)        \
    do                                                \
    {                                                 \
        if (!(expr))                                  \
        {                                             \
            static v8App::Log::LogMessage temp;                   \
            temp.emplace(v8App::Log::MsgKey::Msg, #expr);         \
            v8App::Log::Log::Fatal(temp, file, function, line); \
            ABORT();                                  \
        }                                             \
    } while (0);

#define CHECK(expr) CHECK_FULL(expr, __FILE__, __func__, __LINE__)

#define CHECK_EQ(a, b) CHECK((a) == (b))
#define CHECK_NE(a, b) CHECK((a) != (b))
#define CHECK_GT(a, b) CHECK((a) > (b))
#define CHECK_GE(a, b) CHECK((a) >= (b))
#define CHECK_LT(a, b) CHECK((a) < (b))
#define CHECK_LE(a, b) CHECK((a) <= (b))
#define CHECK_NULL(a) CHECK((a) == nullptr)
#define CHECK_NOT_NULL(a) CHECK((a) != nullptr)

#if V8APP_DEBUG
#define LOG_GENERAL(message) v8App::Log::Log::General(message, __FILE__, __func__, __LINE__)
#define LOG_ERROR(message) v8App::Log::Log::Error(message, __FILE__, __func__, __LINE__)
#define LOG_WARN(message) v8App::Log::Log::Warn(message, __FILE__, __func__, __LINE__)
#define LOG_DEBUG(message) v8App::Log::Log::Debug(message, __FILE__, __func__, __LINE__)
#define LOG_TRACE(message) v8App::Log::Log::Trace(message, __FILE__, __func__, __LINE__)
#define LOG_FATAL(message) v8App::Log::Log::Fatal(message, __FILE__, __func__, __LINE__)

#define DCHECK_EQ(a, b) CHECK((a) == (b))
#define DCHECK_NE(a, b) CHECK((a) != (b))
#define DCHECK_GT(a, b) CHECK((a) > (b))
#define DCHECK_GE(a, b) CHECK((a) >= (b))
#define DCHECK_LT(a, b) CHECK((a) < (b))
#define DCHECK_LE(a, b) CHECK((a) <= (b))
#define DCHECK_NULL(a) CHECK((a) == nullptr)
#define DCHECK_NOT_NULL(a) CHECK((a) != nullptr)

#else
#define LOG_GENERAL(message) v8App::Log::Log::General(message)
#define LOG_ERROR(message) v8App::Log::Log::Error(message)
#define LOG_WARN(message) v8App::Log::Log::Warn(message)
#define LOG_DEBUG(message) v8App::Log::Log::Debug(message)
#define LOG_TRACE(message) v8App::Log::Log::Trace(message)
#define LOG_FATAL(message) v8App::Log::Log::Fatal(message)

#define DCHECK_EQ(a, b) ((void) 0)
#define DCHECK_NE(a, b) ((void) 0)
#define DCHECK_GT(a, b) ((void) 0)
#define DCHECK_GE(a, b) ((void) 0)
#define DCHECK_LT(a, b) ((void) 0)
#define DCHECK_LE(a, b) ((void) 0)
#define DCHECK_NULL(a) ((void) 0)
#define DCHECK_NOT_NULL(a) ((void) 0)

#endif

#endif