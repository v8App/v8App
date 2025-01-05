load("@v8App//:bazel/v8_app_rules.bzl", "v8App_library")

v8App_library(
    name = "testUtils",
    srcs = [
        "TestFiles.cc",
        "TestLogSink.cc",
    ],
    hdrs = [
        "TestFiles.h",
        "TestLogSink.h",
        "TestTime.h",
        "TestTime.hpp",
    ],
    includes=["."],
    copts = [
        "-Isrc/libs/core/include",
    ],
    visibility = ["//visibility:public"],
    deps = [
        "//src/libs/core:core",
    ],
)
