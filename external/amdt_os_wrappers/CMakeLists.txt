cmake_minimum_required (VERSION 3.10)
project (AMDTOSWrappers)

# The version number
set (AMDTOSWRAPPERS_VERSION_MAJOR 2)
set (AMDTOSWRAPPERS_VERSION_MINOR 0)

# Project-specific preprocessor directives
add_definitions(-DAMDTOSWRAPPERS_EXPORTS -D_CRT_SECURE_NO_WARNINGS)

# Windows-specific preprocessor directives
if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    add_definitions(-DDBGHELP_TRANSLATE_TCHAR)
endif()

# Add this definition if static linkage is used with the other projects
add_definitions(-DAMDTBASETOOLS_STATIC)

# Configure a header file to pass some of the CMake settings
# to the source code
configure_file (
  "${PROJECT_SOURCE_DIR}/AMDTOSWrappersConfig.h.in"
  "${PROJECT_BINARY_DIR}/AMDTOSWrappersConfig.h"
  )

# Include direoctories
include_directories("${PROJECT_SOURCE_DIR}/Include")
include_directories("${PROJECT_SOURCE_DIR}/src")
include_directories("${PROJECT_SOURCE_DIR}/../../../Common/Src/Miniz")
include_directories("${PROJECT_SOURCE_DIR}/../")

# Windows-specific source files
file(GLOB WINDOWS_SRC
        "src/osProductVersion.cpp"

        # Common
        "src/common/osApplication.cpp"
        "src/common/osASCIIInputFileImpl.cpp"
        "src/common/osChannel.cpp"
        "src/common/osChannelOperators.cpp"
        "src/common/osCommunicationDebugManager.cpp"
        "src/common/osCommunicationDebugThread.cpp"
        "src/common/osCriticalSection.cpp"
        "src/common/osCriticalSectionLocker.cpp"
        "src/common/osDebuggingFunctions.cpp"
        "src/common/osDebugLog.cpp"
        "src/common/osDirectory.cpp"
        "src/common/osDNSQueryThread.cpp"
        "src/common/osEnvironmentVariable.cpp"
        "src/common/osFile.cpp"
        "src/common/osFileImpl.cpp"
        "src/common/osFileLauncher.cpp"
        "src/common/osFileLauncherThread.cpp"
        "src/common/osFilePath.cpp"
        "src/common/osFilePathByLastAccessDateCompareFunctor.cpp"
        "src/common/osInputFileImpl.cpp"
        "src/common/osModule.cpp"
        "src/common/osOutputFileImpl.cpp"
        "src/common/osProcess.cpp"
        "src/common/osRawMemoryStream.cpp"
        "src/common/osSettingsFileHandler.cpp"
        "src/common/osSocket.cpp"
        "src/common/osTime.cpp"
        "src/common/osTimeInterval.cpp"
        "src/common/osTransferableObject.cpp"
        "src/common/osTransferableObjectCreatorsBase.cpp"
        "src/common/osTransferableObjectCreatorsManager.cpp"

        # win-32
        "src/win32/osApplication.cpp"
        "src/win32/osCriticalSectionImpl.cpp"
        "src/win32/osDebuggingFunctions.cpp"
        "src/win32/osDirectory.cpp"
        "src/win32/osFile.cpp"
        "src/win32/osFileLauncher.cpp"
        "src/win32/osFilePath.cpp"
        "src/win32/osGeneralFunctions.cpp"
        "src/win32/osInputFileImpl.cpp"
        "src/win32/osMachine.cpp"
        "src/win32/osModule.cpp"
        "src/win32/osPortAddress.cpp"
        "src/win32/osProcess.cpp"
        "src/win32/osProcessSharedFile.cpp"
        "src/win32/osReadWriteLock.cpp"
        "src/win32/osSingleApplicationInstance.cpp"
        "src/win32/osStopWatch.cpp"
        "src/win32/osSystemError.cpp"
        "src/win32/osTCPSocket.cpp"
        "src/win32/osTCPSocketClient.cpp"
        "src/win32/osTCPSocketServer.cpp"
        "src/win32/osThread.cpp"
        "src/win32/osThreadLocalData.cpp"
        "src/win32/osTime.cpp"
        "src/win32/osTimer.cpp"
        "src/win32/osUnhandledExceptionHandler.cpp"
        "src/win32/osUser.cpp"
        "src/win32/osWin32Functions.cpp"
)

