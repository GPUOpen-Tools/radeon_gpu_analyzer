// C++.
#include <cassert>

// Qt.
#include <QInputMethodEvent>
#include <QRegularExpression>
#include <QSpacerItem>
#include <QTextCharFormat>

// Local.
#include "radeon_gpu_analyzer_gui/qt/rg_editor_element.h"
#include "radeon_gpu_analyzer_gui/qt/rg_label.h"
#include "radeon_gpu_analyzer_gui/qt/rg_line_edit.h"
#include "radeon_gpu_analyzer_gui/qt/rg_pipeline_state_editor_widget.h"
#include "radeon_gpu_analyzer_gui/qt/rg_pipeline_state_tree.h"
#include "radeon_gpu_analyzer_gui/rg_definitions.h"
#include "radeon_gpu_analyzer_gui/rg_utils.h"

// The tooltip used for editor items.
static const char* kStrEditorElementTooltip = "Click to edit.";

// The width of a single indent within the tree.
static const int kIndentationWidth = 20;

// The horizontal position where the Value column starts.
static const int kValueColumnPosition = 180;

// The stylesheet for each row in the PSO editor tree. Dynamic properties and repolishing
// are used to update the row background color without resetting the stylesheet string.
static const QString kStrRowStylesheet =
"*                                                                      \
{                                                                       \
    background-color: rgba(255, 128 255, 255);                           \
}                                                                       \
                                                                        \
*[selected=true]                                                        \
{                                                                       \
    background-color: rgba(255, 255, 178, 255);                          \
}                                                                       \
                                                                        \
*[resultOccurrence=true][currentResult=false][selected=false]           \
{                                                                       \
    background-color: rgba(192, 192, 192, 255);                          \
}                                                                       \
                                                                        \
*[currentResult=true]                                                   \
{                                                                       \
    background-color: rgba(255, 255, 178, 255);                          \
}                                                                       \
";

RgEditorElement::RgEditorElement(QWidget* parent, const std::string& member_name, RgEditorDataType data_type, std::function<void()> value_changed_callback)
    : QWidget(parent)
    , member_name_(member_name)
    , type_(data_type)
    , value_changed_callback_(value_changed_callback)
    , row_state_(RgRowExpansionState::kExpanded)
    , is_filtered_out_(false)
    , style_flags_(static_cast<uint32_t>(RgStyleFlags::None))
{
    ui_.setupUi(this);

    // Set the expand button's cursor to pointing hand cursor.
    ui_.expandPushButton->setCursor(Qt::PointingHandCursor);

    // Enable mouse tracking to receive mouse move events.
    setMouseTracking(true);

    // Initialize controls within the row.
    InitializeControls();

    // Connect internal signals to slots.
    ConnectSignals();

    // Apply a stylesheet to the row that's used to alter the background color.
    setStyleSheet(kStrRowStylesheet);
}

void RgEditorElement::AppendChildItem(RgEditorElement* item)
{
    item->parent_item_ = this;
    child_items_.push_back(std::shared_ptr<RgEditorElement>(item));

    // Ensure that the expand button is visible when appending children.
    ui_.expandPushButton->setVisible(true);

    // Insert the new child to the end of the layout.
    ui_.childRowsLayout->addWidget(item);

    // Update the widget geometry now that the newest child element has been added.
    ui_.childRowsLayout->update();
}

void RgEditorElement::ClearChildren()
{
    // Destroy all children and clear the child vector.
    for (auto child : child_items_)
    {
        // Remove the child element from the children list layout.
        ui_.childRowsLayout->removeWidget(child.get());
    }

    // Clear the current selection in the tree.
    RgPipelineStateTree* parent_tree = GetParentStateTree();
    if (parent_tree != nullptr)
    {
        parent_tree->SetCurrentSelection(nullptr);
    }

    child_items_.clear();

    // Since the row doesn't have any children, hide the expand/collapse button.
    ui_.expandPushButton->setVisible(false);

    ui_.childRowsLayout->update();
}

