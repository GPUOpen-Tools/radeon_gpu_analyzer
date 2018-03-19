// XML.
#include <tinyxml2/Include/tinyxml2.h>

// Local.
#include <RadeonGPUAnalyzerGUI/include/rgDataTypes.h>
#include <RadeonGPUAnalyzerGUI/include/rgXMLUtils.h>

bool rgXMLUtils::ReadNodeTextString(tinyxml2::XMLNode* pNode, std::string& str)
{
    bool ret = false;
    if (pNode != nullptr)
    {
        tinyxml2::XMLElement* pElem = pNode->ToElement();
        if (pElem != nullptr)
        {
            const char* pStr = pElem->GetText();
            if (pStr != nullptr)
            {
                str = pStr;
                ret = true;
            }
        }
    }
    return ret;
}

bool rgXMLUtils::ReadNodeTextUnsigned(tinyxml2::XMLNode* pNode, unsigned& val)
{
    bool ret = false;
    val = 0;
    if (pNode != nullptr)
    {
        tinyxml2::XMLElement* pElem = pNode->ToElement();
        if (pElem != nullptr)
        {
            ret = (pElem->QueryUnsignedText(&val) == tinyxml2::XML_SUCCESS);
        }
    }
    return ret;
}

bool rgXMLUtils::ReadNodeTextBool(tinyxml2::XMLNode* pNode, bool& val)
{
    bool ret = val = false;
    if (pNode != nullptr)
    {
        tinyxml2::XMLElement* pElem = pNode->ToElement();
        if (pElem != nullptr)
        {
            ret = (pElem->QueryBoolText(&val) == tinyxml2::XML_SUCCESS);
        }
    }
    return ret;
}

std::string rgXMLUtils::GetRGADataModelVersion()
{
    // The RGA data model version that this build uses.
    static const char* RGA_DATA_MODEL_VERSION = "2.0";
    return RGA_DATA_MODEL_VERSION;
}
