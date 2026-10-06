//=============================================================================
// Copyright (c) 2022-2026 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Implementation for an isa spec explorer utility.
//=============================================================================

// C++.
#include <cassert>
#include <filesystem>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <unordered_map>

// Infra.
#include "external/amdt_os_wrappers/Include/osFilePath.h"
#include "external/amdt_os_wrappers/Include/osDirectory.h"
#include "external/amdt_os_wrappers/Include/osApplication.h"

// Shared.
#include "common/rga_shared_utils.h"

// Local.
#include "radeon_gpu_analyzer_backend/be_analysis_summary.h"
#include "radeon_gpu_analyzer_backend/be_isa_spec_metadata.h"

// Error strings.
static const char* kStringErrorManagerImplAllocationFailed = "Error: Manager Implementation object allocation failed";
static const char* kStringErrorDecodeManagerUnknownArch    = "Error: Undefined architecture in specification.";
static const char* kStringErrorDecodeManagerInitFailed     = "Error: Failed to initialize ISA specification. Provided file path: ";

namespace
{
    // Using declarations for isa explorer.
    using IsaExplorer    = amdisa::explorer::Spec;
    using IsaExplorerPtr = std::shared_ptr<IsaExplorer>;

    // Maps from explorer Name() abbreviations to human-readable display names.
    // On the public amd-main branch, explorer Name() returns abbreviations (e.g. "SALU"),
    // not display names (e.g. "Scalar ALU"). These maps bridge the gap.
    static const std::unordered_map<std::string, std::string> kFunctionalGroupDisplayNames = {
        {"SALU",         "Scalar ALU"},
        {"SMEM",         "Scalar Memory"},
        {"VALU",         "Vector ALU"},
        {"VMEM",         "Vector Memory"},
        {"EXPORT",       "Export"},
        {"BRANCH",       "Branch"},
        {"MESSAGE",      "Message"},
        {"WAVE_CONTROL", "Wave Control"},
        {"TRAP",         "Trap"},
    };

    static const std::unordered_map<std::string, std::string> kFunctionalSubgroupDisplayNames = {
        {"FLOATING_POINT", "Floating Point"},
        {"BUFFER",         "Buffer"},
        {"TEXTURE",        "Texture"},
        {"LOAD",           "Load"},
        {"STORE",          "Store"},
        {"SAMPLE",         "Sample"},
        {"BVH",            "BVH"},
        {"ATOMIC",         "Atomic"},
        {"FLAT",           "Flat"},
        {"DATA_SHARE",     "Data Share"},
        {"STATIC",         "Static"},
        {"MFMA",           "MFMA"},
        {"WMMA",           "WMMA"},
        {"TRANSCENDENTAL", "Transcendental"},
    };

    // Look up a display name from an abbreviation map, falling back to the raw name.
    static std::string LookupDisplayName(const std::unordered_map<std::string, std::string>& map, const std::string& name)
    {
        auto it = map.find(name);
        return (it != map.end()) ? it->second : name;
    }

    // Empty functional groups map.
    const std::map<std::string, amdisa::explorer::FunctionalGroup> empty_functional_groups_;

    // The active explorer for the active architecture.
    IsaExplorerPtr isa_explorer = nullptr;

    // Convenience wrapper for handling multiple architectures.
    class BeExplorerManager
    {
    public:
        // Initialize -- Accepts the vector of file paths to specs and initializes the API with the provided spec files.
        bool Initialize(const std::map<amdisa::GpuArchitecture, std::string>& input_spec_file_paths, std::string& err_message);

        // GetExplorer -- Accepts the architecture enum for which amdisa::IsaExplorer is requested.
        // Returns pointer to the explorer if the provided architecture was found, and the nullptr otherwise.
        IsaExplorerPtr GetExplorer(amdisa::GpuArchitecture architecture) const;

        BeExplorerManager() = default;
        ~BeExplorerManager();

    private:
        struct ExplorerManagerImpl;
        ExplorerManagerImpl* manager_impl_ = nullptr;
    };

    // Explorer manager implementation.
    struct BeExplorerManager::ExplorerManagerImpl
    {
        std::map<amdisa::GpuArchitecture, IsaExplorerPtr> arch_to_explorer;
    };

