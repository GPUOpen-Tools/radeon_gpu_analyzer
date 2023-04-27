#!python
# RGA git project names and revisions
#

# key = GitHub release link
# value = location
download_mapping_windows = {
    "https://github.com/nlohmann/json/releases/download/v3.2.0/json.hpp" : "../../Common/Lib/Ext/json/json-3.2.0/single_include/nlohmann"
}
download_mapping_linux = {
    "https://github.com/nlohmann/json/releases/download/v3.2.0/json.hpp" : "../../Common/Lib/Ext/json/json-3.2.0/single_include/nlohmann"
}

# Some repos are only hosted on github - these are defined with an absolute URL based here
github_root = "https://github.com/GPUOpen-Tools/"

# repositories.
git_mapping = {}

github_mapping = {
 # Lib.
    "adl"                            : ["Common/Lib/AMD/ADL",               "master"],
    "appsdk"                         : ["Common/Lib/AMD/APPSDK",            "master"],
    "common_lib_ext_boost_1.59"      : ["Common/Lib/Ext/Boost",             "master"],
    "windows_kits"                   : ["Common/Lib/Ext/Windows-Kits",      "master"],
    "common_lib_ext_tinyxml2_5.0.1"  : ["Common/Lib/Ext/tinyxml2",          "master"],
    "cxxopts"                        : ["Common/Lib/Ext/cxxopts",           "master"],
    "volk"                           : ["Common/Lib/Ext/volk",              "master"],
 # Src.
    "adl_util"                       : ["Common/Src/ADLUtil",               "master"],
    "tsingleton"                     : ["Common/Src/TSingleton",            "master"],
    "common_src_miniz"               : ["Common/Src/Miniz",                 "master"],
    "device_info"                    : ["Common/Src/DeviceInfo",            "7f814f15a0896659af005498d9008fd627446ba0"],
    "dynamic_library_module"         : ["Common/Src/DynamicLibraryModule",  "amd-rga-v2.7"],
    "update_check_api"               : ["Common/Src/update_check_api",      "v2.1.0"],
 # QtCommon.
    "qt_common"                      : ["QtCommon",                         "rga-2.5"],
}

