# Copyright 2020 - 2024 the v8App authors. All right reserved.
# Use of this source code is governed by the MIT license
# that can be found in the LICENSE file.

workspace(name = "v8App")

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive", "http_file")

new_local_repository(
    name = "libuv",
    build_file = "@v8App//:third_party/libuv.BUILD",
    path = "third_party/libuv",
)

http_archive(
    name = "bazel_skylib",
    sha256 = "f7be3474d42aae265405a592bb7da8e171919d74c16f082a5457840f06054728",
    urls = ["https://github.com/bazelbuild/bazel-skylib/releases/download/1.2.1/bazel-skylib-1.2.1.tar.gz"],
)

http_archive(
    name = "build_bazel_rules_apple",
    sha256 = "34c41bfb59cdaea29ac2df5a2fa79e5add609c71bb303b2ebb10985f93fa20e7",
    url = "https://github.com/bazelbuild/rules_apple/releases/download/3.1.1/rules_apple.3.1.1.tar.gz",
)

load(
    "@build_bazel_rules_apple//apple:repositories.bzl",
    "apple_rules_dependencies",
)

apple_rules_dependencies()

load(
    "@build_bazel_rules_swift//swift:repositories.bzl",
    "swift_rules_dependencies",
)

swift_rules_dependencies()

load(
    "@build_bazel_rules_swift//swift:extras.bzl",
    "swift_rules_extra_dependencies",
)

swift_rules_extra_dependencies()

load(
    "@build_bazel_apple_support//lib:repositories.bzl",
    "apple_support_dependencies",
)

apple_support_dependencies()

http_archive(
    name = "build_bazel_rules_android",
    sha256 = "cd06d15dd8bb59926e4d65f9003bfc20f9da4b2519985c27e190cddc8b7a7806",
    strip_prefix = "rules_android-0.1.1",
    urls = ["https://github.com/bazelbuild/rules_android/archive/v0.1.1.zip"],
)

# f8d7d77c06936315286eb55f8de22cd23c188571  is the commit sha for v1.14.0.
# Periodically update to the latest to "live at head"
http_archive(
    name = "com_google_googletest",
    strip_prefix = "googletest-f8d7d77c06936315286eb55f8de22cd23c188571",
    urls = ["https://github.com/google/googletest/archive/f8d7d77c06936315286eb55f8de22cd23c188571.zip"],
    sha256 = "b976cf4fd57b318afdb1bdb27fc708904b3e4bed482859eb94ba2b4bdd077fe2",
    patch_args = ["-p1"],
    patches = ["//:third_party/gtest/windows-death-args.patch"],
)

http_archive(
    name = "com_mariusbancila_stduuid",
    strip_prefix = "stduuid-1.2.3",
    build_file_content = """
cc_library(
    name = "uuid",
    hdrs = glob(["include/*.h"]),
    strip_include_prefix ="include",
    visibility = ["//visibility:public"],
    defines = [
        "UUID_SYSTEM_GENERATOR"
    ],
    include_prefix = "uuid",
    linkstatic = 1
)
""",
    sha256="0f867768ce55f2d8fa361be82f87f0ea5e51438bc47ca30cd92c9fd8b014e84e",
    urls = ["https://github.com/mariusbancila/stduuid/archive/v1.2.3.zip"],
)
