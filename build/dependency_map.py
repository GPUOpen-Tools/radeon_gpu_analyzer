#!python
# RGA git project names and revisions
#

# Binaries dependencies in zip files
zip_files = {
             "dx10_asm.zip"       : "external/",
             "lc.zip"             : "external/",
             "dxc.zip"            : "external/",
             "vulkan_offline.zip" : "external/",
             "vulkan.zip"         : "external/"
            }

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
git_mapping = {
 # Lib.
    "common-lib-AMD-ACL"                  : ["Common/Lib/AMD/ACL",               "master"],
    "common-lib-amd-ADL"                  : ["Common/Lib/AMD/ADL",               "master"],
    "common-lib-amd-APPSDK-3.0"           : ["Common/Lib/AMD/APPSDK",            "master"],
    "common-lib-AMD-CAL-8.95"             : ["Common/Lib/AMD/CAL",               "master"],
    "common-lib-amd-ags-4.0.0"            : ["Common/Lib/AMD/ags",               "master"],
    "common-lib-ext-Boost-1.59"           : ["Common/Lib/Ext/Boost",             "master"],
    "common-lib-ext-tinyxml-2.6.2"        : ["Common/Lib/Ext/tinyxml",           "master"],
    "common-lib-ext-utf8cpp"              : ["Common/Lib/Ext/utf8cpp",           "master"],
    "common-lib-ext-WindowsKits"          : ["Common/Lib/Ext/Windows-Kits",      "master"],
    "common-lib-ext-zlib-1.2.8"           : ["Common/Lib/Ext/zlib",              "master"],
 # Src.
    "common-src-ACLModuleManager"         : ["Common/Src/ACLModuleManager",      "master"],
    "common-src-ADLUtil"                  : ["Common/Src/ADLUtil",               "master"],
    "common-src-AMDTBaseTools"            : ["Common/Src/AMDTBaseTools",         "master"],
    "common-src-AMDTOSWrappers"           : ["Common/Src/AMDTOSWrappers",        "6a5293d6a4f00c70747f935a4122a1b986129396"],
    "common-src-AMDTMutex"                : ["Common/Src/AMDTMutex",             "master"],
    "common-src-CElf"                     : ["Common/Src/CElf",                  "master"],
    "common-src-DeviceInfo"               : ["Common/Src/DeviceInfo",            "amd-navi23-mi100"],
    "common-src-DynamicLibraryModule"     : ["Common/Src/DynamicLibraryModule",  "master"],
    "common-src-TSingleton"               : ["Common/Src/TSingleton",            "master"],
    "common-src-VersionInfo"              : ["Common/Src/VersionInfo",           "master"],
    "common-src-Vsprops"                  : ["Common/Src/Vsprops",               "master"],
    "common-src-Miniz"                    : ["Common/Src/Miniz",                 "master"],
    "common-src-Misc"                     : ["Common/Src/Misc",                  "master"],
    "UpdateCheckAPI"                      : ["Common/Src/UpdateCheckAPI",        "amd-2.0.0"],
 # QtCommon.
    "QtCommon"                            : ["QtCommon",                         "rga-2.5"]
}

github_mapping = {
    "common-lib-ext-tinyxml2-5.0.1"  : ["Common/Lib/Ext/tinyxml2",          "master"],
    "common-lib-ext-yaml-cpp"        : ["Common/Lib/Ext/yaml-cpp",          "master"],
    "cxxopts"                        : ["Common/Lib/Ext/cxxopts",           "master"],
    "volk"                           : ["Common/Lib/Ext/volk",              "master"],
}

git_private_mapping = {
    "code_sanitizer" : ["RGA/external/code_sanitizer", "master"],
    "common-Scripts" : ["common-Scripts", "master"],
    "RGA-Internal"   : ["RGA-Internal",   None]
}
