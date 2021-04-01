#!python
# RGA git project names and revisions
#

# Binaries dependencies in zip files
zip_files = {
             "DX10ASM.zip"   : "Core/DX10ASM/Lib/VS2015/",
             "LC.zip"        : "Core/LC/",
             "dxcompiler.dll": "Core/DX12/DXC/",
             "VkOffline.zip" : "Core/VulkanOffline/",
             "Vulkan.zip"    : "Core/Vulkan/"
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
git_root = "ssh://gerritgit/DevTools/ec/"

# repositories.
git_mapping = {
 # Lib.
    "common-lib-AMD-ACL.git"                  : ["Common/Lib/AMD/ACL",               "master"],
    "common-lib-amd-ADL.git"                  : ["Common/Lib/AMD/ADL",               "master"],
    "common-lib-amd-APPSDK-3.0.git"           : ["Common/Lib/AMD/APPSDK",            "master"],
    "common-lib-AMD-CAL-8.95.git"             : ["Common/Lib/AMD/CAL",               "master"],
    "common-lib-amd-ags-4.0.0.git"            : ["Common/Lib/AMD/ags",               "master"],
    "common-lib-ext-Boost-1.59.git"           : ["Common/Lib/Ext/Boost",             "master"],
    "common-lib-ext-tinyxml-2.6.2.git"        : ["Common/Lib/Ext/tinyxml",           "master"],
    "common-lib-ext-utf8cpp.git"              : ["Common/Lib/Ext/utf8cpp",           "master"],
    "common-lib-ext-WindowsKits.git"          : ["Common/Lib/Ext/Windows-Kits",      "master"],
    "common-lib-ext-zlib-1.2.8.git"           : ["Common/Lib/Ext/zlib",              "master"],
 # Src.
    "common-src-ACLModuleManager.git"         : ["Common/Src/ACLModuleManager",      "master"],
    "common-src-ADLUtil.git"                  : ["Common/Src/ADLUtil",               "master"],
    "common-src-AMDTBaseTools.git"            : ["Common/Src/AMDTBaseTools",         "master"],
    "common-src-AMDTOSWrappers.git"           : ["Common/Src/AMDTOSWrappers",        "6a5293d6a4f00c70747f935a4122a1b986129396"],
    "common-src-AMDTMutex.git"                : ["Common/Src/AMDTMutex",             "master"],
    "common-src-CElf.git"                     : ["Common/Src/CElf",                  "master"],
    "common-src-DeviceInfo.git"               : ["Common/Src/DeviceInfo",            "rga-v2.4.2"],
    "common-src-DynamicLibraryModule.git"     : ["Common/Src/DynamicLibraryModule",  "master"],
    "common-src-TSingleton.git"               : ["Common/Src/TSingleton",            "master"],
    "common-src-VersionInfo.git"              : ["Common/Src/VersionInfo",           "master"],
    "common-src-Vsprops.git"                  : ["Common/Src/Vsprops",               "master"],
    "common-src-Miniz.git"                    : ["Common/Src/Miniz",                 "master"],
    "common-src-Misc.git"                     : ["Common/Src/Misc",                  "master"],
    "UpdateCheckAPI.git"                      : ["Common/Src/UpdateCheckAPI",        "amd-1.1.0"],
 # QtCommon.
    "QtCommon"                                : ["QtCommon",                         "rga-2.4"]
}

github_mapping = {
    "common-lib-ext-tinyxml2-5.0.1"  : ["Common/Lib/Ext/tinyxml2",          "master"],
    "common-lib-ext-yaml-cpp"        : ["Common/Lib/Ext/yaml-cpp",          "master"],
    "cxxopts.git"                    : ["Common/Lib/Ext/cxxopts",           "master"],
    "volk.git"                       : ["Common/Lib/Ext/volk",              "master"],
}