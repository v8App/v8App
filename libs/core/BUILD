load("@v8App//:bazel/v8_app_rules.bzl", "v8App_library", "v8App_test")
load("@v8App//:bazel/platform_configs.bzl", "define_platform_config_settings")

v8App_library(
    name = "core",
    srcs = [
        "src/Assets/AppAssetRoots.cc",
        "src/Assets/BinaryAsset.cc",
        "src/Assets/TextAsset.cc",
        "src/Logging/Log.cc",
        "src/Threads/ThreadPoolDelayedQueue.cc",
        "src/Threads/ThreadPoolQueue.cc",
        "src/Threads/Threads.cc",
        "src/Utils/Paths.cc",
        "src/Utils/VersionString.cc",
    ],
    hdrs = [
        "include/Assets/AppAssetRoots.h",
        "include/Assets/BaseAsset.h",
        "include/Assets/BinaryAsset.h",
        "include/Assets/TextAsset.h",
        "include/Logging/ILogSink.h",
        "include/Logging/Log.h",
        "include/Logging/LogMacros.h",
        "include/Queues/TThreadSafeDelayedQueue.h",
        "include/Queues/TThreadSafeDelayedQueue.hpp",
        "include/Queues/TThreadSafeQueue.h",
        "include/Queues/TThreadSafeQueue.hpp",
        "include/Threads/ThreadPoolDelayedQueue.h",
        "include/Threads/ThreadPoolQueue.h",
        "include/Threads/ThreadPoolTasks.h",
        "include/Threads/Threads.h",
        "include/Time/Time.h",
        "include/Utils/CallbackWrapper.h",
        "include/Utils/Environment.h",
        "include/Utils/Format.h",
        "include/Utils/Paths.h",
        "include/Utils/VersionString.h",
    ],
    copts = [
        "-Ilibs/core/include",
    ],
    defines = [
    ],
    visibility = ["//visibility:public"],
)

v8App_library(
    name = "coreTest",
    srcs = [
        "testLib/TestLogSink.cc",
    ],
    hdrs = [
        "testLib/TestLogSink.h",
        "testLib/TestTime.h",
        "testLib/TestTime.hpp",
    ],
    copts = [
        "-Ilibs/core/include",
        "-Ilibs/core/testLib",
    ],
    visibility = ["//visibility:public"],
    deps = [
        ":core",
    ],
)

v8App_test(
    name = "testCore",
    size = "small",
    srcs = [
        "testLib/TestLogSink.cc",
        "tests/Assets/AppAssetRootsTest.cc",
        "tests/Assets/BaseAssettest.cc",
        "tests/Assets/BinaryAssettest.cc",
        "tests/Assets/TextAssetTest.cc",
        "tests/Logging/LogDeathTest.cc",
        "tests/Logging/LogTest.cc",
        "tests/Queues/TThreadSafeDelayedQueueDeathTest.cc",
        "tests/Queues/TThreadSafeDelayedQueueTest.cc",
        "tests/Queues/TThreadSafeQueueTest.cc",
        "tests/Threads/ThreadPoolQueueTest.cc",
        "tests/Threads/ThreadPoolDelayedQueueTest.cc",
        "tests/Threads/ThreadsTest.cc",
        "tests/Utils/CallbackWrapperTest.cc",
        "tests/Utils/EnvironmentTest.cc",
        "tests/Utils/PathsTest.cc",
        "tests/Utils/VersionStringTest.cc",
    ],
    copts = [
        "-Ilibs/core/include",
        "-Ilibs/core/testLib",
    ],
    defines = [
    ],
    env ={
        "VarExists":"test"
    },
    visibility = ["//visibility:public"],
    deps = [
        ":core",
        ":coreTest",
    ],
)
