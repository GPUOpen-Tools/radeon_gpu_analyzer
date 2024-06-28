//=================================================================
// Copyright 2020 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

// C++.
#include <cassert>
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <algorithm>
#include <iterator>
#include <codecvt>
#include <cctype>

// Local.
#include "rg_dx12_utils.h"

namespace rga
{
    // *** CONSTANTS - START ***

    static const char* kStrErrTextFileWriteFailed = "Error: failed to write text file: ";
    static const char* kStrErrTextFileReadFailed = "Error: failed to read text file: ";
    static const char* kStrErrBinaryFileWriteFailed = "Error: failed to write binary file: ";

    // Schema constants.
    const char* RgDx12Utils::kStrElemSchemaVersion               = "schemaVersion";
    const char* RgDx12Utils::kStrElemSchemaVersion10             = "1.0";
    const char* RgDx12Utils::kStrElemSchemaInputLayoutNum        = "InputLayoutNumElements";
    const char* RgDx12Utils::kStrElemSchemaInputLayout           = "InputLayout";
    const char* RgDx12Utils::kStrElemSchemaPrimitiveTopologyType = "PrimitiveTopologyType";
    const char* RgDx12Utils::kStrElemSchemaNumRenderTargets      = "NumRenderTargets";
    const char* RgDx12Utils::kStrElemSchemaRtvFormats            = "RTVFormats";

    // *** CONSTANTS - END ***

    // Splits comma-separated values wrapped in '{' '}', e.g. { X, Y, Z }.
    bool static ExtractCurlyBracketedValues(const std::string& line, std::vector<std::string>& input_layout_values)
    {
        bool ret = false;
        size_t start_pos = line.find('{');
        size_t end_pos = line.find('}');
        assert(start_pos != std::string::npos);
        assert(end_pos != std::string::npos);
        assert(start_pos + 1 < line.size());
        assert(end_pos > 0);

        if ((start_pos != std::string::npos) && (end_pos != std::string::npos) &&
            (start_pos + 1 < line.size()) && (end_pos > 0))
        {
            std::string value_string = line.substr(start_pos + 1, line.size() - 2);
            RgDx12Utils::SplitString(value_string, ',', input_layout_values);
            for (int32_t i = 0; i < input_layout_values.size(); i++)
            {
                input_layout_values[i] = RgDx12Utils::TrimWhitespace(input_layout_values[i]);
            }
            ret = true;
        }

        return ret;
    }

    bool RgDx12Utils::WriteTextFile(const std::string& filename, const std::string& content)
    {
        bool ret = false;
        std::ofstream output;
        output.open(filename.c_str());

        if (output.is_open())
        {
            output << content << std::endl;
            output.close();
            ret = true;
        }
        else
        {
            std::cerr << kStrErrTextFileWriteFailed << filename << std::endl;
        }

        return ret;
    }

    bool RgDx12Utils::ReadTextFile(const std::string& filename, std::string& content)
    {
        bool ret = false;
        std::ifstream input;
        input.open(filename.c_str());

        if (input.is_open())
        {
            std::stringstream  text_stream;
            text_stream << input.rdbuf();
            content = text_stream.str();
            input.close();
            ret = true;
        }
        else
        {
            std::cout << kStrErrTextFileReadFailed << filename << std::endl;
        }

        return ret;
    }

    bool RgDx12Utils::ReadBinaryFile(const std::string& filename, std::vector<char>& content)
    {
        bool ret = false;
        std::ifstream input;
        input.open(filename.c_str(), std::ios::binary);

        if (input.is_open())
        {
            content = std::vector<char>(std::istreambuf_iterator<char>(input), {});
            ret = !content.empty();
        }
        return ret;
    }

    bool RgDx12Utils::ReadBinaryFile(const std::string& filename, std::vector<unsigned char>& content)
    {
        bool ret = false;
        std::ifstream input;
        input.open(filename.c_str(), std::ios::binary);

        if (input.is_open())
        {
            content = std::vector<unsigned char>(std::istreambuf_iterator<char>(input), {});
            ret = !content.empty();
        }
        return ret;
    }

    bool RgDx12Utils::IsFileExists(const std::string& full_path)
    {
        std::ifstream infile(full_path);
        return infile.good();
    }

    void RgDx12Utils::SplitString(const std::string& str, char delim, std::vector<std::string>& dst)
    {
        std::stringstream ss;
        ss.str(str);
        std::string substr;
        while (std::getline(ss, substr, delim))
        {
            dst.push_back(substr);
        }
    }

    std::string RgDx12Utils::ToLower(const std::string& str)
    {
        std::string lstr = str;
        std::transform(lstr.begin(), lstr.end(), lstr.begin(), [](const char& c) { return static_cast<char>(std::tolower(c)); });
        return lstr;
    }

