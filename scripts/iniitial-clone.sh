#!/usr/bin/env bash

TOP="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

#are we on Linux or Mac OS X
OSX=`uname -a | grep Darwin | wc -l`

#check for git install
git --version >> /dev/null
if [[ $? -ne 0 ]]; then
    echo "Doesn't seem as if git is installed, as getting the version failed"
    exit 1
fi

echo "This script will install google's depot tools and then clone the v8App in the current directory '${TOP}'."
echo "Do you wish to continue?(y/n)"
read l_answer
l_answer=`echo ${l_answer} | awk '{print tolower($0)}'`
if [[ "$l_answer" == "no" ]] || [[ "$l_answer" == "n" ]]; then
    exit 0
fi

git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git
if [[ $? -ne 0 ]]; then
    echo "Failed to clone the depot_tools"
    exit 1
fi

cd ${TOP}/depot_tools
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

cd ${TOP}
if [[ $? -ne 0 ]]; then
    echo "Failed to change back to ${TOP}"
    exit 1
fi

export PATH=${PATH}:${TOP}/depot_tools

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
