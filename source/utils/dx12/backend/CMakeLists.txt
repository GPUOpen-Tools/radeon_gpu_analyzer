#######################################################################################################################
### Copyright (c) 2017-2025 Advanced Micro Devices, Inc. All rights reserved.
### @author AMD Developer Tools Team
#######################################################################################################################

cmake_minimum_required(VERSION 3.10)
project(dx12_backend)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")
set(TINYXML2_DIR "${PROJECT_SOURCE_DIR}/../../../../external/isa_spec_manager/source/third_party/tinyxml2")

file(GLOB  SOURCE_FILES
    "${PROJECT_SOURCE_DIR}/*.cpp"
    "${PROJECT_SOURCE_DIR}/../../../radeon_gpu_analyzer_backend/be_d3d_include_manager.cpp"
)

# API symbol export definition.
add_definitions(-DRGA_API_EXPORTS -DINITGUID -DNOMINMAX -DRGA_DXR_ENABLED)

file(GLOB  INCLUDES
    "${PROJECT_SOURCE_DIR}/*.h"
    "${PROJECT_SOURCE_DIR}/extension/*.h"
    "${PROJECT_SOURCE_DIR}/../../../radeon_gpu_analyzer_backend/be_d3d_include_manager.h"
# TinyXML2.
    "${PROJECT_SOURCE_DIR}/../../../../external/third_party/tinyxml2/Include/tinyxml2.h"
)

include_directories("${PROJECT_SOURCE_DIR}")
include_directories("${PROJECT_SOURCE_DIR}/extension")
include_directories("${PROJECT_SOURCE_DIR}/../../../../external")
include_directories("${PROJECT_SOURCE_DIR}/../../../../external/third_party")
include_directories("${PROJECT_SOURCE_DIR}/../../../../")
include_directories("${PROJECT_SOURCE_DIR}/../../../radeon_gpu_analyzer_backend")

add_library(tinyxml2 ${TINYXML2_DIR}/tinyxml2.cpp)

add_executable(dx12_backend ${SOURCE_FILES} ${INCLUDES})

# Set output folder and executable name.
set_target_properties(dx12_backend PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG   "$<TARGET_FILE_DIR:radeon_gpu_analyzer_cli>/utils/dx12")
set_target_properties(dx12_backend PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE "$<TARGET_FILE_DIR:radeon_gpu_analyzer_cli>/utils/dx12")

# Set link properties
set_target_properties(dx12_backend PROPERTIES OUTPUT_NAME dx12_backend)

target_include_directories(tinyxml2 PUBLIC ${TINYXML2_DIR})

# Set Visual C++ warning level to 4
if(MSVC)
  target_compile_options(dx12_backend PRIVATE /W4)
endif()

# Set link libraries
target_link_libraries (dx12_backend d3dcompiler.lib d3d12.lib dxgi.lib tinyxml2)
