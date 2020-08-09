#!/usr/bin/env bash

# Copyright 2020 The v8App Authors. All rights reserved.
# Use of this source code is governed by a MIT license that can be
# found in the LICENSE file.

TOP="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

#check for git install
git --version >> /dev/null
if [[ $? -ne 0 ]]; then
    echo "Doesn't seem as if git is installed, as getting the version failed"
    exit 1
fi

mkdir v8App
if [[ $? -ne 0 ]]; then
    echo "Failed to create teh v8App root directory"
    exit 1
fi

TOP=${TOP}/v8App

cd ${TOP}
if [[ $? -ne 0 ]]; then
    echo "Failed to change to the v8App root directory"
    exit 1
fi

#are we on Linux or Mac OS X
OSX=`uname -a | grep Darwin | wc -l`

echo "Cloning depot_to0ls"
git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git
if [[ $? -ne 0 ]]; then
    echo "Failed to clone the depot_tools"
    exit 1
fi

export PATH=${PATH}:${TOP}/depot_tools

pushd ${TOP}/depot_tools
if [[ $? -ne 0 ]]; then
    echo "Failed to cd to the depot_tools"
    exit 1
fi

#now update depot_tools
echo "Updating depot_tools"
gclient --version
if [[ $? -ne 0 ]]; then
    echo "Failed to update depot_tools"
    exit 1
fi

popd

echo "Pulling v8App's dependencies"
gclient config --name "v8App" --unmanaged https://github.com/v8App/v8App
if [[ $? -ne 0 ]]; then
    echo "Failed to generate the gclient config"
    exit 1
fi

gclient sync --with_branch_heads --with_tags
if [[ $? -ne 0 ]]; then
    echo "Failed to sync the v8App repository"
    exit 1
fi

echo "/n/nYou will want to add this this path '${TOP}/depot_tools' to your PATH variable through an export or in your shell config."