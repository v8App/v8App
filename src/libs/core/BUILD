load("@v8App//:bazel/v8_app_rules.bzl", "v8App_library")

v8App_library(
    name = "core",
    srcs = [
        "src/Assets/AppAssetRoots.cc",
        "src/Assets/BinaryAsset.cc",
        "src/Assets/TextAsset.cc",
        "src/Logging/Log.cc",
        "src/Logging/LogJSONFIle.cc",
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
        "include/Logging/LogJSONFile.h",
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
        "-Isrc/libs/core/include",
    ],
    defines = [
    ],
    
    visibility = ["//visibility:public"],
)