    bool BeExplorerManager::Initialize(const std::map<amdisa::GpuArchitecture, std::string>& input_spec_file_paths, std::string& err_message)
    {
        bool should_abort = false;

        if (manager_impl_ == nullptr)
        {
            manager_impl_ = new ExplorerManagerImpl();
        }
        else
        {
            delete manager_impl_;
            manager_impl_ = new ExplorerManagerImpl();
        }

        if (manager_impl_ == nullptr)
        {
            should_abort = true;
            err_message  = kStringErrorManagerImplAllocationFailed;
        }

        // Initialize IsaExplorer for each XML spec paths provided.
        for (auto it = input_spec_file_paths.begin(); !should_abort && it != input_spec_file_paths.end(); it++)
        {
            std::string    init_err_message;
            IsaExplorerPtr explorer         = std::make_shared<IsaExplorer>();
            bool           is_explorer_init = explorer->Init(it->second, init_err_message);
            if (is_explorer_init)
            {
                amdisa::GpuArchitecture architecture = it->first;
                if (architecture != amdisa::GpuArchitecture::kUnknown)
                {
                    manager_impl_->arch_to_explorer[architecture] = explorer;
                }
                else
                {
                    should_abort = true;
                    err_message  = kStringErrorDecodeManagerUnknownArch;
                }
            }
            else
            {
                should_abort = true;
                std::stringstream error_ss;
                error_ss << kStringErrorDecodeManagerInitFailed << it->second;
                err_message = error_ss.str();
            }
        }

        return !should_abort;
    }

    IsaExplorerPtr BeExplorerManager::GetExplorer(amdisa::GpuArchitecture architecture) const
    {
        IsaExplorerPtr ret = nullptr;
        // Get the explorer from the map.
        auto explorer_iter = manager_impl_->arch_to_explorer.find(architecture);
        if (explorer_iter != manager_impl_->arch_to_explorer.end())
        {
            ret = explorer_iter->second;
        }
        return ret;
    }

    BeExplorerManager::~BeExplorerManager()
    {
        if (manager_impl_ != nullptr)
        {
            delete manager_impl_;
            manager_impl_ = nullptr;
        }
    }

    // The manager of all the architectures.
    BeExplorerManager explorer_manager;

    // Isa explorer initialization status.
    bool is_explorer_initialized = false;

}  // namespace

bool beKA::BeIsaSpecExplorer::SetArchitecture(amdisa::GpuArchitecture architecture)
{
    if (!is_explorer_initialized)
    {
        return false;
    }

    isa_explorer = explorer_manager.GetExplorer(architecture);

    return isa_explorer != nullptr;
}

bool beKA::BeIsaSpecExplorer::InitializeExplorer(const std::map<amdisa::GpuArchitecture, std::string>& xml_files, std::string& explorer_status_error)
{
    bool       ret = true;
    osFilePath application_dir;
    osGetCurrentApplicationPath(application_dir, false);

    std::filesystem::path isa_spec_dir_path(application_dir.fileDirectoryAsString().asASCIICharArray());
    isa_spec_dir_path /= "utils";
    isa_spec_dir_path /= "isa_spec";
    isa_spec_dir_path.make_preferred();

    std::map<amdisa::GpuArchitecture, std::string> xml_file_paths;

    for (const auto& isa_spec_name : xml_files)
    {
        std::filesystem::path isa_spec_path(isa_spec_dir_path);

        isa_spec_path /= isa_spec_name.second;

        isa_spec_path.make_preferred();

        xml_file_paths[isa_spec_name.first] = isa_spec_path.string();
    }

    if (!xml_file_paths.empty())
    {
        is_explorer_initialized = explorer_manager.Initialize(xml_file_paths, explorer_status_error);

        if (!is_explorer_initialized)
        {
            ret = false;
        }
    }

    return ret;
}

bool beKA::BeIsaSpecExplorer::SetArchitecture(const std::string& target_gpu)
{
    amdisa::GpuArchitecture architecture = amdisa::GpuArchitecture::kUnknown;
    bool                    success      = RgaSharedUtils::GetGpuArchitectureFromTarget(target_gpu, architecture);
    return SetArchitecture(architecture) && success;
}