# Linux-specific source files
# Commented-out source files were left in this code intentionally -
# should consider removing those files from the repo.
file(GLOB LINUX_SRC
    "src/osProductVersion.cpp"
    "src/common/osASCIIInputFileImpl.cpp"
    "src/common/osApplication.cpp"
    "src/common/osChannel.cpp"
    "src/common/osChannelOperators.cpp"
    "src/common/osCommunicationDebugManager.cpp"
    "src/common/osCommunicationDebugThread.cpp"
    "src/common/osCriticalSectionLocker.cpp"
    "src/common/osCriticalSection.cpp"
    "src/common/osDebuggingFunctions.cpp"
    "src/common/osDebugLog.cpp"
    "src/common/osDirectory.cpp"
    "src/common/osDNSQueryThread.cpp"
    "src/common/osFileImpl.cpp"
    "src/common/osFileLauncher.cpp"
    "src/common/osFileLauncherThread.cpp"
    "src/common/osFilePath.cpp"
    "src/common/osFilePathByLastAccessDateCompareFunctor.cpp"
    "src/common/osFile.cpp"
    "src/common/osInputFileImpl.cpp"
    "src/common/osModule.cpp"
    "src/common/osMutexLocker.cpp"
    "src/common/osMutex.cpp"
    "src/common/osOutputFileImpl.cpp"
    "src/common/osProcess.cpp"
    "src/common/osRawMemoryStream.cpp"
    "src/common/osSettingsFileHandler.cpp"
    "src/common/osSocket.cpp"
    "src/common/osTime.cpp"
    "src/common/osTimeInterval.cpp"
    "src/common/osTransferableObjectCreatorsBase.cpp"
    "src/common/osTransferableObjectCreatorsManager.cpp"
    "src/common/osTransferableObject.cpp"
    "src/common/osEnvironmentVariable.cpp"
# linuxSources
    "src/linux/osCondition.cpp"
    "src/linux/osCriticalSectionImpl.cpp"
    "src/linux/osDirectory.cpp"
    "src/linux/osFile.cpp"
    "src/linux/osFileLauncher.cpp"
    "src/linux/osGeneralFunctions.cpp"
    "src/linux/osLinuxProcFileSystemReader.cpp"
    "src/linux/osModule.cpp"
    "src/linux/osMutexImpl.h"
    "src/linux/osMutexImpl.cpp"
    "src/linux/osPortAddress.cpp"
    "src/linux/osProcess.cpp"
    "src/linux/osProcessSharedFile.cpp"
    "src/linux/osReadWriteLock.cpp"
    "src/linux/osSingleApplicationInstance.cpp"
    "src/linux/osSocket.cpp"
    "src/linux/osStopWatch.cpp"
    "src/linux/osSystemError.cpp"
    "src/linux/osTCPSocketClient.cpp"
    "src/linux/osTCPSocketServer.cpp"
    "src/linux/osTCPSocket.cpp"
    "src/linux/osThread.cpp"
    "src/linux/osThreadLocalData.cpp"
    "src/linux/osTime.cpp"
    "src/linux/osTimer.cpp"
    "src/linux/osInputFileImpl.cpp"
    "src/linux/osUnhandledExceptionHandler.cpp"
    "src/linux/osUser.cpp"
# Generic Linux
    "src/linux/osApplication.cpp"
    "src/linux/osDebuggingFunctions.cpp"
    "src/linux/osFilePath.cpp"
    "src/linux/osGeneralFunctions.cpp"
    "src/linux/osMachine.cpp"
)

# Pick up the source files that are relevant to the platform
if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    add_library(AMDTOSWrappers STATIC ${WINDOWS_SRC})
    target_link_libraries(AMDTOSWrappers wsock32 ws2_32)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux" OR ${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    add_library(AMDTOSWrappers STATIC ${LINUX_SRC})
endif()

# Set binary suffix
set_target_properties(AMDTOSWrappers PROPERTIES DEBUG_POSTFIX -d CXX_STANDARD 17)

# Link settings
if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux" OR ${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    target_link_libraries (AMDTOSWrappers rt ${CMAKE_DL_LIBS} -pthread)
endif()

target_link_libraries (AMDTOSWrappers AMDTBaseTools)
