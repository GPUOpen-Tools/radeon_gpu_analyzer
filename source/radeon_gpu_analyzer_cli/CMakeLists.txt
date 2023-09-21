cmake_minimum_required (VERSION 3.10)
project (radeon_gpu_analyzer_cli)

# The version number
set (RadeonGPUAnalyzerCLI_VERSION_MAJOR 2)
set (RadeonGPUAnalyzerCLI_VERSION_MINOR 0)

# Windows-specific preprocessor directives
if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

# Identify if this script runs on a Debian system
set (IsDebian FALSE)
if(EXISTS "/etc/debian_version")
    set(IsDebian TRUE)
endif()

# Add this definitions if static linkage is used with the other projects
add_definitions(-DRGA_BACKEND_STATIC -DAMDTBASETOOLS_STATIC -DAMDTOSWRAPPERS_STATIC)

# Add update_check_api
add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/../../external/update_check_api ${CMAKE_CURRENT_BINARY_DIR}/external/update_check_api)

# Configure a header file to pass some of the CMake settings
# to the source code
configure_file (
  "${PROJECT_SOURCE_DIR}/RadeonGPUAnalyzerCLIConfig.h.in"
  "${PROJECT_BINARY_DIR}/RadeonGPUAnalyzerCLIConfig.h"
  )

# Add the binary tree to the search path for include files
# so that we will find ProjectConfig.h
include_directories("${PROJECT_BINARY_DIR}")

IF (WIN32)
    # get the location of the Visual Studio redist libraries - but don't create an install rule to install them
    # the libs will be copied manually as a post build step
    set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_SKIP TRUE)
    include(InstallRequiredSystemLibraries)
ENDIF(WIN32)

# Include directories
include_directories("${PROJECT_SOURCE_DIR}/../../external/dx11/")
include_directories("${PROJECT_SOURCE_DIR}/../common")
include_directories("${PROJECT_SOURCE_DIR}/../../external")
include_directories("${PROJECT_SOURCE_DIR}/../../external/third-party")
include_directories("${PROJECT_SOURCE_DIR}/../../external/dx11/GSA")
include_directories("${PROJECT_SOURCE_DIR}/../")
include_directories("${PROJECT_SOURCE_DIR}/../../")
include_directories("${PROJECT_SOURCE_DIR}/../../external/celf/Include")
include_directories("${PROJECT_SOURCE_DIR}/../../external/dynamic_library_module")
include_directories("${PROJECT_SOURCE_DIR}/../../external/appsdk/3-0/include")
include_directories("${PROJECT_SOURCE_DIR}/../../external/tsingleton")
include_directories("${PROJECT_SOURCE_DIR}/../../external/device_info")
include_directories("${PROJECT_SOURCE_DIR}/../../external/third_party")
include_directories("${PROJECT_SOURCE_DIR}/../../external/third_party/tinyxml2/Include")
include_directories(${UPDATECHECKAPI_INC_DIRS})

if (NOT IsDebian)
# On Debian we use the native Boost libraries
include_directories("${PROJECT_SOURCE_DIR}/../../external/third_party/Boost/boost_1_59_0")
endif()

# Windows-specific include directories
if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    include_directories("${PROJECT_SOURCE_DIR}/../../external/dx10_asm/include")
endif()

# Source files that are common for Windows and Linux
file(GLOB COMMON_SRC
    "../common/*.cpp"
    "*.cpp"
    "*.h"
    ${UPDATECHECKAPI_SRC}
    ${UPDATECHECKAPI_INC}
)

# Windows-specific include directories
if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    file(GLOB  WINDOWS_INC
        "../common/resources/*CLI.rc"
    )
endif()

