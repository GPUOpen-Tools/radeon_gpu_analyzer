#!python
# Copyright (c) 2021-2022 Advanced Micro Devices, Inc. All rights reserved.
#
# RGA git project names and revisions
#
import sys

# prevent generation of .pyc file
sys.dont_write_bytecode = True

####### Git Dependencies #######

# key = GitHub release link
# value = target location
url_mapping_win = {
    "https://github.com/nlohmann/json/releases/download/v3.2.0/json.hpp" : "../external/json/json-3.2.0/single_include/nlohmann"
}
url_mapping_linux = {
    "https://github.com/nlohmann/json/releases/download/v3.2.0/json.hpp" : "../external/json/json-3.2.0/single_include/nlohmann"
}

# To allow for future updates where we may have cloned the project, store the root of
# the repo in a variable. In future, we can automatically calculate this based on the git config
github_root = "https://github.com/GPUOpen-Tools/"

# repositories.
git_mapping = {}

github_mapping = {
 # Lib.
    "adl"                            : ["../external/adl",                      "master"],
    "appsdk"                         : ["../external/appsdk",                   "master"],
    "common_lib_ext_boost_1.59"      : ["../external/third_party/Boost",        "master"],
    "windows_kits"                   : ["../external/third_party/Windows-Kits", "master"],
    "common_lib_ext_tinyxml2_5.0.1"  : ["../external/third_party/tinyxml2",     "master"],
    "cxxopts"                        : ["../external/third_party/cxxopts",      "master"],
    "volk"                           : ["../external/third_party/volk",         "master"],
 # Src.
    "adl_util"                       : ["../external/adlutil",                  "master"],
    "tsingleton"                     : ["../external/tsingleton",               "master"],
    "common_src_miniz"               : ["../external/miniz",                    "master"],
    "dynamic_library_module"         : ["../external/dynamic_library_module",   "amd-rga-v2.7"],
    "device_info"                    : ["../external/device_info",              "master"],
    "update_check_api"               : ["../external/update_check_api",         "amd-2.1.1"],
  # QtCommon.
    "qt_common"                      : ["../external/QtCommon",                 "rga-2.5"],
}
