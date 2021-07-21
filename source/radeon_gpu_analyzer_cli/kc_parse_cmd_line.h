//=================================================================
// Copyright 2020 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================
#ifndef RGA_RADEONGPUANALYZERCLI_SRC_KC_PARSE_CMD_LINE_H_
#define RGA_RADEONGPUANALYZERCLI_SRC_KC_PARSE_CMD_LINE_H_

struct Config;

/// Parse the command line arguments
/// \param[in]  argc      the number of arguments
/// \param[in]  argv      the array of argument strings
/// \param[out] config_out the output config structure
/// \return true if successful, false otherwise
bool ParseCmdLine(int argc, char* argv[], Config& config_out);

#endif // RGA_RADEONGPUANALYZERCLI_SRC_KC_PARSE_CMD_LINE_H_
