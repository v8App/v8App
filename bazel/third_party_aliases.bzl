# Copyright 2020 the v8App authors. All right reserved.
# Use of this source code is governed by the MIT license
# that can be found in the LICENSE file.

def declare_third_party_aliases():
    native.alias(
        name = "gtest",
        actual = "//third_party/googlttest",
    )
