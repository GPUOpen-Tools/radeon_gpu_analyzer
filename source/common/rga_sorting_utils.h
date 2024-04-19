#pragma once
// C++.
#include <string>
#include <unordered_map>
#include <algorithm>

// Local.
#include "common/rga_shared_utils.h"

// GPU Architectures.
static const char* kStrIpv6    = "Graphics IP v6";
static const char* kStrIpv7    = "Graphics IP v7";
static const char* kStrIpv8    = "Graphics IP v8";
static const char* kStrVega    = "VEGA";
static const char* kStrRdna    = "RDNA";
static const char* kStrCdna    = "CDNA";
static const char* kStrRdna2   = "RDNA2";
static const char* kStrCdna2   = "CDNA2";
static const char* kStrRdna3   = "RDNA3";
static const char* kStrCdna3   = "CDNA3";

enum class GpuArchitecture
{
    GpuArchitectureIpv6,
    GpuArchitectureIpv7,
    GpuArchitectureIpv8,
    GpuArchitectureVega,
    GpuArchitectureRdna,
    GpuArchitectureCdna,
    GpuArchitectureRdna2,
    GpuArchitectureCdna2,
    GpuArchitectureRdna3,
    GpuArchitectureCdna3
};

// GPU Architectures sort priority.
static const std::unordered_map<std::string, GpuArchitecture> kGpuSortPriority = {{kStrIpv6, GpuArchitecture::GpuArchitectureIpv6},
                                                                                  {kStrIpv6, GpuArchitecture::GpuArchitectureIpv7},
                                                                                  {kStrIpv6, GpuArchitecture::GpuArchitectureIpv8},
                                                                                  {kStrVega, GpuArchitecture::GpuArchitectureVega},
                                                                                  {kStrRdna, GpuArchitecture::GpuArchitectureRdna},
                                                                                  {kStrCdna, GpuArchitecture::GpuArchitectureCdna},
                                                                                  {kStrRdna2, GpuArchitecture::GpuArchitectureRdna2},
                                                                                  {kStrCdna2, GpuArchitecture::GpuArchitectureCdna2},
                                                                                  {kStrRdna3, GpuArchitecture::GpuArchitectureRdna3},
                                                                                  {kStrCdna3, GpuArchitecture::GpuArchitectureCdna3}};

// Definition of GpuComparator
template <typename Gpu>
struct GpuComparator
{
public:
    // Comparision operator for sorting T objects.
    bool operator()(const Gpu& a, const Gpu& b) const
    {
        auto found_a = kGpuSortPriority.find(RgaSharedUtils::ToUpper(a.GetArchitectureName()));
        auto found_b = kGpuSortPriority.find(RgaSharedUtils::ToUpper(b.GetArchitectureName()));
        if (found_a != kGpuSortPriority.end() && found_b != kGpuSortPriority.end())
        {
            return found_a->second < found_b->second;
        }
        else if (found_a != kGpuSortPriority.end())
        {
            return true;
        }
        else if (found_b != kGpuSortPriority.end())
        {
            return false;
        }
        else
        {
            return a < b;
        }
    }
};