bool beKA::BeIsaSpecExplorer::PopulateFromSpec(const std::string& target_gpu, RgaAnalysisSummary::TargetArchitectureMetadata& metadata)
{
    bool success = SetArchitecture(target_gpu);
    if (success && isa_explorer != nullptr)
    {
        // Collect unique (group_name, subgroup_combo) pairs from all instructions.
        std::set<std::pair<std::string, std::string>> unique_fg_pairs;
        std::map<std::string, std::string>            fg_name_to_desc;

        for (const auto& [instr_name, instr] : isa_explorer->GetInstructions())
        {
            const auto* func_group = instr.FuncGroup();
            if (func_group == nullptr)
            {
                continue;
            }

            std::string group_name = LookupDisplayName(kFunctionalGroupDisplayNames, func_group->Name());

            // Store description for later lookup.
            if (fg_name_to_desc.find(group_name) == fg_name_to_desc.end())
            {
                fg_name_to_desc[group_name] = func_group->Description();
            }

            // Build ordered subgroup string from the instruction's subgroups.
            std::vector<std::string> subgroup_names;
            for (const auto& sg : instr.FuncSubgroups())
            {
                const std::string sg_name = LookupDisplayName(kFunctionalSubgroupDisplayNames, sg.Name());
                if (sg_name != "-" && !sg_name.empty())
                {
                    subgroup_names.push_back(sg_name);
                }
            }
            std::string subgroup_str = beKA::BeAnalysisSummaryUtils::BuildFunctionalSubgroupStringFromNames(subgroup_names);

            unique_fg_pairs.insert({group_name, subgroup_str});
        }

        // Emit one metadata entry per unique (group, subgroup) pair.
        int id_counter = 0;
        for (const auto& [group_name, sub_group_name] : unique_fg_pairs)
        {
            RgaAnalysisSummary::TargetArchitectureMetadata::FunctionalGroupMetadata fg_meta;
            fg_meta.id_             = id_counter++;
            fg_meta.group_name_     = group_name;
            fg_meta.sub_group_name_ = sub_group_name;

            auto desc_it = fg_name_to_desc.find(group_name);
            if (desc_it != fg_name_to_desc.end())
            {
                fg_meta.description_ = desc_it->second;
            }

            metadata.instruction_functional_groups_.push_back(fg_meta);
        }

        id_counter = 0;
        for (int i = 0; i < static_cast<int>(RegisterType::kCount); ++i)
        {
            RegisterType                                                 register_type = static_cast<RegisterType>(i);
            RgaAnalysisSummary::TargetArchitectureMetadata::MetadataType register_metadata;
            switch (register_type)
            {
            case RegisterType::kVGPR:
                register_metadata.id_   = id_counter++;
                register_metadata.name_ = "VGPR";
                break;
            case RegisterType::kSGPR:
                register_metadata.id_   = id_counter++;
                register_metadata.name_ = "SGPR";
                break;
            default:
                continue;
            }
            metadata.register_types_.push_back(register_metadata);
        }

        id_counter = 0;
        for (int i = 0; i < static_cast<int>(BranchType::kCount); ++i)
        {
            BranchType                                                   branch_type = static_cast<BranchType>(i);
            RgaAnalysisSummary::TargetArchitectureMetadata::MetadataType branch_metadata;
            branch_metadata.id_   = id_counter++;
            branch_metadata.name_ = GetBranchTypeStr(branch_type);
            metadata.branch_types_.push_back(branch_metadata);
        }
    }

    return success;
}

std::string beKA::BeIsaSpecExplorer::GetBranchTypeStr(BranchType branch_type)
{
    std::string branch_type_str;
    switch (branch_type)
    {
    case BranchType::kFallThrough:
        branch_type_str = "FallThrough";
        break;
    case BranchType::kDirectBranch:
        branch_type_str = "DirectBranch";
        break;
    case BranchType::kIndirectBranch:
        branch_type_str = "IndirectBranch";
        break;
    default:
        assert(false);
    }
    return branch_type_str;
}
