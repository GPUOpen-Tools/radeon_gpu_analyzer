cmake_minimum_required (VERSION 3.5)

project (RadeonGPUAnalyzerGUI)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Add RGA macro.
add_definitions(-D__RGA)

# Enable C++ exceptions
if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc")
endif()

# Qt
if (NO_DEFAULT_QT)
  set (NO_DEFAULT_QT_OPTION "NO_DEFAULT_PATH")
endif()

# Find Vulkan SDK
if (RGA_ENABLE_VULKAN)
    # Try finding Vulkan SDK automatically if the cmake version is 3.7+ and
    # Vulkan SDK include path is not specified explicitly.
    if (NOT DEFINED VULKAN_SDK_INC_DIR)
        if (${CMAKE_VERSION} VERSION_GREATER "3.7.0" OR ${CMAKE_VERSION} VERSION_EQUAL "3.7.0")
            find_package(Vulkan REQUIRED)
        else()
            message(FATAL_ERROR "Cannot find VulkanSDK. Please use cmake 3.7+ to automatically detect the VulkanSDK installation or specify paths to the VulkanSDK include & lib directories explicitly.")
        endif()
    else()
        set(Vulkan_INCLUDE_DIRS ${VULKAN_SDK_INC_DIR})
    endif()
endif()

find_package(Qt5 5.5 COMPONENTS Core Gui Widgets Test REQUIRED
             HINTS ${QT_PACKAGE_ROOT} ${NO_DEFAULT_QT_OPTION})

get_target_property(QtCoreLocation Qt5::Core LOCATION)
get_filename_component(Qt5_LIB_DIR ${QtCoreLocation} DIRECTORY)
set(Qt5_ROOT_DIR "${Qt5_LIB_DIR}/..")

# Add this definitions if static linkage is used with the other projects
add_definitions(-DAMDTBASETOOLS_STATIC -DAMDTOSWRAPPERS_STATIC)

# Add UpdateCheckAPI
add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/../../Common/Src/UpdateCheckAPI UpdateCheckAPI)

# Include folders
include_directories("${PROJECT_SOURCE_DIR}/Include/")
include_directories("${PROJECT_SOURCE_DIR}/Include/Qt/")
if (RGA_ENABLE_VULKAN)
    include_directories("${Vulkan_INCLUDE_DIRS}")
endif()
include_directories("${PROJECT_SOURCE_DIR}/../")
include_directories("${PROJECT_SOURCE_DIR}/../../Common/Lib/Ext/")
include_directories("${PROJECT_SOURCE_DIR}/../../Common/Src")
include_directories("${PROJECT_SOURCE_DIR}/../../")
include_directories("${PROJECT_SOURCE_DIR}/../Utils/Include")
include_directories(${UPDATECHECKAPI_INC_DIRS})
if (GUI_AUTOMATION)
    include_directories("${PROJECT_SOURCE_DIR}/../../RGA-Internal/tests_gui/include")
endif()

add_subdirectory(../../QtCommon/Scaling QtCommon/Scaling)
add_subdirectory(../../QtCommon/Util QtCommon/Util)
add_subdirectory(../../QtCommon/CustomWidgets QtCommon/CustomWidgets)

# TinyXML library
if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    find_library(TINYXML_LIB "tinyxml2.lib" HINTS "${PROJECT_SOURCE_DIR}/../../Common/Lib/Ext/tinyxml2/Lib/VS2015/Win64/Release/MD")
    find_library(TINYXML_LIBD "tinyxml2-d.lib" HINTS "${PROJECT_SOURCE_DIR}/../../Common/Lib/Ext/tinyxml2/Lib/VS2015/Win64/Debug/MDd")
else()
    find_library(TINYXML_LIB "libtinyxml2.a" HINTS "${PROJECT_SOURCE_DIR}/../../Common/Lib/Ext/tinyxml2/Lib/Linux/x64/Release")
endif()

file(GLOB  CPP_INC
     "Include/*.h"
     ${UPDATECHECKAPI_INC}
)

file(GLOB  CPP_SRC
     "Src/*.cpp"
     "../Utils/Src/*.cpp"
     "../Utils/Vulkan/Src/*.cpp"
     ${UPDATECHECKAPI_SRC}
     ${UPDATECHECKAPI_QT_SRC}
)

file(GLOB  UI_SRC
     "Ui/*.ui"
     ${UPDATECHECKAPI_QT_UI}
)

file(GLOB  MOC_INC
     "Include/Qt/*.h"
     ${UPDATECHECKAPI_QT_INC}
)

file(GLOB  RES_SRC
    "Res/*.qrc"
)

file(GLOB  WINDOWS_INC
     "Include/Res/icons/*.ico"
     "../Utils/Res/*GUI.rc"
     "../Utils/Res/*.h"
)