# Pick up the source files that are relevant to the platform
if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    add_executable(radeon_gpu_analyzer_cli ${WINDOWS_INC} ${COMMON_SRC} ${WINDOWS_SRC})
    
    # 64-bit target
    find_library(BOOST_LIBRARY1 libboost_filesystem-vc140-mt-1_59 HINTS "${PROJECT_SOURCE_DIR}/../../external/third_party/Boost/boost_1_59_0/lib/VC14/x86_64")
    find_library(BOOST_LIBRARY2 libboost_system-vc140-mt-1_59 HINTS "${PROJECT_SOURCE_DIR}/../../external/third_party/Boost/boost_1_59_0/lib/VC14/x86_64")
    find_library(BOOST_LIBRARY3 libboost_program_options-vc140-mt-1_59 HINTS "${PROJECT_SOURCE_DIR}/../../external/third_party/Boost/boost_1_59_0/lib/VC14/x86_64")
    find_library(BOOST_LIBRARY4 libboost_regex-vc140-mt-1_59 HINTS "${PROJECT_SOURCE_DIR}/../../external/third_party/Boost/boost_1_59_0/lib/VC14/x86_64")
    find_library(BOOST_LIBRARY1D libboost_filesystem-vc140-mt-gd-1_59 HINTS "${PROJECT_SOURCE_DIR}/../../external/third_party/Boost/boost_1_59_0/lib/VC14/x86_64")
    find_library(BOOST_LIBRARY2D libboost_system-vc140-mt-gd-1_59 HINTS "${PROJECT_SOURCE_DIR}/../../external/third_party/Boost/boost_1_59_0/lib/VC14/x86_64")
    find_library(BOOST_LIBRARY3D libboost_program_options-vc140-mt-gd-1_59 HINTS "${PROJECT_SOURCE_DIR}/../../external/third_party/Boost/boost_1_59_0/lib/VC14/x86_64")
    find_library(BOOST_LIBRARY4D libboost_regex-vc140-mt-gd-1_59 HINTS "${PROJECT_SOURCE_DIR}/../../external/third_party/Boost/boost_1_59_0/lib/VC14/x86_64")
 
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux" OR ${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    # DeviceInfoLib objects are defined in radeon_gpu_analyzer_backend/CMakeLists.txt
    add_executable(radeon_gpu_analyzer_cli ${COMMON_SRC} ${LINUX_SRC} $<TARGET_OBJECTS:DeviceInfoLib>)
    set_target_properties(radeon_gpu_analyzer_cli PROPERTIES ENABLE_EXPORTS ON)
    
    if(IsDebian)
        # Debian-specific
        # Configure Boost link settings
        set(Boost_USE_STATIC_LIBS ON)
        set(Boost_USE_MULTITHREADED ON)
        set(Boost_USE_STATIC_RUNTIME OFF)

        # We use 1.55 as the minimum version
        find_package(Boost 1.55.0 COMPONENTS filesystem system regex program_options)

        if(Boost_FOUND)
            include_directories(${Boost_INCLUDE_DIRS})
        endif()
    else()
        # RHEL
        find_library(BOOST_LIBRARY1 "libboost_filesystem.a" HINTS "${PROJECT_SOURCE_DIR}/../../external/third_party/Boost/boost_1_59_0/lib/RHEL6/x86_64")
        find_library(BOOST_LIBRARY2 "libboost_system.a" HINTS "${PROJECT_SOURCE_DIR}/../../external/third_party/Boost/boost_1_59_0/lib/RHEL6/x86_64")
        find_library(BOOST_LIBRARY3 "libboost_program_options.a" HINTS "${PROJECT_SOURCE_DIR}/../../external/third_party/Boost/boost_1_59_0/lib/RHEL6/x86_64")
        find_library(BOOST_LIBRARY4 "libboost_regex.a" HINTS "${PROJECT_SOURCE_DIR}/../../external/third_party/Boost/boost_1_59_0/lib/RHEL6/x86_64")
    endif()

endif()

# Set Visual C++ warning level to 4
if(MSVC)
  target_compile_options(radeon_gpu_analyzer_cli PRIVATE /W4)
endif()

# Other libraries
if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    find_library(TINYXML_LIB "tinyxml2.lib" HINTS "${PROJECT_SOURCE_DIR}/../../external/third_party/tinyxml2/Lib/VS2015/Win64/Release/MD")
    find_library(YAMLCPP_LIB "libyaml-cppmd.lib" HINTS "${PROJECT_SOURCE_DIR}/../../external/yaml-cpp/lib/win64/Release")
    find_library(SPIRVCROSS_LIB "spirv-cross-core.lib" HINTS "${PROJECT_SOURCE_DIR}/../../external/vulkan/tools/windows/lib/Release")
    find_library(TINYXML_LIBD "tinyxml2-d.lib" HINTS "${PROJECT_SOURCE_DIR}/../../external/third_party/tinyxml2/Lib/VS2015/Win64/Debug/MDd")
    find_library(YAMLCPP_LIBD "libyaml-cppmdd.lib" HINTS "${PROJECT_SOURCE_DIR}/../../external/yaml-cpp/lib/win64/Debug")
    find_library(SPIRVCROSS_LIBD "spirv-cross-core.lib" HINTS "${PROJECT_SOURCE_DIR}/../../external/vulkan/tools/windows/lib/Debug")
