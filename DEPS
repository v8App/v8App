# This file is used to manage the dependencies of the V8App src repo. It is
# used by gclient to determine what version of each dependency to check out, and
# where.
#
# For more information, please refer to the official documentation:
#   https://TODO.com
#
# When adding a new dependency, please update the top-level .gitignore file
# to list the dependency's destination directory.
#
# -----------------------------------------------------------------------------
# Rolling deps
# -----------------------------------------------------------------------------
# All repositories in this file are git-based, using Chromium git mirrors where
# necessary (e.g., a git mirror is used when the source project is SVN-based).
# To update the revision that Chromium pulls for a given dependency:
#
#  # Create and switch to a new branch
#  git new-branch depsroll
#  # Run roll-dep (provided by depot_tools) giving the dep's path and optionally
#  # a regex that will match the line in this file that contains the current
#  # revision. The script ALWAYS rolls the dependency to the latest revision
#  # in origin/master. The path for the dep should start with src/.
#  roll-dep src/third_party/foo_package/src foo_package.git
#  # You should now have a modified DEPS file; commit and upload as normal
#  git commit -aspv_he
#  git cl upload

gclient_gn_args_file = 'v8App/build/config/gclient_args.gni'
gclient_gn_args = [
]

vars = {
    # Fetch the prebuilt binaries for llvm-cov and llvm-profdata. Needed to
    # process the raw profiles produced by instrumented targets (built with
    # the gn arg 'use_clang_coverage').
    'checkout_clang_coverage_tools': False,

    # Fetch clang-tidy into the same bin/ directory as our clang binary.
    'checkout_clang_tidy': False,

    # luci-go CIPD package version.
    # Make sure the revision is uploaded by infra-packagers builder.
    # https://ci.chromium.org/p/infra-internal/g/infra-packagers/console
    'luci_go': 'git_revision:37a855b64d59b7f079c9a0e5368f2757099d14d3',


    'chromium_git': 'https://chromium.googlesource.com',
    'github_git': 'https://github.com',
    'android_git': 'https://android.googlesource.com',

    # GN CIPD package version.
    'gn_version': 'git_revision:5ed3c9cc67b090d5e311e4bd2aba072173e82db9',

    # Also, if you change these, update buildtools/DEPS too. Also update the
    # libc++ svn_revision in //buildtools/deps_revisions.gni.
    'clang_format_revision': '96636aa0e9f047f17447f2d45a094d0b59ed7917',
    'libcxx_revision': '78d6a7767ed57b50122a161b91f59f19c9bd0d19',
    'libcxxabi_revision': '0d529660e32d77d9111912d73f2c74fc5fa2a858',
    'libunwind_revision': '69d9b84cca8354117b9fe9705a4430d789ee599b',

    # Three lines of non-changing comments so that
    # the commit queue can handle CLs rolling google-toolbox-for-mac
    # and whatever else without interference from each other.
    'google_toolbox_for_mac_revision': 'aa1a3d2d447905999f119efbb70b3786c5eafa13',

    # Three lines of non-changing comments so that
    # the commit queue can handle CLs rolling V8
    # and whatever else without interference from each other.
    'chromium_build_revision':'26e9d485d01d6e0eb9dadd21df767a63494c8fea',
    # Three lines of non-changing comments so that
    # the commit queue can handle CLs rolling V8
    # and whatever else without interference from each other.
    'instrumented_libraries_revision': 'bb3f1802c237dd19105dd0f7919f99e536a39d10',
    # Three lines of non-changing comments so that
    # the commit queue can handle CLs rolling V8
    # and whatever else without interference from each other.
    'build_tools_revision':'7977eb176752aeec29d888cfe8e677ac12ed1c41',
    # Three lines of non-changing comments so that
    # the commit queue can handle CLs rolling V8
    # and whatever else without interference from each other.
    'libuv_revision': 'f868c9ab0c307525a16fff99fd21e32a6ebc3837',
}

