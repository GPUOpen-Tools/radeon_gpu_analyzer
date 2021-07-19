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
    "${PROJECT_SOURCE_DIR}/../../../../../Common/Lib/Ext/volk/volk.c"
)

file(GLOB  INCLUDES
    "${PROJECT_SOURCE_DIR}/*.h"
    "${PROJECT_SOURCE_DIR}/../../../common/vulkan/*.h"
# volk.
    "${PROJECT_SOURCE_DIR}/../../../../../Common/Lib/Ext/volk/volk.h"
)

include_directories("${PROJECT_SOURCE_DIR}/")
include_directories("${PROJECT_SOURCE_DIR}/../../../../../Common/Lib/Ext/")
include_directories("${PROJECT_SOURCE_DIR}/../../../../")
include_directories(${Vulkan_INCLUDE_DIRS})


# Linux specific configurations
if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux" OR ${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    # Compiler & linker configuration
    add_definitions(-std=c++11)
endif()

# Windows specific configuration
if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    add_definitions(/EHa)
endif()

add_executable(vulkan_backend ${SOURCE_FILES} ${INCLUDES})

# Set output folder and executable name.
set_target_properties(vulkan_backend PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG   "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/utils/Vulkan")
set_target_properties(vulkan_backend PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/utils/Vulkan")

# Set link properties
set_target_properties(vulkan_backend PROPERTIES OUTPUT_NAME vulkan_backend)

#Set link libraries
if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    target_link_libraries(vulkan_backend ${Vulkan_LIBRARIES})
else()
    target_link_libraries(vulkan_backend ${Vulkan_LIBRARIES} ${CMAKE_DL_LIBS})
endif()
