//=====================================================================
// Copyright 2025 Advanced Micro Devices, Inc. All rights reserved.

// C++.
#include <optional>
#include <string>
#include <unordered_map>

// Shared.
#include "common/rga_xml_constants.h"

#pragma once

// Types of rga entry points.
enum class RgaEntryType
{
    kUnknown,
    kOpenclKernel,
    kDxrRayGeneration,
    kDxrIntersection,
    kDxrAnyHit,
    kDxrClosestHit,
    kDxrMiss,
    kDxrCallable,
    kDxrTraversal,
    kDxrUnknown,
    kDxVertex,
    kDxHull,
    kDxDomain,
    kDxGeometry,
    kDxPixel,
    kDxCompute,
    kDxMesh,
    kDxAmplification,
    kVkRayGeneration,
    kVkIntersection,
    kVkAnyHit,
    kVkClosestHit,
    kVkMiss,
    kVkCallable,
    kVkTraversal,
    kVkUnknown,
    kVkVertex,
    kVkTessControl,
    kVkTessEval,
    kVkGeometry,
    kVkFragment,
    kVkCompute,
    kVkMesh,
    kVkTask,
    kGlVertex,
    kGlTessControl,
    kGlTessEval,
    kGlGeometry,
    kGlFragment,
    kGlCompute
};

class RgaEntryTypeUtils
{
public:
    // Returns true if the compute bit is set for given entry_type.
    static bool IsComputeBitSet(RgaEntryType rga_entry_type)
    {
        bool ret = false;
        switch (rga_entry_type)
        {
        case RgaEntryType::kDxCompute:
        case RgaEntryType::kGlCompute:
            ret = true;
            break;
        default:
            ret = false;
            break;
        }
        return ret;
    }

    // Get the type of rga entry as a string.
    static bool GetEntryTypeStr(RgaEntryType rga_entry_type, std::string& entry_type_str)
    {
        bool ret = true;
        switch (rga_entry_type)
        {
        case RgaEntryType::kOpenclKernel:
            entry_type_str = kStrXmlNodeOpencl;
            break;
        case RgaEntryType::kDxrRayGeneration:
            entry_type_str = kStrXmlNodeDxrRayGeneration;
            break;
        case RgaEntryType::kDxrIntersection:
            entry_type_str = kStrXmlNodeDxrIntersection;
            break;
        case RgaEntryType::kDxrAnyHit:
            entry_type_str = kStrXmlNodeDxrAnyHit;
            break;
        case RgaEntryType::kDxrClosestHit:
            entry_type_str = kStrXmlNodeDxrClosestHit;
            break;
        case RgaEntryType::kDxrMiss:
            entry_type_str = kStrXmlNodeDxrMiss;
            break;
        case RgaEntryType::kDxrCallable:
            entry_type_str = kStrXmlNodeDxrCallable;
            break;
        case RgaEntryType::kDxrTraversal:
            entry_type_str = kStrXmlNodeDxrTraversal;
            break;
        case RgaEntryType::kDxrUnknown:
            entry_type_str = kStrXmlNodeDxrUnknown;
            break;
        case RgaEntryType::kDxVertex:
            entry_type_str = kStrXmlNodeDxVertex;
            break;
        case RgaEntryType::kDxHull:
            entry_type_str = kStrXmlNodeDxHull;
            break;
        case RgaEntryType::kDxDomain:
            entry_type_str = kStrXmlNodeDxDomain;
            break;
        case RgaEntryType::kDxGeometry:
            entry_type_str = kStrXmlNodeDxGeometry;
            break;
        case RgaEntryType::kDxPixel:
            entry_type_str = kStrXmlNodeDxPixel;
            break;
        case RgaEntryType::kDxCompute:
            entry_type_str = kStrXmlNodeDxCompute;
            break;
        case RgaEntryType::kDxMesh:
            entry_type_str = kStrXmlNodeDxMesh;
            break;
        case RgaEntryType::kDxAmplification:
            entry_type_str = kStrXmlNodeDxAmplification;
            break;
        case RgaEntryType::kGlVertex:
            entry_type_str = kStrXmlNodeGlVertex;
            break;
        case RgaEntryType::kGlTessControl:
            entry_type_str = kStrXmlNodeGlTessCtrl;
            break;
        case RgaEntryType::kGlTessEval:
            entry_type_str = kStrXmlNodeGlTessEval;
            break;
        case RgaEntryType::kGlGeometry:
            entry_type_str = kStrXmlNodeGlGeometry;
            break;
        case RgaEntryType::kGlFragment:
            entry_type_str = kStrXmlNodeGlFragment;
            break;
        case RgaEntryType::kGlCompute:
            entry_type_str = kStrXmlNodeGlCompute;
            break;
        case RgaEntryType::kVkRayGeneration:
            entry_type_str = kStrXmlNodeVkRayGeneration;
            break;
        case RgaEntryType::kVkIntersection:
            entry_type_str = kStrXmlNodeVkIntersection;
            break;
        case RgaEntryType::kVkAnyHit:
            entry_type_str = kStrXmlNodeVkAnyHit;
            break;
        case RgaEntryType::kVkClosestHit:
            entry_type_str = kStrXmlNodeVkClosestHit;
            break;
        case RgaEntryType::kVkMiss:
            entry_type_str = kStrXmlNodeVkMiss;
            break;
        case RgaEntryType::kVkCallable:
            entry_type_str = kStrXmlNodeVkCallable;
            break;
        case RgaEntryType::kVkTraversal:
            entry_type_str = kStrXmlNodeVkTraversal;
            break;
        case RgaEntryType::kVkUnknown:
            entry_type_str = kStrXmlNodeVkUnknown;
            break;
        case RgaEntryType::kVkVertex:
            entry_type_str = kStrXmlNodeVkVertex;
            break;
        case RgaEntryType::kVkTessControl:
            entry_type_str = kStrXmlNodeVkTessCtrl;
            break;
        case RgaEntryType::kVkTessEval:
            entry_type_str = kStrXmlNodeVkTessEval;
            break;
        case RgaEntryType::kVkGeometry:
            entry_type_str = kStrXmlNodeVkGeometry;
            break;
        case RgaEntryType::kVkFragment:
            entry_type_str = kStrXmlNodeVkFragment;
            break;
        case RgaEntryType::kVkCompute:
            entry_type_str = kStrXmlNodeVkCompute;
            break;
        case RgaEntryType::kVkMesh:
            entry_type_str = kStrXmlNodeVkMesh;
            break;
        case RgaEntryType::kVkTask:
            entry_type_str = kStrXmlNodeVkTask;
            break;
        case RgaEntryType::kUnknown:
            entry_type_str = kStrXmlNodeUnknown;
            break;
        default:
            ret = false;
            break;
        }
        return ret;
    }