    void RgDx12Utils::InitGraphicsPipelineStateDesc(D3D12_GRAPHICS_PIPELINE_STATE_DESC& pso_desc)
    {
        // Reset the structure.
        memset(&pso_desc, 0, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));

        // Default D3D12 rasterizer state.
        D3D12_RASTERIZER_DESC default_dx12_rasterizer_state;
        default_dx12_rasterizer_state.FillMode = D3D12_FILL_MODE_SOLID;
        default_dx12_rasterizer_state.CullMode = D3D12_CULL_MODE_BACK;
        default_dx12_rasterizer_state.FrontCounterClockwise = FALSE;
        default_dx12_rasterizer_state.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
        default_dx12_rasterizer_state.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
        default_dx12_rasterizer_state.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
        default_dx12_rasterizer_state.DepthClipEnable = TRUE;
        default_dx12_rasterizer_state.MultisampleEnable = FALSE;
        default_dx12_rasterizer_state.AntialiasedLineEnable = FALSE;
        default_dx12_rasterizer_state.ForcedSampleCount = 0;
        default_dx12_rasterizer_state.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

        // Default D3D12 blend state.
        D3D12_BLEND_DESC default_dx12_blend_state;
        default_dx12_blend_state.AlphaToCoverageEnable = FALSE;
        default_dx12_blend_state.IndependentBlendEnable = FALSE;
        const D3D12_RENDER_TARGET_BLEND_DESC default_render_target_blend_desc =
        {
            FALSE,FALSE,
            D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
            D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
            D3D12_LOGIC_OP_NOOP,
            D3D12_COLOR_WRITE_ENABLE_ALL,
        };
        for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
        {
            default_dx12_blend_state.RenderTarget[i] = default_render_target_blend_desc;
        }

        // Default D3D12 depth stencil state.
        D3D12_DEPTH_STENCIL_DESC default_dx12_depth_stencil_state;
        default_dx12_depth_stencil_state.DepthEnable = TRUE;
        default_dx12_depth_stencil_state.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
        default_dx12_depth_stencil_state.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
        default_dx12_depth_stencil_state.StencilEnable = FALSE;
        default_dx12_depth_stencil_state.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
        default_dx12_depth_stencil_state.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
        const D3D12_DEPTH_STENCILOP_DESC defaultStencilOp =
        { D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_COMPARISON_FUNC_ALWAYS };
        default_dx12_depth_stencil_state.FrontFace = defaultStencilOp;
        default_dx12_depth_stencil_state.BackFace = defaultStencilOp;

        // Set the D3D12 defaults.
        pso_desc.BlendState = default_dx12_blend_state;
        pso_desc.DepthStencilState = default_dx12_depth_stencil_state;
        pso_desc.RasterizerState = default_dx12_rasterizer_state;

        pso_desc.SampleMask = UINT_MAX;

        // Assume triangle primitive topology type.
        pso_desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

        // Assume single render target.
        pso_desc.NumRenderTargets = 1;
        pso_desc.RTVFormats[0] = DXGI_FORMAT_R32G32B32A32_FLOAT;

