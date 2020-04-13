#include <rgDx12Utils.h>

// C++.
#include <cassert>
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <algorithm>
#include <iterator>

namespace rga
{
    // *** CONSTANTS - START ***

    static const char* STR_ERR_TEXT_FILE_WRITE_FAILED = "Error: failed to write text file: ";
    static const char* STR_ERR_TEXT_FILE_READ_FAILED = "Error: failed to read text file: ";
    static const char* STR_ERR_BINARY_FILE_WRITE_FAILED = "Error: failed to write binary file: ";

    // *** CONSTANTS - END ***

    // Splits comma-separated values wrapped in '{' '}', e.g. { X, Y, Z }.
    bool static ExtractCurlyBracketedValues(const std::string& line, std::vector<std::string>& inputLayoutValues)
    {
        bool ret = false;
        size_t startPos = line.find('{');
        size_t endPos = line.find('}');
        assert(startPos != std::string::npos);
        assert(endPos != std::string::npos);
        assert(startPos + 1 < line.size());
        assert(endPos > 0);

        if ((startPos != std::string::npos) && (endPos != std::string::npos) &&
            (startPos + 1 < line.size()) && (endPos > 0))
        {
            std::string valuesString = line.substr(startPos + 1, line.size() - 2);
            rgDx12Utils::SplitString(valuesString, ',', inputLayoutValues);
            for (int32_t i = 0; i < inputLayoutValues.size(); i++)
            {
                inputLayoutValues[i] = rgDx12Utils::TrimWhitespace(inputLayoutValues[i]);
            }
            ret = true;
        }

        return ret;
    }

    bool rgDx12Utils::WriteTextFile(const std::string& fileName, const std::string& content)
    {
        bool ret = false;
        std::ofstream output;
        output.open(fileName.c_str());

        if (output.is_open())
        {
            output << content << std::endl;
            output.close();
            ret = true;
        }
        else
        {
            std::cerr << STR_ERR_TEXT_FILE_WRITE_FAILED << fileName << std::endl;
        }

        return ret;
    }

    bool rgDx12Utils::ReadTextFile(const std::string& fileName, std::string& content)
    {
        bool ret = false;
        std::ifstream input;
        input.open(fileName.c_str());

        if (input.is_open())
        {
            std::stringstream  textStream;
            textStream << input.rdbuf();
            content = textStream.str();
            input.close();
            ret = true;
        }
        else
        {
            std::cout << STR_ERR_TEXT_FILE_READ_FAILED << fileName << std::endl;
        }

        return ret;
    }

    bool rgDx12Utils::ReadBinaryFile(const std::string& fileName, std::vector<char>& content)
    {
        bool ret = false;
        std::ifstream input;
        input.open(fileName.c_str(), std::ios::binary);

        if (input.is_open())
        {
            content = std::vector<char>(std::istreambuf_iterator<char>(input), {});
            ret = !content.empty();
        }
        return ret;
    }

    void rgDx12Utils::SplitString(const std::string& str, char delim, std::vector<std::string>& dst)
    {
        std::stringstream ss;
        ss.str(str);
        std::string substr;
        while (std::getline(ss, substr, delim))
        {
            dst.push_back(substr);
        }
    }

    void rgDx12Utils::InitGraphicsPipelineStateDesc(D3D12_GRAPHICS_PIPELINE_STATE_DESC& psoDesc)
    {
        // Reset the structure.
        memset(&psoDesc, 0, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));

        // Default D3D12 rasterizer state.
        D3D12_RASTERIZER_DESC DefaultDx12RasterizerState;
        DefaultDx12RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
        DefaultDx12RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
        DefaultDx12RasterizerState.FrontCounterClockwise = FALSE;
        DefaultDx12RasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
        DefaultDx12RasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
        DefaultDx12RasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
        DefaultDx12RasterizerState.DepthClipEnable = TRUE;
        DefaultDx12RasterizerState.MultisampleEnable = FALSE;
        DefaultDx12RasterizerState.AntialiasedLineEnable = FALSE;
        DefaultDx12RasterizerState.ForcedSampleCount = 0;
        DefaultDx12RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

        // Default D3D12 blend state.
        D3D12_BLEND_DESC DefaultDx12BlendState;
        DefaultDx12BlendState.AlphaToCoverageEnable = FALSE;
        DefaultDx12BlendState.IndependentBlendEnable = FALSE;
        const D3D12_RENDER_TARGET_BLEND_DESC defaultRenderTargetBlendDesc =
        {
            FALSE,FALSE,
            D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
            D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
            D3D12_LOGIC_OP_NOOP,
            D3D12_COLOR_WRITE_ENABLE_ALL,
        };
        for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
        {
            DefaultDx12BlendState.RenderTarget[i] = defaultRenderTargetBlendDesc;
        }

