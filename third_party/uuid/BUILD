load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "uuid",
    hdrs = ["@std_uuid//file"],
    strip_include_prefix ="/external/std_uuid/file/third_party/uuid",
    visibility = ["//visibility:public"],

    include_prefix = "uuid",
    linkstatic = True
)