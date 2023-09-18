load("@rules_cc//cc:defs.bzl", "cc_library")
load("@com_github_v8app_v8app//:bazel/platform_configs.bzl", "define_platform_config_settings")

define_platform_config_settings()

cc_library(
    name = "v8",
    hdrs = glob(["**/*.h"]),
    srcs = select({
        "macos-x86_64":glob(["third_party/v8/macos-x64-relase/*.lib"]),
        "windows":glob(['third_party/v8/win-x64-release/*.lib']),
    }),
    visibility = ["//visibility:public"],
)