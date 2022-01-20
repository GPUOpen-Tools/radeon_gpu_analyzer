#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_OUTPUT_FILE_UTILS_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_OUTPUT_FILE_UTILS_H_

// Local.
#include "source/common/vulkan/rg_pipeline_types.h"
#include "radeon_gpu_analyzer_gui/rg_data_types.h"

class RgOutputFileUtils
{
public:
    // Parse the live VGPR data.
    static bool ParseLiveVgprsData(const std::string&                                   live_vgpr_file_full_path,
                                   std::vector<std::shared_ptr<RgIsaLine>>&             disassembled_isa_lines,
                                   std::vector<std::shared_ptr<RgIsaLineInstruction>>&  vgpr_isa_lines,
                                   std::vector<QString>&                                vgpr_file_lines,
                                   RgLiveregData&                                       livereg_data);

    // Parse the live VGPR ISA line.
    static void ParseLiveVgprIsaLine(const QString& livereg_line, std::vector<std::shared_ptr<RgIsaLineInstruction>>& vgpr_isa_lines, bool is_label);

    // Get the architecture specific information.
    static bool GetArchInformation(RgLiveregData& livereg_data,
                                   std::vector<QString>& vgpr_file_lines);

    // Calculate the maximum number of VGPRs for this shader.
    static int CalculateMaxVgprs(const std::vector<std::shared_ptr<RgIsaLineInstruction>>& vgpr_isa_lines);

    // Find out if this is a valid line.
    static bool IsValidLine(const QString& livereg_line);

    // Find out is this is a label line.
    static bool IsLabelLine(const QString& livereg_line);

    // Read the live VGPR output file.
    static bool ReadLiveVgprsFile(const std::string& live_vgpr_file_full_path, std::vector<QString>& vgpr_file_lines);

private:
    RgOutputFileUtils()  = delete;
    ~RgOutputFileUtils() = delete;
};
#endif  // RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_OUTPUT_FILE_UTILS_H_