#load("@v8App//:bazel/platform_configs.bzl", "define_platform_config_settings")
load("@v8App//:bazel/v8_app_rules.bzl", "v8App_library", "v8App_test")

v8App_library(
    name = "jsRuntime",
    srcs = [
        "src/CppBridge/V8Arguments.cc",
        "src/CppBridge/V8FunctionTemplate.cc",
        "src/CppBridge/V8NativeObject.cc",
        "src/CppBridge/V8ObjectTemplateBuilder.cc",
        "src/CppBridge/V8TypeConverter.cc",
        "src/CodeCache.cc",
        #"src/EmbeddedScripts.cc",
        "src/ForegroundTaskRunner.cc",
        "src/JSApp.cc",
        "src/JSContext.cc",
        "src/JSContextModules.cc",
        "src/JSModuleInfo.cc",
        "src/JSRuntime.cc",
        "src/JSUtilities.cc",
        "src/NestableQueue.cc",
        #"src/StartupData.cc",
        #"src/V8ExternalRegistry.cc",
        "src/V8Jobs.cc",
        "src/V8Platform.cc",
        "src/WorkerTaskRunner.cc",
    ],
    hdrs = [
        "include/CppBridge/V8Arguments.h",
        "include/CppBridge/V8FunctionTemplate.h",
        "include/CppBridge/V8NativeObject.h",
        "include/CppBridge/V8NativeObjectHandle.h",
        "include/CppBridge/V8ObjectTemplateBuilder.h",
        "include/CppBridge/V8TypeConverter.h",
        "include/CodeCache.h",
        "include/ForegroundTaskRunner.h",
        "include/JSApp.h",
        "include/JSContext.h",
        "include/JSContextModules.h",
        "include/JSModuleInfo.h",
        "include/JSRuntime.h",
        "include/JSUtilities.h",
        "include/NestableQueue.h",
        #"include/V8ExternalRegistry.h",
        "include/V8Jobs.h",
        "include/V8Platform.h",
        "include/V8Types.h",
        "include/WorkerTaskRunner.h",
        "@std_uuid//file",
    ],
    copts = [
        "-Ilibs/core/include",
        "-Ilibs/jsRuntime/include",
        "-Ithird_party/v8/include",
    ],
    defines = [
    ],
    visibility = ["//visibility:public"],
    deps = [
        "//libs/core",
        "//third_party/v8",
    ],
)

v8App_test(
    name = "testJSRuntimeCore",
    size = "small",
    srcs = [
        "testsCore/JSAppTest.cc",
        "testsCore/JSContextDeathTest.cc",
        "testsCore/JSContextTest.cc",
        "testsCore/JSRuntimeTest.cc",
        "testsCore/V8JobsDeathTest.cc",
        "testsCore/V8JobsTest.cc",
        "testsCore/V8PlatformDeathTest.cc",
        "testsCore/V8PlatformInitDeathTest.cc",
        "testsCore/V8PlatformInitFixture.h",
        "testsCore/V8PlatformTest.cc",
    ],
    copts = [
        "-Ilibs/core/include",
        "-Ilibs/core/testLib",
        "-Ilibs/jsRuntime/include",
        "-Ithird_party/v8/include",
        "-Ithird_party/bazel-runfiles/include",
    ],
    data = glob(["tests/test-files/**"]) + [
        "tests/test-files",
        ":build-test-env-file",
        "//third_party/v8:icu_dat",
        "//third_party/v8:snapshot_blob",
    ],
    defines = [
    ],
    env = {
        "V8_ICU_DATA": "$(rlocationpath //third_party/v8:icu_dat)",
        "V8_SNAPSHOT_BIN": "$(rlocationpath //third_party/v8:snapshot_blob)",
    },
    linkopts = [
        "-lz",
        "-lstdc++",
    ],
    visibility = ["//visibility:public"],
    deps = [
        ":jsRuntime",
        "//libs/core",
        "//libs/core:coreTest",
        "//third_party/v8",
    ],
)

v8App_test(
    name = "testJSRuntime",
    size = "small",
    srcs = [
        #        "tests/CppBridge/V8ArgumentsTest.cc",
        #        "tests/CppBridge/V8FunctionTemplateTest.cc",
        #        "tests/CppBridge/V8NativeObjecttest.cc",
        #        "tests/CppBridge/V8ObjectTemplateTest.cc",
        #        "tests/CppBridge/V8TypeConverterTest.cc",
        "tests/CodeCacheTest.cc",
        "tests/ForegroundTaskRunnerDeathTest.cc",
        "tests/ForegroundTaskRunnerTest.cc",
        "tests/JSModuleInfotest.cc",
        "tests/JSModulesTest.cc",
        "tests/JSUtilitiesTest.cc",
        "tests/NestableQueueTest.cc",
        #        "tests/V8StartupICUDataTest.cc",
        "tests/V8TestFixture.cc",
        "tests/V8TestFixture.h",
        "tests/WorkerTaskRunnerDeathTest.cc",
        "tests/WorkerTaskRunnerTest.cc",
    ],
    copts = [
        "-Ilibs/core/include",
        "-Ilibs/core/testLib",
        "-Ilibs/jsRuntime/include",
        "-Ithird_party/v8/include",
    ],
    data = glob(["tests/test-files/**"]) + [
        "tests/test-files",
        ":build-test-env-file",
        "//third_party/v8:icu_dat",
        "//third_party/v8:snapshot_blob",
    ],
    defines = [
    ],
    env = {
        "V8_ICU_DATA": "$(rlocationpath //third_party/v8:icu_dat)",
        "V8_SNAPSHOT_BIN": "$(rlocationpath //third_party/v8:snapshot_blob)",
    },
    linkopts = [
        "-lz",
        "-lstdc++",
    ],
    visibility = ["//visibility:public"],
    deps = [
        ":jsRuntime",
        "//libs/core",
        "//libs/core:coreTest",
        "//third_party/v8",
    ],
)

genrule(
    name = "build-test-env-file",
    srcs = [
        "@//third_party/v8:icu_dat",
        "@//third_party/v8:snapshot_blob",
    ],
    outs = ["test.env"],
    cmd = 'echo "V8_ICU_DATA=$(rlocationpath //third_party/v8:icu_dat)" >> $@;' +
          'echo "V8_SNAPSHOT_BIN=$(rlocationpath //third_party/v8:snapshot_blob)" >> $@;',
)
