# Copyright 2020 - 2024 the v8App authors. All right reserved.
# Use of this source code is governed by the MIT license
# that can be found in the LICENSE file.
load("@bazel_skylib//lib:selects.bzl", "selects")

config_setting(
    name = "macos-x86_64",
    constraint_values = [
        "@platforms//os:macos",
        "@platforms//cpu:x86_64",
    ],
)

config_setting(
    name = "macos-arm64",
    constraint_values = [
        "@platforms//os:macos",
        "@platforms//cpu:arm64",
    ],
)

config_setting(
    name = "ios-x86_64",
    constraint_values = [
        "@platforms//os:ios",
        "@platforms//cpu:x86_64",
    ],
)

config_setting(
    name = "ios-armv7",
    constraint_values = [
        "@platforms//os:ios",
        "@platforms//cpu:armv74",
    ],
)

config_setting(
    name = "ios-arm64",
    constraint_values = [
        "@platforms//os:ios",
        "@platforms//cpu:arm64",
    ],
)

config_setting(
    name = "android-x86_64",
    constraint_values = [
        "@platforms//os:adroid",
        "@platforms//cpu:x86_64",
    ],
)


config_setting(
    name = "android-armv7",
    constraint_values = [
        "@platforms//os:adroid",
        "@platforms//cpu:armv7",
    ],
)

config_setting(
    name = "android-arm64",
    constraint_values = [
        "@platforms//os:adroid",
        "@platforms//cpu:arm64",
    ],
)

config_setting(
    name = "windows-x86_64",
    constraint_values = [
        "@platforms//os:windows",
        "@platforms//cpu:x86_64",
    ],
)

config_setting(
    name = "macos-debug",
    constraint_values = [
        "@platforms//os:macos",
    ],
    values = {
        "compilation_mode":"dbg"
    }
)

config_setting(
    name = "ios-debug",
    constraint_values = [
        "@platforms//os:ios",
    ],
    values = {
        "compilation_mode":"dbg"
    }
)

config_setting(
    name = "android-debug",
    constraint_values = [
        "@platforms//os:android",
    ],
    values = {
        "compilation_mode":"dbg"
    }
)

config_setting(
    name = "windows-debug",
    constraint_values = [
        "@platforms//os:windows",
    ],
    values = {
        "compilation_mode":"dbg"
    }
)

config_setting(
    name = "macos-x86_64-debug",
    constraint_values = [
        "@platforms//os:macos",
        "@platforms//cpu:x86_64",
    ],
    values = {
        "compilation_mode":"dbg"
    }
)

config_setting(
    name = "macos-arm64-debug",
    constraint_values = [
        "@platforms//os:macos",
        "@platforms//cpu:arm64",
    ],
    values = {
        "compilation_mode":"dbg"
    }
)

config_setting(
    name = "windows-x86_64-debug",
    constraint_values = [
        "@platforms//os:windows",
        "@platforms//cpu:x86_64",
    ],
    values = {
        "compilation_mode":"dbg"
    }
)