RgEditorElement* RgEditorElement::FindNextAnscestor(RgEditorElement* current_element)
{
    RgEditorElement* result = nullptr;

    // If no current element has been provided, start at this element.
    if (current_element == nullptr)
    {
        current_element = this;
    }

    assert(current_element != nullptr);
    if (current_element != nullptr)
    {
        // Get the direct parent to figure out if the current element has any children.
        RgEditorElement* parent_element = current_element->GetParentItem();
        if (parent_element != nullptr)
        {
            // Does the selected row have a younger sibling?
            int row_index = current_element->GetRowIndex();

            int num_younger_siblings = (parent_element->ChildCount() - 1) - row_index;
            if (num_younger_siblings > 0)
            {
                int sibling_row = -1;

                // Try to find a visible younger sibling row.
                for (int sibling_index = 0; sibling_index < num_younger_siblings; ++sibling_index)
                {
                    RgEditorElement* younger_sibling = parent_element->GetChild(row_index + sibling_index + 1);
                    assert(younger_sibling != nullptr);
                    if (younger_sibling != nullptr && younger_sibling->isVisible())
                    {
                        sibling_row = sibling_index;
                        break;
                    }
                }

                if (sibling_row != -1)
                {
                    // Select the first youngest sibling.
                    result = parent_element->GetChild(row_index + 1);
                }
            }

            if (result == nullptr)
            {
                // Need to find the first parent ancestor with a single child.
                result = FindNextAnscestor(parent_element);
            }
        }
    }

    return result;
}

RgEditorElement* RgEditorElement::FindLastVisibleDescendant(RgEditorElement* current_element) const
{
    RgEditorElement* result = nullptr;

    assert(current_element != nullptr);
    if (current_element != nullptr)
    {
        // Find the last visible child starting from the bottom of the list.
        int num_children = current_element->ChildCount();
        for (int child_index = num_children - 1; child_index >= 0; child_index--)
        {
            RgEditorElement* child = current_element->GetChild(child_index);
            assert(child != nullptr);
            if (child != nullptr)
            {
                // Only consider visible rows- skip hidden rows that are collapsed or filtered out.
                if (child->isVisible())
                {
                    // If the child has children, and is expanded, find the last visible descendant.
                    int child_count = child->ChildCount();
                    if (child_count > 0 && child->GetExpansionState() == RgRowExpansionState::kExpanded)
                    {
                        // Search for the last descendant using the child as the root element.
                        result = FindLastVisibleDescendant(child);
                    }
                    else
                    {
                        // If there are no descendant rows, we have found the last descendant.
                        result = child;
                    }

                    // If the row was visible, something got selected. We're done searching.
                    break;
                }
            }
        }
    }

    return result;
}

RgEditorElement* RgEditorElement::GetLastVisibleDescendant()
{
    // Start searching for the descendant at this element. Returns itself if 0 children.
    RgEditorElement* result = FindLastVisibleDescendant(this);

    // If there aren't any visible rows above the element, this element is the last visible descendant.
    if (result == nullptr)
    {
        result = this;
    }

    return result;
}

RgPipelineStateTree* RgEditorElement::GetParentStateTree() const
{
    RgPipelineStateTree* parent_tree = nullptr;

    // Start with the current row.
    const RgEditorElement* current_element = this;

    // Step up through the row's parent elements.
    while (current_element->GetParentItem() != nullptr)
    {
        current_element = current_element->GetParentItem();
    }

    // Ensure that the root parent element's tree pointer is valid.
    if (current_element->parent_tree_ != nullptr)
    {
        parent_tree = current_element->parent_tree_;
    }

    return parent_tree;
}

void RgEditorElement::SetParentStateTree(RgPipelineStateTree* parent_tree)
{
    assert(parent_tree != nullptr);
    if (parent_tree != nullptr)
    {
        parent_tree_ = parent_tree;
    }
}

RgEditorElement* RgEditorElement::GetChild(int row_index) const
{
    return child_items_[row_index].get();
}

bool RgEditorElement::GetIsEditable() const
{
    // All types, besides 'void', can be edited within the treeview.
    return type_ != RgEditorDataType::kVoid;
}

int RgEditorElement::GetRowHeight() const
{
    return ui_.rowInfo->height();
}

