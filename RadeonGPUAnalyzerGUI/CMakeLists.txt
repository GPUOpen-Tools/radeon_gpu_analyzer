cmake_minimum_required (VERSION 3.5)

project (RadeonGPUAnalyzerGUI)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Enable C++ exceptions
if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc")
endif()

# Qt
if (NO_DEFAULT_QT)
  set (NO_DEFAULT_QT_OPTION "NO_DEFAULT_PATH")
endif()
find_package(Qt5 5.5 COMPONENTS Core Gui Widgets REQUIRED
             HINTS ${QT_PACKAGE_ROOT} ${NO_DEFAULT_QT_OPTION})

# Add this definitions if static linkage is used with the other projects
add_definitions(-DAMDTBASETOOLS_STATIC -DAMDTOSWRAPPERS_STATIC)

# Include folders
include_directories("${PROJECT_SOURCE_DIR}/include/")
include_directories("${PROJECT_SOURCE_DIR}/include/qt/")
include_directories("${PROJECT_SOURCE_DIR}/../")
include_directories("${PROJECT_SOURCE_DIR}/../../Common/Lib/Ext/")
include_directories("${PROJECT_SOURCE_DIR}/../../Common/Src")
include_directories("${PROJECT_SOURCE_DIR}/../../")
include_directories("${PROJECT_SOURCE_DIR}/../Utils/include")

add_subdirectory(../../QtCommon/Scaling QtCommon/Scaling)
add_subdirectory(../../QtCommon/Util QtCommon/Util)
add_subdirectory(../../QtCommon/CustomWidgets QtCommon/CustomWidgets)

# TinyXML library
if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    if (${CMAKE_BUILD_TYPE} STREQUAL "Release")
        find_library(TINYXML_LIB "tinyxml2.lib" HINTS "${PROJECT_SOURCE_DIR}/../../Common/Lib/Ext/tinyxml2/Lib/VS2015/Win64/Release/MD")
    elseif(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
        find_library(TINYXML_LIB "tinyxml2-d.lib" HINTS "${PROJECT_SOURCE_DIR}/../../Common/Lib/Ext/tinyxml2/Lib/VS2015/Win64/Debug/MDd")
    endif()
else()
    find_library(TINYXML_LIB "libtinyxml2.a" HINTS "${PROJECT_SOURCE_DIR}/../../Common/Lib/Ext/tinyxml2/Lib/Linux/x64/Release")
endif()

file(GLOB  CPP_INC
     "include/*.h"
)

file(GLOB  CPP_SRC
     "src/*.cpp"
     "../Utils/src/*.cpp"
)

file(GLOB  UI_SRC
     "ui/*.ui"
)

file(GLOB  MOC_INC
     "include/qt/*.h"
)

file(GLOB  RES_SRC
    "res/*.qrc"
)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    file(GLOB  WINDOWS_INC
         "include/res/icons/*.ico"
         "../Utils/Res/*GUI.rc"
         "../Utils/Res/*.h"
    )
endif()

qt5_wrap_cpp(GENERATED_MOC  ${MOC_INC})
qt5_wrap_ui (GENERATED_UI   ${UI_SRC})
qt5_add_resources(GENERATED_RES ${RES_SRC})

add_definitions(${Qt5Widgets_DEFINITIONS})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS}")

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux" OR ${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    # Libraries used by AMDTOSWrappers on Linux
    set(EXTRA_LIBS -lpthread -ldl)
endif()

if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    add_executable(RadeonGPUAnalyzerGUI WIN32 ${WINDOWS_INC} ${CPP_SRC} ${CPP_INC} ${MOC_INC} ${GENERATED_MOC} ${GENERATED_UI} ${GENERATED_RES})
else()
    add_executable(RadeonGPUAnalyzerGUI ${CPP_SRC} ${CPP_INC} ${MOC_INC} ${GENERATED_MOC} ${GENERATED_UI} ${GENERATED_RES})

    set_target_properties(RadeonGPUAnalyzerGUI PROPERTIES OUTPUT_NAME "RadeonGPUAnalyzerGUI-bin")
    # Disable RPATH
    set(CMAKE_SKIP_RPATH TRUE)
    set_property(TARGET RadeonGPUAnalyzerGUI APPEND_STRING PROPERTY
                 LINK_FLAGS "-Wl,-rpath-link,${Qt5_DIR}/../../")
endif()

target_link_libraries(RadeonGPUAnalyzerGUI AMDTBaseTools AMDTOSWrappers QtScaling QtUtil QtCustomWidgets ${TINYXML_LIB} Qt5::Gui Qt5::Widgets Qt5::Core ${EXTRA_LIBS})

if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(DEBUG_ARG "-debug")
    endif()
    get_target_property(QtCoreLocation Qt5::Core LOCATION)
    get_filename_component(Qt5_DIR ${QtCoreLocation} DIRECTORY)
    add_custom_command(TARGET RadeonGPUAnalyzerGUI
                       POST_BUILD
                       COMMAND ${PROJECT_SOURCE_DIR}/../Build/windows/copy_post_build_gui.bat ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_BUILD_TYPE} ${Qt5_DIR} ${DEBUG_ARG}
                       WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/../Build/windows
                      )
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux" OR ${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    add_custom_command(TARGET RadeonGPUAnalyzerGUI
                       POST_BUILD
                       COMMAND /bin/sh ${PROJECT_SOURCE_DIR}/../Build/linux/copy_post_build_gui.sh ${Qt5_DIR}/../..
                       WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/../Build/linux
                      )
endif()