else()
    find_library(TINYXML_LIB "libtinyxml2.a" HINTS "${PROJECT_SOURCE_DIR}/../../external/third_party/tinyxml2/Lib/Linux/x64/Release")
    find_library(YAMLCPP_LIB "libyaml-cpp.a" HINTS "${PROJECT_SOURCE_DIR}/../../external/yaml-cpp/lib/linux/gcc5")
    find_library(SPIRVCROSS_LIB "libspirv-cross-core.a" HINTS "${PROJECT_SOURCE_DIR}/../../external/vulkan/tools/linux/lib")
endif()

# Set the name of the executable
if(AMD_INTERNAL)
    if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
        set_target_properties(radeon_gpu_analyzer_cli PROPERTIES OUTPUT_NAME rga-internal)
    elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux" OR ${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
        # On Linux we have a launch script that configures the environment prior to launching the executable.
        set_target_properties(radeon_gpu_analyzer_cli PROPERTIES OUTPUT_NAME rga-bin-internal)
    endif()
else()
    if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
        set_target_properties(radeon_gpu_analyzer_cli PROPERTIES OUTPUT_NAME rga)
    elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux" OR ${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
        # On Linux we have a launch script that configures the environment prior to launching the executable.
        set_target_properties(radeon_gpu_analyzer_cli PROPERTIES OUTPUT_NAME rga-bin)
    endif()
endif()

# Link settings
if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    target_link_libraries (radeon_gpu_analyzer_cli radeon_gpu_analyzer_backend AMDTOSWrappers AMDTBaseTools
                           optimized ${BOOST_LIBRARY1}
                           optimized ${BOOST_LIBRARY2}
                           optimized ${BOOST_LIBRARY3}
                           optimized ${BOOST_LIBRARY4}
                           optimized ${TINYXML_LIB}
                           optimized ${YAMLCPP_LIB}
                           optimized ${SPIRVCROSS_LIB}
                           debug ${BOOST_LIBRARY1D}
                           debug ${BOOST_LIBRARY2D}
                           debug ${BOOST_LIBRARY3D}
                           debug ${BOOST_LIBRARY4D}
                           debug ${TINYXML_LIBD}
                           debug ${YAMLCPP_LIBD}
                           debug ${SPIRVCROSS_LIBD}
                           general ${UPDATECHECKAPI_LIBS})
else()
    if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
        target_link_libraries (radeon_gpu_analyzer_cli radeon_gpu_analyzer_backend AMDTOSWrappers AMDTBaseTools
                               ${Boost_LIBRARIES}
                               ${TINYXML_LIB} ${YAMLCPP_LIB} ${SPIRVCROSS_LIB}
                               ${UPDATECHECKAPI_LIBS})
    else()
        target_link_libraries (radeon_gpu_analyzer_cli radeon_gpu_analyzer_backend AMDTOSWrappers AMDTBaseTools
                               ${BOOST_LIBRARY1} ${BOOST_LIBRARY3}
                               ${TINYXML_LIB} ${YAMLCPP_LIB} ${SPIRVCROSS_LIB}
                               ${UPDATECHECKAPI_LIBS})
    endif()
endif()

# Configure the post-build actions.
set(INTERNAL_ARG "")
if(AMD_INTERNAL)
    set(INTERNAL_ARG "-internal")
endif()

if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    set(DEBUG_ARG "-debug")
    set(REL_ARG "")
endif()

