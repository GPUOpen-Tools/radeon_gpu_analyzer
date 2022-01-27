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

git_mapping = {
}

github_mapping = {
 # Lib.
    "common-lib-amd-ADL"                  : ["Common/Lib/AMD/ADL",               "master"],
    "common-lib-amd-APPSDK-3.0"           : ["Common/Lib/AMD/APPSDK",            "master"],
    "common-lib-ext-Boost-1.59"           : ["Common/Lib/Ext/Boost",             "master"],
    "common-lib-ext-WindowsKits"          : ["Common/Lib/Ext/Windows-Kits",      "master"],
    "common-lib-ext-tinyxml2-5.0.1"       : ["Common/Lib/Ext/tinyxml2",          "master"],
    "cxxopts"                             : ["Common/Lib/Ext/cxxopts",           "master"],
    "volk"                                : ["Common/Lib/Ext/volk",              "master"],
 # Src.
    "common-src-ADLUtil"                  : ["Common/Src/ADLUtil",               "master"],
    "common-src-DynamicLibraryModule"     : ["Common/Src/DynamicLibraryModule",  "master"],
    "common-src-TSingleton"               : ["Common/Src/TSingleton",            "master"],
    "common-src-Miniz"                    : ["Common/Src/Miniz",                 "master"],
    "update_check_api"                    : ["Common/Src/update_check_api",      "v2.0.0"],
    "common-src-DeviceInfo"               : ["Common/Src/DeviceInfo",            "rga-v2.6"],
 # QtCommon.
    "QtCommon"                            : ["QtCommon",                         "rga-2.5"],
}
