// C++.
#include <sstream>

// Infra.
#include <QtCommon/Util/QtUtil.h>

// Local.
#include <RadeonGPUAnalyzerGUI/include/qt/rgResourceUsageView.h>
#include <RadeonGPUAnalyzerGUI/include/rgDataTypes.h>
#include <RadeonGPUAnalyzerGUI/include/rgStringConstants.h>

// *** INTERNALLY-LINKED AUXILIARY FUNCTIONS - BEGIN ***

// Use this to apply a different font color for highlighting hazards.
const char* BEGIN_HAZARD_FONT = "<FONT COLOR = '#ffffb2'> ";
const char* END_HAZARD_FONT = "</FONT> ";

static const char* StartResourceSection(bool isHazard)
{
    return isHazard ? BEGIN_HAZARD_FONT : "";
}

static const char* EndResourceSection(bool isHazard)
{
    return isHazard ? END_HAZARD_FONT : "";
}

// *** INTERNALLY-LINKED AUXILIARY FUNCTIONS - END ***

rgResourceUsageView::rgResourceUsageView(QWidget* pParent)
    : QWidget(pParent)
{
    ui.setupUi(this);
}

void rgResourceUsageView::PopulateView(const rgResourceUsageData& resourceUsage)
{
    // The size of the instruction cache in bytes.
    const int ICACHE_SIZE_IN_BYTES = 32768;

    // Max LDS allocation is 64KB.
    const int MAX_LDS_USAGE = 65536;

    // True if we should warn the user about resource usage info
    // that may imply a performance issue.
    bool isResourceUsageHazard = false;

    // Register spills.
    QString sgprSpills;
    QString vgprSpills;

    bool isVgprHazard = false;
    bool isSgprHazard = false;
    bool isLdsHazard = false;
    bool isScratchMemoryHazard = false;
    bool isIcacheHazard = false;


    // SGPR spills.
    if (resourceUsage.m_sgprSpills > 0)
    {
        sgprSpills = QString::number(resourceUsage.m_sgprSpills);
        isSgprHazard = true;
    }

    // VGPR spills.
    if (resourceUsage.m_vgprSpills > 0)
    {
        vgprSpills = QString::number(resourceUsage.m_vgprSpills);
        isVgprHazard = true;
    }

    // Build the string to be presented.
    std::stringstream resourceUsageHeaderStream;

    // Add the header title text to the beginning of the stream.
    resourceUsageHeaderStream << "" << STR_GPU_RESOURCE_USAGE << "    ";

    // Append a string to display the number of VGPRs used over the number available.
    resourceUsageHeaderStream << "    | " << StartResourceSection(isVgprHazard) << "<b>" << STR_RESOURCE_USAGE_VGPRS << "</b>: " <<
        resourceUsage.m_usedVgprs << " / " << resourceUsage.m_availableVgprs;

    // Only add VGPR spills if relevant (> 0).
    if (!vgprSpills.isEmpty())
    {
        resourceUsageHeaderStream << " -> " << vgprSpills.toStdString() << " " << STR_RESOURCE_USAGE_SPILLS;
    }

    resourceUsageHeaderStream << EndResourceSection(isVgprHazard);
    resourceUsageHeaderStream << " | ";


    // Create a string to display the number of SGPRs used over the number available.
    resourceUsageHeaderStream << StartResourceSection(isSgprHazard) << "<b>" << STR_RESOURCE_USAGE_SGPRS << ":</b> " << resourceUsage.m_usedSgprs << " / " << resourceUsage.m_availableSgprs;

    // Only add SGPR spills if relevant (> 0).
    if (!sgprSpills.isEmpty())
    {
        resourceUsageHeaderStream << " -> " << sgprSpills.toStdString() << " " << STR_RESOURCE_USAGE_SPILLS;
    }
    resourceUsageHeaderStream << EndResourceSection(isSgprHazard);
    resourceUsageHeaderStream << " | ";

    // LDS.
    // Convert the LDS used and available byte counts to an abbreviated file size with an acronym.
    QString ldsBytesUsed;
    QString ldsBytesAvailable;
    QtCommon::QtUtil::GetFilesizeAcronymFromByteCount(resourceUsage.m_availableLdsBytes, ldsBytesAvailable);
    isLdsHazard = (resourceUsage.m_usedLdsBytes > MAX_LDS_USAGE);

    // Set the LDS usage string in the view.
    resourceUsageHeaderStream << StartResourceSection(isLdsHazard) << "<b>" << STR_RESOURCE_USAGE_LDS << "</b>: " << resourceUsage.m_usedLdsBytes << (resourceUsage.m_usedLdsBytes > 0 ? " B" : "") << " / " << ldsBytesAvailable.toStdString() << EndResourceSection(isLdsHazard) << " | ";

    // Scratch memory.
    QString scratchMem;
    QtCommon::QtUtil::GetFilesizeAcronymFromByteCount(resourceUsage.m_scratchMemory, scratchMem);
    isScratchMemoryHazard = (resourceUsage.m_scratchMemory > 0);
    resourceUsageHeaderStream << StartResourceSection(isScratchMemoryHazard) << "<b>" << STR_RESOURCE_USAGE_SCRATCH << "</b>: " << scratchMem.toStdString() << EndResourceSection(isScratchMemoryHazard) << " | ";

    // Instruction cache.
    if (resourceUsage.m_isaSize > 0)
    {
        // Check if the size of the code is larger the size of the instruction cache.
        isIcacheHazard = (resourceUsage.m_isaSize > ICACHE_SIZE_IN_BYTES);

        // Only display the iCache usage if there is a hazard.
        if (isIcacheHazard)
        {
            resourceUsageHeaderStream << StartResourceSection(isIcacheHazard) << "<b>" << STR_RESOURCE_USAGE_ICACHE << "</b>: " << resourceUsage.m_isaSize << " B / " << "32KB " << EndResourceSection(isIcacheHazard) << "|";
        }
    }

    // We have a hazard if any of the resources produces a hazard.
    isResourceUsageHazard = isVgprHazard || isVgprHazard || isLdsHazard || isScratchMemoryHazard || isIcacheHazard;

    // Show the warning icon if a hazard was detected.
    ui.warningLabel->setVisible(isResourceUsageHazard);

    // Set the resource usage text in the view's title bar.
    std::string resourceUsageString = resourceUsageHeaderStream.str();
    ui.resourceUsageHeaderLabel->setText(resourceUsageString.c_str());
}

std::string rgResourceUsageView::GetResourceUsageText()
{
    return ui.resourceUsageHeaderLabel->text().toStdString();
}

QFont rgResourceUsageView::GetResourceUsageFont()
{
    return ui.resourceUsageHeaderLabel->font();
}