        // Assume a single multisample per pixel.
        pso_desc.SampleDesc.Count = 1;
    }

    bool RgDx12Utils::ParseGpsoFile(const std::string& filename, D3D12_GRAPHICS_PIPELINE_STATE_DESC& pso_desc)
    {
        // Error messages.
        const char* kStrErrorUnknownSchemaVersion               = "Error: unknown schema version: ";
        const char* kStrErrorUnrecognizedInputClassification    = "Error: unrecognized Input Classification ";
        const char* kStrErrorFailedToParseGpsoFile              = "Error: failed to parse .gpso file.";
        const char* kStrErrorInvalidNumInputLayoutValues1       = "Error: invalid number of input items between \"{}\" for line \"";
        const char* kStrErrorInvalidNumInputLayoutValues2       = "\". Expected 7, found ";
        const char* kStrErrorInvalidNumInputLayoutValues3       = ".";
        const char* kStrErrorFailedToParseInputLayoutDefinition = "Error: unable to parse input layout definition: ";
        const char* kStrErrorFailedToParseRtvFormatValues       = "Error: failed to parse the RTV format values, line ";

        // Warning messages.
        const char* kStrWarningZeroInputLayoutElementItems        = "Warning: number of input layout element items is zero.";
        const char* kStrWarningUnrecognizedDxgiFormat1            = "Warning: unrecognized DXGI Format \"";
        const char* kStrWarningUnrecognizedDxgiFormat2            = "\", assuming DXGI_FORMAT_UNKNOWN.";
        const char* kStrWarningUnrecognizedPrimitiveTopologyType1 = "Error: unrecognized primitive topology type: \"";
        const char* kStrWarningUnrecognizedPrimitiveTopologyType2 = "\", assuming D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED.";
        const char* kStrWarningRenderTargetsMismatch              = "Warning: mismatch between number of RTV format values and the NumRenderTargets.";
        const char* kStrWarningNumRenderTargetsExceedsMax         = "Warning: NumRenderTargets exceeds max of 8, assuming 8.";

        // Container for input element descriptors.
        D3D12_INPUT_ELEMENT_DESC* elem = {};

        // Number of input layout elements.
        uint32_t num_input_layout_elems = 0;
        std::string gpso_content;
        bool ret = false;

        // A flag indicating if we should abort
        // the processing process due to an error.
        bool should_abort = false;

        // Flags that indicate whether we covered the relevant blocks.
        bool is_version_checked = false;
        bool is_input_layout_num_elements_checked = false;
        bool is_input_layout_checked = false;
        bool is_primitive_toplogy_type_checked = false;
        bool is_num_render_targets_checked = false;
        bool is_rtv_formats_checked = false;

        // Read the input file
        bool is_file_read = ReadTextFile(filename, gpso_content);
        assert(is_file_read);
        if (is_file_read)
        {
            try
            {
                std::istringstream f(gpso_content);
                std::string line;
                while (!should_abort && std::getline(f, line))
                {
                    // Skip empty lines.
                    if (line.empty())
                    {
                        continue;
                    }

                    if (line.find('#') == 0)
                    {
                        // Schema version.
                        if (!is_version_checked && line.find(kStrElemSchemaVersion) != std::string::npos)
                        {
                            while (std::getline(f, line))
                            {
                                if (line.empty())
                                {
                                    // Skip empty lines.
                                    continue;
                                }
                                else
                                {
                                    // Verify that we are processing a known schema.
                                    assert(line.compare(kStrElemSchemaVersion10) == 0);
                                    if (line.compare(kStrElemSchemaVersion10) != 0)
                                    {
                                        std::cout << kStrErrorUnknownSchemaVersion << line << std::endl;
                                        should_abort = true;
                                    }
                                    else
                                    {
                                        // Version verified.
                                        is_version_checked = true;
                                    }

                                    // Continue to the next element.
                                    break;
                                }
                            }
                        }
                        else if (!is_input_layout_num_elements_checked &&
                            line.find(kStrElemSchemaInputLayoutNum) != std::string::npos)
                        {
                            while (std::getline(f, line))
                            {
                                if (line.empty())
                                {
                                    // Skip empty lines.
                                    continue;
                                }
                                else
                                {
                                    std::string numElements = TrimWhitespace(line);
                                    num_input_layout_elems = std::stoi(numElements);
                                    if (num_input_layout_elems == 0)
                                    {
                                        std::cout << kStrWarningZeroInputLayoutElementItems << std::endl;
                                        should_abort = false;
                                    }
                                    is_input_layout_num_elements_checked = true;
                                    break;
                                }
                            }
                        }
                        else if (!is_input_layout_checked && line.find(kStrElemSchemaInputLayout) != std::string::npos)
                        {
                            if (num_input_layout_elems > 0)
                            {
                                // Allocate the array of InputLayout elements.
                                elem = new D3D12_INPUT_ELEMENT_DESC[num_input_layout_elems]{0};

                                for (uint32_t i = 0; i < num_input_layout_elems; i++)
                                {
                                    while (std::getline(f, line))
                                    {
                                        if (line.empty())
                                        {
                                            // Skip empty lines.
                                            continue;
                                        }
                                        else
                                        {
                                            std::vector<std::string> input_layout_values;
                                            bool                     is_input_layout_parsed = ExtractCurlyBracketedValues(line, input_layout_values);
                                            assert(is_input_layout_parsed);
                                            if (is_input_layout_parsed)
                                            {
                                                assert(input_layout_values.size() == 7);
                                                if (input_layout_values.size() == 7)
                                                {
                                                    // SemanticName.
                                                    std::string semantic_name = TrimWhitespaceAndCommas(input_layout_values[0]);
                                                    char*       semantic      = new char[semantic_name.size() + 1]{0};
                                                    std::copy(semantic_name.begin(), semantic_name.end(), semantic);
                                                    elem[i].SemanticName = semantic;

                                                    // SemanticIndex.
                                                    std::string semantic_index = TrimWhitespace(input_layout_values[1]);
                                                    int         index          = std::stoi(semantic_index);
                                                    elem[i].SemanticIndex      = index;

                                                    // Format.
                                                    std::string format_str = TrimWhitespace(input_layout_values[2]);
                                                    DXGI_FORMAT dxgi_format;
                                                    bool        is_format_found = StrToDxgiFormat(format_str, dxgi_format);
                                                    assert(is_format_found);

                                                    if (is_format_found)
                                                    {
                                                        elem[i].Format = dxgi_format;

                                                        // InputSlot.
                                                        std::string input_slot_str = TrimWhitespace(input_layout_values[3]);
                                                        int         input_slot     = std::stoi(input_slot_str);
                                                        elem[i].InputSlot          = input_slot;

                                                        // AlignedByteOffset.
                                                        std::string aligned_Byte_offset_str = TrimWhitespace(input_layout_values[4]);
                                                        int         aligned_byte_offset     = 0;
                                                        try
                                                        {
                                                            aligned_byte_offset       = std::stoi(aligned_Byte_offset_str);
                                                            elem[i].AlignedByteOffset = aligned_byte_offset;
                                                        }
                                                        catch (const std::exception& e)
                                                        {
                                                            // Handle known special cases.
                                                            if (aligned_Byte_offset_str.compare("D3D12_APPEND_ALIGNED_ELEMENT") == 0)
                                                            {
                                                                aligned_byte_offset = 0xffffffff;
                                                            }
                                                            else
                                                            {
                                                                // No special case found - parsing error.
                                                                throw e;
                                                            }
                                                        }

                                                        // Input slot classification.
                                                        std::string                input_classification_str = TrimWhitespace(input_layout_values[5]);
                                                        D3D12_INPUT_CLASSIFICATION input_classification;
                                                        bool                       input_classification_found =
                                                            StrToInputClassification(input_classification_str, input_classification);
                                                        assert(input_classification_found);

                                                        if (input_classification_found)
                                                        {
                                                            elem[i].InputSlotClass = input_classification;

                                                            // InstanceDataStepRate.
                                                            std::string instance_data_step_rate_str = TrimWhitespace(input_layout_values[6]);
                                                            int         instance_data_step_rate     = std::stoi(instance_data_step_rate_str);
                                                            elem[i].InstanceDataStepRate            = instance_data_step_rate;
                                                        }
                                                        else
                                                        {
                                                            std::cout << kStrErrorUnrecognizedInputClassification << input_layout_values[5] << std::endl;
                                                        }
                                                    }
                                                    else
                                                    {
                                                        std::cout << kStrWarningUnrecognizedDxgiFormat1 << input_layout_values[2]
                                                                  << kStrWarningUnrecognizedDxgiFormat2 << std::endl;
                                                    }
                                                }
                                                else
                                                {
                                                    std::cout << kStrErrorInvalidNumInputLayoutValues1 << line << kStrErrorInvalidNumInputLayoutValues2
                                                              << input_layout_values.size() << kStrErrorInvalidNumInputLayoutValues3 << std::endl;

                                                    // Abort.
                                                    should_abort = true;
                                                }
                                            }
                                            else
                                            {
                                                std::cout << kStrErrorFailedToParseInputLayoutDefinition << line << std::endl;

                                                // Abort.
                                                should_abort = true;
                                                break;
                                            }

                                            // Assuming success if we did not have to abort due to an error.
                                            is_input_layout_checked = !should_abort;
                                        }

                                        // Continue to the next element.
                                        break;
                                    }
                                }

                                if (!should_abort)
                                {
                                    // Set the input layout descriptors.
                                    pso_desc.InputLayout = {elem, num_input_layout_elems};
                                }
                            }
                            else
                            {
                                is_input_layout_checked = true;
                            }
                        }
                        else if (!is_primitive_toplogy_type_checked && line.find(kStrElemSchemaPrimitiveTopologyType) != std::string::npos)
                        {
                            while (std::getline(f, line))
                            {
                                if (line.empty())
                                {
                                    // Skip empty lines.
                                    continue;
                                }
                                else
                                {
                                    std::string topology_type_str = TrimWhitespace(line);
                                    D3D12_PRIMITIVE_TOPOLOGY_TYPE prim_topology_type = D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED;
                                    ret = StrToPrimitiveTopologyType(topology_type_str, prim_topology_type);
                                    assert(ret);
                                    if (ret)
                                    {
                                        pso_desc.PrimitiveTopologyType = prim_topology_type;
                                        is_primitive_toplogy_type_checked = true;
                                    }
                                    else
                                    {
                                        std::cout << kStrWarningUnrecognizedPrimitiveTopologyType1 <<
                                            topology_type_str << kStrWarningUnrecognizedPrimitiveTopologyType2 << std::endl;
                                    }

                                    // Continue to the next element.
                                    break;
                                }
                            }
                        }
                        else if (!is_num_render_targets_checked &&
                            line.find(kStrElemSchemaNumRenderTargets) != std::string::npos)
                        {
                            while (std::getline(f, line))
                            {
                                if (line.empty())
                                {
                                    // Skip empty lines.
                                    continue;
                                }
                                else
                                {
                                    std::string num_render_target_str = TrimWhitespace(line);
                                    pso_desc.NumRenderTargets = std::stoi(num_render_target_str);
                                    is_num_render_targets_checked = true;

                                    // Continue to the next element.
                                    break;
                                }
                            }
                        }
                        else if (!is_rtv_formats_checked && line.find(kStrElemSchemaRtvFormats) != std::string::npos)
                        {
                            if (pso_desc.NumRenderTargets > 0)
                            {
                                if (pso_desc.NumRenderTargets > 8)
                                {
                                    std::cout << kStrWarningNumRenderTargetsExceedsMax << std::endl;

                                    // Do not exceed the maximum.
                                    pso_desc.NumRenderTargets     = 8;
                                    is_num_render_targets_checked = true;
                                }

                                while (std::getline(f, line))
                                {
                                    if (line.empty())
                                    {
                                        // Skip empty lines.
                                        continue;
                                    }
                                    else
                                    {
                                        // Extract the RTV format values.
                                        std::vector<std::string> rtv_format_values;
                                        bool                     rtv_format_parsed = ExtractCurlyBracketedValues(line, rtv_format_values);

                                        if (rtv_format_parsed)
                                        {
                                            assert(rtv_format_values.size() == pso_desc.NumRenderTargets);
                                            if (rtv_format_values.size() != pso_desc.NumRenderTargets)
                                            {
                                                std::cout << kStrWarningRenderTargetsMismatch << std::endl;
                                            }

                                            for (uint32_t i = 0; i < pso_desc.NumRenderTargets; i++)
                                            {
                                                DXGI_FORMAT format_value = DXGI_FORMAT_UNKNOWN;
                                                ret                      = RgDx12Utils::StrToDxgiFormat(rtv_format_values[i], format_value);
                                                assert(ret);
                                                if (ret)
                                                {
                                                    pso_desc.RTVFormats[i] = format_value;
                                                }
                                                else
                                                {
                                                    std::cout << kStrWarningUnrecognizedDxgiFormat1 << rtv_format_values[i]
                                                              << kStrWarningUnrecognizedDxgiFormat2 << std::endl;
                                                }
                                            }

                                            // Assume success as long as we did not have to abort.
                                            is_rtv_formats_checked = !should_abort;
                                        }
                                        else
                                        {
                                            std::cout << kStrErrorFailedToParseRtvFormatValues << line << std::endl;
                                            should_abort = true;
                                        }
                                    }
                                }
                            }
                            else
                            {
                                // No RTV formats, must set RTVFormat 0 to DXGI_FORMAT_UNKNOWN.
                                pso_desc.NumRenderTargets = 0;
                                pso_desc.RTVFormats[0]    = DXGI_FORMAT_UNKNOWN;
                                is_rtv_formats_checked = true;
                            }
                        }
                    }
                }
            }
            catch (...)
            {
                std::cout << kStrErrorFailedToParseGpsoFile << std::endl;
                ret = false;
            }
        }

        ret = !should_abort && (is_version_checked && is_input_layout_num_elements_checked && is_input_layout_checked &&
            is_primitive_toplogy_type_checked && is_num_render_targets_checked && is_rtv_formats_checked);
        return ret;
    }

    bool RgDx12Utils::StrToDxgiFormat(const std::string& str, DXGI_FORMAT& format)
    {
        static std::map<std::string, DXGI_FORMAT> format_mapping;
        bool ret = false;
        if (format_mapping.empty())
        {
            // Fill up the map.
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_UNKNOWN", DXGI_FORMAT_UNKNOWN));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R32G32B32A32_TYPELESS", DXGI_FORMAT_R32G32B32A32_TYPELESS));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R32G32B32A32_FLOAT", DXGI_FORMAT_R32G32B32A32_FLOAT));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R32G32B32A32_UINT", DXGI_FORMAT_R32G32B32A32_UINT));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R32G32B32A32_SINT", DXGI_FORMAT_R32G32B32A32_SINT));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R32G32B32_TYPELESS", DXGI_FORMAT_R32G32B32_TYPELESS));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R32G32B32_FLOAT", DXGI_FORMAT_R32G32B32_FLOAT));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R32G32B32_UINT", DXGI_FORMAT_R32G32B32_UINT));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R32G32B32_SINT", DXGI_FORMAT_R32G32B32_SINT));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R16G16B16A16_TYPELESS", DXGI_FORMAT_R16G16B16A16_TYPELESS));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R16G16B16A16_FLOAT", DXGI_FORMAT_R16G16B16A16_FLOAT));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R16G16B16A16_UNORM", DXGI_FORMAT_R16G16B16A16_UNORM));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R16G16B16A16_UINT", DXGI_FORMAT_R16G16B16A16_UINT));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R16G16B16A16_SNORM", DXGI_FORMAT_R16G16B16A16_SNORM));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R16G16B16A16_SINT", DXGI_FORMAT_R16G16B16A16_SINT));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R32G32_TYPELESS", DXGI_FORMAT_R32G32_TYPELESS));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R32G32_FLOAT", DXGI_FORMAT_R32G32_FLOAT));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R32G32_UINT", DXGI_FORMAT_R32G32_UINT));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R32G32_SINT", DXGI_FORMAT_R32G32_SINT));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R32G8X24_TYPELESS", DXGI_FORMAT_R32G8X24_TYPELESS));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_D32_FLOAT_S8X24_UINT", DXGI_FORMAT_D32_FLOAT_S8X24_UINT));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS", DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_X32_TYPELESS_G8X24_UINT", DXGI_FORMAT_X32_TYPELESS_G8X24_UINT));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R10G10B10A2_TYPELESS", DXGI_FORMAT_R10G10B10A2_TYPELESS));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R10G10B10A2_UNORM", DXGI_FORMAT_R10G10B10A2_UNORM));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R10G10B10A2_UINT", DXGI_FORMAT_R10G10B10A2_UINT));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R11G11B10_FLOAT", DXGI_FORMAT_R11G11B10_FLOAT));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R8G8B8A8_TYPELESS", DXGI_FORMAT_R8G8B8A8_TYPELESS));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R8G8B8A8_UNORM", DXGI_FORMAT_R8G8B8A8_UNORM));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R8G8B8A8_UNORM_SRGB", DXGI_FORMAT_R8G8B8A8_UNORM_SRGB));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R8G8B8A8_UINT", DXGI_FORMAT_R8G8B8A8_UINT));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R8G8B8A8_SNORM", DXGI_FORMAT_R8G8B8A8_SNORM));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R8G8B8A8_SINT", DXGI_FORMAT_R8G8B8A8_SINT));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R16G16_TYPELESS", DXGI_FORMAT_R16G16_TYPELESS));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R16G16_FLOAT", DXGI_FORMAT_R16G16_FLOAT));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R16G16_UNORM", DXGI_FORMAT_R16G16_UNORM));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R16G16_UINT", DXGI_FORMAT_R16G16_UINT));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R16G16_SNORM", DXGI_FORMAT_R16G16_SNORM));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R16G16_SINT", DXGI_FORMAT_R16G16_SINT));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R32_TYPELESS", DXGI_FORMAT_R32_TYPELESS));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_D32_FLOAT", DXGI_FORMAT_D32_FLOAT));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R32_FLOAT", DXGI_FORMAT_R32_FLOAT));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R32_UINT", DXGI_FORMAT_R32_UINT));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R32_SINT", DXGI_FORMAT_R32_SINT));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R24G8_TYPELESS", DXGI_FORMAT_R24G8_TYPELESS));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_D24_UNORM_S8_UINT", DXGI_FORMAT_D24_UNORM_S8_UINT));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R24_UNORM_X8_TYPELESS", DXGI_FORMAT_R24_UNORM_X8_TYPELESS));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_X24_TYPELESS_G8_UINT", DXGI_FORMAT_X24_TYPELESS_G8_UINT));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R8G8_TYPELESS", DXGI_FORMAT_R8G8_TYPELESS));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R8G8_UNORM", DXGI_FORMAT_R8G8_UNORM));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R8G8_UINT", DXGI_FORMAT_R8G8_UINT));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R8G8_SNORM", DXGI_FORMAT_R8G8_SNORM));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R8G8_SINT", DXGI_FORMAT_R8G8_SINT));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R16_TYPELESS", DXGI_FORMAT_R16_TYPELESS));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R16_FLOAT", DXGI_FORMAT_R16_FLOAT));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_D16_UNORM", DXGI_FORMAT_D16_UNORM));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R16_UNORM", DXGI_FORMAT_R16_UNORM));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R16_UINT", DXGI_FORMAT_R16_UINT));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R16_SNORM", DXGI_FORMAT_R16_SNORM));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R16_SINT", DXGI_FORMAT_R16_SINT));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R8_TYPELESS", DXGI_FORMAT_R8_TYPELESS));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R8_UNORM", DXGI_FORMAT_R8_UNORM));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R8_UINT", DXGI_FORMAT_R8_UINT));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R8_SNORM", DXGI_FORMAT_R8_SNORM));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R8_SINT", DXGI_FORMAT_R8_SINT));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_A8_UNORM", DXGI_FORMAT_A8_UNORM));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R1_UNORM", DXGI_FORMAT_R1_UNORM));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R9G9B9E5_SHAREDEXP", DXGI_FORMAT_R9G9B9E5_SHAREDEXP));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R8G8_B8G8_UNORM", DXGI_FORMAT_R8G8_B8G8_UNORM));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_G8R8_G8B8_UNORM", DXGI_FORMAT_G8R8_G8B8_UNORM));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_BC1_TYPELESS", DXGI_FORMAT_BC1_TYPELESS));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_BC1_UNORM", DXGI_FORMAT_BC1_UNORM));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_BC1_UNORM_SRGB", DXGI_FORMAT_BC1_UNORM_SRGB));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_BC2_TYPELESS", DXGI_FORMAT_BC2_TYPELESS));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_BC2_UNORM", DXGI_FORMAT_BC2_UNORM));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_BC2_UNORM_SRGB", DXGI_FORMAT_BC2_UNORM_SRGB));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_BC3_TYPELESS", DXGI_FORMAT_BC3_TYPELESS));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_BC3_UNORM", DXGI_FORMAT_BC3_UNORM));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_BC3_UNORM_SRGB", DXGI_FORMAT_BC3_UNORM_SRGB));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_BC4_TYPELESS", DXGI_FORMAT_BC4_TYPELESS));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_BC4_UNORM", DXGI_FORMAT_BC4_UNORM));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_BC4_SNORM", DXGI_FORMAT_BC4_SNORM));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_BC5_TYPELESS", DXGI_FORMAT_BC5_TYPELESS));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_BC5_UNORM", DXGI_FORMAT_BC5_UNORM));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_BC5_SNORM", DXGI_FORMAT_BC5_SNORM));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_B5G6R5_UNORM", DXGI_FORMAT_B5G6R5_UNORM));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_B5G5R5A1_UNORM", DXGI_FORMAT_B5G5R5A1_UNORM));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_B8G8R8A8_UNORM", DXGI_FORMAT_B8G8R8A8_UNORM));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_B8G8R8X8_UNORM", DXGI_FORMAT_B8G8R8X8_UNORM));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM", DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_B8G8R8A8_TYPELESS", DXGI_FORMAT_B8G8R8A8_TYPELESS));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_B8G8R8A8_UNORM_SRGB", DXGI_FORMAT_B8G8R8A8_UNORM_SRGB));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_B8G8R8X8_TYPELESS", DXGI_FORMAT_B8G8R8X8_TYPELESS));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_B8G8R8X8_UNORM_SRGB", DXGI_FORMAT_B8G8R8X8_UNORM_SRGB));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_BC6H_TYPELESS", DXGI_FORMAT_BC6H_TYPELESS));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_BC6H_UF16", DXGI_FORMAT_BC6H_UF16));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_BC6H_SF16", DXGI_FORMAT_BC6H_SF16));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_BC7_TYPELESS", DXGI_FORMAT_BC7_TYPELESS));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_BC7_UNORM", DXGI_FORMAT_BC7_UNORM));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_BC7_UNORM_SRGB", DXGI_FORMAT_BC7_UNORM_SRGB));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_AYUV", DXGI_FORMAT_AYUV));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_Y410", DXGI_FORMAT_Y410));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_Y416", DXGI_FORMAT_Y416));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_NV12", DXGI_FORMAT_NV12));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_P010", DXGI_FORMAT_P010));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_P016", DXGI_FORMAT_P016));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_420_OPAQUE", DXGI_FORMAT_420_OPAQUE));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_YUY2", DXGI_FORMAT_YUY2));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_Y210", DXGI_FORMAT_Y210));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_Y216", DXGI_FORMAT_Y216));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_NV11", DXGI_FORMAT_NV11));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_AI44", DXGI_FORMAT_AI44));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_IA44", DXGI_FORMAT_IA44));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_P8", DXGI_FORMAT_P8));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_A8P8", DXGI_FORMAT_A8P8));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_B4G4R4A4_UNORM", DXGI_FORMAT_B4G4R4A4_UNORM));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_P208", DXGI_FORMAT_P208));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_V208", DXGI_FORMAT_V208));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_V408", DXGI_FORMAT_V408));
            format_mapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_FORCE_UINT", DXGI_FORMAT_FORCE_UINT));
        }

        auto iter = format_mapping.find(str);
        assert(iter != format_mapping.end());
        if (iter != format_mapping.end())
        {
            format = iter->second;
            ret = true;
        }

        return ret;
    }

    bool RgDx12Utils::StrToInputClassification(const std::string& str, D3D12_INPUT_CLASSIFICATION& input_classification)
    {
        static std::map<std::string, D3D12_INPUT_CLASSIFICATION> input_classification_mapping;
        bool ret = false;
        if (input_classification_mapping.empty())
        {
            // Fill up the map.
            input_classification_mapping.insert(std::make_pair<std::string,
                D3D12_INPUT_CLASSIFICATION>("D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA",
                    D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA));

            input_classification_mapping.insert(std::make_pair<std::string,
                D3D12_INPUT_CLASSIFICATION>("D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA",
                    D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA));
        }

        auto iter = input_classification_mapping.find(str);
        assert(iter != input_classification_mapping.end());
        if (iter != input_classification_mapping.end())
        {
            input_classification = iter->second;
            ret = true;
        }

        return ret;
    }

    bool RgDx12Utils::StrToPrimitiveTopologyType(const std::string& str, D3D12_PRIMITIVE_TOPOLOGY_TYPE& primitive_topology_type)
    {
        static std::map<std::string, D3D12_PRIMITIVE_TOPOLOGY_TYPE> primitive_topology_type_mapping;
        bool ret = false;
        if (primitive_topology_type_mapping.empty())
        {
            // Fill up the map.
            primitive_topology_type_mapping.insert(std::make_pair<std::string,
                D3D12_PRIMITIVE_TOPOLOGY_TYPE>("D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED",
                    D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED));

            primitive_topology_type_mapping.insert(std::make_pair<std::string,
                D3D12_PRIMITIVE_TOPOLOGY_TYPE>("D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT",
                    D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT));

            primitive_topology_type_mapping.insert(std::make_pair<std::string,
                D3D12_PRIMITIVE_TOPOLOGY_TYPE>("D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE",
                    D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE));

            primitive_topology_type_mapping.insert(std::make_pair<std::string,
                D3D12_PRIMITIVE_TOPOLOGY_TYPE>("D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE",
                    D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE));

            primitive_topology_type_mapping.insert(std::make_pair<std::string,
                D3D12_PRIMITIVE_TOPOLOGY_TYPE>("D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH",
                    D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH));
        }

        auto iter = primitive_topology_type_mapping.find(str);
        assert(iter != primitive_topology_type_mapping.end());
        if (iter != primitive_topology_type_mapping.end())
        {
            primitive_topology_type = iter->second;
            ret = true;
        }

        return ret;
    }

    std::wstring RgDx12Utils::strToWstr(const std::string& str)
    {
        const std::size_t    len = static_cast<std::size_t>(swprintf(nullptr, 0, L"%.*hs", static_cast<int>(str.size()), str.data()));
        std::vector<wchar_t> vec(len + 1);
        swprintf(vec.data(), len + 1, L"%.*hs", static_cast<int>(str.size()), str.data());
        return std::wstring(vec.data(), len);
    }

    std::string RgDx12Utils::wstrToStr(const std::wstring& wstr)
    {
        const std::size_t len = static_cast<std::size_t>(snprintf(nullptr, 0, "%.*ws", static_cast<int>(wstr.size()), wstr.data()));
        std::vector<char> vec(len + 1);
        snprintf(vec.data(), len + 1, "%.*ws", static_cast<int>(wstr.size()), wstr.data());
        return std::string(vec.data(), len);
    }

    static std::string TrimCharacters(const std::string& str, const std::string& chars_to_trim)
    {
        std::string ret;
        const auto str_begin = str.find_first_not_of(chars_to_trim);
        if (str_begin != std::string::npos)
        {
            const auto str_end = str.find_last_not_of(chars_to_trim);
            const auto str_range = str_end - str_begin + 1;
            ret = str.substr(str_begin, str_range);
        }
        return ret;
    }

    std::string RgDx12Utils::TrimWhitespace(const std::string& str)
    {
        const std::string& kStrWhitespaceChar = " \t";
        return TrimCharacters(str, kStrWhitespaceChar);
    }

    std::string RgDx12Utils::TrimNewline(const std::string& str)
    {
        const std::string& kStrNewlineChar = "\n";
        return TrimCharacters(str, kStrNewlineChar);
    }

    std::string RgDx12Utils::TrimWhitespaceAndCommas(const std::string& str)
    {
        const std::string& kStrWhitespaceChar = " \t\"";
        return TrimCharacters(str, kStrWhitespaceChar);
    }

    bool RgDx12Utils::WriteBinaryFile(const std::string& filename, const std::vector<char>& content)
    {
        bool ret = false;
        std::ofstream output;
        output.open(filename.c_str(), std::ios::binary);

        if (output.is_open() && !content.empty())
        {
            output.write(&content[0], content.size());
            output.close();
            ret = true;
        }
        else
        {
            std::cerr << kStrErrBinaryFileWriteFailed << filename << std::endl;
        }

        return ret;
    }

    bool RgDx12Utils::WriteBinaryFile(const std::string& filename, const std::vector<unsigned char>& content)
    {
        bool ret = false;
        std::ofstream output;
        output.open(filename.c_str(), std::ios::binary);

        if (output.is_open() && !content.empty())
        {
            output.write((char*)&content[0], content.size());
            output.close();
            ret = true;
        }
        else
        {
            std::cerr << kStrErrBinaryFileWriteFailed << filename << std::endl;
        }

        return ret;
    }

}