allowed_hosts = [
  'chromium.googlesource.com',
  'android.googlesource.com',
  'github.com'
]

deps = {
    'v8App/third_party/libuv/src':
        Var('github_git') + '/libuv/libuv.git' + '@' +  Var('libuv_revision'),
    'v8App/build':
        Var('chromium_git') + '/chromium/src/build.git' + '@' + Var('chromium_build_revision'),
    'v8/third_party/instrumented_libraries':
        Var('chromium_git') + '/chromium/src/third_party/instrumented_libraries.git' + '@' + Var('instrumented_libraries_revision'),
   'v8App/buildtools':
        Var('chromium_git') + '/chromium/src/buildtools.git' + '@' + Var('build_tools_revision'),
    'v8App/buildtools/clang_format/script':
        Var('chromium_git') + '/chromium/llvm-project/cfe/tools/clang-format.git' + '@' + Var('clang_format_revision'),
    'v8App/buildtools/linux64': {
        'packages': [
          {
            'package': 'gn/gn/linux-amd64',
            'version': Var('gn_version'),
          }
        ],
        'dep_type': 'cipd',
        'condition': 'host_os == "linux"',
    },
    'v8App/buildtools/mac': {
        'packages': [
          {
            'package': 'gn/gn/mac-amd64',
            'version': Var('gn_version'),
          }
        ],
        'dep_type': 'cipd',
        'condition': 'host_os == "mac"',
    },
    'v8App/buildtools/third_party/libc++/trunk':
    Var('chromium_git') + '/chromium/llvm-project/libcxx.git' + '@' + Var('libcxx_revision'),
    'v8App/buildtools/third_party/libc++abi/trunk':
    Var('chromium_git') + '/chromium/llvm-project/libcxxabi.git' + '@' + Var('libcxxabi_revision'),
    'v8App/buildtools/third_party/libunwind/trunk':
    Var('chromium_git') + '/external/llvm.org/libunwind.git' + '@' + Var('libunwind_revision'),
    'v8App/buildtools/win': {
    'packages': [
      {
        'package': 'gn/gn/windows-amd64',
        'version': Var('gn_version'),
      }
    ],
    'dep_type': 'cipd',
    'condition': 'host_os == "win"',
    },
    'v8App/third_party/elfutils/src': {
      'url': Var('chromium_git') + '/external/elfutils.git' + '@' + '249673729a7e5dbd5de4f3760bdcaa3d23d154d7',
      'condition': 'checkout_android_native_support',
    },
    'v8App/third_party/google_toolbox_for_mac/src': {
      'url': Var('chromium_git') + '/external/github.com/google/google-toolbox-for-mac.git' + '@' + Var('google_toolbox_for_mac_revision'),
      'condition': 'checkout_ios or checkout_mac',
    },
    'v8App/third_party/gperf': {
      'url': Var('chromium_git') + '/chromium/deps/gperf.git' + '@' + 'd892d79f64f9449770443fb06da49b5a1e5d33c1',
      'condition': 'checkout_win',
    },
    # Parses Windows PE/COFF executable format.
    'v8App/third_party/pefile': {
      'url': Var('chromium_git') + '/external/pefile.git' + '@' + '72c6ae42396cb913bcab63c15585dc3b5c3f92f1',
      'condition': 'checkout_win',
    },
    'v8App/tools/clang':
    Var('chromium_git') + '/chromium/src/tools/clang.git' + '@' + '535dbf16a84c7fc238f7ed11b5a75381407e38f6',
    'src/tools/luci-go': {
      'packages': [
        {
          'package': 'infra/tools/luci/isolate/${{platform}}',
          'version': Var('luci_go'),
        },
        {
          'package': 'infra/tools/luci/isolated/${{platform}}',
          'version': Var('luci_go'),
        },
        {
          'package': 'infra/tools/luci/swarming/${{platform}}',
          'version': Var('luci_go'),
        },
      ],
      'dep_type': 'cipd',
    },
    'v8App/tools/clang/dsymutil': {
    'packages': [
      {
        'package': 'chromium/llvm-build-tools/dsymutil',
        'version': 'M56jPzDv1620Rnm__jTMYS62Zi8rxHVq7yw0qeBFEgkC',
      }
    ],
    'condition': 'checkout_mac',
    'dep_type': 'cipd',
    },
}
include_rules = [
]

