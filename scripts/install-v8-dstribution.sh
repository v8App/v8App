#!/usr/bin/env bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
source ${DIR}/functions

TOP=
find_top TOP v8App
if [[ $? -ne 0 ]]; then
    exit 1
fi

cd ${TOP}
if [[ $? -ne 0 ]]; then
    echio "Failed to change to the top directory : ${TOP}"
    exit 1
fi

V8_VERSION=`cat ${TOP}/v8Version`
V8_DIST_PREFIX=v8-${V8_VERSION}_

MAC_OS=0
ANDROID=0
IOS=0
WINDOWS=0
LINUX=0

for dist in ${@}
do
    case ${dist} in
        macos)
            MAC_OS=1
            ;;
        android)
            ANDROID=1
            ;;
        ios)
            IOS=1
            ;;
        linux)
            LINUX=1
            ;;
        windows)
            WINDOWS=1
            ;;
        all)
            MAC_OS=1
            ANDROID=1
            IOS=1
            LINUX=1
            WINDOWS=1
    esac
done

if [[ ${MAC_OS} -eq 0 ]] && [[ ${ANDROID} -eq 0 ]] && [[ ${IOS} -eq 0 ]] && [[ ${LINUX} -eq 0 ]] && [[ ${WINDOWS} -eq 0 ]]; then
    echo "Failed to find a distribution to download"
    exit 1
fi

if [[ ! -e ${TOP}/v8 ]]; then
    mkdir ${TOP}/v8
    if [[ $? -ne 0 ]]; then
        echo "Failed to create the v8 distribution directory"
        exit 1
    fi
fi

pushd ${TOP}/third_party/v8

#pull the includes first
#to do add curl commend to pull the distribution
if [[ ! -e include ]]; then
unzip ${V8_DIST_PREFIX}include.zip
    if [[ $? -ne 0 ]]; then
        echo "Failed to unzip the include distribution"
        exit 1
    fi

    rm -f ${V8_DIST_PREFIX}include.zip
    if [[ $? -ne 0 ]]; then
        echo "Failed to remove the include distribution"
        exit 1
    fi
fi

if [[ ${MAC_OS} -eq 1 ]]; then
    DISTRIBUTIONS=(macos-x64-debug)
    for dist in ${DISTRIBUTIONS[@]}
    do
        #to do add a curl to pull the file from the cloud
        #for now we just drop them into the folder
        full_dist=${V8_DIST_PREFIX}${dist}
        unzip ${full_dist}.zip
        if [[ $? -ne 0 ]]; then
            echo "Failed to unzip the ${full_dist}.zip distribution"
            exit 1
        fi
        rm -f ${full_dist}.zip
        if [[ $? -ne 0 ]]; then
            echo "Failed to remove the ${full_dist}.zip distribution"
            exit 1
        fi
        mv ${full_dist} ${dist}
        if [[ $? -ne 0 ]]; then
            echo "Failed to rename ${full_dist} to ${dist}"
            exit 1
        fi
    done
fi

#TODO: add other platforms