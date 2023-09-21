cmake_minimum_required(VERSION 3.10)
project(dx12_backend)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")

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
include_directories("${PROJECT_SOURCE_DIR}/../../../../external/third_party/tinyxml2/Include")
include_directories("${PROJECT_SOURCE_DIR}/../../../../external/third_party")
include_directories("${PROJECT_SOURCE_DIR}/../../../../")
include_directories("${PROJECT_SOURCE_DIR}/../../../radeon_gpu_analyzer_backend")

find_library(TINYXML_LIB "tinyxml2.lib" HINTS "${PROJECT_SOURCE_DIR}/../../../../external/third_party/tinyxml2/Lib/VS2015/Win64/Release/MD")
find_library(TINYXML_LIBD "tinyxml2-d.lib" HINTS "${PROJECT_SOURCE_DIR}/../../../../external/third_party/tinyxml2/Lib/VS2015/Win64/Debug/MDd")

add_executable(dx12_backend ${SOURCE_FILES} ${INCLUDES})

# Set output folder and executable name.
set_target_properties(dx12_backend PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG   "$<TARGET_FILE_DIR:radeon_gpu_analyzer_cli>/utils/dx12")
set_target_properties(dx12_backend PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE "$<TARGET_FILE_DIR:radeon_gpu_analyzer_cli>/utils/dx12")

# Set link properties
set_target_properties(dx12_backend PROPERTIES OUTPUT_NAME dx12_backend)

# Set Visual C++ warning level to 4
if(MSVC)
  target_compile_options(dx12_backend PRIVATE /W4)
endif()

# Set link libraries
target_link_libraries (dx12_backend d3dcompiler.lib d3d12.lib dxgi.lib optimized ${TINYXML_LIB} debug ${TINYXML_LIBD})
