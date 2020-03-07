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
    'chromium_git': 'https://chromium.googlesource.com',
    'github_git': 'https://github.com',

    # GN CIPD package version.
    'gn_version': 'git_revision:97cc440d84f050f99ff0161f9414bfa2ffa38f65',

    # Three lines of non-changing comments so that
    # the commit queue can handle CLs rolling V8
    # and whatever else without interference from each other.
    'v8_revision': 'c72055a12fbc1556cde7bdd37410c8634450a5f8',
    # Three lines of non-changing comments so that
    # the commit queue can handle CLs rolling V8
    # and whatever else without interference from each other.
    'chromium_build_revision':'9dba72ca2e45182db8787bafb81122b3fd7cc295',
    # Three lines of non-changing comments so that
    # the commit queue can handle CLs rolling V8
    # and whatever else without interference from each other.
    'libuv_revision': 'f868c9ab0c307525a16fff99fd21e32a6ebc3837',

}

allowed_hosts = [
  'chromium.googlesource.com',
]

deps = {
    'v8App/third_party/libuv':
        Var('github_git') + '/libuv/libuv.git' + '@' +  Var('libuv_revision'),
    'v8App/v8':
        Var('chromium_git') + '/v8/v8.git' + '@' +  Var('v8_revision'),
    'v8App/build':
        Var('chromium_git') + '/chromium/src/build.git' + '@' + Var('chromium_build_revision'),
    'v8App/buildtools':
    Var('chromium_git') + '/chromium/src/buildtools.git' + '@' + 'afc5b798c72905e85f9991152be878714c579958',
    'v8App/buildtools/clang_format/script':
    Var('chromium_git') + '/chromium/llvm-project/cfe/tools/clang-format.git' + '@' + '96636aa0e9f047f17447f2d45a094d0b59ed7917',
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
    Var('chromium_git') + '/chromium/llvm-project/libcxx.git' + '@' + '78d6a7767ed57b50122a161b91f59f19c9bd0d19',
    'v8App/buildtools/third_party/libc++abi/trunk':
    Var('chromium_git') + '/chromium/llvm-project/libcxxabi.git' + '@' + '0d529660e32d77d9111912d73f2c74fc5fa2a858',
    'v8App/buildtools/third_party/libunwind/trunk':
    Var('chromium_git') + '/external/llvm.org/libunwind.git' + '@' + '69d9b84cca8354117b9fe9705a4430d789ee599b',
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
  'v8App/third_party/zlib':
    Var('chromium_git') + '/chromium/src/third_party/zlib.git'+ '@' + 'b9b9a5af7cca2e683e5f2aead8418e5bf9d5a7d5',
}

hooks = [
  # Pull clang-format binaries using checked-in hashes.
  {
    'name': 'clang_format_win',
    'pattern': '.',
    'condition': 'host_os == "win"',
    'action': [ 'download_from_google_storage',
                '--no_resume',
                '--platform=win32',
                '--no_auth',
                '--bucket', 'chromium-clang-format',
                '-s', 'v8App/buildtools/win/clang-format.exe.sha1',
    ],
  },
  {
    'name': 'clang_format_mac',
    'pattern': '.',
    'condition': 'host_os == "mac"',
    'action': [ 'download_from_google_storage',
                '--no_resume',
                '--platform=darwin',
                '--no_auth',
                '--bucket', 'chromium-clang-format',
                '-s', 'v8App/buildtools/mac/clang-format.sha1',
    ],
  },
  {
    'name': 'clang_format_linux',
    'pattern': '.',
    'condition': 'host_os == "linux"',
    'action': [ 'download_from_google_storage',
                '--no_resume',
                '--platform=linux*',
                '--no_auth',
                '--bucket', 'chromium-clang-format',
                '-s', 'v8App/buildtools/linux64/clang-format.sha1',
    ],
  },
  {
    # Update the Windows toolchain if necessary.
    'name': 'win_toolchain',
    'pattern': '.',
    'condition': 'checkout_win',
    'action': ['python', 'v8/build/vs_toolchain.py', 'update'],
  },
  {
    # Update the Mac toolchain if necessary.
    'name': 'mac_toolchain',
    'pattern': '.',
    'condition': 'checkout_mac',
    'action': ['python', 'v8/build/mac_toolchain.py'],
  },
  {
    # Note: On Win, this should run after win_toolchain, as it may use it.
    'name': 'clang',
    'pattern': '.',
    # clang not supported on aix
    'condition': 'host_os != "aix"',
    'action': ['python', 'v8/tools/clang/scripts/update.py'],
  },
]