# Add source files for GUI automated tests.
if (GUI_AUTOMATION)
    file(GLOB  TESTS_GUI_CPP_INC
         "../../RGA-Internal/tests_gui/include/*.h"
    )
    set(CPP_INC ${CPP_INC} ${TESTS_GUI_CPP_INC})
    file(GLOB  TESTS_GUI_CPP_SRC
         "../../RGA-Internal/tests_gui/src/*.cpp"
    )
    set(CPP_SRC ${CPP_SRC} ${TESTS_GUI_CPP_SRC})
    file(GLOB  TESTS_GUI_MOC_INC
         "../../RGA-Internal/tests_gui/include/qt/*.h"
    )
    set(MOC_INC ${MOC_INC} ${TESTS_GUI_MOC_INC})

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DRGA_GUI_AUTOMATION")

    # Create a Visual Studio project filter (folder) for Automation files.
    source_group("Tests-GUI" FILES ${TESTS_GUI_CPP_INC} ${TESTS_GUI_CPP_SRC} ${TESTS_GUI_MOC_INC})
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
    add_executable(RadeonGPUAnalyzer WIN32 ${WINDOWS_INC} ${CPP_SRC} ${CPP_INC} ${MOC_INC} ${GENERATED_MOC} ${GENERATED_UI} ${GENERATED_RES})
else()
    add_executable(RadeonGPUAnalyzer ${CPP_SRC} ${CPP_INC} ${MOC_INC} ${GENERATED_MOC} ${GENERATED_UI} ${GENERATED_RES})

    set_target_properties(RadeonGPUAnalyzer PROPERTIES OUTPUT_NAME "RadeonGPUAnalyzer-bin")
    # Disable RPATH
    set(CMAKE_SKIP_RPATH TRUE)
    set_property(TARGET RadeonGPUAnalyzer APPEND_STRING PROPERTY
                 LINK_FLAGS "-Wl,-rpath-link,${Qt5_LIB_DIR}")
endif()

if (GUI_AUTOMATION)
    set(QT_TEST_LIB Qt5::Test)
    set(AUTO_ARG "-automation")
endif()

if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    target_link_libraries(RadeonGPUAnalyzer AMDTBaseTools AMDTOSWrappers QtScaling QtUtil QtCustomWidgets optimized ${TINYXML_LIB} debug ${TINYXML_LIBD} general Qt5::Gui Qt5::Widgets Qt5::Core ${QT_TEST_LIB} ${UPDATECHECKAPI_LIBS} ${EXTRA_LIBS})
else()
    target_link_libraries(RadeonGPUAnalyzer AMDTBaseTools AMDTOSWrappers QtScaling QtUtil QtCustomWidgets ${TINYXML_LIB} Qt5::Gui Qt5::Widgets Qt5::Core ${QT_TEST_LIB} ${UPDATECHECKAPI_LIBS} ${EXTRA_LIBS})
endif()

# Copy the RGA-Updates file into the output directory for ease of testing.
add_custom_command(TARGET RadeonGPUAnalyzer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "copying RGA-Updates.json to output directory"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/../Updates/RGA-Updates.json ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)

# On Windows also copy into the binary directory, which is the default working directory of Visual Studio.
if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    add_custom_command(TARGET RadeonGPUAnalyzer POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "copying RGA-Updates.json to binary directory"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/../Updates/RGA-Updates.json ${CMAKE_CURRENT_BINARY_DIR}
    )
endif()

# Setup the post-build script.
if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    set(DEBUG_ARG "-debug")
    get_target_property(QtCoreLocation Qt5::Core LOCATION)
    get_filename_component(Qt5_DIR ${QtCoreLocation} DIRECTORY)
    add_custom_command(TARGET RadeonGPUAnalyzer
                       POST_BUILD
                       COMMAND ${PROJECT_SOURCE_DIR}/../Build/Util/Windows/copy_post_build_gui.bat ${CMAKE_RUNTIME_OUTPUT_DIRECTORY} ${Qt5_LIB_DIR} $<$<CONFIG:debug>:-debug> ${AUTO_ARG}
                       WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/../Build/Util/Windows
                      )
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux" OR ${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    # Detect path to the Qt plugins folder.
    # The GUI post-build script will copy 2 Qt libraries from the plugins folder: libqsvg.so and libqxcb.so
    find_library(QT_LIBQSVG "libqsvg.so" PATHS "${Qt5_ROOT_DIR}/plugins/imageformats" "/usr/lib/x86_64-linux-gnu/qt5/plugins/imageformats")
    find_library(QT_LIBQXCB "libqxcb.so" PATHS "${Qt5_ROOT_DIR}/plugins/platforms" "/usr/lib/x86_64-linux-gnu/qt5/plugins/platforms")
    if (NOT QT_LIBQSVG OR NOT QT_LIBQXCB)
        message(FATAL_ERROR "Error: cannot find reqired Qt libraries: libqsvg.so and libqxcb.so.")
    endif()
    get_filename_component(QT_PLUGINS_DIR  "${QT_LIBQSVG}" DIRECTORY)
    set (QT_PLUGINS_DIR "${QT_PLUGINS_DIR}/..")

    add_custom_command(TARGET RadeonGPUAnalyzer
                       POST_BUILD
                       COMMAND /bin/sh ${PROJECT_SOURCE_DIR}/../Build/Util/Linux/copy_post_build_gui.sh ${CMAKE_RUNTIME_OUTPUT_DIRECTORY} ${Qt5_LIB_DIR} ${QT_PLUGINS_DIR} ${AUTO_ARG}
                       WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/../Build/Util/Linux
                      )
endif()