    static std::optional<RgaEntryType> GetEntryType(const std::string& str)
    {
        static const std::unordered_map<std::string, RgaEntryType> map = {{kStrXmlNodeOpencl, RgaEntryType::kOpenclKernel},
                                                                          {kStrXmlNodeDxrRayGeneration, RgaEntryType::kDxrRayGeneration},
                                                                          {kStrXmlNodeDxrIntersection, RgaEntryType::kDxrIntersection},
                                                                          {kStrXmlNodeDxrAnyHit, RgaEntryType::kDxrAnyHit},
                                                                          {kStrXmlNodeDxrClosestHit, RgaEntryType::kDxrClosestHit},
                                                                          {kStrXmlNodeDxrMiss, RgaEntryType::kDxrMiss},
                                                                          {kStrXmlNodeDxrCallable, RgaEntryType::kDxrCallable},
                                                                          {kStrXmlNodeDxrTraversal, RgaEntryType::kDxrTraversal},
                                                                          {kStrXmlNodeDxrUnknown, RgaEntryType::kDxrUnknown},
                                                                          {kStrXmlNodeDxVertex, RgaEntryType::kDxVertex},
                                                                          {kStrXmlNodeDxHull, RgaEntryType::kDxHull},
                                                                          {kStrXmlNodeDxDomain, RgaEntryType::kDxDomain},
                                                                          {kStrXmlNodeDxGeometry, RgaEntryType::kDxGeometry},
                                                                          {kStrXmlNodeDxPixel, RgaEntryType::kDxPixel},
                                                                          {kStrXmlNodeDxCompute, RgaEntryType::kDxCompute},
                                                                          {kStrXmlNodeDxMesh, RgaEntryType::kDxMesh},
                                                                          {kStrXmlNodeDxAmplification, RgaEntryType::kDxAmplification},
                                                                          {kStrXmlNodeGlVertex, RgaEntryType::kGlVertex},
                                                                          {kStrXmlNodeGlTessCtrl, RgaEntryType::kGlTessControl},
                                                                          {kStrXmlNodeGlTessEval, RgaEntryType::kGlTessEval},
                                                                          {kStrXmlNodeGlGeometry, RgaEntryType::kGlGeometry},
                                                                          {kStrXmlNodeGlFragment, RgaEntryType::kGlFragment},
                                                                          {kStrXmlNodeGlCompute, RgaEntryType::kGlCompute},
                                                                          {kStrXmlNodeVkRayGeneration, RgaEntryType::kVkRayGeneration},
                                                                          {kStrXmlNodeVkIntersection, RgaEntryType::kVkIntersection},
                                                                          {kStrXmlNodeVkAnyHit, RgaEntryType::kVkAnyHit},
                                                                          {kStrXmlNodeVkClosestHit, RgaEntryType::kVkClosestHit},
                                                                          {kStrXmlNodeVkMiss, RgaEntryType::kVkMiss},
                                                                          {kStrXmlNodeVkCallable, RgaEntryType::kVkCallable},
                                                                          {kStrXmlNodeVkTraversal, RgaEntryType::kVkTraversal},
                                                                          {kStrXmlNodeVkUnknown, RgaEntryType::kVkUnknown},
                                                                          {kStrXmlNodeVkVertex, RgaEntryType::kVkVertex},
                                                                          {kStrXmlNodeVkTessCtrl, RgaEntryType::kVkTessControl},
                                                                          {kStrXmlNodeVkTessEval, RgaEntryType::kVkTessEval},
                                                                          {kStrXmlNodeVkGeometry, RgaEntryType::kVkGeometry},
                                                                          {kStrXmlNodeVkFragment, RgaEntryType::kVkFragment},
                                                                          {kStrXmlNodeVkCompute, RgaEntryType::kVkCompute},
                                                                          {kStrXmlNodeVkMesh, RgaEntryType::kVkMesh},
                                                                          {kStrXmlNodeVkTask, RgaEntryType::kVkTask}};

        auto it = map.find(str);
        if (it != map.end())
        {
            return it->second;
        }
        return std::nullopt;
    }

    static bool CompareEntryTypeStr(const std::string& entry_str_lhs, const std::string& entry_str_rhs)
    {
        auto lhs = GetEntryType(entry_str_lhs);
        auto rhs = GetEntryType(entry_str_rhs);

        if (lhs && rhs)
        {
            return static_cast<int>(*lhs) < static_cast<int>(*rhs);
        }
        else if (lhs)
        {
            return true;
        }
        else if (rhs)
        {
            return false;
        }
        else
        {
            return entry_str_lhs < entry_str_rhs;
        }
    }
};
