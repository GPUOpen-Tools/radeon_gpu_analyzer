//=================================================================
// Copyright 2017 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

#ifndef _PARSE_CMD_LINE_H_
#define _PARSE_CMD_LINE_H_

class Config;

/// Parse the command line arguments
/// \param[in]  argc      the number of arguments
/// \param[in]  argv      the array of argument strings
/// \param[out] configOut the output config structure
/// \return true if successful, false otherwise
bool ParseCmdLine(int argc, char* argv[], Config& configOut);

#endif
