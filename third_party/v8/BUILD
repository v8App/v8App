load("@rules_cc//cc:defs.bzl", "cc_library")
load("@v8App//:bazel/platform_configs.bzl", "define_platform_config_settings")

define_platform_config_settings()

alias(
    name = "icu_dat",
    actual=select({
        ":windows-x64": "@v8App//third_party/v8:win-x64-release/icudtl.dat",
        ":windows-x64-debug": "@v8App//third_party/v8:win-x64-debug/icudtl.dat",
        ":macos-arm64": "@v8App//third_party/v8:macos-arm64-release/icudtl.dat",
        ":macos-arm64-debug": "@v8App//third_party/v8:macos-arm64-debug/icudtl.dat",
        ":macos-x64": "@v8App//third_party/v8:macos-x64-release/icudtl.dat",
        ":macos-x64-debug": "@v8App//third_party/v8:macos-x64-debug/icudtl.dat",
    }),
    visibility = ["//visibility:public"],
)

alias(
    name = "snapshot_blob",
    actual=select({
        ":windows-x64": "@v8App//third_party/v8:win-x64-releasesnapshot_blob.bin",
        ":windows-x64-debug": "@v8App//third_party/v8:win-x64-debug/snapshot_blob.bin",
        ":macos-arm64": "@v8App//third_party/v8:macos-arm64-release/snapshot_blob.bin",
        ":macos-arm64-debug": "@v8App//third_party/v8:macos-arm64-debug/snapshot_blob.bin",
        ":macos-x64": "@v8App//third_party/v8:macos-x64-release/snapshot_blob.bin",
        ":macos-x64-debug": "@v8App//third_party/v8:macos-x64-debug/snapshot_blob.bin",
    }),
    visibility = ["//visibility:public"],
)

cc_library(
    name = "v8",
    hdrs = glob(["**/*.h"]),
    srcs = select({
        ":windows-x64":glob(['win-x64-release/*.a']),
        ":windows-x64-debug":glob(['win-x64-debug/*.a']),
        ":macos-arm64":glob(['macos-arm64-release/*.a']),
        ":macos-arm64-debug":glob(['macos-arm64-debug/*.a']),
        ":macos-x64":glob(['macos-x64-release/*.a']),
        ":macos-x64-debug":glob(['macos-x64-debug/*.a']),
    }),
    visibility = ["//visibility:public"],
    data = [":snapshot_blob", ":icu_dat"],
)