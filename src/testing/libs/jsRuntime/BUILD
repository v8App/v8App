load("@v8App//:bazel/v8_app_rules.bzl", "v8App_test")

v8App_test(
    name = "testJSRuntimePlatform",
    size = "small",
    srcs = [
        "platform/V8AppPlatformDeathTest.cc",
        "platform/V8AppPlatformInitDeathTest.cc",
        "platform/V8AppPlatformTest.cc",
    ],
    copts = [
        "-Isrc/libs/core/include",
        "-Isrc/libs/jsRuntime/include",
        "-Isrc/testing/libs/testUtils",
        "-Ithird_party/v8/include",
        "-Ithird_party/bazel-runfiles/include",
    ],
    linkopts = [
        "-lz",
        "-lstdc++",
    ],
    visibility = ["//visibility:public"],
    deps = [
        "//src/libs/core",
        "//src/libs/jsRuntime",
        "//src/testing/libs/testUtils",
        "//third_party/v8",
        "@com_google_googletest//:gtest_main",
    ],
)

v8App_test(
    name = "testJSRuntimeCore",
    size = "small",
    srcs = [
        "coreRuntime/JSAppTest.cc",
        "coreRuntime/JSContextDeathTest.cc",
        "coreRuntime/JSContextTest.cc",
        "coreRuntime/JSRuntimeDeathTest.cc",
        "coreRuntime/JSRuntimeTest.cc",
        "coreRuntime/V8InitApp.h",
        "coreRuntime/V8JobsDeathTest.cc",
        "coreRuntime/V8JobsTest.cc",
        "//src/testing:testMainV8",
    ],
    copts = [
        "-Isrc/libs/core/include",
        "-Isrc/libs/jsRuntime/include",
        "-Isrc/testing/libs/testUtils",
        "-Ithird_party/v8/include",
        "-Ithird_party/bazel-runfiles/include",
        "-Isrc/testing",
    ],
    data = [
        "//src/testing:build-test-env-file",
        "//third_party/v8:icu_dat",
        "//third_party/v8:snapshot_blob",
    ],
    defines = [
        "USE_JSRUNTIME=1",
    ],
    env = {
        "V8_ICU_DATA": "$(rlocationpath //third_party/v8:icu_dat)",
        "V8_SNAPSHOT_BIN": "$(rlocationpath //third_party/v8:snapshot_blob)",
    },
    visibility = ["//visibility:public"],
    deps = [
        "//src/libs/core",
        "//src/libs/jsRuntime",
        "//src/testing/libs/testUtils",
        "//third_party/v8",
    ],
)

v8App_test(
    name = "testJSRuntime",
    size = "small",
    srcs = [
        "runtime/CppBridge/CallbackRegistryTest.cc",
        "runtime/CppBridge/V8ArgumentsTest.cc",
        "runtime/CppBridge/V8FunctionTemplateTest.cc",
        "runtime/CppBridge/V8CppObjectTest.cc",
        "runtime/CppBridge/V8ObjectTemplateBuilderTest.cc",
        "runtime/CppBridge/V8TypeConverterTest.cc",
        "runtime/CodeCacheTest.cc",
        "runtime/ForegroundTaskRunnerDeathTest.cc",
        "runtime/ForegroundTaskRunnerTest.cc",
        "runtime/JSContextModulesDeathTest.cc",
        "runtime/JSContextModulesTest.cc",
        "runtime/JSModuleInfotest.cc",
#        "runtime/JSSnapshotCreatorTest.cc",
        "runtime/JSUtilitiesTest.cc",
        "runtime/NestableQueueTest.cc",
        "runtime/V8SnapshotProviderTest.cc",
        "runtime/WorkerTaskRunnerDeathTest.cc",
        "runtime/WorkerTaskRunnerTest.cc",
        "//src/testing:testMainV8",
    ],
    copts = [
        "-Isrc/libs/core/include",
        "-Isrc/libs/jsRuntime/include",
        "-Isrc/testing/libs/testUtils",
        "-Isrc/testing",
        "-Ithird_party/v8/include",
    ],
    data = [
        "runtime/test-files",
        "//src/testing:build-test-env-file",
        "//third_party/v8:icu_dat",
        "//third_party/v8:snapshot_blob",
    ],
    defines = [
        "USE_JSRUNTIME=1",
    ],
    env = {
        "TestFiles": "v8App/src/testing/libs/jsRuntime/runtime/test-files",
        "V8_ICU_DATA": "$(rlocationpath //third_party/v8:icu_dat)",
        "V8_SNAPSHOT_BIN": "$(rlocationpath //third_party/v8:snapshot_blob)",
    },

    visibility = ["//visibility:public"],
    deps = [
        "//src/libs/core",
        "//src/libs/jsRuntime",
        "//src/testing/libs/testUtils",
        "//third_party/v8",
    ],
)
