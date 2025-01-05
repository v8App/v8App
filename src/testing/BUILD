load("@v8App//:bazel/v8_app_rules.bzl", "v8App_binary")

filegroup(
    name = "testMain",
    srcs = [
        "test_main.cc",
        "test_main.h",
    ],
    visibility = ["//visibility:public"],
)

filegroup(
    name = "testMainV8",
    srcs = [
        "TestSnapshotProvider.cc",
        "TestSnapshotProvider.h",
        "TestSnapshotCreator.h",
        "V8Fixture.cc",
        "V8Fixture.h",
        "test_main.cc",
        "test_main.h",
    ],
    visibility = ["//visibility:public"],
)

v8App_binary(
    name = "helloWorld",
    srcs = [
        "hello_world.cc",
    ],
    copts = [
        "-Ithird_party/v8/include",
    ],
    data = [
        "//src/testing:build-test-env-file",
        "//third_party/v8:icu_dat",
        "//third_party/v8:snapshot_blob",
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
        "//third_party/v8",
    ],
)

v8App_binary(
    name = "cppgc-obj-sample",
    srcs = [
        "cppgc-obj-sample.cc",
    ],
    copts = [
        "-Ithird_party/v8/include",
    ],
    data = [
        "//src/testing:build-test-env-file",
        "//third_party/v8:icu_dat",
        "//third_party/v8:snapshot_blob",
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
        "//third_party/v8",
    ],
)

#Used with VSCode to export the locations to an env file that we
#can then import with launch config
genrule(
    name = "build-test-env-file",
    srcs = [
        "@//third_party/v8:icu_dat",
        "@//third_party/v8:snapshot_blob",
    ],
    outs = ["test.env"],
    cmd = 'echo "V8_ICU_DATA=$(rlocationpath //third_party/v8:icu_dat)" >> $@;' +
          'echo "V8_SNAPSHOT_BIN=$(rlocationpath //third_party/v8:snapshot_blob)" >> $@;',
    visibility = ["//visibility:public"],
)