# Copy text files into build folder
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PROJECT_SOURCE_DIR}/../../EULA.txt $<TARGET_FILE_DIR:${PROJECT_NAME}>/EULA.txt
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PROJECT_SOURCE_DIR}/../../README.md $<TARGET_FILE_DIR:${PROJECT_NAME}>/README.md
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PROJECT_SOURCE_DIR}/../../LICENSE.txt $<TARGET_FILE_DIR:${PROJECT_NAME}>/LICENSE.txt
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PROJECT_SOURCE_DIR}/../../RGA_NOTICES.txt $<TARGET_FILE_DIR:${PROJECT_NAME}>/RGA_NOTICES.txt
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PROJECT_SOURCE_DIR}/../../documentation/RGA_RELEASE_NOTES.txt $<TARGET_FILE_DIR:${PROJECT_NAME}>/RGA_RELEASE_NOTES.txt
)

# Copy external compiler tools into Linux build folder.
if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux" OR ${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "Copying external files to $<TARGET_FILE_DIR:${PROJECT_NAME}>/"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PROJECT_SOURCE_DIR}/../../external/update_check_api/rtda/linux/rtda $<TARGET_FILE_DIR:${PROJECT_NAME}>/rtda
        COMMAND chmod +x $<TARGET_FILE_DIR:${PROJECT_NAME}>/rtda
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PROJECT_SOURCE_DIR}/../../external/opengl/glc/linux/glc $<TARGET_FILE_DIR:${PROJECT_NAME}>/utils/glc
        COMMAND chmod +x $<TARGET_FILE_DIR:${PROJECT_NAME}>/utils/glc
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PROJECT_SOURCE_DIR}/../../source/utils/shader_analysis/linux/x64/shae $<TARGET_FILE_DIR:${PROJECT_NAME}>/utils/shae
        COMMAND chmod +x $<TARGET_FILE_DIR:${PROJECT_NAME}>/utils/shae
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PROJECT_SOURCE_DIR}/../../external/vulkan_offline/linux/amdllpc $<TARGET_FILE_DIR:${PROJECT_NAME}>/utils/amdllpc
        COMMAND chmod +x $<TARGET_FILE_DIR:${PROJECT_NAME}>/utils/amdllpc
        COMMAND ${CMAKE_COMMAND} -E echo "Copying shell scripts to $<TARGET_FILE_DIR:${PROJECT_NAME}>/"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PROJECT_SOURCE_DIR}/../../build/util/linux/rga $<TARGET_FILE_DIR:${PROJECT_NAME}>/rga
        COMMAND chmod +x $<TARGET_FILE_DIR:${PROJECT_NAME}>/rga
    )

    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "Copying OpenCL files to $<TARGET_FILE_DIR:${PROJECT_NAME}>/utils/lc"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PROJECT_SOURCE_DIR}/../../external/lc/disassembler/linux/amdgpu-dis $<TARGET_FILE_DIR:${PROJECT_NAME}>/utils/lc/disassembler/amdgpu-dis
        COMMAND chmod +x $<TARGET_FILE_DIR:${PROJECT_NAME}>/utils/lc/disassembler/amdgpu-dis
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/../../external/lc/opencl/linux/bin $<TARGET_FILE_DIR:${PROJECT_NAME}>/utils/lc/opencl/bin/
        COMMAND chmod +x $<TARGET_FILE_DIR:${PROJECT_NAME}>/utils/lc/opencl/bin/clang-18
        COMMAND chmod +x $<TARGET_FILE_DIR:${PROJECT_NAME}>/utils/lc/opencl/bin/lld
        COMMAND chmod +x $<TARGET_FILE_DIR:${PROJECT_NAME}>/utils/lc/opencl/bin/llvm-readobj
        COMMAND ln -s -f ./clang-18 $<TARGET_FILE_DIR:${PROJECT_NAME}>/utils/lc/opencl/bin/clang
        COMMAND ln -s -f ./lld $<TARGET_FILE_DIR:${PROJECT_NAME}>/utils/lc/opencl/bin/ld.lld
        COMMAND ln -s -f ./llvm-readobj $<TARGET_FILE_DIR:${PROJECT_NAME}>/utils/lc/opencl/bin/llvm-readelf
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PROJECT_SOURCE_DIR}/../../external/lc/opencl/linux/include/opencl-c.h $<TARGET_FILE_DIR:${PROJECT_NAME}>/utils/lc/opencl/include/opencl-c.h
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PROJECT_SOURCE_DIR}/../../external/lc/opencl/linux/include/opencl-c-base.h $<TARGET_FILE_DIR:${PROJECT_NAME}>/utils/lc/opencl/lib/clang/18/include/opencl-c-base.h
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PROJECT_SOURCE_DIR}/../../external/lc/opencl/additional-targets $<TARGET_FILE_DIR:${PROJECT_NAME}>/utils/lc/opencl/additional-targets
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/../../external/lc/opencl/linux/lib/bitcode $<TARGET_FILE_DIR:${PROJECT_NAME}>/utils/lc/opencl/lib/bitcode/
    )

    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "Copying Vulkan files to $<TARGET_FILE_DIR:${PROJECT_NAME}>/utils/vulkan"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PROJECT_SOURCE_DIR}/../../external/vulkan/tools/linux/bin/glslangValidator $<TARGET_FILE_DIR:${PROJECT_NAME}>/utils/vulkan/glslangValidator
        COMMAND chmod +x $<TARGET_FILE_DIR:${PROJECT_NAME}>/utils/vulkan/glslangValidator
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PROJECT_SOURCE_DIR}/../../external/vulkan/tools/linux/bin/spirv-as $<TARGET_FILE_DIR:${PROJECT_NAME}>/utils/vulkan/spirv-as
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PROJECT_SOURCE_DIR}/../../external/vulkan/tools/linux/bin/spirv-dis $<TARGET_FILE_DIR:${PROJECT_NAME}>/utils/vulkan/spirv-dis
        COMMAND chmod +x $<TARGET_FILE_DIR:${PROJECT_NAME}>/utils/vulkan/spirv-as
        COMMAND chmod +x $<TARGET_FILE_DIR:${PROJECT_NAME}>/utils/vulkan/spirv-dis
    )

