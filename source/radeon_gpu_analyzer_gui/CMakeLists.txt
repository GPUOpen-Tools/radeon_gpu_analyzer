cmake_minimum_required (VERSION 3.10)

project (radeon_gpu_analyzer_gui)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Add cmake utilities
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(devtools_qt_helper)
include(dev_tools)

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

find_package(Qt6 COMPONENTS Core Gui Widgets Network Widgets Svg Test REQUIRED
             HINTS ${QT_PACKAGE_ROOT} ${NO_DEFAULT_QT_OPTION})

get_target_property(QtCoreLocation Qt6::Core LOCATION)
get_filename_component(Qt6_LIB_DIR ${QtCoreLocation} DIRECTORY)
set(Qt6_ROOT_DIR "${Qt6_LIB_DIR}/..")

# Add this definitions if static linkage is used with the other projects
add_definitions(-DAMDTBASETOOLS_STATIC -DAMDTOSWRAPPERS_STATIC)

# Add UpdateCheckAPI
add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/../../external/update_check_api update_check_api)

# Include folders
include_directories("${PROJECT_SOURCE_DIR}/")
include_directories("${PROJECT_SOURCE_DIR}/qt/")
if (RGA_ENABLE_VULKAN)
    include_directories("${Vulkan_INCLUDE_DIRS}")
endif()
include_directories("${PROJECT_SOURCE_DIR}/../")
include_directories("${PROJECT_SOURCE_DIR}/../../../")
include_directories("${PROJECT_SOURCE_DIR}")
include_directories("${PROJECT_SOURCE_DIR}/../../external/third_party/")
include_directories("${PROJECT_SOURCE_DIR}/../../external")
include_directories("${PROJECT_SOURCE_DIR}/../../external")
include_directories("${PROJECT_SOURCE_DIR}/../../")
include_directories("${PROJECT_SOURCE_DIR}/../common")
include_directories("${PROJECT_SOURCE_DIR}/../../external/third_party/spdlog/spdlog-1.14.1/include")
include_directories(${UPDATECHECKAPI_INC_DIRS})
if (GUI_AUTOMATION)
    include_directories("${PROJECT_SOURCE_DIR}/../../external/rga_internal_utils/tests_gui/include")
endif()
include_directories("${PROJECT_SOURCE_DIR}/../../external/qt_common/")

add_subdirectory("${PROJECT_SOURCE_DIR}/../../external/qt_common" "../qt_common")

# TinyXML library
if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    find_library(TINYXML_LIB "tinyxml2.lib" HINTS "${PROJECT_SOURCE_DIR}/../../external/third_party/tinyxml2/Lib/VS2015/Win64/Release/MD")
    find_library(TINYXML_LIBD "tinyxml2-d.lib" HINTS "${PROJECT_SOURCE_DIR}/../../external/third_party/tinyxml2/Lib/VS2015/Win64/Debug/MDd")
else()
    find_library(TINYXML_LIB "libtinyxml2.a" HINTS "${PROJECT_SOURCE_DIR}/../../external/third_party/tinyxml2/Lib/Linux/x64/Release")
endif()

file(GLOB  CPP_INC
     "*.h"
     ${UPDATECHECKAPI_INC}
)

file(GLOB  CPP_SRC
     "*.cpp"
     "../common/*.cpp"
     "../common/vulkan/*.cpp"
     ${UPDATECHECKAPI_SRC}
     ${UPDATECHECKAPI_QT_SRC}
)

file(GLOB  UI_SRC
     "ui/*.ui"
     ${UPDATECHECKAPI_QT_UI}
)

file(GLOB  MOC_INC
     "qt/*.h"
     ${UPDATECHECKAPI_QT_INC}
)

file(GLOB  RES_SRC
    "resources/*.qrc"
)

file(GLOB  WINDOWS_INC
     "resources/icons/*.ico"
     "../common/resources/*GUI.rc"
     "../common/resources/*.h"
)

# Add source files for GUI automated tests.
if (GUI_AUTOMATION)
    file(GLOB  TESTS_GUI_CPP_INC
         "../../external/rga_internal_utils/tests_gui/include/*.h"
    )
    set(CPP_INC ${CPP_INC} ${TESTS_GUI_CPP_INC})
    file(GLOB  TESTS_GUI_CPP_SRC
         "../../external/rga_internal_utils/tests_gui/src/*.cpp"
    )
    set(CPP_SRC ${CPP_SRC} ${TESTS_GUI_CPP_SRC})
    file(GLOB  TESTS_GUI_MOC_INC
         "../../external/rga_internal_utils/tests_gui/include/qt/*.h"

    )
    set(MOC_INC ${MOC_INC} ${TESTS_GUI_MOC_INC})

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DRGA_GUI_AUTOMATION")

    # Create a Visual Studio project filter (folder) for Automation files.
    source_group("Tests-GUI" FILES ${TESTS_GUI_CPP_INC} ${TESTS_GUI_CPP_SRC} ${TESTS_GUI_MOC_INC})
endif()

