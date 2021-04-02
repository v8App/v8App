#!/usr/bin/env bash

bazel test -c dbg --spawn_strategy=local ${1} 