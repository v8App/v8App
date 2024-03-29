# Copyright 2020 the v8App authors. All right reserved.
# Use of this source code is governed by the MIT license
# that can be found in the LICENSE file.
load("@rules_cc//cc:defs.bzl", "cc_library")
load("@v8App//:bazel/platform_configs.bzl", "define_platform_config_settings")

define_platform_config_settings()

COMMON_LIBUV_HEADERS = [
    "include/uv.h",
    "include/uv/errno.h",
    "include/uv/threadpool.h",
    "include/uv/tree.h",
    "include/uv/version.h",
]

UNIX_LIBUV_HEADERS = [
    "include/uv/unix.h",
]

LINUX_LIBUV_HEADERS = [
    "include/uv/linux.h",
]

ANDROID_LIBUV_HEADERS = [
    "include/uv/android-ifaddrs.h",
]

DARWIN_LIBUV_HEADERS = [
    "include/uv/darwin.h",
]

WINDOWS_LIBUV_HEADERS = [
    "include/uv/win.h",
]

COMMON_LIBUV_SOURCES = [
    "src/fs-poll.c",
    "src/heap-inl.h",
    "src/idna.c",
    "src/idna.h",
    "src/inet.c",
    "src/queue.h",
    "src/random.c",
    "src/strscpy.c",
    "src/strscpy.h",
    "src/threadpool.c",
    "src/timer.c",
    "src/uv-common.c",
    "src/uv-common.h",
    "src/uv-data-getter-setters.c",
    "src/version.c",
]

UNIX_LIBUV_SOURCES = [
    "src/unix/async.c",
    "src/unix/atomic-ops.h",
    "src/unix/core.c",
    "src/unix/dl.c",
    "src/unix/fs.c",
    "src/unix/getaddrinfo.c",
    "src/unix/getnameinfo.c",
    "src/unix/internal.h",
    "src/unix/loop-watcher.c",
    "src/unix/loop.c",
    "src/unix/pipe.c",
    "src/unix/poll.c",
    "src/unix/process.c",
    "src/unix/random-devurandom.c",
    "src/unix/signal.c",
    "src/unix/spinlock.h",
    "src/unix/stream.c",
    "src/unix/tcp.c",
    "src/unix/thread.c",
    "src/unix/tty.c",
    "src/unix/udp.c",
]

LINUX_LIBUV_SOURCES = [
    "src/unix/linux-core.c",
    "src/unix/linux-inotify.c",
    "src/unix/linux-syscalls.c",
    "src/unix/linux-syscalls.h",
    "src/unix/procfs-exepath.c",
    "src/unix/proctitle.c",
    "src/unix/random-getrandom.c",
    "src/unix/random-sysctl-linux.c",
]

ANDROID_LIBUV_SOURCES = [
    "src/unix/android-ifaddrs.c",
    "src/unix/linux-core.c",
    "src/unix/linux-inotify.c.c",
    "src/unix/procfs-exepath.c",
    "src/unix/proctitle.c",
    "src/unix/pthread-fixes.c",
    "src/unix/random-getentropy.c",
    "src/unix/random-getrandom.c",
    "src/unix/random-sysctl-linux.c",
]

DARWIN_LIBUV_SOURCES = [
    "src/unix/bsd-ifaddrs.c",
    "src/unix/darwin-proctitle.c",
    "src/unix/darwin.c",
    "src/unix/darwin-stub.h",
    "src/unix/fsevents.c",
    "src/unix/kqueue.c",
    "src/unix/proctitle.c",
    "src/unix/random-getentropy.c",
]