uint32_t RgEditorElement::GetStyleFlags() const
{
    return style_flags_;
}

int RgEditorElement::ChildCount() const
{
    return static_cast<int>(child_items_.size());
}

int RgEditorElement::ColumnCount() const
{
    // Always show only two columns: Member name & Value.
    return static_cast<int>(RgRowData::kRowDataCount);
}

QVariant RgEditorElement::Data(int column) const
{
    QVariant result;

    if (column == static_cast<int>(RgRowData::kRowDataMemberName))
    {
        result = QVariant(member_name_.c_str());
    }

    return result;
}

RgPipelineStateEditorWidget* RgEditorElement::GetEditorWidget()
{
    return nullptr;
}

void RgEditorElement::AddStyleFlag(RgStyleFlags style_flag)
{
    // Get the row's current style.
    uint32_t row_style_flags = GetStyleFlags();

    // Modify the row style flags by adding the given flag into the bitfield.
    row_style_flags |= static_cast<uint32_t>(style_flag);

    // Apply the updated style flags to the row.
    SetStyleFlags(row_style_flags);
}

void RgEditorElement::RemoveStyleFlag(RgStyleFlags style_flag)
{
    // Get the row's current style.
    uint32_t row_style_flags = GetStyleFlags();

    // Remove the given style flag.
    row_style_flags &= ~static_cast<uint32_t>(style_flag);

    // Apply the updated style flags to the row.
    SetStyleFlags(row_style_flags);
}

RgEditorElement* RgEditorElement::GetParentItem() const
{
    return parent_item_;
}

RgEditorDataType RgEditorElement::GetType() const
{
    return type_;
}

int RgEditorElement::GetRowIndex() const
{
    assert(parent_item_ != nullptr);
    if (parent_item_ != nullptr)
    {
        // Find this item within the parent's children.
        auto child_iter = std::find_if(
            parent_item_->child_items_.begin(), parent_item_->child_items_.end(), [this](std::shared_ptr<RgEditorElement> child) { return child.get() == this; });
        if (child_iter != parent_item_->child_items_.end())
        {
            // Compute the child index for this item.
            return (child_iter - parent_item_->child_items_.begin());
        }
    }

    return 0;
}

void RgEditorElement::SetExpansionState(RgRowExpansionState state, bool update_children)
{
    // Update the row's expansion state.
    row_state_ = state;

    // Toggle the visibility of all children based on the row state.
    bool is_row_expanded = (row_state_ == RgRowExpansionState::kExpanded);
    if (is_row_expanded)
    {
        ui_.expandPushButton->setIcon(QIcon(kIconResourceExpandedRow));
    }
    else
    {
        ui_.expandPushButton->setIcon(QIcon(kIconResourceCollapsedRow));
    }

    int num_children = ChildCount();
    for (int child_index = 0; child_index < num_children; ++child_index)
    {
        RgEditorElement* child_element = child_items_[child_index].get();
        assert(child_element != nullptr);
        if (child_element != nullptr)
        {
            // If the row is not currently filtered out, make it visible.
            if (!child_element->is_filtered_out_)
            {
                // Set the visibility of child rows.
                child_element->setVisible(is_row_expanded);

                // Recursively update children if necessary.
                if (update_children)
                {
                    child_items_[child_index]->SetExpansionState(state, update_children);
                }
            }
        }
    }
}

RgRowExpansionState RgEditorElement::GetExpansionState() const
{
    return row_state_;
}

