// C++.
#include <sstream>

// Infra.
#include "QtCommon/Util/QtUtil.h"

// Local.
#include "radeon_gpu_analyzer_gui/qt/rg_resource_usage_view.h"
#include "radeon_gpu_analyzer_gui/rg_data_types.h"
#include "radeon_gpu_analyzer_gui/rg_string_constants.h"

// *** INTERNALLY-LINKED AUXILIARY FUNCTIONS - BEGIN ***

// Use this to apply a different font color for highlighting hazards.
const char* kBeginHazardFont = "<FONT COLOR = '#ffffb2'> ";
const char* kEndHazardFont = "</FONT> ";

static const char* StartResourceSection(bool is_hazard)
{
    return is_hazard ? kBeginHazardFont : "";
}

static const char* EndResourceSection(bool is_hazard)
{
    return is_hazard ? kEndHazardFont : "";
}

// *** INTERNALLY-LINKED AUXILIARY FUNCTIONS - END ***

RgResourceUsageView::RgResourceUsageView(QWidget* parent)
    : QWidget(parent)
{
    ui_.setupUi(this);
}

void RgResourceUsageView::PopulateView(const RgResourceUsageData& resource_usage)
{
    // The size of the instruction cache in bytes.
    const int kIcacheSizeInBytes = 32768;

    // True if we should warn the user about resource usage info
    // that may imply a performance issue.
    bool is_resource_usage_hazard = false;

    // Register spills.
    QString sgpr_spills;
    QString vgpr_spills;

    bool is_vgpr_hazard = false;
    bool is_vgpr_spilled = false;
    bool is_sgpr_hazard = false;
    bool is_sgpr_spilled = false;
    bool is_lds_hazard = false;
    bool is_scratch_memory_hazard = false;
    bool is_icache_hazard = false;

    // SGPR spills.
    if (resource_usage.sgpr_spills > 0)
    {
        sgpr_spills = QString::number(resource_usage.sgpr_spills);
        is_sgpr_spilled = true;
    }

    // It's a SGPR hazard either if we spilled, or reached at our maximum usage.
    is_sgpr_hazard = is_sgpr_spilled || (resource_usage.used_sgprs >= resource_usage.available_sgprs);

    // VGPR spills.
    if (resource_usage.vgpr_spills > 0)
    {
        vgpr_spills = QString::number(resource_usage.vgpr_spills);
        is_vgpr_spilled = true;
    }

    // It's a VGPR hazard either if we spilled, or reached at our maximum usage.
    is_vgpr_hazard = is_vgpr_spilled || (resource_usage.used_vgprs >= resource_usage.available_vgprs);

    // Build the string to be presented.
    std::stringstream resource_usage_header_stream;

    // Add the header title text to the beginning of the stream.
    resource_usage_header_stream << "" << kStrGpuResourceUsage << "    ";

    // Append a string to display the number of VGPRs used over the number available.
    resource_usage_header_stream << "    | " << StartResourceSection(is_vgpr_hazard) << "<b>" << kStrResourceUsageVgprs << "</b>: " <<
        resource_usage.used_vgprs << " / " << resource_usage.available_vgprs;

    // Only add VGPR spills if relevant (> 0).
    if (!vgpr_spills.isEmpty())
    {
        resource_usage_header_stream << " -> " << vgpr_spills.toStdString() << " " << kStrResourceUsageSpills;
    }

    resource_usage_header_stream << EndResourceSection(is_vgpr_hazard);
    resource_usage_header_stream << " | ";

    // Create a string to display the number of SGPRs used over the number available.
    resource_usage_header_stream << StartResourceSection(is_sgpr_hazard) << "<b>" << kStrResourceUsageSgprs << ":</b> " << resource_usage.used_sgprs << " / " << resource_usage.available_sgprs;

    // Only add SGPR spills if relevant (> 0).
    if (!sgpr_spills.isEmpty())
    {
        resource_usage_header_stream << " -> " << sgpr_spills.toStdString() << " " << kStrResourceUsageSpills;
    }
    resource_usage_header_stream << EndResourceSection(is_sgpr_hazard);
    resource_usage_header_stream << " | ";

    // LDS.
    // Convert the LDS used and available byte counts to an abbreviated file size with an acronym.
    QString lds_bytes_used;
    QString lds_bytes_available;
    QtCommon::QtUtil::GetFilesizeAcronymFromByteCount(resource_usage.available_lds_bytes, lds_bytes_available);
    is_lds_hazard = (resource_usage.used_lds_bytes >= resource_usage.available_lds_bytes);

    // Set the LDS usage string in the view.
    resource_usage_header_stream << StartResourceSection(is_lds_hazard) << "<b>" << kStrResourceUsageLds << "</b>: " << resource_usage.used_lds_bytes << (resource_usage.used_lds_bytes > 0 ? " B" : "") << " / " << lds_bytes_available.toStdString() << EndResourceSection(is_lds_hazard) << " | ";

    // Scratch memory.
    QString scratch_mem;
    QtCommon::QtUtil::GetFilesizeAcronymFromByteCount(resource_usage.scratch_memory, scratch_mem);
    is_scratch_memory_hazard = (resource_usage.scratch_memory > 0);
    resource_usage_header_stream << StartResourceSection(is_scratch_memory_hazard) << "<b>" << kStrResourceUsageScratch << "</b>: " << scratch_mem.toStdString() << EndResourceSection(is_scratch_memory_hazard) << " | ";

    // Instruction cache.
    if (resource_usage.isa_size > 0)
    {
        // Check if the size of the code is larger the size of the instruction cache.
        is_icache_hazard = (resource_usage.isa_size > kIcacheSizeInBytes);

        // Only display the iCache usage if there is a hazard.
        if (is_icache_hazard)
        {
            resource_usage_header_stream << StartResourceSection(is_icache_hazard) << "<b>" << kStrResourceUsageIcache << "</b>: " << resource_usage.isa_size << " B / " << "32KB " << EndResourceSection(is_icache_hazard) << "|";
        }
    }

    // We have a hazard if any of the resources produces a hazard.
    is_resource_usage_hazard = is_vgpr_hazard || is_sgpr_hazard || is_lds_hazard || is_scratch_memory_hazard || is_icache_hazard;

    // Show the warning icon if a hazard was detected.
    ui_.warningLabel->setVisible(is_resource_usage_hazard);

    // Set the resource usage text in the view's title bar.
    std::string resourceUsageString = resource_usage_header_stream.str();
    ui_.resourceUsageHeaderLabel->setText(resourceUsageString.c_str());
}

std::string RgResourceUsageView::GetResourceUsageText()
{
    return ui_.resourceUsageHeaderLabel->text().toStdString();
}

QFont RgResourceUsageView::GetResourceUsageFont()
{
    return ui_.resourceUsageHeaderLabel->font();
}

void RgResourceUsageView::mousePressEvent(QMouseEvent* event)
{
    emit ResourceUsageViewClickedSignal();

    // Pass the event onto the base class.
    QWidget::mousePressEvent(event);
}

void RgResourceUsageView::focusOutEvent(QFocusEvent* event)
{
    emit ResourceUsageViewFocusOutEventSignal();

    // Pass the event onto the base class.
    QWidget::focusOutEvent(event);
}
