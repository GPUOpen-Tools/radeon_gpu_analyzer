cmake_minimum_required (VERSION 2.6)
project (RGA)

# Boolean option for enabling AMD Internal build mode
option(AMD_INTERNAL "Add -DAMD_INTERNAL=ON for AMD Internal build mode." OFF)

# Define build level here
add_definitions(-DAMDT_PUBLIC -DCMAKE_BUILD)

# Identify the build architecture
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    # Target architecture is 64-bit
    option(CMAKE_64BIT_TARGET "On if the target architecture is 64-bit" ON)
endif()

# Linux specific configurations
if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")

    # Compiler configuration
    add_definitions(-std=c++11 -g -fmessage-length=0 -Wno-unknown-pragmas -pthread)

    # Generic preprocessor directives
    add_definitions(-DLINUX)
    add_definitions(-D_LINUX)

    # Debug-specific preprocessor directives
    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_definitions(-D_DEBUG)
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

# Continue to the following subdirectories
add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/../Common/Src/AMDTBaseTools CMAKE_CURRENT_BINARY_DIR}/../Common/Src/AMDTBaseTools)
add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/../Common/Src/AMDTOSWrappers CMAKE_CURRENT_BINARY_DIR}/../Common/Src/AMDTOSWrappers)
add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/../Common/Src/AMDTOSAPIWrappers CMAKE_CURRENT_BINARY_DIR}/../Common/Src/AMDTOSAPIWrappers)
add_subdirectory (RadeonGPUAnalyzerBackend)
add_subdirectory (RadeonGPUAnalyzerCLI)
if(AMD_INTERNAL)
    add_subdirectory (${PROJECT_SOURCE_DIR}/../RGA-Internal/Tests ${CMAKE_CURRENT_BINARY_DIR}/Tests)
endif()
