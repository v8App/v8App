load("@bazel_skylib//lib:selects.bzl", "selects")

def _default_args():
    return struct(
        defines = select({
                      "//:windows": [
                          "UNICODE",
                          "_UNICODE",
                          "V8APP_WINDOWS",
                      ],
                      "//:macos": [
                          "V8APP_MACOS",
                      ],
                      "//:ios": [
                          "V8APP_IOS",
                      ],
                      "//:android": [
                          "V8APP_ANDROID",
                      ],
                      "//conditions:default": [],
                  }) +
                  select({
                      "//:windows-debug": ["V8APP_DEBUG"],
                      "//:macos-debug": ["V8APP_DEBUG"],
                      "//conditions:default": [],
                  }) +
                  selects.with_or({
                      ("@platforms//cpu:x86_64", "@platforms//cpu:arm64"): ["PLATFOEM_64"],
                      ("@platforms//cpu:x86_32", "@platforms//cpu:armv7"): ["PLATFORM_32"],
                  }) +
                  ["V8_COMPRESS_POINTERS"],
        copts = select({
            "//:windows": [
                "-std:c++20",
            ],
            "//conditions:default": ["-std=c++20"],
        }),
        linkopts = select({
            "//:windows": [
                "winmm.lib",
                "advapi32.lib",
            ],
            "//:macos": {
            },
            "//conditions:default": [
                "-pthread",
            ],
        }),
    )

def v8App_library(
        name,
        srcs,
        deps = [],
        defines = [],
        includes = [],
        copts = [],
        linkopts = [],
        **kwargs):
    default = _default_args()
    native.cc_library(
        name = name,
        srcs = srcs,
        deps = deps,
        defines = defines + default.defines,
        includes = includes,
        copts = copts + default.copts,
        linkopts = linkopts + default.linkopts,
        linkstatic = 1,
        **kwargs
    )

def v8App_binary(
        name,
        srcs,
        deps = [],
        defines = [],
        includes = [],
        copts = [],
        linkopts = [],
        **kwargs):
    default = _default_args()
    native.cc_binary(
        name = name,
        srcs = srcs,
        deps = deps,
        defines = defines + default.defines,
        includes = includes,
        copts = copts + default.copts,
        linkopts = linkopts + default.linkopts,
        linkstatic = 1,
        **kwargs
    )

def v8App_test(
        name,
        srcs,
        size = "small",
        deps = [],
        defines = [],
        includes = [],
        copts = [],
        linkopts = [],
        **kwargs):
    default = _default_args()
    native.cc_test(
        name = name,
        size = size,
        srcs = srcs,
        deps = deps + [
            "@com_google_googletest//:gtest",
            "@bazel_tools//tools/cpp/runfiles",
        ],
        defines = defines + default.defines + ["UNIT_TESTING"],
        includes = includes,
        copts = copts + default.copts,
        linkopts = linkopts + default.linkopts,
        linkstatic = 1,
        **kwargs
    )