void RgEditorElement::SetStyleFlags(uint32_t style_flags)
{
    // Don't change or repolish anything if the flags are not modified.
    if (style_flags != style_flags_)
    {
        // Update the style flags.
        style_flags_ = style_flags;

        // Update the "selected" property in the row to highlight the currently-selected row.
        bool is_selected_row = (style_flags_ & static_cast<uint32_t>(RgStyleFlags::CurrentRow)) == static_cast<uint32_t>(RgStyleFlags::CurrentRow);
        assert(ui_.rowInfo != nullptr);
        if (ui_.rowInfo != nullptr)
        {
            ui_.rowInfo->setProperty("selected", is_selected_row);

            // Update the "currentResult" property in the row to highlight the current search result.
            bool is_current_result =
                (style_flags_ & static_cast<uint32_t>(RgStyleFlags::SearchResultCurrent)) == static_cast<uint32_t>(RgStyleFlags::SearchResultCurrent);
            ui_.rowInfo->setProperty("currentResult", is_current_result);

            // Update the "resultOccurrence" property in the row to indicate that this row includes a
            // search result- it's just not the current search result.
            bool is_result_occurrence =
                (style_flags_ & static_cast<uint32_t>(RgStyleFlags::SearchResultOccurrence)) == static_cast<uint32_t>(RgStyleFlags::SearchResultOccurrence);
            ui_.rowInfo->setProperty("resultOccurrence", is_result_occurrence);

            // Repolish the row's visual style.
            RgUtils::StyleRepolish(ui_.rowInfo, true);

            // Update to repaint the row.
            update();
        }
    }
}

void RgEditorElement::SetFocusedColumn(RgRowData column)
{
    // Set the focus on the member name label. Editor
    // widgets can be focused on separately using tab.
    if (column == RgRowData::kRowDataMemberName)
    {
        ui_.memberNameLabel->setFocus();
    }
}

void RgEditorElement::SetValueChangedCallback(std::function<void()> value_changed_callback)
{
    assert(value_changed_callback != nullptr);
    if (value_changed_callback != nullptr)
    {
        value_changed_callback = value_changed_callback;
    }
}

void RgEditorElement::SetTooltipText()
{
    // Get the editor widget for the row.
    // Some rows are used exclusively as parent rows, and don't make use of editor widgets.
    RgPipelineStateEditorWidget* editor_widget = GetEditorWidget();
    if (editor_widget != nullptr)
    {
        // Find out if the element is editable.
        bool is_editable = GetIsEditable();

        // Check to see if the user is pointing to a valid cell.
        if (is_editable)
        {
            // Set the cursor to pointing hand cursor.
            editor_widget->setCursor(Qt::PointingHandCursor);

            // Set the tooltip text for the editor widget.
            RgUtils::SetToolAndStatusTip(kStrEditorElementTooltip, editor_widget);
        }
        else
        {
            // Set the cursor to arrow cursor.
            editor_widget->setCursor(Qt::ArrowCursor);
        }
    }
}

void RgEditorElement::InitializeControls()
{
    // Set the member name in the label.
    ui_.memberNameLabel->setText(member_name_.c_str());
}

void RgEditorElement::InitializeRows(RgEditorElement* root_element)
{
    if (root_element == nullptr)
    {
        root_element = this;
    }

    assert(root_element != nullptr);
    if (root_element != nullptr)
    {
        // Update the indentation for the row based on the number of ancestor parent rows.
        root_element->UpdateIndentation();

        // Set the tooltip text for the row.
        root_element->SetTooltipText();

        int num_children = root_element->ChildCount();
        for (int child_index = 0; child_index < num_children; ++child_index)
        {
            RgEditorElement* child = root_element->GetChild(child_index);
            InitializeRows(child);
        }
    }
}

void RgEditorElement::HandleValueChanged()
{
    ValueChangedHandler();
}

void RgEditorElement::ExpandTreeEntry()
{
    // Expand the tree entry.
    SetExpansionState(RgRowExpansionState::kExpanded);
}

void RgEditorElement::HandleExpandClicked()
{
    // Toggle the expanded/collapsed state of the row.
    RgRowExpansionState new_row_state = (row_state_ == RgRowExpansionState::kExpanded) ?
        RgRowExpansionState::kCollapsed : RgRowExpansionState::kExpanded;

    // Update the expansion state.
    SetExpansionState(new_row_state);
}

void RgEditorElement::HandleNameLabelFocusIn()
{
    // Get the parent tree that this element is connected to.
    RgPipelineStateTree* parent_tree = GetParentStateTree();
    assert(parent_tree != nullptr);
    if (parent_tree != nullptr)
    {
        // Set the current selection to this row and the member name column.
        parent_tree->SetCurrentSelection(this, static_cast<int>(RgRowData::kRowDataMemberName));
    }
}

