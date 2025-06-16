#######################################################################################################################
### Copyright (c) 2017-2025 Advanced Micro Devices, Inc. All rights reserved.
### @author AMD Developer Tools Team
#######################################################################################################################
cmake_minimum_required(VERSION 3.10)
project(vulkan_backend)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")

# Try finding Vulkan SDK automatically if the cmake version is 3.7+ and
# Vulkan SDK include & lib paths are not specified explicitly.
if (NOT DEFINED VULKAN_SDK_INC_DIR OR NOT DEFINED VULKAN_SDK_LIB_DIR)
    if (${CMAKE_VERSION} VERSION_GREATER "3.7.0" OR ${CMAKE_VERSION} VERSION_EQUAL "3.7.0")
        find_package(Vulkan REQUIRED)
    else()
        message(FATAL_ERROR "Cannot find VulkanSDK. Please use cmake 3.7+ to automatically detect the VulkanSDK installation or specify paths to the VulkanSDK include & lib directories explicitly.")
    endif()
else()
    set(Vulkan_INCLUDE_DIRS ${VULKAN_SDK_INC_DIR})
    if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
        set(Vulkan_LIBRARIES "${VULKAN_SDK_LIB_DIR}/vulkan-1.lib")
    else()
        set(Vulkan_LIBRARIES "${VULKAN_SDK_LIB_DIR}/libvulkan.so")
    endif()
endif()

file(GLOB  SOURCE_FILES
    "${PROJECT_SOURCE_DIR}/*.cpp"
    "${PROJECT_SOURCE_DIR}/../../../common/vulkan/*.cpp"
# volk.
    "${PROJECT_SOURCE_DIR}/../../../../external/third_party/volk/volk.c"
)

file(GLOB  INCLUDES
    "${PROJECT_SOURCE_DIR}/*.h"
    "${PROJECT_SOURCE_DIR}/../../../common/vulkan/*.h"
# volk.
    "${PROJECT_SOURCE_DIR}/../../../../external/third_party/volk/volk.h"
)

include_directories("${PROJECT_SOURCE_DIR}/")
include_directories("${PROJECT_SOURCE_DIR}/../../../../external/")
include_directories("${PROJECT_SOURCE_DIR}/../../../../external/third_party")
include_directories("${PROJECT_SOURCE_DIR}/../../../../")
include_directories(${Vulkan_INCLUDE_DIRS})


# Linux specific configurations
if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux" OR ${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    # Compiler & linker configuration
    add_definitions(-std=c++17)
endif()

# Windows specific configuration
if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    add_definitions(/EHa)
endif()

add_executable(vulkan_backend ${SOURCE_FILES} ${INCLUDES})

# Set link properties
# Set output folder and executable name.
set_target_properties(vulkan_backend PROPERTIES OUTPUT_NAME vulkan_backend)
set_target_properties(vulkan_backend PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG   "$<TARGET_FILE_DIR:radeon_gpu_analyzer_cli>/utils/vulkan")
set_target_properties(vulkan_backend PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE "$<TARGET_FILE_DIR:radeon_gpu_analyzer_cli>/utils/vulkan")

# Set Visual C++ warning level to 4
if(MSVC)
  target_compile_options(vulkan_backend PRIVATE /W4)
endif()

#Set link libraries
if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    target_link_libraries(vulkan_backend ${Vulkan_LIBRARIES})
else()
    target_link_libraries(vulkan_backend ${Vulkan_LIBRARIES} ${CMAKE_DL_LIBS})
endif()