WINDOWS_LIBUV_SOURCES = [
    "src/win/async.c",
    "src/win/atomicops-inl.h",
    "src/win/core.c",
    "src/win/detect-wakeup.c",
    "src/win/dl.c",
    "src/win/error.c",
    "src/win/fs-event.c",
    "src/win/fs.c",
    "src/win/fs-fd-hash-inl.h",
    "src/win/getaddrinfo.c",
    "src/win/getnameinfo.c",
    "src/win/handle-inl.h",
    "src/win/handle.c",
    "src/win/internal.h",
    "src/win/loop-watcher.c",
    "src/win/pipe.c",
    "src/win/poll.c",
    "src/win/process-stdio.c",
    "src/win/process.c",
    "src/win/req-inl.h",
    "src/win/signal.c",
    "src/win/snprintf.c",
    "src/win/stream-inl.h",
    "src/win/stream.c",
    "src/win/tcp.c",
    "src/win/thread.c",
    "src/win/tty.c",
    "src/win/udp.c",
    "src/win/util.c",
    "src/win/winapi.c",
    "src/win/winapi.h",
    "src/win/winsock.c",
    "src/win/winsock.h",
]

cc_library(
    name = "libuv",
    srcs = select({
        "android": COMMON_LIBUV_SOURCES + UNIX_LIBUV_SOURCES + ANDROID_LIBUV_SOURCES,
        "macos": COMMON_LIBUV_SOURCES + UNIX_LIBUV_SOURCES + DARWIN_LIBUV_SOURCES,
        "ios": COMMON_LIBUV_SOURCES + UNIX_LIBUV_SOURCES + DARWIN_LIBUV_SOURCES,
        "windows": COMMON_LIBUV_SOURCES + WINDOWS_LIBUV_SOURCES,
        "//conditions:default": COMMON_LIBUV_SOURCES + UNIX_LIBUV_SOURCES + LINUX_LIBUV_SOURCES,
    }),
    hdrs = select({
        "android": COMMON_LIBUV_HEADERS + UNIX_LIBUV_HEADERS + ANDROID_LIBUV_HEADERS,
        "macos": COMMON_LIBUV_HEADERS + UNIX_LIBUV_HEADERS + DARWIN_LIBUV_HEADERS,
        "ios": COMMON_LIBUV_HEADERS + UNIX_LIBUV_HEADERS + DARWIN_LIBUV_HEADERS,
        "windows": COMMON_LIBUV_HEADERS + WINDOWS_LIBUV_HEADERS,
        "//conditions:default": COMMON_LIBUV_HEADERS + UNIX_LIBUV_HEADERS + LINUX_LIBUV_HEADERS,
    }),
    copts = [
        "-D_LARGEFILE_SOURCE",
        "-D_FILE_OFFSET_BITS=64",
        "-pthread",
        "--std=gnu89",
        "-pedantic",
        "-Wno-error",
        "-Wno-strict-aliasing",
        "-Wstrict-aliasing",
        "-O2",
        "-Wno-implicit-function-declaration",
        "-Wno-unused-function",
        "-Wno-unused-variable",
    ] + select({
        "android": [
            "-D_GNU_SOURCE",
        ],
        "macos": [
            "-D_DARWIN_USE_64_BIT_INODE=1",
            "-D_DARWIN_UNLIMITED_SELECT=1",
        ],
        "ios": [
            "-D_DARWIN_USE_64_BIT_INODE=1",
            "-D_DARWIN_UNLIMITED_SELECT=1",
        ],
        "windows": [
            "-DWIN32_LEAN_AND_MEAN",
            "-D_WIN32_WINNT=0x0600",
        ],
        "//conditions:default": [
            "-Wno-omit-frame-pointer",
        ],
    }),
    includes = [
        "include",
        "src",
    ],
    linkopts = select({
        "windows": [
            "-Xcrosstool-compilation-mode=$(COMPILATION_MODE)",
            "-Wl,Iphlpapi.lib",
            "-Wl,Psapi.lib",
            "-Wl,User32.lib",
            "-Wl,Userenv.lib",
        ],
        "//conditions:default": [],
    }),
    visibility = [
        "//visibility:public",
    ],
)
