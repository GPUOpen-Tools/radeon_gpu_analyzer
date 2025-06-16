//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Implementation for Item widget in RGA Build view's File Menu.
//=============================================================================

// Local.
#include "radeon_gpu_analyzer_gui/qt/rg_menu_item.h"
#include "radeon_gpu_analyzer_gui/qt/rg_menu.h"

RgMenuItem::RgMenuItem(RgMenu* parent) :
    QWidget(parent),
    parent_menu_(parent)
{
    setAttribute(Qt::WA_LayoutUsesWidgetRect);
}

RgMenu* RgMenuItem::GetParentMenu() const
{
    return parent_menu_;
}
