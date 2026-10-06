#!python
# Copyright (c) 2024-2025 Advanced Micro Devices, Inc. All rights reserved.
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


github_mapping = {
 # Lib.
    "appsdk"                         : ["../external/appsdk",                   "55a6940ebc963daec69152314a1bb94943287d4c"],
    "common_lib_ext_boost_1.59"      : ["../external/third_party/Boost",        "d50f858fdff41c549595bb1c84d236206d117772"],
    "cxxopts"                        : ["../external/third_party/cxxopts",      "e725ea308468ab50751ba7f930842a4c061226e9"],
    "volk"                           : ["../external/third_party/volk",         "eef21b173a78b4db96a7b13df9e096068c9bacd6"],
 # Src.
    "tsingleton"                     : ["../external/tsingleton",               "d048b8fdea9d84e8939116a442ef70608189f6e2"],
    "common_src_miniz"               : ["../external/miniz",                    "a958cde31565769681aa3d7934c3d38c52940f4e"],
    "dynamic_library_module"         : ["../external/dynamic_library_module",   "276d02d20e19af19f3502021ad5c023ba1264a60"],
    "device_info"                    : ["../external/device_info",              "5fdc92d174775d9ad22326d679b03dd22f0d3488"],
    "update_check_api"               : ["../external/update_check_api",         "649bde797ee74c10b2afeaffa8b42d2a91407c06"],
 # Qt tools.
    "qt_common"                      : ["../external/qt_common",                "a7552266363c29e902ad605a3da8a7368cdb7540"],
    "qt_isa_gui"                     : ["../external/qt_isa_gui",               "2425fed03a2dedae727d7a04ceb2a964fdcc5f97"],
}
