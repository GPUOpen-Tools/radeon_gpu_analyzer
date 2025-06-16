#######################################################################################################################
### Copyright (c) 2017-2025 Advanced Micro Devices, Inc. All rights reserved.
### @author AMD Developer Tools Team
#######################################################################################################################

cmake_minimum_required (VERSION 3.10)

# Add RGA CLI and Backend
if(NOT BUILD_GUI_ONLY)
    add_subdirectory (radeon_gpu_analyzer_backend)
    add_subdirectory (radeon_gpu_analyzer_cli)
if(NOT BUILD_GUI_ONLY)
    if (RGA_ENABLE_VULKAN)
        add_dependencies (radeon_gpu_analyzer_cli vulkan_backend)
    endif()
    if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
        add_dependencies (radeon_gpu_analyzer_cli dx12_backend)
    endif()
endif()
endif()

# Add RGA GUI
if(NOT BUILD_CLI_ONLY)
    add_subdirectory (radeon_gpu_analyzer_gui)
    add_dependencies (radeon_gpu_analyzer_gui radeon_gpu_analyzer_cli)
endif()

