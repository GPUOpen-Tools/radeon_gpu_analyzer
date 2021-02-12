cmake_minimum_required(VERSION 3.5.0)
project(dx12_backend)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")

file(GLOB  SOURCE_FILES
    "${PROJECT_SOURCE_DIR}/src/*.cpp"
    "${PROJECT_SOURCE_DIR}/../../../RadeonGPUAnalyzerBackend/src/be_d3d_include_manager.cpp"
)

# API symbol export definition.
add_definitions(-DRGA_API_EXPORTS -DINITGUID -DNOMINMAX -DRGA_DXR_ENABLED)

file(GLOB  INCLUDES
    "${PROJECT_SOURCE_DIR}/src/*.h"
    "${PROJECT_SOURCE_DIR}/src/extension/*.h"
    "${PROJECT_SOURCE_DIR}/../../../RadeonGPUAnalyzerBackend/Src/be_d3d_include_manager.h"
# TinyXML2.
    "${PROJECT_SOURCE_DIR}/../../../../Common/Lib/Ext/tinyxml2/tinyxml2.h"
)

include_directories("${PROJECT_SOURCE_DIR}/src")
include_directories("${PROJECT_SOURCE_DIR}/src/extension")
include_directories("${PROJECT_SOURCE_DIR}/../../../../Common/Lib/Ext/tinyxml2/Include")
include_directories("${PROJECT_SOURCE_DIR}/../../../../Common/Lib/Ext")
include_directories("${PROJECT_SOURCE_DIR}/../../../../RGA/")
include_directories("${PROJECT_SOURCE_DIR}/../../../RadeonGPUAnalyzerBackend/Src")

find_library(TINYXML_LIB "tinyxml2.lib" HINTS "${PROJECT_SOURCE_DIR}/../../../../Common/Lib/Ext/tinyxml2/Lib/VS2015/Win64/Release/MD")
find_library(TINYXML_LIBD "tinyxml2-d.lib" HINTS "${PROJECT_SOURCE_DIR}/../../../../Common/Lib/Ext/tinyxml2/Lib/VS2015/Win64/Debug/MDd")

add_executable(dx12_backend ${SOURCE_FILES} ${INCLUDES})

# Set output folder and executable name.
set_target_properties(dx12_backend PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG   "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/utils/DX12")
set_target_properties(dx12_backend PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/utils/DX12")

# Set link properties
set_target_properties(dx12_backend PROPERTIES OUTPUT_NAME dx12_backend)

# Set link libraries
target_link_libraries (dx12_backend d3dcompiler.lib d3d12.lib dxgi.lib optimized ${TINYXML_LIB} debug ${TINYXML_LIBD})

# Custom command: create DXC folder for post-build script
file(MAKE_DIRECTORY ${RUNTIME_OUTPUT_DIRECTORY}/DXC)
