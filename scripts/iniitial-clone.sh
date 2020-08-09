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

echo "Cloning v8App"
git clone https://github.com/v8App/v8App.git
if [[ $? -ne 0 ]]; then
    echo "Failed to clone the v8App repository"
    exit 1
fi