endif()

# Copy VC redist files into Windows build folder.
# Copy external compiler tools into Windows build folder.
IF(WIN32)
    target_compile_options(radeon_gpu_analyzer_cli PRIVATE /W4)
    # Copy the VisualStudio redist files
    # the list of all redist files is contained in the variable CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS. We only want to copy
    # a subset of these so we extract the directory from the first entry and then manually copy the files we want
    foreach(VSLIB ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS})
        message("VSLIB is ${VSLIB}")
        get_filename_component(VSREDISTDIR ${VSLIB} DIRECTORY)
        message("Visual Studio redistributable files directory = ${VSREDISTDIR}")
        break()
    endforeach()

    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "Copying Visual Studio redist files from ${VSREDISTDIR} to $<TARGET_FILE_DIR:${PROJECT_NAME}>"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${VSREDISTDIR}/msvcp140.dll $<TARGET_FILE_DIR:${PROJECT_NAME}>/msvcp140.dll
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${VSREDISTDIR}/msvcp140_1.dll $<TARGET_FILE_DIR:${PROJECT_NAME}>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${VSREDISTDIR}/concrt140.dll $<TARGET_FILE_DIR:${PROJECT_NAME}>/concrt140.dll
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${VSREDISTDIR}/vccorlib140.dll $<TARGET_FILE_DIR:${PROJECT_NAME}>/vccorlib140.dll
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${VSREDISTDIR}/vcruntime140.dll $<TARGET_FILE_DIR:${PROJECT_NAME}>/vcruntime140.dll
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${VSREDISTDIR}/vcruntime140_1.dll $<TARGET_FILE_DIR:${PROJECT_NAME}>/vcruntime140_1.dll
    )
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "Copying external files to $<TARGET_FILE_DIR:${PROJECT_NAME}>/utils"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PROJECT_SOURCE_DIR}/../../external/update_check_api/rtda/windows/rtda.exe $<TARGET_FILE_DIR:${PROJECT_NAME}>/rtda.exe
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PROJECT_SOURCE_DIR}/../../external/opengl/glc/windows/glc.exe $<TARGET_FILE_DIR:${PROJECT_NAME}>/utils/glc.exe
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PROJECT_SOURCE_DIR}/../../source/utils/shader_analysis/windows/x64/shae.exe $<TARGET_FILE_DIR:${PROJECT_NAME}>/utils/shae.exe
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PROJECT_SOURCE_DIR}/../../source/utils/dx11/bin/dx11_adapter.exe $<TARGET_FILE_DIR:${PROJECT_NAME}>/utils/dx11_adapter.exe
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PROJECT_SOURCE_DIR}/../../external/vulkan_offline/windows/amdllpc.exe $<TARGET_FILE_DIR:${PROJECT_NAME}>/utils/amdllpc.exe
        COMMAND ${CMAKE_COMMAND} -E echo "Copying $(WindowsSdkDir_10)bin/$(WindowsTargetPlatformVersion)/x64/d3dcompiler_47.dll"
        COMMAND ${CMAKE_COMMAND} -E copy \"$(WindowsSdkDir_10)bin/$(WindowsTargetPlatformVersion)/x64/d3dcompiler_47.dll\" $<TARGET_FILE_DIR:${PROJECT_NAME}>/utils/
    )

    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "Copying OpenCL files to $<TARGET_FILE_DIR:${PROJECT_NAME}>/utils/lc"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PROJECT_SOURCE_DIR}/../../external/lc/disassembler/windows/amdgpu-dis.exe $<TARGET_FILE_DIR:${PROJECT_NAME}>/utils/lc/disassembler/amdgpu-dis.exe
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/../../external/lc/opencl/windows/bin $<TARGET_FILE_DIR:${PROJECT_NAME}>/utils/lc/opencl/bin/
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PROJECT_SOURCE_DIR}/../../external/lc/opencl/windows/include/opencl-c.h $<TARGET_FILE_DIR:${PROJECT_NAME}>/utils/lc/opencl/include/opencl-c.h
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PROJECT_SOURCE_DIR}/../../external/lc/opencl/windows/include/opencl-c-base.h $<TARGET_FILE_DIR:${PROJECT_NAME}>/utils/lc/opencl/lib/clang/18/include/opencl-c-base.h
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PROJECT_SOURCE_DIR}/../../external/lc/opencl/additional-targets $<TARGET_FILE_DIR:${PROJECT_NAME}>/utils/lc/opencl/additional-targets
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/../../external/lc/opencl/windows/lib/bitcode $<TARGET_FILE_DIR:${PROJECT_NAME}>/utils/lc/opencl/lib/bitcode/
    )

    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "Copying Vulkan and DX12 tools to $<TARGET_FILE_DIR:${PROJECT_NAME}>/utils"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PROJECT_SOURCE_DIR}/../../external/vulkan/tools/windows/bin/glslangValidator.exe $<TARGET_FILE_DIR:${PROJECT_NAME}>/utils/vulkan/glslangValidator.exe
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PROJECT_SOURCE_DIR}/../../external/vulkan/tools/windows/bin/spirv-as.exe $<TARGET_FILE_DIR:${PROJECT_NAME}>/utils/vulkan/spirv-as.exe
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PROJECT_SOURCE_DIR}/../../external/vulkan/tools/windows/bin/spirv-dis.exe $<TARGET_FILE_DIR:${PROJECT_NAME}>/utils/vulkan/spirv-dis.exe
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PROJECT_SOURCE_DIR}/../../external/dxc/dxc.exe $<TARGET_FILE_DIR:${PROJECT_NAME}>/utils/dx12/dxc/dxc.exe
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PROJECT_SOURCE_DIR}/../../external/dxc/dxcompiler.dll $<TARGET_FILE_DIR:${PROJECT_NAME}>/utils/dx12/dxc/dxcompiler.dll
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/../../external/dx12/agility_sdk $<TARGET_FILE_DIR:${PROJECT_NAME}>/utils/dx12/agility_sdk/
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PROJECT_SOURCE_DIR}/../../external/dxc/dxil.dll $<TARGET_FILE_DIR:${PROJECT_NAME}>/utils/dx12/dxc/dxil.dll
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PROJECT_SOURCE_DIR}/../../external/dx12/withdll.exe $<TARGET_FILE_DIR:${PROJECT_NAME}>/utils/dx12/withdll.exe
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PROJECT_SOURCE_DIR}/../../external/dx12/umdrepoint.dll $<TARGET_FILE_DIR:${PROJECT_NAME}>/utils/dx12/umdrepoint.dll
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PROJECT_SOURCE_DIR}/../../external/dx11/amdxx/atidxx64.dll $<TARGET_FILE_DIR:${PROJECT_NAME}>/utils/dx11/atidxx64.dll
    )
ENDIF(WIN32)

