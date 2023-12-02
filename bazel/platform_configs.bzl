# Copyright 2020 the v8App authors. All right reserved.
# Use of this source code is governed by the MIT license
# that can be found in the LICENSE file.

def define_platform_config_settings(name = ""):
    """ """
    native.config_setting(
        name = "debug",
        values = {
            "compilation_mode": "dbg",
        },
        visibility = ["//visibility:public"],
    )
    native.config_setting(
        name = "macos-x64",
        constraint_values = [
            "@platforms//os:macos",
            "@platforms//cpu:x86_64",
        ],
        visibility = ["//visibility:public"],
    )
    native.config_setting(
        name = "macos-x64-debug",
        constraint_values = [
            "@platforms//os:macos",
            "@platforms//cpu:x86_64",
        ],
        values = {
            "compilation_mode": "dbg",
        },
        visibility = ["//visibility:public"],
    )
    native.config_setting(
        name = "macos-arm64",
        constraint_values = [
            "@platforms//os:macos",
            "@platforms//cpu:arm64e",
        ],
        visibility = ["//visibility:public"],
    )
    native.config_setting(
        name = "macos-arm64-debug",
        constraint_values = [
            "@platforms//os:macos",
            "@platforms//cpu:arm64e",
        ],
        values = {
            "compilation_mode": "dbg",
        },
        visibility = ["//visibility:public"],
    )
    native.config_setting(
        name = "ios-x64",
        constraint_values = [
            "@platforms//os:ios",
            "@platforms//cpu:x86_64",
        ],
        visibility = ["//visibility:public"],
    )
    native.config_setting(
        name = "ios-x64-debug",
        constraint_values = [
            "@platforms//os:ios",
            "@platforms//cpu:x86_64",
        ],
        values = {
            "compilation_mode": "dbg",
        },
        visibility = ["//visibility:public"],
    )
    native.config_setting(
        name = "ios-armv7",
        constraint_values = [
            "@platforms//os:ios",
            "@platforms//cpu:armv7",
        ],
        visibility = ["//visibility:public"],
    )
    native.config_setting(
        name = "ios-armv7-debug",
        constraint_values = [
            "@platforms//os:ios",
            "@platforms//cpu:armv7",
        ],
        values = {
            "compilation_mode": "dbg",
        },
        visibility = ["//visibility:public"],
    )
    native.config_setting(
        name = "ios-arm64",
        constraint_values = [
            "@platforms//os:ios",
            "@platforms//cpu:arm64",
        ],
        visibility = ["//visibility:public"],
    )
    native.config_setting(
        name = "ios-arm64-debug",
        constraint_values = [
            "@platforms//os:ios",
            "@platforms//cpu:arm64",
        ],
        values = {
            "compilation_mode": "dbg",
        },
        visibility = ["//visibility:public"],
    )
    native.config_setting(
        name = "android-armv7",
        constraint_values = [
            "@platforms//os:android",
            "@platforms//cpu:armv7",
        ],
        visibility = ["//visibility:public"],
    )
    native.config_setting(
        name = "android-armv7-debug",
        constraint_values = [
            "@platforms//os:android",
            "@platforms//cpu:armv7",
        ],
        values = {
            "compilation_mode": "dbg",
        },
        visibility = ["//visibility:public"],
    )
    native.config_setting(
        name = "android-arm64",
        constraint_values = [
            "@platforms//os:android",
            "@platforms//cpu:arm64",
        ],
        visibility = ["//visibility:public"],
    )
    native.config_setting(
        name = "android-arm64-debug",
        constraint_values = [
            "@platforms//os:android",
            "@platforms//cpu:arm64",
        ],
        values = {
            "compilation_mode": "dbg",
        },
        visibility = ["//visibility:public"],
    )
    native.config_setting(
        name = "windows-x64",
        constraint_values = [
            "@platforms//os:windows",
            "@platforms//cpu:x86_64",
        ],
        visibility = ["//visibility:public"],
    )
    native.config_setting(
        name = "windows-x64-debug",
        constraint_values = [
            "@platforms//os:windows",
            "@platforms//cpu:x86_64",
        ],
        values = {
            "compilation_mode": "dbg",
        },
        visibility = ["//visibility:public"],
    )
    native.config_setting(
        name = "macos",
        constraint_values = [
            "@platforms//os:macos",
        ],
        visibility = ["//visibility:public"],
    )
    native.config_setting(
        name = "macos-debug",
        constraint_values = [
            "@platforms//os:macos",
        ],
        values = {
            "compilation_mode": "dbg",
        },
        visibility = ["//visibility:public"],
    )
    native.config_setting(
        name = "ios",
        constraint_values = [
            "@platforms//os:ios",
        ],
        visibility = ["//visibility:public"],
    )
    native.config_setting(
        name = "ios-debug",
        constraint_values = [
            "@platforms//os:ios",
        ],
        values = {
            "compilation_mode": "dbg",
        },
        visibility = ["//visibility:public"],
    )
    native.config_setting(
        name = "android",
        constraint_values = [
            "@platforms//os:android",
        ],
        visibility = ["//visibility:public"],
    )
    native.config_setting(
        name = "android-debug",
        constraint_values = [
            "@platforms//os:android",
        ],
        values = {
            "compilation_mode": "dbg",
        },
        visibility = ["//visibility:public"],
    )
    native.config_setting(
        name = "windows",
        constraint_values = [
            "@platforms//os:windows",
        ],
        visibility = ["//visibility:public"],
    )
    native.config_setting(
        name = "windows-debug",
        constraint_values = [
            "@platforms//os:windows",
        ],
        values = {
            "compilation_mode": "dbg",
        },
        visibility = ["//visibility:public"],
    )
