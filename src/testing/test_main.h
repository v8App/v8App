// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef __TEST_MAIN_H__
#define __TEST_MAIN_H__
#include <string>
#include <filesystem>

#ifdef USE_JSRUNTIME
#include "V8Types.h"
#endif

#include "tools/cpp/runfiles/runfiles.h"

using bazel::tools::cpp::runfiles::Runfiles;

extern std::unique_ptr<Runfiles> s_Runfiles;
extern std::filesystem::path s_TestDir;

#ifdef USE_JSRUNTIME
extern v8App::JSRuntime::V8StartupData s_V8StartupData;
#endif

#endif