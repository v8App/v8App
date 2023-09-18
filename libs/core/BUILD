load("@rules_cc//cc:defs.bzl", "cc_library", "cc_test")

cc_library(
    name = "core",
    srcs = [
        "src/Assets/AppAssetRoots.cc",
        "src/Logging/Log.cc",
        "src/Threads/ThreadPool.cc",
        "src/Utils/VersionString.cc",
    ],
    hdrs = [
        "include/Assets/AppAssetRoots.h",
        "include/Logging/ILogSink.h",
        "include/Logging/Log.h",
        "include/Logging/LogMacros.h",
        "include/Threads/ThreadPool.h",
        "include/Time/Time.h",
        "include/Utils/CallbackWrapper.h",
        "include/Utils/VersionString.h",
    ],
    copts = [
        "-Ilibs/core/include",
        "-std:c++20",
    ],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "coreTest",
    srcs = [
        "testLib/TestLogSink.cc",
    ],
    hdrs=[
        "testLib/TestLogSink.h",
    ],
    copts = [
        "-Ilibs/core/include",
        "-Ilibs/core/testLib",
        "-std:c++20",
    ],
    visibility = ["//visibility:public"],
    deps = [
        ":core",
    ],
)
cc_test(
    name = "testCore",
    size="small",
    srcs = [
        "testLib/TestLogSink.cc",
        "tests/Assets/AppAssetRootsTest.cc",
        "tests/Logging/LogDeathTest.cc",
        "tests/Logging/LogTest.cc",
        "tests/Threads/ThreadPoolTest.cc",
        "tests/Utils/CallbackWrapperTest.cc",
        "tests/Utils/VersionStringTest.cc",
    ],
    copts = [
        "-Ilibs/core/include",
        "-Ilibs/core/testLib",
        "-std:c++20",
    ],
    visibility = ["//visibility:public"],
    deps = [
        ":coreTest",
        ":core",
        "@com_google_googletest//:gtest_main"
    ],
)
