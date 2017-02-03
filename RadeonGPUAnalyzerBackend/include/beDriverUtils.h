#ifndef __beDriverUtils_h
#define __beDriverUtils_h

// Local.
#include <RadeonGPUAnalyzerBackend/include/beRGADllBuild.h>

// Infra.
#include <AMDTBaseTools/Include/gtString.h>

class RGA_BACKEND_DECLDIR beDriverUtils
{
public:
    static bool GetDriverPackagingVersion(gtString& driverPackagingVersion);
private:
    beDriverUtils();
    ~beDriverUtils();
};

#endif // __beDriverUtils_h
