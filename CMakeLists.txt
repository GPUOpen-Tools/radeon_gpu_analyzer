cmake_minimum_required (VERSION 3.10)
project (RGA)

# Define C++ standard for RGA
set(CMAKE_CXX_STANDARD 17)

# Boolean option for enabling AMD Internal build mode
option(AMD_INTERNAL "Add -DAMD_INTERNAL=ON for AMD Internal build mode." OFF)

# Boolean option for enabling automated testing mode in the GUI executable
option(GUI_AUTOMATION "Add -DGUI_AUTOMATION=ON to enable GUI automated testing mode." OFF)

# Boolean option for disabling the GUI build (building CLI only)
option(BUILD_CLI_ONLY "Add -DBUILD_CLI_ONLY=ON to disable the GUI build (build CLI only)." OFF)

# Boolean option for disabling the CLI build (building GUI only)
option(BUILD_GUI_ONLY "Add -DBUILD_GUI_ONLY=ON to disable the CLI build (build GUI only)." OFF)

# Boolean option to disable looking for default Qt package
option(NO_DEFAULT_QT "Add -DNO_DEFAULT_QT=ON to disable using default Qt package installed on the system." OFF)

# Boolean option to enable live Vulkan mode.
option(RGA_ENABLE_VULKAN "Add -DRGA_ENABLE_VULKAN=OFF to build RGA without Vulkan mode support. By default, Vulkan mode is enabled." ON)

# Boolean option to enable live Vulkan mode.
option(RGA_DISABLE_DX10 "Add -DRGA_DISABLE_DX10=OFF to build RGA without DX10 support. By default, DX10 support is enabled." OFF)

# Define build level here
add_definitions(-DAMDT_PUBLIC -DCMAKE_BUILD)

# Identify the build architecture
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    # Target architecture is 64-bit
    option(CMAKE_64BIT_TARGET "On if the target architecture is 64-bit" ON)
endif()

# Define cppcheck options
if (CMAKE_CXX_CPPCHECK)
    if (NOT ${CMAKE_GENERATOR} MATCHES "^.*Makefiles$|^Ninja$")
        add_custom_target(ANALYZE_CPPCHECK DEPENDS RGA
            COMMAND ${CMAKE_CXX_CPPCHECK} ${CMAKE_SOURCE_DIR}
        )
    else()
        list(
            APPEND CMAKE_CXX_CPPCHECK
                "--xml"
                ${CMAKE_SOURCE_DIR}
        )
    endif()
endif()

# Pass the build number and build date
if (NOT "$ENV{BUILD_NUMBER}" STREQUAL "")
    add_definitions(-DRGA_BUILD_NUMBER=$ENV{BUILD_NUMBER})
endif()
if (NOT "$ENV{RGA_BUILD_DATE}" STREQUAL "")
    add_definitions(-DRGA_BUILD_DATE=$ENV{RGA_BUILD_DATE})
endif()

# Linux specific configurations
if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux" OR ${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    # Compiler & linker configuration
    add_definitions(-fmessage-length=0 -Wno-unknown-pragmas -pthread)
    set(CMAKE_EXE_LINKER_FLAGS "-static-libstdc++ -static-libgcc ${CMAKE_EXE_LINKER_FLAGS}")

    # Disable PIE (position-independent executable) to avoid link errors (gcc 6+ generates PIE by default).
    if (CMAKE_COMPILER_IS_GNUCC AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 6.0)
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -no-pie")
    endif()

    # Generic preprocessor directives
    add_definitions(-DLINUX)
    add_definitions(-D_LINUX)

    # Debug-specific preprocessor directives
    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_definitions(-g -D_DEBUG)
    endif()

    # Output directories
    set(BUILD_CONFIG "release")
    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(BUILD_CONFIG "debug")
    endif()
    if (GUI_AUTOMATION)
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/build/linux/make_test/${BUILD_CONFIG}")
        set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/build/linux/make_test/${BUILD_CONFIG}/lib")
        set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/build/linux/make_test/${BUILD_CONFIG}/arch")
    else()
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/build/linux/make/${BUILD_CONFIG}")
        set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/build/linux/make/${BUILD_CONFIG}/lib")
        set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/build/linux/make/${BUILD_CONFIG}/arch")
    endif()

elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")

    # Preprocessor directives
    add_definitions(-D_USRDLL -DUNICODE -D_UNICODE)

    # Output directories - use Windows path separaters so xcopy commands work
    if (GUI_AUTOMATION)
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}\\build\\win_test\\$<$<CONFIG:debug>:debug>$<$<CONFIG:release>:release>)
        set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}\\build\\win_test\\$<$<CONFIG:debug>:debug>$<$<CONFIG:release>:release>\\lib)
        set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}\\build\\win_test\\$<$<CONFIG:debug>:debug>$<$<CONFIG:release>:release>\\arch)
    else()
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}\\build\\win\\$<$<CONFIG:debug>:debug>$<$<CONFIG:release>:release>)
        set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}\\build\\win\\$<$<CONFIG:debug>:debug>$<$<CONFIG:release>:release>\\lib)
        set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}\\build\\win\\$<$<CONFIG:debug>:debug>$<$<CONFIG:release>:release>\\arch)
    endif()

    # Use multiple cores to compile all projects.
    add_definitions(/MP)
endif()

# Windows solution generator is a multi config generator, so define
# CMAKE_CONFIGURATTION_TYPES, which enables usage of $<$<CONFIG:value1>value2> syntax for Windows settings.
# Linux make is not a multi config tool.  This CMakeLists.txt assumes
# cmake is invoked with CMAKE_BUILD_TYPE set to Debug or Release on Linux.
# NOTE: User can still do a debug build if cmake is run for Release config
# by setting the CFLAGS and CXXFLAGS environment variables to "-g -D_DEBUG" before running make.
if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    # Visual Studio
    set(CMAKE_CONFIGURATION_TYPES "Release;Debug")
endif()

# Vulkan support
if (RGA_ENABLE_VULKAN)
    add_definitions("-DRGA_ENABLE_VULKAN")
endif()

# Add RGA CLI projects
if(NOT BUILD_GUI_ONLY)
    if (RGA_ENABLE_VULKAN)
        add_subdirectory (source/utils/vulkan/backend)
    endif()
    if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
        add_subdirectory (source/utils/dx12/backend)
    endif()
endif()

# Add source folder
add_subdirectory (source)

# Add amdt_os_wrappers and amdt_base_tools.
add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/external/amdt_base_tools)
add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/external/amdt_os_wrappers)

# Disable specific MS compiler warnings for Common targets
if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    target_compile_options(AMDTBaseTools PRIVATE "/wd4309")
    target_compile_options(AMDTOSWrappers PRIVATE "/wd4275" "/wd4309")
endif()

# Add Tests
if(AMD_INTERNAL)
    add_subdirectory (${PROJECT_SOURCE_DIR}/../rga_internal_utils/tests ${CMAKE_CURRENT_BINARY_DIR}/tests)
endif()
