#!python
# Copyright (c) 2024 Advanced Micro Devices, Inc. All rights reserved.
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
    "https://github.com/nlohmann/json/releases/download/v3.11.3/json.hpp" : "../external/json/json-3.11.3/single_include/nlohmann",
    "https://github.com/nlohmann/json/releases/download/v3.11.3/json_fwd.hpp" : "../external/json/json-3.11.3/single_include/nlohmann",
    "https://github.com/gabime/spdlog/archive/refs/tags/v1.14.1.zip" : "../external/third_party/spdlog"
}
url_mapping_linux = {
    "https://github.com/nlohmann/json/releases/download/v3.11.3/json.hpp" : "../external/json/json-3.11.3/single_include/nlohmann",
    "https://github.com/nlohmann/json/releases/download/v3.11.3/json_fwd.hpp" : "../external/json/json-3.11.3/single_include/nlohmann",
    "https://github.com/gabime/spdlog/archive/refs/tags/v1.14.1.tar.gz": "../external/third_party/spdlog"
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
    "device_info"                    : ["../external/device_info",              "c374c2b328bea66a92db6e49f6b32607f97b3e2c"],
    "update_check_api"               : ["../external/update_check_api",         "v2.1.1"],
 # QtCommon.
    "qt_common"                      : ["../external/QtCommon",                 "6377814493f6e5c97fd5c44fccdd22767bf86355"],
}
