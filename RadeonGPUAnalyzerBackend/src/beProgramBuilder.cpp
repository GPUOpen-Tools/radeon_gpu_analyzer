//=================================================================
// Copyright 2017 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

#include <include/beProgramBuilder.h>

using namespace std;

void beProgramBuilder::UsePlatformNativeLineEndings(std::string& text)
{
    // If text is empty
    if (text.length() <= 0)
    {
        return;
    }

    // Remove all carriage returns.
    // This will put us into Linux format (from either Mac or Windows).
    // [With the AMD OpenCL stack as of April 2012, this does nothing.]
    text.erase(std::remove(text.begin(), text.end(), '\r'), text.end());

    // Add a linefeed at the end if there's not one there already.
    if (text[text.length() - 1] != '\n')
    {
        text += '\n';
    }

#ifdef _WIN32
    // Now convert all of the \n to \r\n.
    size_t pos = 0;

    while ((pos = text.find('\n', pos)) != string::npos)
    {
        text.replace(pos, 1, "\r\n");
        pos += 2;
    }

#endif
}

