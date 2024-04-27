load("@v8App//:bazel/v8_app_rules.bzl", "v8App_library")

v8App_library(
    name = "application",
    srcs = select({
        "@platforms//os:macos": [
            "src/platforms/macos/MacPlatform.cc",
        ],
        "@platforms//os:windows": [
            "src/platforms/windows/WinPlatform.cc",
        ],
    }) + [
        "src/Application.cc",
    ],
    hdrs = select({
        "@platforms//os:macos": [
            "include/platforms/macos/MacPlatform.h",
        ],
        "@platforms//os:windows": [
            "include/platforms/windows/WinPlatform.h",
        ],
        }) + [
            "include/Application.h",
            "include/BasePlatform.h",
        ],
    copts = [
        "-Isrc/libs/application/include",
    ],
    defines = [
    ],
    visibility = ["//visibility:public"],
)