        // Default D3D12 depth stencil state.
        D3D12_DEPTH_STENCIL_DESC defaultDx12DepthStencilState;
        defaultDx12DepthStencilState.DepthEnable = TRUE;
        defaultDx12DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
        defaultDx12DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
        defaultDx12DepthStencilState.StencilEnable = FALSE;
        defaultDx12DepthStencilState.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
        defaultDx12DepthStencilState.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
        const D3D12_DEPTH_STENCILOP_DESC defaultStencilOp =
        { D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_COMPARISON_FUNC_ALWAYS };
        defaultDx12DepthStencilState.FrontFace = defaultStencilOp;
        defaultDx12DepthStencilState.BackFace = defaultStencilOp;

        // Set the D3D12 defaults.
        psoDesc.BlendState = DefaultDx12BlendState;
        psoDesc.DepthStencilState = defaultDx12DepthStencilState;
        psoDesc.RasterizerState = DefaultDx12RasterizerState;

        psoDesc.SampleMask = UINT_MAX;

        // Assume triangle primitive topology type.
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

        // Assume single render target.
        psoDesc.NumRenderTargets = 1;
        psoDesc.RTVFormats[0] = DXGI_FORMAT_R32G32B32A32_FLOAT;

        // Assume a single multisample per pixel.
        psoDesc.SampleDesc.Count = 1;
    }

    bool rgDx12Utils::ParseGpsoFile(const std::string& fileName, D3D12_GRAPHICS_PIPELINE_STATE_DESC& psoDesc)
    {
        // Error messages.
        const char* ERROR_UNKNOWN_SCHEMA_VERSION = "Error: unknown schema version: ";
        const char* ERROR_UNRECOGNIZED_INPUT_CLASSIFICATION = "Error: unrecognized Input Classification ";
        const char* ERROR_ZERO_INPUT_LAYOUT_ELEMENT_ITEMS = "Error: number of input layout element items must be non-zero.";
        const char* ERROR_FAILED_TO_PARSE_GPSO_FILE = "Error: failed to parse .gpso file.";
        const char* ERROR_FAILED_INVALID_NUM_INPUT_LAYOUT_VALUES_A = "Error: invalid number of input items between \"{}\" for line \"";
        const char* ERROR_FAILED_INVALID_NUM_INPUT_LAYOUT_VALUES_B = "\". Expected 7, found ";
        const char* ERROR_FAILED_INVALID_NUM_INPUT_LAYOUT_VALUES_C = ".";
        const char* ERROR_FAILED_PARSE_INPUT_LAYOUT_DEFINITION = "Error: unable to parse input layout definition: ";
        const char* ERROR_FAILED_PARSE_RTV_FORMAT_VALUES = "Error: failed to parse the RTV format values, line ";

        // Warning messages.
        const char* WARNING_UNRECOGNIZED_DXGI_FORMAT_A = "Warning: unrecognized DXGI Format \"";
        const char* WARNING_UNRECOGNIZED_DXGI_FORMAT_B = "\", assuming DXGI_FORMAT_UNKNOWN.";
        const char* WARNING_UNRECOGNIZED_PRIMITIVE_TOPOLOGY_TYPE_A = "Error: unrecognized primitive topology type: \"";
        const char* WARNING_UNRECOGNIZED_PRIMITIVE_TOPOLOGY_TYPE_B = "\", assuming D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED.";
        const char* WARNING_NUM_RENDER_TARGETS_MISMATCH = "Warning: mismatch between number of RTV format values and the NumRenderTargets.";
        const char* WARNING_NUM_RENDER_TARGETS_EXCEEDS_MAX = "Warning: NumRenderTargets exceeds max of 8, assuming 8.";

        // Schema constants.
        const char* ELEM_SCHEMA_VERSION = "schemaVersion";
        const char* ELEM_SCHEMA_VERSION_1_0 = "1.0";
        const char* ELEM_SCHEMA_INPUT_LAYOUT_NUM = "InputLayoutNumElements";
        const char* ELEM_SCHEMA_INPUT_LAYOUT = "InputLayout";
        const char* ELEM_SCHEMA_PRIMITIVE_TOPOLOGY_TYPE = "PrimitiveTopologyType";
        const char* ELEM_SCHEMA_NUM_RENDER_TARGETS = "NumRenderTargets";
        const char* ELEM_SCHEMA_RTV_FORMATS = "RTVFormats";

        // Container for input element descriptors.
        D3D12_INPUT_ELEMENT_DESC* pElem = {};

        // Number of input layout elements.
        uint32_t numInputLayoutElems = 0;
        std::string gpsoContent;
        bool ret = false;

        // A flag indicating if we should abort
        // the processing process due to an error.
        bool shouldAbort = false;

        // Flags that indicate whether we covered the relevant blocks.
        bool isVersionChecked = false;
        bool isInputLayoutNumElementsChecked = false;
        bool isInputLayoutChecked = false;
        bool isPrimitiveTopologyTypeChecked = false;
        bool isNumRenderTargetsChecked = false;
        bool isRTVFormatsChecked = false;

        // Read the input file
        bool isFileRead = ReadTextFile(fileName, gpsoContent);
        assert(isFileRead);
        if (isFileRead)
        {
            try
            {
                std::istringstream f(gpsoContent);
                std::string line;
                while (!shouldAbort && std::getline(f, line))
                {
                    // Skip empty lines.
                    if (line.empty())
                    {
                        continue;
                    }

                    if (line.find('#') == 0)
                    {
                        // Schema version.
                        if (!isVersionChecked && line.find(ELEM_SCHEMA_VERSION) != std::string::npos)
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
                                    assert(line.compare(ELEM_SCHEMA_VERSION_1_0) == 0);
                                    if (line.compare(ELEM_SCHEMA_VERSION_1_0) != 0)
                                    {
                                        std::cout << ERROR_UNKNOWN_SCHEMA_VERSION << line << std::endl;
                                        shouldAbort = true;
                                    }
                                    else
                                    {
                                        // Version verified.
                                        isVersionChecked = true;
                                    }

                                    // Continue to the next element.
                                    break;
                                }
                            }
                        }
                        else if (!isInputLayoutNumElementsChecked &&
                            line.find(ELEM_SCHEMA_INPUT_LAYOUT_NUM) != std::string::npos)
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
                                    numInputLayoutElems = std::stoi(numElements);
                                    assert(numInputLayoutElems > 0);
                                    if (numInputLayoutElems == 0)
                                    {
                                        std::cout << ERROR_ZERO_INPUT_LAYOUT_ELEMENT_ITEMS << std::endl;
                                        shouldAbort = true;
                                    }
                                    else
                                    {
                                        // We got the number of input layout descriptor elements.
                                        isInputLayoutNumElementsChecked = true;
                                    }
                                    break;
                                }
                            }
                        }
                        else if (!isInputLayoutChecked && line.find(ELEM_SCHEMA_INPUT_LAYOUT) != std::string::npos)
                        {
                            // Allocate the array of InputLayout elements.
                            pElem = new D3D12_INPUT_ELEMENT_DESC[numInputLayoutElems]{};

                            for (uint32_t i = 0; i < numInputLayoutElems; i++)
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
                                        std::vector<std::string> inputLayoutValues;
                                        bool isInputLayoutParsed = ExtractCurlyBracketedValues(line, inputLayoutValues);
                                        assert(isInputLayoutParsed);
                                        if (isInputLayoutParsed)
                                        {
                                            assert(inputLayoutValues.size() == 7);
                                            if (inputLayoutValues.size() == 7)
                                            {
                                                // SemanticName.
                                                std::string semanticName = TrimWhitespaceAndCommas(inputLayoutValues[0]);
                                                char* pSemantic = new char[semanticName.size() + 1]{ 0 };
                                                std::copy(semanticName.begin(), semanticName.end(), pSemantic);
                                                pElem[i].SemanticName = pSemantic;

                                                // SemanticIndex.
                                                std::string semanticIndex = TrimWhitespace(inputLayoutValues[1]);
                                                int index = std::stoi(semanticIndex);
                                                pElem[i].SemanticIndex = index;

                                                // Format.
                                                std::string formatStr = TrimWhitespace(inputLayoutValues[2]);
                                                DXGI_FORMAT dxgiFormat;
                                                bool isFormatFound = StrToDxgiFormat(formatStr, dxgiFormat);
                                                assert(isFormatFound);

                                                if (isFormatFound)
                                                {
                                                    pElem[i].Format = dxgiFormat;

                                                    // InputSlot.
                                                    std::string inputSlotStr = TrimWhitespace(inputLayoutValues[3]);
                                                    int inputSlot = std::stoi(inputSlotStr);
                                                    pElem[i].InputSlot = inputSlot;

                                                    // AlignedByteOffset.
                                                    std::string alignedByteOffsetStr = TrimWhitespace(inputLayoutValues[4]);
                                                    int alignedByteOffset = 0;
                                                    try
                                                    {
                                                        alignedByteOffset = std::stoi(alignedByteOffsetStr);
                                                        pElem[i].AlignedByteOffset = alignedByteOffset;
                                                    }
                                                    catch (const std::exception& e)
                                                    {
                                                        // Handle known special cases.
                                                        if (alignedByteOffsetStr.compare("D3D12_APPEND_ALIGNED_ELEMENT") == 0)
                                                        {
                                                            alignedByteOffset = 0xffffffff;
                                                        }
                                                        else
                                                        {
                                                            // No special case found - parsing error.
                                                            throw e;
                                                        }
                                                    }

                                                    // Input slot classification.
                                                    std::string inputClassificationStr = TrimWhitespace(inputLayoutValues[5]);
                                                    D3D12_INPUT_CLASSIFICATION inputClassification;
                                                    bool isInputClassificationFound = StrToInputClassification(inputClassificationStr, inputClassification);
                                                    assert(isInputClassificationFound);

                                                    if (isInputClassificationFound)
                                                    {
                                                        pElem[i].InputSlotClass = inputClassification;

                                                        // InstanceDataStepRate.
                                                        std::string instanceDataStepRateStr = TrimWhitespace(inputLayoutValues[6]);
                                                        int instanceDataStepRate = std::stoi(instanceDataStepRateStr);
                                                        pElem[i].InstanceDataStepRate = instanceDataStepRate;
                                                    }
                                                    else
                                                    {
                                                        std::cout << ERROR_UNRECOGNIZED_INPUT_CLASSIFICATION << inputLayoutValues[5] << std::endl;
                                                    }
                                                }
                                                else
                                                {
                                                    std::cout << WARNING_UNRECOGNIZED_DXGI_FORMAT_A << inputLayoutValues[2] <<
                                                        WARNING_UNRECOGNIZED_DXGI_FORMAT_B << std::endl;
                                                }
                                            }
                                            else
                                            {
                                                std::cout << ERROR_FAILED_INVALID_NUM_INPUT_LAYOUT_VALUES_A <<
                                                    line << ERROR_FAILED_INVALID_NUM_INPUT_LAYOUT_VALUES_B <<
                                                    inputLayoutValues.size() << ERROR_FAILED_INVALID_NUM_INPUT_LAYOUT_VALUES_C << std::endl;

                                                // Abort.
                                                shouldAbort = true;
                                            }
                                        }
                                        else
                                        {
                                            std::cout << ERROR_FAILED_PARSE_INPUT_LAYOUT_DEFINITION << line << std::endl;

                                            // Abort.
                                            shouldAbort = true;
                                            break;
                                        }

                                        // Assuming success if we did not have to abort due to an error.
                                        isInputLayoutChecked = !shouldAbort;
                                    }

                                    // Continue to the next element.
                                    break;
                                }
                            }

                            if (!shouldAbort)
                            {
                                // Set the input layout descriptors.
                                psoDesc.InputLayout = { pElem, numInputLayoutElems };
                            }
                        }
                        else if (!isPrimitiveTopologyTypeChecked &&
                            line.find(ELEM_SCHEMA_PRIMITIVE_TOPOLOGY_TYPE) != std::string::npos)
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
                                    std::string topologyTypeStr = TrimWhitespace(line);
                                    D3D12_PRIMITIVE_TOPOLOGY_TYPE primTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED;
                                    ret = StrToPrimitiveTopologyType(topologyTypeStr, primTopologyType);
                                    assert(ret);
                                    if (ret)
                                    {
                                        psoDesc.PrimitiveTopologyType = primTopologyType;
                                        isPrimitiveTopologyTypeChecked = true;
                                    }
                                    else
                                    {
                                        std::cout << WARNING_UNRECOGNIZED_PRIMITIVE_TOPOLOGY_TYPE_A <<
                                            topologyTypeStr << WARNING_UNRECOGNIZED_PRIMITIVE_TOPOLOGY_TYPE_B << std::endl;
                                    }

                                    // Continue to the next element.
                                    break;
                                }
                            }
                        }
                        else if (!isNumRenderTargetsChecked &&
                            line.find(ELEM_SCHEMA_NUM_RENDER_TARGETS) != std::string::npos)
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
                                    std::string numRenderTargetStr = TrimWhitespace(line);
                                    psoDesc.NumRenderTargets = std::stoi(numRenderTargetStr);
                                    isNumRenderTargetsChecked = true;

                                    // Continue to the next element.
                                    break;
                                }
                            }
                        }
                        else if (!isRTVFormatsChecked && line.find(ELEM_SCHEMA_RTV_FORMATS) != std::string::npos)
                        {

                            if (psoDesc.NumRenderTargets > 8)
                            {
                                std::cout << WARNING_NUM_RENDER_TARGETS_EXCEEDS_MAX << std::endl;

                                // Do not exceed the maximum.
                                psoDesc.NumRenderTargets = 8;
                                isNumRenderTargetsChecked = true;
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
                                    std::vector<std::string> rtvFormatValues;
                                    bool isRtvFormatParsed = ExtractCurlyBracketedValues(line, rtvFormatValues);

                                    if (isRtvFormatParsed)
                                    {
                                        assert(rtvFormatValues.size() == psoDesc.NumRenderTargets);
                                        if (rtvFormatValues.size() != psoDesc.NumRenderTargets)
                                        {
                                            std::cout << WARNING_NUM_RENDER_TARGETS_MISMATCH << std::endl;
                                        }

                                        for (uint32_t i = 0; i < psoDesc.NumRenderTargets; i++)
                                        {
                                            DXGI_FORMAT formatValue = DXGI_FORMAT_UNKNOWN;
                                            ret = rgDx12Utils::StrToDxgiFormat(rtvFormatValues[i], formatValue);
                                            assert(ret);
                                            if (ret)
                                            {
                                                psoDesc.RTVFormats[i] = formatValue;
                                            }
                                            else
                                            {
                                                std::cout << WARNING_UNRECOGNIZED_DXGI_FORMAT_A << rtvFormatValues[i] <<
                                                    WARNING_UNRECOGNIZED_DXGI_FORMAT_B << std::endl;
                                            }

                                            // Continue to the next element.
                                            break;
                                        }

                                        // Assume success as long as we did not have to abort.
                                        isRTVFormatsChecked = !shouldAbort;
                                    }
                                    else
                                    {
                                        std::cout << ERROR_FAILED_PARSE_RTV_FORMAT_VALUES << line << std::endl;
                                        shouldAbort = true;
                                    }
                                }
                            }
                        }
                    }
                }
            }
            catch (...)
            {
                std::cout << ERROR_FAILED_TO_PARSE_GPSO_FILE << std::endl;
                ret = false;
            }
        }

        ret = !shouldAbort && (isVersionChecked && isInputLayoutNumElementsChecked && isInputLayoutChecked &&
            isPrimitiveTopologyTypeChecked && isNumRenderTargetsChecked && isRTVFormatsChecked);
        return ret;
    }

    bool rgDx12Utils::StrToDxgiFormat(const std::string& str, DXGI_FORMAT& format)
    {
        static std::map<std::string, DXGI_FORMAT> formatMapping;
        bool ret = false;
        if (formatMapping.empty())
        {
            // Fill up the map.
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_UNKNOWN", DXGI_FORMAT_UNKNOWN));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R32G32B32A32_TYPELESS", DXGI_FORMAT_R32G32B32A32_TYPELESS));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R32G32B32A32_FLOAT", DXGI_FORMAT_R32G32B32A32_FLOAT));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R32G32B32A32_UINT", DXGI_FORMAT_R32G32B32A32_UINT));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R32G32B32A32_SINT", DXGI_FORMAT_R32G32B32A32_SINT));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R32G32B32_TYPELESS", DXGI_FORMAT_R32G32B32_TYPELESS));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R32G32B32_FLOAT", DXGI_FORMAT_R32G32B32_FLOAT));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R32G32B32_UINT", DXGI_FORMAT_R32G32B32_UINT));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R32G32B32_SINT", DXGI_FORMAT_R32G32B32_SINT));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R16G16B16A16_TYPELESS", DXGI_FORMAT_R16G16B16A16_TYPELESS));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R16G16B16A16_FLOAT", DXGI_FORMAT_R16G16B16A16_FLOAT));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R16G16B16A16_UNORM", DXGI_FORMAT_R16G16B16A16_UNORM));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R16G16B16A16_UINT", DXGI_FORMAT_R16G16B16A16_UINT));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R16G16B16A16_SNORM", DXGI_FORMAT_R16G16B16A16_SNORM));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R16G16B16A16_SINT", DXGI_FORMAT_R16G16B16A16_SINT));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R32G32_TYPELESS", DXGI_FORMAT_R32G32_TYPELESS));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R32G32_FLOAT", DXGI_FORMAT_R32G32_FLOAT));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R32G32_UINT", DXGI_FORMAT_R32G32_UINT));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R32G32_SINT", DXGI_FORMAT_R32G32_SINT));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R32G8X24_TYPELESS", DXGI_FORMAT_R32G8X24_TYPELESS));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_D32_FLOAT_S8X24_UINT", DXGI_FORMAT_D32_FLOAT_S8X24_UINT));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS", DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_X32_TYPELESS_G8X24_UINT", DXGI_FORMAT_X32_TYPELESS_G8X24_UINT));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R10G10B10A2_TYPELESS", DXGI_FORMAT_R10G10B10A2_TYPELESS));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R10G10B10A2_UNORM", DXGI_FORMAT_R10G10B10A2_UNORM));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R10G10B10A2_UINT", DXGI_FORMAT_R10G10B10A2_UINT));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R11G11B10_FLOAT", DXGI_FORMAT_R11G11B10_FLOAT));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R8G8B8A8_TYPELESS", DXGI_FORMAT_R8G8B8A8_TYPELESS));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R8G8B8A8_UNORM", DXGI_FORMAT_R8G8B8A8_UNORM));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R8G8B8A8_UNORM_SRGB", DXGI_FORMAT_R8G8B8A8_UNORM_SRGB));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R8G8B8A8_UINT", DXGI_FORMAT_R8G8B8A8_UINT));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R8G8B8A8_SNORM", DXGI_FORMAT_R8G8B8A8_SNORM));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R8G8B8A8_SINT", DXGI_FORMAT_R8G8B8A8_SINT));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R16G16_TYPELESS", DXGI_FORMAT_R16G16_TYPELESS));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R16G16_FLOAT", DXGI_FORMAT_R16G16_FLOAT));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R16G16_UNORM", DXGI_FORMAT_R16G16_UNORM));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R16G16_UINT", DXGI_FORMAT_R16G16_UINT));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R16G16_SNORM", DXGI_FORMAT_R16G16_SNORM));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R16G16_SINT", DXGI_FORMAT_R16G16_SINT));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R32_TYPELESS", DXGI_FORMAT_R32_TYPELESS));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_D32_FLOAT", DXGI_FORMAT_D32_FLOAT));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R32_FLOAT", DXGI_FORMAT_R32_FLOAT));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R32_UINT", DXGI_FORMAT_R32_UINT));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R32_SINT", DXGI_FORMAT_R32_SINT));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R24G8_TYPELESS", DXGI_FORMAT_R24G8_TYPELESS));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_D24_UNORM_S8_UINT", DXGI_FORMAT_D24_UNORM_S8_UINT));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R24_UNORM_X8_TYPELESS", DXGI_FORMAT_R24_UNORM_X8_TYPELESS));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_X24_TYPELESS_G8_UINT", DXGI_FORMAT_X24_TYPELESS_G8_UINT));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R8G8_TYPELESS", DXGI_FORMAT_R8G8_TYPELESS));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R8G8_UNORM", DXGI_FORMAT_R8G8_UNORM));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R8G8_UINT", DXGI_FORMAT_R8G8_UINT));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R8G8_SNORM", DXGI_FORMAT_R8G8_SNORM));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R8G8_SINT", DXGI_FORMAT_R8G8_SINT));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R16_TYPELESS", DXGI_FORMAT_R16_TYPELESS));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R16_FLOAT", DXGI_FORMAT_R16_FLOAT));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_D16_UNORM", DXGI_FORMAT_D16_UNORM));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R16_UNORM", DXGI_FORMAT_R16_UNORM));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R16_UINT", DXGI_FORMAT_R16_UINT));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R16_SNORM", DXGI_FORMAT_R16_SNORM));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R16_SINT", DXGI_FORMAT_R16_SINT));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R8_TYPELESS", DXGI_FORMAT_R8_TYPELESS));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R8_UNORM", DXGI_FORMAT_R8_UNORM));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R8_UINT", DXGI_FORMAT_R8_UINT));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R8_SNORM", DXGI_FORMAT_R8_SNORM));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R8_SINT", DXGI_FORMAT_R8_SINT));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_A8_UNORM", DXGI_FORMAT_A8_UNORM));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R1_UNORM", DXGI_FORMAT_R1_UNORM));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R9G9B9E5_SHAREDEXP", DXGI_FORMAT_R9G9B9E5_SHAREDEXP));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R8G8_B8G8_UNORM", DXGI_FORMAT_R8G8_B8G8_UNORM));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_G8R8_G8B8_UNORM", DXGI_FORMAT_G8R8_G8B8_UNORM));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_BC1_TYPELESS", DXGI_FORMAT_BC1_TYPELESS));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_BC1_UNORM", DXGI_FORMAT_BC1_UNORM));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_BC1_UNORM_SRGB", DXGI_FORMAT_BC1_UNORM_SRGB));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_BC2_TYPELESS", DXGI_FORMAT_BC2_TYPELESS));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_BC2_UNORM", DXGI_FORMAT_BC2_UNORM));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_BC2_UNORM_SRGB", DXGI_FORMAT_BC2_UNORM_SRGB));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_BC3_TYPELESS", DXGI_FORMAT_BC3_TYPELESS));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_BC3_UNORM", DXGI_FORMAT_BC3_UNORM));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_BC3_UNORM_SRGB", DXGI_FORMAT_BC3_UNORM_SRGB));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_BC4_TYPELESS", DXGI_FORMAT_BC4_TYPELESS));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_BC4_UNORM", DXGI_FORMAT_BC4_UNORM));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_BC4_SNORM", DXGI_FORMAT_BC4_SNORM));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_BC5_TYPELESS", DXGI_FORMAT_BC5_TYPELESS));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_BC5_UNORM", DXGI_FORMAT_BC5_UNORM));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_BC5_SNORM", DXGI_FORMAT_BC5_SNORM));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_B5G6R5_UNORM", DXGI_FORMAT_B5G6R5_UNORM));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_B5G5R5A1_UNORM", DXGI_FORMAT_B5G5R5A1_UNORM));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_B8G8R8A8_UNORM", DXGI_FORMAT_B8G8R8A8_UNORM));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_B8G8R8X8_UNORM", DXGI_FORMAT_B8G8R8X8_UNORM));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM", DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_B8G8R8A8_TYPELESS", DXGI_FORMAT_B8G8R8A8_TYPELESS));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_B8G8R8A8_UNORM_SRGB", DXGI_FORMAT_B8G8R8A8_UNORM_SRGB));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_B8G8R8X8_TYPELESS", DXGI_FORMAT_B8G8R8X8_TYPELESS));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_B8G8R8X8_UNORM_SRGB", DXGI_FORMAT_B8G8R8X8_UNORM_SRGB));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_BC6H_TYPELESS", DXGI_FORMAT_BC6H_TYPELESS));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_BC6H_UF16", DXGI_FORMAT_BC6H_UF16));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_BC6H_SF16", DXGI_FORMAT_BC6H_SF16));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_BC7_TYPELESS", DXGI_FORMAT_BC7_TYPELESS));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_BC7_UNORM", DXGI_FORMAT_BC7_UNORM));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_BC7_UNORM_SRGB", DXGI_FORMAT_BC7_UNORM_SRGB));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_AYUV", DXGI_FORMAT_AYUV));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_Y410", DXGI_FORMAT_Y410));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_Y416", DXGI_FORMAT_Y416));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_NV12", DXGI_FORMAT_NV12));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_P010", DXGI_FORMAT_P010));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_P016", DXGI_FORMAT_P016));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_420_OPAQUE", DXGI_FORMAT_420_OPAQUE));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_YUY2", DXGI_FORMAT_YUY2));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_Y210", DXGI_FORMAT_Y210));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_Y216", DXGI_FORMAT_Y216));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_NV11", DXGI_FORMAT_NV11));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_AI44", DXGI_FORMAT_AI44));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_IA44", DXGI_FORMAT_IA44));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_P8", DXGI_FORMAT_P8));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_A8P8", DXGI_FORMAT_A8P8));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_B4G4R4A4_UNORM", DXGI_FORMAT_B4G4R4A4_UNORM));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_P208", DXGI_FORMAT_P208));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_V208", DXGI_FORMAT_V208));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_V408", DXGI_FORMAT_V408));
            formatMapping.insert(std::make_pair<std::string, DXGI_FORMAT>("DXGI_FORMAT_FORCE_UINT", DXGI_FORMAT_FORCE_UINT));
        }

        auto iter = formatMapping.find(str);
        assert(iter != formatMapping.end());
        if (iter != formatMapping.end())
        {
            format = iter->second;
            ret = true;
        }

        return ret;
    }

    bool rgDx12Utils::StrToInputClassification(const std::string& str, D3D12_INPUT_CLASSIFICATION& inputClassification)
    {
        static std::map<std::string, D3D12_INPUT_CLASSIFICATION> inputClassificationMapping;
        bool ret = false;
        if (inputClassificationMapping.empty())
        {
            // Fill up the map.
            inputClassificationMapping.insert(std::make_pair<std::string,
                D3D12_INPUT_CLASSIFICATION>("D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA",
                    D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA));

            inputClassificationMapping.insert(std::make_pair<std::string,
                D3D12_INPUT_CLASSIFICATION>("D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA",
                    D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA));
        }

        auto iter = inputClassificationMapping.find(str);
        assert(iter != inputClassificationMapping.end());
        if (iter != inputClassificationMapping.end())
        {
            inputClassification = iter->second;
            ret = true;
        }

        return ret;
    }

    bool rgDx12Utils::StrToPrimitiveTopologyType(const std::string& str, D3D12_PRIMITIVE_TOPOLOGY_TYPE& primitiveTopologyType)
    {
        static std::map<std::string, D3D12_PRIMITIVE_TOPOLOGY_TYPE> primitiveTopologyTypeMapping;
        bool ret = false;
        if (primitiveTopologyTypeMapping.empty())
        {
            // Fill up the map.
            primitiveTopologyTypeMapping.insert(std::make_pair<std::string,
                D3D12_PRIMITIVE_TOPOLOGY_TYPE>("D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED",
                    D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED));

            primitiveTopologyTypeMapping.insert(std::make_pair<std::string,
                D3D12_PRIMITIVE_TOPOLOGY_TYPE>("D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT",
                    D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT));

            primitiveTopologyTypeMapping.insert(std::make_pair<std::string,
                D3D12_PRIMITIVE_TOPOLOGY_TYPE>("D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE",
                    D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE));

            primitiveTopologyTypeMapping.insert(std::make_pair<std::string,
                D3D12_PRIMITIVE_TOPOLOGY_TYPE>("D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE",
                    D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE));

            primitiveTopologyTypeMapping.insert(std::make_pair<std::string,
                D3D12_PRIMITIVE_TOPOLOGY_TYPE>("D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH",
                    D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH));
        }

        auto iter = primitiveTopologyTypeMapping.find(str);
        assert(iter != primitiveTopologyTypeMapping.end());
        if (iter != primitiveTopologyTypeMapping.end())
        {
            primitiveTopologyType = iter->second;
            ret = true;
        }

        return ret;
    }

    static std::string TrimCharacters(const std::string& str, const std::string& charsToTrim)
    {
        std::string ret;
        const auto strBegin = str.find_first_not_of(charsToTrim);
        if (strBegin != std::string::npos)
        {
            const auto strEnd = str.find_last_not_of(charsToTrim);
            const auto strRange = strEnd - strBegin + 1;
            ret = str.substr(strBegin, strRange);
        }
        return ret;
    }

    std::string rgDx12Utils::TrimWhitespace(const std::string& str)
    {
        const std::string& STR_WHITESPACE_CHAR = " \t";
        return TrimCharacters(str, STR_WHITESPACE_CHAR);
    }

    std::string rgDx12Utils::TrimNewline(const std::string& str)
    {
        const std::string& STR_NEWLINE_CHAR = "\n";
        return TrimCharacters(str, STR_NEWLINE_CHAR);
    }

    std::string rgDx12Utils::TrimWhitespaceAndCommas(const std::string& str)
    {
        const std::string& STR_WHITESPACE_CHAR = " \t\"";
        return TrimCharacters(str, STR_WHITESPACE_CHAR);
    }

    bool rgDx12Utils::DumpRootSignature(const std::string& outputFileName, ComPtr<ID3DBlob> signature)
    {
        auto *p = reinterpret_cast<char *>(signature->GetBufferPointer());
        auto  n = signature->GetBufferSize();
        std::vector<char> buff;
        buff.reserve(n);
        std::copy(p, p + n, std::back_inserter(buff));
        return rgDx12Utils::WriteBinaryFile(outputFileName, buff);
    }

    bool rgDx12Utils::DumpRootSignature(const std::string& outputFileName, const ComPtr<ID3D12Device> m_device,
        const D3D12_VERSIONED_ROOT_SIGNATURE_DESC& rsDesc)
    {
        bool ret = false;
        ComPtr<ID3DBlob> signature;
        ComPtr<ID3DBlob> error;
        ComPtr<ID3D12RootSignature> rootSignature;
        HRESULT hrRs = D3D12SerializeVersionedRootSignature(&rsDesc, &signature, &error);
        assert(SUCCEEDED(hrRs));
        if (SUCCEEDED(hrRs))
        {
            hrRs = m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
            assert(SUCCEEDED(hrRs));
            if (SUCCEEDED(hrRs))
            {
                ret = DumpRootSignature(outputFileName, signature);
            }
        }

        return ret;
    }

    bool rgDx12Utils::WriteBinaryFile(const std::string& fileName, const std::vector<char>& content)
    {
        bool ret = false;
        std::ofstream output;
        output.open(fileName.c_str(), std::ios::binary);

        if (output.is_open() && !content.empty())
        {
            output.write(&content[0], content.size());
            output.close();
            ret = true;
        }
        else
        {
            std::cerr << STR_ERR_BINARY_FILE_WRITE_FAILED << fileName << std::endl;
        }

        return ret;
    }
}