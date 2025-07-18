#######################################################################################################################
### Copyright (c) 2017-2025 Advanced Micro Devices, Inc. All rights reserved.
### @author AMD Developer Tools Team
#######################################################################################################################
cmake_minimum_required (VERSION 3.10)
project (radeon_gpu_analyzer_backend)

# The version number
set (radeon_gpu_analyzer_backend_VERSION_MAJOR 1)
set (radeon_gpu_analyzer_backend_VERSION_MINOR 2)

# Before fetching the isa_decoder, set this variable so we only pull in 
# the isa_decoder, and skip the command line interface, examples, and tests.
set (EXCLUDE_ISA_CLI_EXAMPLES_TESTS ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Add cmake utilities
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(cli_external)

# Project-specific preprocessor directives
add_definitions(-DRGA_BACKEND_EXPORTS)

# Windows-specific preprocessor directives
if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

# Add these definitions if static linkage is used with the other projects
add_definitions(-DAMDTBASETOOLS_STATIC -DAMDTOSWRAPPERS_STATIC)

# Identify if this script runs on a Debian system
set (IsDebian FALSE)
if(EXISTS "/etc/debian_version")
    set(IsDebian TRUE)
endif()

# Configure a header file to pass some of the CMake settings
# to the source code
configure_file (
  "${PROJECT_SOURCE_DIR}/RadeonGPUAnalyzerBackendConfig.h.in"
  "${PROJECT_BINARY_DIR}/radeon_gpu_analyzer_backendConfig.h"
  )

# Include directories
include_directories("${PROJECT_SOURCE_DIR}/../")
include_directories("${PROJECT_SOURCE_DIR}/../../")
include_directories("${PROJECT_SOURCE_DIR}/../../external")
include_directories("${PROJECT_SOURCE_DIR}/../../external/dx11")
include_directories("${PROJECT_SOURCE_DIR}/emulator/parser")
include_directories("${PROJECT_SOURCE_DIR}")
include_directories("${PROJECT_SOURCE_DIR}/../../external/celf/Include")
include_directories("${PROJECT_SOURCE_DIR}/../../external/device_info")
include_directories("${PROJECT_SOURCE_DIR}/../../external/dynamic_library_module")
include_directories("${PROJECT_SOURCE_DIR}/../../external/tsingleton")
include_directories("${PROJECT_SOURCE_DIR}/../../external/DX10ASM")
include_directories("${PROJECT_SOURCE_DIR}/../../external/APPSDK/3-0/include")
include_directories("${PROJECT_SOURCE_DIR}/../../external/yaml-cpp/include")
include_directories("${PROJECT_SOURCE_DIR}/../common")
include_directories("${PROJECT_SOURCE_DIR}/../../external/third_party/spdlog/spdlog-1.14.1/include")
include_directories("${PROJECT_SOURCE_DIR}/../../external/third_party")
include_directories("${PROJECT_SOURCE_DIR}/../../external/dx11/GSA")
if (NOT IsDebian)
include_directories("${PROJECT_SOURCE_DIR}/../../external/third_party/Boost/boost_1_59_0")
endif()

# Windows-specific include files
if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    include_directories("${PROJECT_SOURCE_DIR}/../../external/dx10_asm/include")
    include_directories("${PROJECT_SOURCE_DIR}/../utils/dx12/backend")
    include_directories("${PROJECT_SOURCE_DIR}/../../external/dxc/inc")
endif()

# Source files that are common to Windows and Linux
file(GLOB COMMON_SRC
    "*.cpp"
    "*.h"
    "emulator/parser/be_instruction.cpp"
    "emulator/parser/be_isa_parser.cpp"
    "emulator/parser/be_isa_program_graph.cpp"
    "emulator/parser/be_parser_si.cpp"
    "emulator/parser/be_parser_si_ds.cpp"
    "emulator/parser/be_parser_si_exp.cpp"
    "emulator/parser/be_parser_si_mimg.cpp"
    "emulator/parser/be_parser_si_mtbuf.cpp"
    "emulator/parser/be_parser_si_mubuf.cpp"
    "emulator/parser/be_parser_si_smrd.cpp"
    "emulator/parser/be_parser_si_sop1.cpp"
    "emulator/parser/be_parser_si_sop2.cpp"
    "emulator/parser/be_parser_si_sopc.cpp"
    "emulator/parser/be_parser_si_sopk.cpp"
    "emulator/parser/be_parser_si_sopp.cpp"
    "emulator/parser/be_parser_si_vintrp.cpp"
    "emulator/parser/be_parser_si_vop.cpp"
    "${PROJECT_SOURCE_DIR}/../../external/dynamic_library_module/DynamicLibraryModule.cpp"
    "${PROJECT_SOURCE_DIR}/../../external/celf/Src/CElf.cpp"
    "${PROJECT_SOURCE_DIR}/../../external/celf/Src/CElfSection.cpp"
    "${PROJECT_SOURCE_DIR}/../../external/celf/Src/CElfStringTable.cpp"
    "${PROJECT_SOURCE_DIR}/../../external/celf/Src/CElfSymbolTable.cpp"
)

# Windows-specific source files
file(GLOB WINDOWS_SRC
    "${PROJECT_SOURCE_DIR}/../../external/dynamic_library_module/DXXModule.cpp"
    "${PROJECT_SOURCE_DIR}/../utils/dx12/backend/rg_dxr_state_desc_reader.cpp"
    "${PROJECT_SOURCE_DIR}/../utils/dx12/backend/rg_dxr_output_metadata.cpp"
    "${PROJECT_SOURCE_DIR}/../utils/dx12/backend/rg_dx12_utils.cpp"
    "autogen/*.cpp"
    "autogen/*.h"
)

# Device Info lib sources
file(GLOB  DEVICE_INFO_SRC
    "${PROJECT_SOURCE_DIR}/../../external/device_info/DeviceInfoUtils.cpp"
    "${PROJECT_SOURCE_DIR}/../../external/device_info/DeviceInfo.cpp"
)

if(AMD_INTERNAL)
    file(GLOB  DEVICE_INFO_SRC
         "${PROJECT_SOURCE_DIR}/../../external/device_info_internal/DeviceInfoInternal.cpp"
         ${DEVICE_INFO_SRC}
    )
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DAMD_INTERNAL -DAMDT_INTERNAL")
endif()

# Pick up the source files that are relevant to the platform
if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    add_library(radeon_gpu_analyzer_backend STATIC ${COMMON_SRC} ${WINDOWS_SRC} ${DEVICE_INFO_SRC})
    if(NOT RGA_DISABLE_DX10)
        if (CMAKE_64BIT_TARGET)
            find_library(DX10ASM_LIBRARY_RELEASE DX10ASMLib_s HINTS "${PROJECT_SOURCE_DIR}/../../external/dx10_asm/lib/VS2015/x64/Release_Static")
            find_library(DX10ASM_LIBRARY_DEBUG DX10ASMLib_s HINTS "${PROJECT_SOURCE_DIR}/../../external/dx10_asm/lib/VS2015/x64/Debug_Static")
        else()
            find_library(DX10ASM_LIBRARY_RELEASE DX10ASMLib_s HINTS "${PROJECT_SOURCE_DIR}/../../external/dx10_asm/lib/VS2015/Win32/Release_Static")
            find_library(DX10ASM_LIBRARY_DEBUG DX10ASMLib_s HINTS "${PROJECT_SOURCE_DIR}/../../external/dx10_asm/lib/VS2015/Win32/Debug_Static")
        endif()
        target_link_libraries(radeon_gpu_analyzer_backend optimized ${DX10ASM_LIBRARY_RELEASE} debug ${DX10ASM_LIBRARY_DEBUG})
    else()
        add_definitions(-DDISABLE_DX10)
    endif()
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux" OR ${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    add_library(radeon_gpu_analyzer_backend STATIC ${COMMON_SRC} ${LINUX_SRC})
    add_library(DeviceInfoLib OBJECT ${DEVICE_INFO_SRC})
endif()

# set binary suffix
set_target_properties(radeon_gpu_analyzer_backend PROPERTIES DEBUG_POSTFIX -d)

# Set Visual C++ warning level to 4
if(MSVC)
  target_compile_options(radeon_gpu_analyzer_backend PRIVATE /W4)
endif()

# Link settings
if (CMAKE_64BIT_TARGET)
    link_directories(${PROJECT_SOURCE_DIR}/../../../Lib/Ext/zlib/1.2.8/contrib/vstudio/vc14/x64/ZlibStatRelease)
else()
    link_directories(${PROJECT_SOURCE_DIR}/../../../Lib/Ext/zlib/1.2.8/contrib/vstudio/vc14/x86/ZlibStatRelease)
endif()

# Link libraries
target_link_libraries(radeon_gpu_analyzer_backend AMDTOSWrappers AMDTBaseTools)