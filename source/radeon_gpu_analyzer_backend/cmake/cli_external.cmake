#######################################################################################################################
### Copyright (c) 2017-2026 Advanced Micro Devices, Inc. All rights reserved.
### @author AMD Developer Tools Team
#######################################################################################################################
include(FetchContent)

set (GITHUB_REPO_PREFIX "https://github.com/GPUOpen-Tools")
set (ISA_SPEC_MANAGER_BRANCH "512971f75854ef8496d5acbd5576ad5d082cdcb3")

if (NOT TARGET isa_decoder)
    if (NOT ISA_SPEC_MANAGER_DIR)
        # isa_spec_manager
        FetchContent_Declare(
                isa_spec_manager
                GIT_REPOSITORY "${GITHUB_REPO_PREFIX}/isa_spec_manager.git"
                GIT_TAG "${ISA_SPEC_MANAGER_BRANCH}"
                SOURCE_DIR "${PROJECT_SOURCE_DIR}/../../external/isa_spec_manager"
        )
        FetchContent_MakeAvailable(isa_spec_manager)
    else ()
        add_subdirectory(${ISA_SPEC_MANAGER_DIR} isa_spec_manager)
    endif ()
endif ()

# The isa_explorer target may not exist if isa_spec_manager was first included by
# qt_isa_gui with EXCLUDE_ISA_CLI_EXAMPLES_TESTS=ON. Add it explicitly when needed.
if (NOT TARGET isa_explorer)
    add_subdirectory("${PROJECT_SOURCE_DIR}/../../external/isa_spec_manager/source/isa_explorer" isa_explorer)
endif ()

set(ISA_DECODER_XML_URL "https://gpuopen.com/download/machine-readable-isa/latest/")
set(ISA_DECODER_XML_DIR "${PROJECT_SOURCE_DIR}/../../external/isa_spec_xml")

FetchContent_Declare(
       isa_spec_xml
       URL ${ISA_DECODER_XML_URL}
       SOURCE_DIR ${ISA_DECODER_XML_DIR}
       DOWNLOAD_EXTRACT_TIMESTAMP true
    )

message(STATUS "Downloading ${ISA_DECODER_XML_URL} into ${ISA_DECODER_XML_DIR}")

FetchContent_MakeAvailable(isa_spec_xml)

if (EXISTS "${ISA_DECODER_XML_DIR}")
    message(STATUS "ISA decoder XML directory created successfully at ${ISA_DECODER_XML_DIR}")
else ()
    message(FATAL_ERROR "Directory ${ISA_DECODER_XML_DIR} was not created.")
endif ()
