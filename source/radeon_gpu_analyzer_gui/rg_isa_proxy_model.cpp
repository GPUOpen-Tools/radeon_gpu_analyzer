//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Implementation for shader ISA Disassembly view proxy model.
//=============================================================================

//Qt.
#include <QGridLayout>

// Local.
#include "radeon_gpu_analyzer_gui/qt/rg_isa_proxy_model.h"


RgIsaProxyModel::RgIsaProxyModel(QObject* parent, std::vector<bool> columns_visiblity)
    : IsaProxyModel(parent, columns_visiblity)
{
    assert(columns_visiblity.size() == RgIsaItemModel::kColumnCount);
    for (uint32_t column = 0; column < RgIsaItemModel::kColumnCount; column++)
    {
        if (column < IsaItemModel::kColumnCount)
        {
            continue;
        }

        visible_columns_[column - IsaItemModel::kColumnCount] = columns_visiblity[column];
        column_order_[column - IsaItemModel::kColumnCount]    = column;
    }
}

RgIsaProxyModel::~RgIsaProxyModel()
{
}

void RgIsaProxyModel::SetColumnVisibility(uint32_t column, bool visibility, QHeaderView* header)
{
    if (column < IsaItemModel::kColumnCount)
    {
        return IsaProxyModel::SetColumnVisibility(column, visibility, header);
    }

    column = column - IsaItemModel::kColumnCount;

    if (column >= visible_columns_.size())
    {
        return;
    }

    visible_columns_[column] = visibility;

    if (!visibility && header != nullptr)
    {
        int proxy_column = mapFromSource(sourceModel()->index(0, column + IsaItemModel::kColumnCount)).column();
        int visual_index = header->visualIndex(proxy_column);

        column_order_[column] = visual_index;
    }

    invalidateFilter();

    if (visibility && header != nullptr)
    {
        int proxy_column = mapFromSource(sourceModel()->index(0, column + IsaItemModel::kColumnCount)).column();
        if (column_order_[column] >= columnCount())
        {
            column_order_[column] = columnCount() - 1;
        }
        header->moveSection(proxy_column, column_order_[column]);
    }

    if (column == RgIsaItemModel::kIsaColumnVgprPressure - IsaItemModel::kColumnCount)
    {
        emit EnableShowMaxVgprOptionSignal(IsVgprColumnVisible());
    }

    std::vector<bool> column_visibility;
    column_visibility.resize(RgIsaItemModel::kColumnCount - 1);
    for (uint32_t col = 1; col < RgIsaItemModel::kColumnCount; col++)
    {
        column_visibility[col - 1] = filterAcceptsColumn(col, QModelIndex{});
    }
    // Save the changes.
    RgConfigManager& config_manager = RgConfigManager::Instance();
    config_manager.Instance().SetDisassemblyColumnVisibility(column_visibility);
    config_manager.SaveGlobalConfigFile();
}

void RgIsaProxyModel::CreateViewingOptionsCheckbox(uint32_t column, QWidget* parent)
{
    if (column < RgIsaItemModel::kColumnCount)
    {
        if (column < IsaItemModel::kColumnCount)
        {
            IsaProxyModel::CreateViewingOptionsCheckbox(column, parent);
            return;
        }

        auto source_model = sourceModel();
        if (source_model)
        {
            QString column_name;
            if (column == RgIsaItemModel::Columns::kIsaColumnVgprPressure)
            {
                column_name = kStrDisassemblyLiveVgprHeaderPart;
            }
            else
            {
                column_name = source_model->headerData(column, Qt::Horizontal, Qt::DisplayRole).toString();
            }
            viewing_options_checkboxes_[column - IsaItemModel::kColumnCount] = new QCheckBox(column_name, parent);

            QCheckBox* checkbox = viewing_options_checkboxes_[column - IsaItemModel::kColumnCount];
            if (checkbox)
            {

                QSizePolicy size_policy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed);
                checkbox->setSizePolicy(size_policy);

                if (parent && parent->layout())
                {
                    QGridLayout* grid_layout = qobject_cast<QGridLayout*>(parent->layout());
                    if (grid_layout)
                    {
                        switch (column)
                        {
                        case RgIsaItemModel::Columns::kIsaColumnVgprPressure:
                            grid_layout->addWidget(checkbox, 1, 2, 1, 1);
                            break;
                        case RgIsaItemModel::Columns::kIsaColumnFunctionalUnit:
                            grid_layout->addWidget(checkbox, 2, 2, 1, 1);
                            break;
                        default:
                            break;
                        }
                    }
                }

                checkbox->setCursor(Qt::PointingHandCursor);

                checkbox->setChecked(visible_columns_[column - IsaItemModel::kColumnCount]);
            }
        }
    }
}

const QCheckBox* RgIsaProxyModel::GetViewingOptionsCheckbox(uint32_t column)
{
    if (column < RgIsaItemModel::kColumnCount)
    {
        if (column < IsaItemModel::kColumnCount)
        {
            return IsaProxyModel::GetViewingOptionsCheckbox(column);
        }
        return viewing_options_checkboxes_[column - IsaItemModel::kColumnCount];
    }
    return nullptr;
}

uint32_t RgIsaProxyModel::GetSourceColumnIndex(const QCheckBox* checkbox)
{
    uint32_t source_column_index = RgIsaItemModel::kColumnCount;

    for (uint32_t column = 0; column < RgIsaItemModel::kColumnCount; column++)
    {
        if (checkbox == GetViewingOptionsCheckbox(column))
        {
            source_column_index = column;
        }
    }

    return source_column_index;
}

uint32_t RgIsaProxyModel::GetNumberOfViewingOptions()
{
    return RgIsaItemModel::kColumnCount;
}

bool RgIsaProxyModel::IsVgprColumnVisible()
{
    return visible_columns_[RgIsaItemModel::kIsaColumnVgprPressure - IsaItemModel::kColumnCount];
}

bool RgIsaProxyModel::filterAcceptsColumn(int source_column, const QModelIndex& source_parent) const
{
    if (source_column < IsaItemModel::kColumnCount)
    {
        return IsaProxyModel::filterAcceptsColumn(source_column, source_parent);
    }

    source_column = source_column - IsaItemModel::kColumnCount;

    if (source_column >= static_cast<int>(visible_columns_.size()))
    {
        return true;
    }

    return visible_columns_[source_column];
}