qt6_wrap_cpp(GENERATED_MOC  ${MOC_INC})
qt6_wrap_ui (GENERATED_UI   ${UI_SRC})
qt6_add_resources(GENERATED_RES ${RES_SRC})

add_definitions(${Qt6Widgets_DEFINITIONS})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Qt6Widgets_EXECUTABLE_COMPILE_FLAGS}")

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux" OR ${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    # Libraries used by AMDTOSWrappers on Linux
    set(EXTRA_LIBS -lpthread -ldl)
endif()

if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    add_executable(radeon_gpu_analyzer_gui WIN32 ${WINDOWS_INC} ${CPP_SRC} ${CPP_INC} ${MOC_INC} ${GENERATED_MOC} ${GENERATED_UI} ${GENERATED_RES})
    set_target_properties(radeon_gpu_analyzer_gui PROPERTIES OUTPUT_NAME "RadeonGPUAnalyzer")
else()
    add_executable(radeon_gpu_analyzer_gui ${CPP_SRC} ${CPP_INC} ${MOC_INC} ${GENERATED_MOC} ${GENERATED_UI} ${GENERATED_RES})

    set_target_properties(radeon_gpu_analyzer_gui PROPERTIES OUTPUT_NAME "RadeonGPUAnalyzer-bin")
    # Disable RPATH
    set(CMAKE_SKIP_RPATH TRUE)
    set_property(TARGET radeon_gpu_analyzer_gui APPEND_STRING PROPERTY
                 LINK_FLAGS "-Wl,-rpath-link,${Qt6_LIB_DIR}")
endif()

# Set Visual C++ warning level to 4
if(MSVC)
  target_compile_options(radeon_gpu_analyzer_gui PRIVATE /W4)
endif()

if (GUI_AUTOMATION)
    set(QT_TEST_LIB Qt6::Test)
    set(AUTO_ARG "-automation")
endif()

if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    target_link_libraries(radeon_gpu_analyzer_gui AMDTBaseTools AMDTOSWrappers optimized ${TINYXML_LIB} debug ${TINYXML_LIBD} general Qt6::Gui Qt6::Widgets Qt6::Core QtCustomWidgets QtUtils ${QT_TEST_LIB} ${UPDATECHECKAPI_LIBS} ${EXTRA_LIBS})
else()
    target_link_libraries(radeon_gpu_analyzer_gui AMDTBaseTools AMDTOSWrappers ${TINYXML_LIB} Qt6::Gui Qt6::Widgets Qt6::Core QtCustomWidgets QtUtils ${QT_TEST_LIB} ${UPDATECHECKAPI_LIBS} ${EXTRA_LIBS})
endif()

# Copy the RGA-Updates file into the output directory for ease of testing.
add_custom_command(TARGET radeon_gpu_analyzer_gui POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "copying RGA-Updates.json to output directory"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/../../updates/RGA-Updates.json $<TARGET_FILE_DIR:${PROJECT_NAME}>
)

# Setup the post-build script.
if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    set(DEBUG_ARG "-debug")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "Copying external files to $<TARGET_FILE_DIR:${PROJECT_NAME}>/utils"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/../../external/update_check_api/rtda/windows/rtda.exe $<TARGET_FILE_DIR:${PROJECT_NAME}>/rtda.exe
    )
    if (GUI_AUTOMATION)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E echo "Copying Qt6 test library to $<TARGET_FILE_DIR:${PROJECT_NAME}>"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${Qt6_LIB_DIR}/Qt6Test$<$<CONFIG:debug>:d>.dll $<TARGET_FILE_DIR:${PROJECT_NAME}>/Qt6Test$<$<CONFIG:debug>:d>.dll
        )
    endif()
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux" OR ${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "Copying rtda to $<TARGET_FILE_DIR:${PROJECT_NAME}>"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/../../external/update_check_api/rtda/linux/rtda $<TARGET_FILE_DIR:${PROJECT_NAME}>/rtda
        COMMAND ${CMAKE_COMMAND} -E echo "Copying RadeonGPUAnalyzer shell script to $<TARGET_FILE_DIR:${PROJECT_NAME}>"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PROJECT_SOURCE_DIR}/../../build/util/linux/RadeonGPUAnalyzer $<TARGET_FILE_DIR:${PROJECT_NAME}>/RadeonGPUAnalyzer
        COMMAND chmod +x $<TARGET_FILE_DIR:${PROJECT_NAME}>/RadeonGPUAnalyzer
    )
    if (GUI_AUTOMATION)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E echo "Copying GUI test app and test kit files"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${Qt6_LIB_DIR}/libQt6Test.so.6 $<TARGET_FILE_DIR:${PROJECT_NAME}>/lib/libQt6Test.so.6
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/../../external/rga_internal_utils/tests_gui/run.py $<TARGET_FILE_DIR:${PROJECT_NAME}>/run.py
        )
    endif()

endif()

# Attach *deployqt post-build commands for packaging
deploy_qt_install_hook(${PROJECT_NAME} "")