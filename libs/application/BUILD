load("@v8App//:bazel/v8_app_rules.bzl", "v8App_library", "v8App_test")
load("@v8App//:bazel/platform_configs.bzl", "define_platform_config_settings")

define_platform_config_settings()

v8App_library(
    name = "application",
    srcs = select({
        "//:macos": [
            "src/platforms/macos/MacPlatform.cc",
        ],
        "//:windows": [
            "src/platforms/windows/WinPlatform.cc",
        ],
    }) + [
        "src/Application.cc",
    ],
    hdrs = select({
        "//:macos": [
            "include/platforms/macos/MacPlatform.h",
        ],
        "//:windows": [
            "include/platforms/windows/WinPlatform.h",
        ] + [
            "include/Application.h",
            "include/BasePlatform.h",
        ],
    }),
    copts = [
        "-Ilibs/application/include",
    ],
    defines = [
    ],
    visibility = ["//visibility:public"],
)

v8App_test(
    name = "testApplication",
    srcs = [
        "tests/ApplicationTest.cc",
    ],
    defines = [
    ],
    visibility = ["//visibility:public"],
    deps = [
        ":application",
    ],
)
