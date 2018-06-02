cmake_minimum_required (VERSION 2.6)
project (RGA)

# Boolean option for enabling AMD Internal build mode
option(AMD_INTERNAL "Add -DAMD_INTERNAL=ON for AMD Internal build mode." OFF)

# Boolean option for disabling the GUI build (building CLI only)
option(BUILD_CLI_ONLY "Add -DBUILD_CLI_ONLY=ON to disable the GUI build (build CLI only)" OFF)

# Boolean option for disabling the CLI build (building GUI only)
option(BUILD_GUI_ONLY "Add -DBUILD_GUI_ONLY=ON to disable the CLI build (build GUI only)" OFF)

# Boolean option to disable looking for default Qt package
option(NO_DEFAULT_QT "Add -DNO_DEFAULT_QT=ON to disable using default Qt package installed on the system" OFF)

# Define build level here
add_definitions(-DAMDT_PUBLIC -DCMAKE_BUILD)

# Identify the build architecture
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    # Target architecture is 64-bit
    option(CMAKE_64BIT_TARGET "On if the target architecture is 64-bit" ON)
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
    add_definitions(-std=c++11 -fmessage-length=0 -Wno-unknown-pragmas -pthread)
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
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/output/bin")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/output/lib")
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/output/arch")

elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")

    # Preprocessor directives
    add_definitions(-D_USRDLL -DUNICODE -D_UNICODE)

    # Output directories
    if (CMAKE_64BIT_TARGET)
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/output/x64/bin")
        set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/output/x64/lib")
        set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/output/x64/arch")
    else()
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/output/x86/bin")
        set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/output/x86/lib")
        set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/output/x86/arch")
    endif()
endif()

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_CONFIGURATION_TYPES "Debug")
else()
    set(CMAKE_CONFIGURATION_TYPES "Release")
endif()

# Add Common projects
add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/../Common/Src/AMDTBaseTools ${CMAKE_CURRENT_BINARY_DIR}/Common/Src/AMDTBaseTools)
add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/../Common/Src/AMDTOSWrappers ${CMAKE_CURRENT_BINARY_DIR}/Common/Src/AMDTOSWrappers)

# Disable specific warnings for Common targets
if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    target_compile_options(AMDTOSWrappers PRIVATE "-wd 4275")
endif()

# Add RGA CLI projects
if(NOT BUILD_GUI_ONLY)
    add_subdirectory (RadeonGPUAnalyzerBackend)
    add_subdirectory (RadeonGPUAnalyzerCLI)
endif()

# Add RGA GUI
if(NOT BUILD_CLI_ONLY)
    add_subdirectory (RadeonGPUAnalyzerGUI)
    add_dependencies (RadeonGPUAnalyzerGUI RadeonGPUAnalyzerCLI)
endif()

# Add Tests
if(AMD_INTERNAL)
    add_subdirectory (${PROJECT_SOURCE_DIR}/../RGA-Internal/Tests ${CMAKE_CURRENT_BINARY_DIR}/Tests)
endif()
