//=====================================================================
// Copyright 2016-2025 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osDirectorySerializer.h
///
//=====================================================================

//------------------------------ osDirectorySerializer.h ------------------------------

#ifndef __osDirectorySerializer
#define __osDirectorySerializer

//// Local:
#include <amdt_os_wrappers/Include/osOSWrappersDLLBuild.h>

// Infra:
#include <amdt_base_tools/Include/AMDTDefinitions.h>
#include <amdt_os_wrappers/Include/osDirectory.h>

//struct mz_zip_archive;
// ----------------------------------------------------------------------------------
// Class Name:           osDirectorySerializer
// General Description:  Archives and compresses a directory using Miniz archiving tool
// Author:               Naama Zur
// Creation Date:        31/12/2015
// ----------------------------------------------------------------------------------
class OS_API osDirectorySerializer
{
public:
    /// Ctor:
    osDirectorySerializer();

    /// Dtor:
    virtual ~osDirectorySerializer();

    /// Compresses a directory and all its content
    /// param sessionDir - the directory to compress
    /// param outFileFullPath - the full name to be assigned to the output path
    bool CompressDir(const osDirectory& sessionDir, const gtString& outFileFullPath);

    /// Decompresses a directory and all its content
    /// param compressedFile - the input file, containing a compressed archive created with this class
    /// param dstFolderPath - the destination path
    /// param archiveRootDir - output parameter which will carry the root dir of the decompression (equal to dstFolderPath if createRootDir is false)
    /// param createRootDir - specifies whether to create a root directory inside dstFolderPath
    bool DecompressDir(const gtString& compressedFile, const gtString& dstFolderPath, gtString& archiveRootDir, bool createRootDir = false);

private:
    // compression

    bool AddDirToZip(const osDirectory& sessionDir, const gtString& relativePath, void* pZip);
    bool AddFilesToZip(const osDirectory& sessionDir, const gtString& relativePath, void* pZip);
    //bool AddDirToZip(const osDirectory &sessionDir, const gtString& relativePath, mz_zip_archive* pZip);
    //bool AddFilesToZip(const osDirectory &sessionDir, const gtString& relativePath, mz_zip_archive* pZip);

    // decompression
    bool CreateOutputDir(gtString& dstFileFullPath, const gtString& compressedFile);

    // helper path manipulation methods
    void TrimFileExtension(gtString& dstFileFullPath);
    bool ValidateFileDirectoryExists(const gtString& fileFullPath);
    void ComposeItemFullPath(gtString& archiveRootDir, const char* relFileName, gtString& outItemName);
    void ComposeItemRelPath(const osDirectory& osDir, const gtString& relativePath, gtString& relPathAddition);

    gtString m_outFileFullPath;
};


#endif  // __osDirectorySerializer