skip_child_includes = [
    'build',
    'third_party'
]

hooks = [
#  {
    # Verify that we have the right GN binary and force-install it if we
    # don't, in order to work around crbug.com/944367.
    # TODO(crbug.com/944667) Get rid of this when cipd is ensuring we
    # have the right binary more carefully and we no longer need this.
#    'name': 'ensure_gn_version',
#    'pattern': '.',
#    'action': [
#      'python',
#      'v8App/buildtools/ensure_gn_version.py',
#      Var('gn_version')
#    ],
#  },
  {
    'name': 'sysroot_arm',
    'pattern': '.',
    'condition': 'checkout_linux and checkout_arm',
    'action': ['python', 'v8App/build/linux/sysroot_scripts/install-sysroot.py',
               '--arch=arm'],
  },
  {
    'name': 'sysroot_arm64',
    'pattern': '.',
    'condition': 'checkout_linux and checkout_arm64',
    'action': ['python', 'v8App/build/linux/sysroot_scripts/install-sysroot.py',
               '--arch=arm64'],
  },
  {
    'name': 'sysroot_x86',
    'pattern': '.',
    'condition': 'checkout_linux and (checkout_x86 or checkout_x64)',
    'action': ['python', 'v8App/build/linux/sysroot_scripts/install-sysroot.py',
               '--arch=x86'],
  },
  {
    'name': 'sysroot_x64',
    'pattern': '.',
    'condition': 'checkout_linux and checkout_x64',
    'action': ['python', 'v8App/build/linux/sysroot_scripts/install-sysroot.py',
               '--arch=x64'],
  },
  {
    # Case-insensitivity for the Win SDK. Must run before win_toolchain below.
    'name': 'ciopfs_linux',
    'pattern': '.',
    'condition': 'checkout_win and host_os == "linux"',
    'action': [ 'python',
                'v8App/third_party/depot_tools/download_from_google_storage.py',
                '--no_resume',
                '--no_auth',
                '--bucket', 'chromium-browser-clang/ciopfs',
                '-s', 'v8App/build/ciopfs.sha1',
    ]
  },
  {
    # Update the Windows toolchain if necessary.  Must run before 'clang' below.
    'name': 'win_toolchain',
    'pattern': '.',
    'condition': 'checkout_win',
    'action': ['python', 'v8App/build/vs_toolchain.py', 'update', '--force'],
  },
  {
    # Update the Mac toolchain if necessary.
    'name': 'mac_toolchain',
    'pattern': '.',
    'condition': 'checkout_mac',
    'action': ['python', 'v8App/build/mac_toolchain.py'],
  },
  {
    # Update the prebuilt clang toolchain.
    # Note: On Win, this should run after win_toolchain, as it may use it.
    'name': 'clang',
    'pattern': '.',
    'condition': 'not llvm_force_head_revision',
    'action': ['python', 'v8App/tools/clang/scripts/update.py'],
  },
  {
    # Build the clang toolchain from tip-of-tree.
    # Note: On Win, this should run after win_toolchain, as it may use it.
    'name': 'clang_tot',
    'pattern': '.',
    'condition': 'llvm_force_head_revision',
    'action': ['python', 'v8App/tools/clang/scripts/build.py',
               '--llvm-force-head-revision',
               '--with-android={checkout_android}'],
  },
  {
    # This is supposed to support the same set of platforms as 'clang' above.
    'name': 'clang_coverage',
    'pattern': '.',
    'condition': 'checkout_clang_coverage_tools',
    'action': ['python', 'v8App/tools/clang/scripts/update.py',
               '--package=coverage_tools'],
  },
  {
    # This is also supposed to support the same set of platforms as 'clang'
    # above. LLVM ToT support isn't provided at the moment.
    'name': 'clang_tidy',
    'pattern': '.',
    'condition': 'checkout_clang_tidy',
    'action': ['python', 'v8App/tools/clang/scripts/update.py',
               '--package=clang-tidy'],
  },
  {
    # Mac doesn't use lld so it's not included in the default clang bundle
    # there.  lld is however needed in win and Fuchsia cross builds, so
    # download it there. Should run after the clang hook.
    'name': 'lld/mac',
    'pattern': '.',
    'condition': 'host_os == "mac" and (checkout_win or checkout_fuchsia)',
    'action': ['python', 'v8App/tools/clang/scripts/update.py',
               '--package=lld_mac'],
  },
  # Pull clang-format binaries using checked-in hashes.
  {
    'name': 'clang_format_win',
    'pattern': '.',
    'condition': 'host_os == "win"',
    'action': [ 'python',
                'v8App/third_party/depot_tools/download_from_google_storage.py',
                '--no_resume',
                '--no_auth',
                '--bucket', 'chromium-clang-format',
                '-s', 'v8App/buildtools/win/clang-format.exe.sha1',
    ],
  },
  {
    'name': 'clang_format_mac',
    'pattern': '.',
    'condition': 'host_os == "mac"',
    'action': [ 'python',
                'v8App/third_party/depot_tools/download_from_google_storage.py',
                '--no_resume',
                '--no_auth',
                '--bucket', 'chromium-clang-format',
                '-s', 'v8App/buildtools/mac/clang-format.sha1',
    ],
  },
  {
    'name': 'clang_format_linux',
    'pattern': '.',
    'condition': 'host_os == "linux"',
    'action': [ 'python',
                'v8App/third_party/depot_tools/download_from_google_storage.py',
                '--no_resume',
                '--no_auth',
                '--bucket', 'chromium-clang-format',
                '-s', 'v8App/buildtools/linux64/clang-format.sha1',
    ],
  },
  # Pull rc binaries using checked-in hashes.
  {
    'name': 'rc_win',
    'pattern': '.',
    'condition': 'checkout_win and host_os == "win"',
    'action': [ 'python',
                'v8App/third_party/depot_tools/download_from_google_storage.py',
                '--no_resume',
                '--no_auth',
                '--bucket', 'chromium-browser-clang/rc',
                '-s', 'v8App/build/toolchain/win/rc/win/rc.exe.sha1',
    ],
  },
  {
    'name': 'rc_mac',
    'pattern': '.',
    'condition': 'checkout_win and host_os == "mac"',
    'action': [ 'python',
                'v8App/third_party/depot_tools/download_from_google_storage.py',
                '--no_resume',
                '--no_auth',
                '--bucket', 'chromium-browser-clang/rc',
                '-s', 'v8App/build/toolchain/win/rc/mac/rc.sha1',
    ],
  },
  {
    'name': 'rc_linux',
    'pattern': '.',
    'condition': 'checkout_win and host_os == "linux"',
    'action': [ 'python',
                'v8App/third_party/depot_tools/download_from_google_storage.py',
                '--no_resume',
                '--no_auth',
                '--bucket', 'chromium-browser-clang/rc',
                '-s', 'v8App/build/toolchain/win/rc/linux64/rc.sha1',
    ]
  },
  # Download and initialize "vpython" VirtualEnv environment packages.
  {
    'name': 'vpython_common',
    'pattern': '.',
    'action': [ 'vpython',
                '-vpython-spec', 'v8App/.vpython',
                '-vpython-tool', 'install',
    ],
  },
  # Download and v8 distributions.
  {
    'name': 'v8 Distributions',
    'pattern': '.',
    'action': [ 'bash',
                'scripts/install-v8-distributions',
    ],
  },
]