void RgEditorElement::HandleEditorFocusIn()
{
    // Get the parent tree that this element is connected to.
    RgPipelineStateTree* parent_tree = GetParentStateTree();
    assert(parent_tree != nullptr);
    if (parent_tree != nullptr)
    {
        // Set the current selection to this row and the value column.
        parent_tree->SetCurrentSelection(this, static_cast<int>(RgRowData::kRowDataMemberValue));
    }
}

void RgEditorElement::HandleEditorFocusOut()
{
    // Get the parent tree that this element is connected to.
    RgPipelineStateTree* parent_tree = GetParentStateTree();
    assert(parent_tree != nullptr);
    if (parent_tree != nullptr)
    {
        // Select the current row only- no specific column.
        parent_tree->SetCurrentSelection(this);
    }
}

int RgEditorElement::ComputeIndentationWidth(RgEditorElement* element)
{
    if (element != nullptr)
    {
        RgEditorElement* parent = element->GetParentItem();
        if (parent != nullptr)
        {
            return kIndentationWidth + ComputeIndentationWidth(parent);
        }
    }

    return 0;
}

void RgEditorElement::ConnectSignals()
{
    // Connect the row's expand/collapse button.
    bool is_connected = connect(ui_.expandPushButton, &QPushButton::clicked, this, &RgEditorElement::HandleExpandClicked);
    assert(is_connected);

    // Connect the member name label's focus in signal.
    is_connected = connect(ui_.memberNameLabel, &RgLabel::LabelFocusInEventSignal, this, &RgEditorElement::HandleNameLabelFocusIn);
    assert(is_connected);
}

void RgEditorElement::UpdateIndentation()
{
    // Adjust the row's indentation spacer width based on number of ancestors.
    int indent_width = ComputeIndentationWidth(this);

    if (child_items_.empty())
    {
        ui_.expandPushButton->setVisible(false);
        indent_width += ui_.expandPushButton->width();
    }

    // Insert spacing at the beginning of the layout to indent.
    QHBoxLayout* layout = static_cast<QHBoxLayout*>(ui_.rowInfo->layout());
    if (layout != nullptr && indent_width > 0)
    {
        // If the spacer item has already been initialized,
        // remove it and destroy it before re-creating a new one.
        if (spacer_item_ != nullptr)
        {
            layout->removeItem(spacer_item_);
            RG_SAFE_DELETE(spacer_item_);
        }

        spacer_item_ = new QSpacerItem(indent_width, 0, QSizePolicy::Fixed);
        layout->insertSpacerItem(0, spacer_item_);
    }

    // Get the font metrics.
    QFontMetrics font_metrics(ui_.memberNameLabel->font());

    // Calculate the width of the member name label.
    const QRect bounding_rect = font_metrics.boundingRect(member_name_.c_str());
    const int width = bounding_rect.width();

    // Set the width of the value indent spacer. This aligns all editor widgets under the value
    // column to the same horizontal position.
    ui_.valueIndentSpacer->changeSize(kValueColumnPosition - width, 0, QSizePolicy::Fixed);
}

void RgEditorElement::ClearSearchStringData()
{
    string_highlight_data_.clear();
    UpdateButtonSubString();
    ResetLineEditSubString();
    ResetLabelSubString();
}

void RgEditorElement::ClearSearchStringDataVector()
{
    string_highlight_data_.clear();
}

void RgEditorElement::HighlightButtonSubString(int start_location, const std::string& search_string, bool is_current_match)
{
    is_current_match_ = is_current_match;

    // Update search string location.
    UpdateStringMatchingLocation(start_location, static_cast<int>(search_string.size()), search_string);
    UpdateButtonSubString();
}

void RgEditorElement::UpdateButtonSubString()
{
    QPushButton *pPushButton = ui_.editorHost->findChild<QPushButton*>();
    ArrowIconComboBox* combo_box   = dynamic_cast<ArrowIconComboBox*>(pPushButton);
    if (combo_box != nullptr)
    {
        // Update the button string.
        combo_box->SetHighLightSubStringData(string_highlight_data_);
        combo_box->SetHighLightSubString(true);
        combo_box->update();
    }
}

