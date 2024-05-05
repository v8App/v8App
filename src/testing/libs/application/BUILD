load("@v8App//:bazel/v8_app_rules.bzl", "v8App_test")

v8App_test(
    name = "testApplication",
    srcs = [
        "ApplicationTest.cc",
    ],
    defines = [
    ],
    visibility = ["//visibility:public"],
    deps = [
        "//src/libs/application:application",
    ],
)
