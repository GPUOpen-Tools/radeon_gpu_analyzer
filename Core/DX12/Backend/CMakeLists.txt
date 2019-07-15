cmake_minimum_required(VERSION 3.5.0)
project(DX12Backend)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")

file(GLOB  SOURCE_FILES
    "${PROJECT_SOURCE_DIR}/Src/*.cpp"
    "${PROJECT_SOURCE_DIR}/../../../RadeonGPUAnalyzerBackend/Src/beD3DIncludeManager.cpp"
)

# API symbol export definition.
add_definitions(-DRGA_API_EXPORTS -DINITGUID -DNOMINMAX)

file(GLOB  INCLUDES
    "${PROJECT_SOURCE_DIR}/Include/*.h"
    "${PROJECT_SOURCE_DIR}/../../../RadeonGPUAnalyzerBackend/Include/beD3DIncludeManager.h"
# TinyXML2.
    "${PROJECT_SOURCE_DIR}/../../../../Common/Lib/Ext/tinyxml2/tinyxml2.h"
)

include_directories("${PROJECT_SOURCE_DIR}/Include")
include_directories("${PROJECT_SOURCE_DIR}/../../../../Common/Lib/Ext/tinyxml2/Include")
include_directories("${PROJECT_SOURCE_DIR}/../../../../Common/Lib/Ext")
include_directories("${PROJECT_SOURCE_DIR}/../../../../RGA/")
include_directories("${PROJECT_SOURCE_DIR}/../../../RadeonGPUAnalyzerBackend/Include")

if (${CMAKE_BUILD_TYPE} STREQUAL "Release")
    find_library(TINYXML_LIB "tinyxml2.lib" HINTS "${PROJECT_SOURCE_DIR}/../../../../Common/Lib/Ext/tinyxml2/Lib/VS2015/Win64/Release/MD")
elseif(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    find_library(TINYXML_LIB "tinyxml2-d.lib" HINTS "${PROJECT_SOURCE_DIR}/../../../../Common/Lib/Ext/tinyxml2/Lib/VS2015/Win64/Debug/MDd")
endif()

add_executable(DX12Backend ${SOURCE_FILES} ${INCLUDES})

# Set output folder and executable name.
set_target_properties(DX12Backend PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG   "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_BUILD_TYPE}/x64/DX12")
set_target_properties(DX12Backend PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_BUILD_TYPE}/x64/DX12")

# Set link properties
set_target_properties(DX12Backend PROPERTIES OUTPUT_NAME DX12Backend)

# Set link libraries
target_link_libraries (DX12Backend d3dcompiler.lib d3d12.lib dxgi.lib ${TINYXML_LIB})

# Custom command: create DXC folder for post-build script
file(MAKE_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_BUILD_TYPE}/x64/DX12/DXC)

