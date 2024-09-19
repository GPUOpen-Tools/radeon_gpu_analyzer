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
static const char* kStrRdna3_5 = "RDNA3.5";

enum class GpuArchitecture
{
    kGpuArchitectureIpv6,
    kGpuArchitectureIpv7,
    kGpuArchitectureIpv8,
    kGpuArchitectureVega,
    kGpuArchitectureRdna,
    kGpuArchitectureCdna,
    kGpuArchitectureRdna2,
    kGpuArchitectureCdna2,
    kGpuArchitectureRdna3,
    kGpuArchitectureCdna3,
    kGpuArchitectureRdna3_5
};

// GPU Architectures sort priority.
static const std::unordered_map<std::string, GpuArchitecture> kGpuSortPriority = {{kStrIpv6, GpuArchitecture::kGpuArchitectureIpv6},
                                                                                  {kStrIpv6, GpuArchitecture::kGpuArchitectureIpv7},
                                                                                  {kStrIpv6, GpuArchitecture::kGpuArchitectureIpv8},
                                                                                  {kStrVega, GpuArchitecture::kGpuArchitectureVega},
                                                                                  {kStrRdna, GpuArchitecture::kGpuArchitectureRdna},
                                                                                  {kStrCdna, GpuArchitecture::kGpuArchitectureCdna},
                                                                                  {kStrRdna2, GpuArchitecture::kGpuArchitectureRdna2},
                                                                                  {kStrCdna2, GpuArchitecture::kGpuArchitectureCdna2},
                                                                                  {kStrRdna3, GpuArchitecture::kGpuArchitectureRdna3},
                                                                                  {kStrCdna3, GpuArchitecture::kGpuArchitectureCdna3},
                                                                                  {kStrRdna3_5, GpuArchitecture::kGpuArchitectureRdna3_5}};

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