void RgEditorElement::UpdateStringMatchingLocation(int start_location, int length, const std::string& search_string)
{
    bool is_found = false;

    // Remove any entries that do not match the search string anymore.
    int count = 0;
    std::vector<int> remove_entries;
    for (auto& string_data : string_highlight_data_)
    {
        if (string_data.highlight_string != search_string)
        {
            remove_entries.push_back(count);
        }
        count++;
    }
    for (std::vector<int>::reverse_iterator it = remove_entries.rbegin(); it != remove_entries.rend(); ++it)
    {
        string_highlight_data_.remove(*it);
    }

    // Update existing locations, if any.
    for (auto& string_data : string_highlight_data_)
    {
        if (string_data.start_location == start_location)
        {
            string_data.end_location = start_location + length;
            string_data.highlight_string = search_string;
            is_found = true;
            SetHighlightColor(string_data);
            break;
        }
    }

    // Create a new entry if a matching entry not found.
    if (!is_found)
    {
        StringHighlightData string_highlight_data = {};
        string_highlight_data.start_location = start_location;
        string_highlight_data.end_location = start_location + length;
        string_highlight_data.highlight_string = search_string;
        SetHighlightColor(string_highlight_data);
        string_highlight_data_.push_back(string_highlight_data);
    }
}

void  RgEditorElement::SetHighlightColor(StringHighlightData& string_highlight_data)
{
    // The color in which the current match would be highlighted.
    static const QColor kHighlightColorMatchOther = QColor::fromRgba(qRgba(254, 206, 0, 200));

    // The color in which any match would be highlighted.
    static QColor kHighlightColorMatchCurrent = QColor::fromRgba(qRgba(165, 175, 146, 200));

    if (is_current_match_)
    {
        string_highlight_data.highlight_color = kHighlightColorMatchCurrent;
    }
    else
    {
        string_highlight_data.highlight_color = kHighlightColorMatchOther;
    }
}

void RgEditorElement::ResetButtonSubString()
{
    QPushButton *push_buttons = ui_.editorHost->findChild<QPushButton*>();
    ArrowIconComboBox* combo_box    = dynamic_cast<ArrowIconComboBox*>(push_buttons);
    assert(combo_box != nullptr);
    if (combo_box != nullptr)
    {
        combo_box->ClearHighLightSubStringData();
        combo_box->SetHighLightSubString(false);
        combo_box->update();
    }
}

void RgEditorElement::ResetLineEditSubString()
{
    RgLineEdit* line_edit = ui_.editorHost->findChild<RgLineEdit*>();
    if (line_edit != nullptr)
    {
        // Clear search string highlight.
        line_edit->SetHighlightSubString(false);
        line_edit->update();
    }
}

void RgEditorElement::HighlightLineEditSubString(int start_location, const std::string& search_string, bool is_current_match)
{
    is_current_match_ = is_current_match;

    RgLineEdit* line_edit = ui_.editorHost->findChild<RgLineEdit*>();
    if (line_edit != nullptr)
    {
        // Update search string location.
        UpdateStringMatchingLocation(start_location, static_cast<int>(search_string.size()), search_string);
        line_edit->SetHighlightSubStringData(string_highlight_data_);
        line_edit->SetHighlightSubString(true);
        line_edit->update();
    }
}

void RgEditorElement::ResetLabelSubString()
{
    RgLabel* label = ui_.rowInfo->findChild<RgLabel*>();
    if (label != nullptr)
    {
        // Clear search string highlight.
        label->SetHighlightSubString(false);
        label->update();
    }
}

void RgEditorElement::HighlightLabelSubString(int start_location, const std::string& search_string, bool is_current_match)
{
    is_current_match_ = is_current_match;

    RgLabel* label = ui_.rowInfo->findChild<RgLabel*>();
    if (label != nullptr)
    {
        // Update search string location.
        UpdateStringMatchingLocation(start_location, static_cast<int>(search_string.size()), search_string);
        label->SetHighlightSubStringData(string_highlight_data_);
        label->SetHighlightSubString(true);
        label->update();
    }
}
