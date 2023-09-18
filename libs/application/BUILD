load("@rules_cc//cc:defs.bzl", "cc_library", "cc_test")
load("@com_github_v8app_v8app//:bazel/platform_configs.bzl", "define_platform_config_settings")

define_platform_config_settings()

cc_library(
    name = "application",
    srcs = select({
        "macos": [
            "src/platforms/macos/MacPlatform.cc",
        ],
        "windows": [
            "src/platforms/windows/WinPlatform.cc",
        ],
    }) + [
        "src/Application.cc",
    ],
    hdrs = select({
        "macos": [
            "include/platforms/macos/MacPlatform.h",
        ],
        "windows": [
            "include/platforms/windows/WinPlatform.h",
        ] + [
            "include/Application.h",
            "include/BasePlatform.h",
        ],
    }),
    copts = [
        "-Ilibs/application/include",
        "-std:c++20",
    ],
    visibility = ["//visibility:public"],
)

cc_test(
    name = "testApplication",
    size="small",
    srcs = [
        "tests/ApplicationTest.cc",
    ],
    visibility = ["//visibility:public"],
    deps = [
        ":application",
        "//src:test-main",
    ],
)
