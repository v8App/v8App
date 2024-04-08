// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <string>
#include <filesystem>

#include "v8-snapshot.h"

#include "tools/cpp/runfiles/runfiles.h"

using bazel::tools::cpp::runfiles::Runfiles;

extern std::unique_ptr<Runfiles> s_Runfiles;
extern std::filesystem::path s_TestDir;
extern v8::StartupData s_V8StartupData;

