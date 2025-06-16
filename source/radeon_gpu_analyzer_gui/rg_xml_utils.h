//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header for set of utility functions to simplify processing XML.
//=============================================================================
#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_RG_XML_UTILS_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_RG_XML_UTILS_H_

// A set of utility functions to simplify processing XML.
class RgXMLUtils
{
public:
    // Returns the text value of a given node.
    static bool ReadNodeTextString(tinyxml2::XMLNode* node, std::string& str);

    // Reads a node's text as an unsigned number.
    static bool ReadNodeTextUnsigned(tinyxml2::XMLNode* node, unsigned& val);

    // Reads a node's text as a boolean value.
    static bool ReadNodeTextBool(tinyxml2::XMLNode* node, bool& val);

    // The RGA data model version that this build uses.
    static std::string GetRGADataModelVersion();

    // Append a new XML element to the given parent node.
    template <typename T>
    static void AppendXMLElement(tinyxml2::XMLDocument &xml_doc, tinyxml2::XMLElement* parent_element, const char* elem_name, T elem_value)
    {
        if (parent_element != nullptr && elem_name != nullptr)
        {
            tinyxml2::XMLElement* elem = xml_doc.NewElement(elem_name);
            if (elem != nullptr)
            {
                elem->SetText(elem_value);
                parent_element->InsertEndChild(elem);
            }
        }
    }

private:
    RgXMLUtils() = default;
    ~RgXMLUtils() = default;
};
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_RG_XML_UTILS_H_
