#pragma once

// A set of utility functions to simplify processing XML.
class rgXMLUtils
{
public:
    // Returns the text value of a given node.
    static bool ReadNodeTextString(tinyxml2::XMLNode* pNode, std::string& str);

    // Reads a node's text as an unsigned number.
    static bool ReadNodeTextUnsigned(tinyxml2::XMLNode* pNode, unsigned& val);

    // Reads a node's text as a boolean value.
    static bool ReadNodeTextBool(tinyxml2::XMLNode* pNode, bool& val);

    // The RGA data model version that this build uses.
    static std::string GetRGADataModelVersion();

    // Append a new XML element to the given parent node.
    template <typename T>
    static void AppendXMLElement(tinyxml2::XMLDocument &xmlDoc, tinyxml2::XMLElement* pParentElement, const char* pElemName, T elemValue)
    {
        if (pParentElement != nullptr && pElemName != nullptr)
        {
            tinyxml2::XMLElement* pElem = xmlDoc.NewElement(pElemName);
            if (pElem != nullptr)
            {
                pElem->SetText(elemValue);
                pParentElement->InsertEndChild(pElem);
            }
        }
    }

private:
    rgXMLUtils() = default;
    ~rgXMLUtils() = default;
};