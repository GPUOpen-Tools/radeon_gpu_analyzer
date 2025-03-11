// XML.
#include "tinyxml2.h"

// Local.
#include "radeon_gpu_analyzer_gui/rg_data_types.h"
#include "radeon_gpu_analyzer_gui/rg_xml_utils.h"
#include "radeon_gpu_analyzer_gui/rg_config_file_definitions.h"

bool RgXMLUtils::ReadNodeTextString(tinyxml2::XMLNode* node, std::string& str)
{
    bool ret = false;
    if (node != nullptr)
    {
        tinyxml2::XMLElement* elem = node->ToElement();
        if (elem != nullptr)
        {
            const char* value = elem->GetText();
            if (value != nullptr)
            {
                str = value;
                ret = true;
            }
        }
    }
    return ret;
}

bool RgXMLUtils::ReadNodeTextUnsigned(tinyxml2::XMLNode* node, unsigned& val)
{
    bool ret = false;
    val = 0;
    if (node != nullptr)
    {
        tinyxml2::XMLElement* elem = node->ToElement();
        if (elem != nullptr)
        {
            ret = (elem->QueryUnsignedText(&val) == tinyxml2::XML_SUCCESS);
        }
    }
    return ret;
}

bool RgXMLUtils::ReadNodeTextBool(tinyxml2::XMLNode* node, bool& val)
{
    bool ret = val = false;
    if (node != nullptr)
    {
        tinyxml2::XMLElement* elem = node->ToElement();
        if (elem != nullptr)
        {
            ret = (elem->QueryBoolText(&val) == tinyxml2::XML_SUCCESS);
        }
    }
    return ret;
}

std::string RgXMLUtils::GetRGADataModelVersion()
{
    return kRgaDataModel;
}
