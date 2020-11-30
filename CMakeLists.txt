cmake_minimum_required (VERSION 3.10)
project (RGA)

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
    set(BUILD_CONFIG "Release")
    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(BUILD_CONFIG "Debug")
    endif()
    if (GUI_AUTOMATION)
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/Output_Test/${BUILD_CONFIG}/bin")
        set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/Output_Test/${BUILD_CONFIG}/lib")
        set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/Output_Test/${BUILD_CONFIG}/arch")
    else()
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/Output/${BUILD_CONFIG}/bin")
        set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/Output/${BUILD_CONFIG}/lib")
        set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/Output/${BUILD_CONFIG}/arch")
    endif()

elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")

    # Preprocessor directives
    add_definitions(-D_USRDLL -DUNICODE -D_UNICODE)

    # Output directories - use Windows path separaters so xcopy commands work
    if (GUI_AUTOMATION)
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}\\Output_Test\\$<$<CONFIG:debug>:Debug>$<$<CONFIG:release>:Release>\\bin)
        set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}\\Output_Test\\$<$<CONFIG:debug>:Debug>$<$<CONFIG:release>:Release>\\lib)
        set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}\\Output_Test\\$<$<CONFIG:debug>:Debug>$<$<CONFIG:release>:Release>\\arch)
    else()
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}\\Output\\$<$<CONFIG:debug>:Debug>$<$<CONFIG:release>:Release>\\bin)
        set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}\\Output\\$<$<CONFIG:debug>:Debug>$<$<CONFIG:release>:Release>\\lib)
        set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}\\Output\\$<$<CONFIG:debug>:Debug>$<$<CONFIG:release>:Release>\\arch)
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

# Add Common projects
add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/../Common/Src/AMDTBaseTools ${CMAKE_CURRENT_BINARY_DIR}/Common/Src/AMDTBaseTools)
add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/../Common/Src/AMDTOSWrappers ${CMAKE_CURRENT_BINARY_DIR}/Common/Src/AMDTOSWrappers)

# Disable specific MS compiler warnings for Common targets
if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    target_compile_options(AMDTBaseTools PRIVATE "/wd4309")
    target_compile_options(AMDTOSWrappers PRIVATE "/wd4275" "/wd4309")
endif()

# Add RGA CLI projects
if(NOT BUILD_GUI_ONLY)
    add_subdirectory (RadeonGPUAnalyzerBackend)
    add_subdirectory (RadeonGPUAnalyzerCLI)
    if (RGA_ENABLE_VULKAN)
        add_subdirectory (Core/Vulkan/Backend)
        add_dependencies (radeon_gpu_analyzer_cli VulkanBackend)
    endif()
    if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
        add_subdirectory (Core/DX12/Backend)
        add_dependencies (radeon_gpu_analyzer_cli dx12_backend)
    endif()
endif()

# Add RGA GUI
if(NOT BUILD_CLI_ONLY)
    add_subdirectory (RadeonGPUAnalyzerGUI)
    add_dependencies (RadeonGPUAnalyzer radeon_gpu_analyzer_cli)
endif()

# Add Tests
if(AMD_INTERNAL)
    add_subdirectory (${PROJECT_SOURCE_DIR}/../RGA-Internal/Tests ${CMAKE_CURRENT_BINARY_DIR}/Tests)
endif()
