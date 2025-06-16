//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header for shader ISA Disassembly view proxy model.
//=============================================================================

#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_ISA_PROXY_MODEL_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_ISA_PROXY_MODEL_H_

// C++.
#include <array>

// Qt.
#include <QCheckBox>
#include <QHeaderView>
#include <QSortFilterProxyModel>

// Infra.
#include "qt_isa_gui/widgets/isa_proxy_model.h"

// Local.
#include "radeon_gpu_analyzer_gui/qt/rg_isa_item_model.h"

// RgIsaProxyModel is a filter model meant to filter default columns for a RgIsaItemModel.
// It filters out RgIsaItemModel columns set to be invisible via the gui.
class RgIsaProxyModel : public IsaProxyModel
{
    Q_OBJECT

public:
    // All columns visible by default.
    explicit RgIsaProxyModel(QObject* parent = nullptr, std::vector<bool> columns_visiblity = {});

    virtual ~RgIsaProxyModel();

    // Change the visibility of a column and invalidate this model.
    void SetColumnVisibility(uint32_t column, bool visibility, QHeaderView* header = nullptr) override;

    // Create the visibility checkbox related to a column.
    virtual void CreateViewingOptionsCheckbox(uint32_t column, QWidget* parent = nullptr) override;

    // Get the visibility checkbox related to a column.
    virtual const QCheckBox* GetViewingOptionsCheckbox(uint32_t column) override;

    // Get the source column index related to the checkbox.
    virtual uint32_t GetSourceColumnIndex(const QCheckBox* checkbox) override;

    // Get the number of columns in this model.
    virtual uint32_t GetNumberOfViewingOptions() override;

    // Get the visiblity of the VGPR column in this model.
    bool IsVgprColumnVisible();

signals:
    // A signal to enable/disable the Edit->Go to next maximum live VGPR line option.
    void EnableShowMaxVgprOptionSignal(bool is_enabled);

protected:
    // Override filterAcceptsColumn to filter columns set to be invisible.
    virtual bool filterAcceptsColumn(int source_column, const QModelIndex& source_parent) const Q_DECL_OVERRIDE;

private:
    // Keep track of which columns should be visible.
    std::array<bool, RgIsaItemModel::kColumnCount - IsaItemModel::kColumnCount> visible_columns_;

    // Corresponding checkboxes to each column.
    std::array<QCheckBox*, RgIsaItemModel::kColumnCount - IsaItemModel::kColumnCount> viewing_options_checkboxes_;

    // Keep track of where a hidden column should be placed when it is reshown.
    int column_order_[RgIsaItemModel::kColumnCount - IsaItemModel::kColumnCount];
};

#endif  // RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_ISA_PROXY_MODEL_H_
