load("@v8App//:bazel/v8_app_rules.bzl", "v8App_test")

v8App_test(
    name = "testCore",
    size = "small",
    srcs = [
        "Assets/AppAssetRootsTest.cc",
        "Assets/BaseAssettest.cc",
        "Assets/BinaryAssettest.cc",
        "Assets/TextAssetTest.cc",
        "Logging/LogDeathTest.cc",
        "Logging/LogTest.cc",
        "Queues/TThreadSafeDelayedQueueDeathTest.cc",
        "Queues/TThreadSafeDelayedQueueTest.cc",
        "Queues/TThreadSafeQueueTest.cc",
        "Threads/ThreadPoolQueueTest.cc",
        "Threads/ThreadPoolDelayedQueueTest.cc",
        "Threads/ThreadsTest.cc",
        "Utils/CallbackWrapperTest.cc",
        "Utils/EnvironmentTest.cc",
        "Utils/PathsTest.cc",
        "Utils/VersionStringTest.cc",
        "//src/testing:testMain",
    ],
    copts = [
        "-Isrc/libs/core/include",        
        "-Isrc/testing/libs/testUtils",
        "-Isrc/testing/",
    ],
    env ={
        "VarExists":"test",
    },
    visibility = ["//visibility:public"],
    deps = [
        "//src/libs/core:core",
        "//src/testing/libs/testUtils:testUtils",
    ],
)
