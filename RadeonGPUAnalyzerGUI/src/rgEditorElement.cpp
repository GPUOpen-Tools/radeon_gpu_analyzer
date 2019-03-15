// C++.
#include <cassert>

// Qt.
#include <QInputMethodEvent>
#include <QRegExp>
#include <QSpacerItem>
#include <QTextCharFormat>

// QtCommon.
#include <QtCommon/Scaling/ScalingManager.h>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgEditorElement.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgLabel.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgLineEdit.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgPipelineStateEditorWidget.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgPipelineStateTree.h>
#include <RadeonGPUAnalyzerGUI/Include/rgDefinitions.h>
#include <RadeonGPUAnalyzerGUI/Include/rgUtils.h>

// The tooltip used for editor items.
static const char* s_EDITOR_ELEMENT_TOOLTIP = "Click to edit.";

// The width of a single indent within the tree.
static const int s_INDENTATION_WIDTH = 20;

// The horizontal position where the Value column starts.
static const int s_VALUE_COLUMN_POSITION = 180;

// The stylesheet for each row in the PSO editor tree. Dynamic properties and repolishing
// are used to update the row background color without resetting the stylesheet string.
static const QString s_ROW_STYLESHEET =
"*                                                                      \
{                                                                       \
    background-color: rgb(255, 128 255, 255);                           \
}                                                                       \
                                                                        \
*[selected=true]                                                        \
{                                                                       \
    background-color: rgb(255, 255, 178, 255);                          \
}                                                                       \
                                                                        \
*[resultOccurrence=true][currentResult=false][selected=false]           \
{                                                                       \
    background-color: rgb(192, 192, 192, 255);                          \
}                                                                       \
                                                                        \
*[currentResult=true]                                                   \
{                                                                       \
    background-color: rgb(255, 255, 178, 255);                          \
}                                                                       \
";

rgEditorElement::rgEditorElement(QWidget* pParent, const std::string& memberName, rgEditorDataType dataType, std::function<void()> valueChangedCallback)
    : QWidget(pParent)
    , m_memberName(memberName)
    , m_type(dataType)
    , m_valueChangedCallback(valueChangedCallback)
    , m_rowState(rgRowExpansionState::Expanded)
    , m_isFilteredOut(false)
    , m_styleFlags(static_cast<uint32_t>(rgStyleFlags::None))
{
    ui.setupUi(this);

    // Set the expand button's cursor to pointing hand cursor.
    ui.expandPushButton->setCursor(Qt::PointingHandCursor);

    // Enable mouse tracking to receive mouse move events.
    setMouseTracking(true);

    // Initialize controls within the row.
    InitializeControls();

    // Connect internal signals to slots.
    ConnectSignals();

    // Apply a stylesheet to the row that's used to alter the background color.
    setStyleSheet(s_ROW_STYLESHEET);
}

void rgEditorElement::AppendChildItem(rgEditorElement* pItem)
{
    pItem->m_pParentItem = this;
    m_childItems.push_back(pItem);

    // Ensure that the expand button is visible when appending children.
    ui.expandPushButton->setVisible(true);

    // Insert the new child to the end of the layout.
    ui.childRowsLayout->addWidget(pItem);

    // Register the new row with the ScalingManager.
    ScalingManager::Get().RegisterObject(pItem);

    // Update the widget geometry now that the newest child element has been added.
    ui.childRowsLayout->update();
}

void rgEditorElement::ClearChildren()
{
    // Destroy all children and clear the child vector.
    for (auto childIter = m_childItems.begin(); childIter != m_childItems.end(); ++childIter)
    {
        // Remove the child element from the children list layout.
        ui.childRowsLayout->removeWidget(*childIter);

        // Destroy the child element.
        RG_SAFE_DELETE(*childIter);
    }

    m_childItems.clear();

    // Since the row doesn't have any children, hide the expand/collapse button.
    ui.expandPushButton->setVisible(false);

    ui.childRowsLayout->update();
}

rgEditorElement* rgEditorElement::FindNextAnscestor(rgEditorElement* pCurrentElement)
{
    rgEditorElement* pResult = nullptr;

    // If no current element has been provided, start at this element.
    if (pCurrentElement == nullptr)
    {
        pCurrentElement = this;
    }

    assert(pCurrentElement != nullptr);
    if (pCurrentElement != nullptr)
    {
        // Get the direct parent to figure out if the current element has any children.
        rgEditorElement* pParentElement = pCurrentElement->GetParentItem();
        if (pParentElement != nullptr)
        {
            // Does the selected row have a younger sibling?
            int rowIndex = pCurrentElement->GetRowIndex();

            int numYoungerSiblings = (pParentElement->ChildCount() - 1) - rowIndex;
            if (numYoungerSiblings > 0)
            {
                int siblingRow = -1;

                // Try to find a visible younger sibling row.
                for (int siblingIndex = 0; siblingIndex < numYoungerSiblings; ++siblingIndex)
                {
                    rgEditorElement* pYoungerSibling = pParentElement->GetChild(rowIndex + siblingIndex + 1);
                    assert(pYoungerSibling != nullptr);
                    if (pYoungerSibling != nullptr && pYoungerSibling->isVisible())
                    {
                        siblingRow = siblingIndex;
                        break;
                    }
                }

                if (siblingRow != -1)
                {
                    // Select the first youngest sibling.
                    pResult = pParentElement->GetChild(rowIndex + 1);
                }
            }

            if (pResult == nullptr)
            {
                // Need to find the first parent ancestor with a single child.
                pResult = FindNextAnscestor(pParentElement);
            }
        }
    }

    return pResult;
}

rgEditorElement* rgEditorElement::FindLastVisibleDescendant(rgEditorElement* pCurrentElement) const
{
    rgEditorElement* pResult = nullptr;

    assert(pCurrentElement != nullptr);
    if (pCurrentElement != nullptr)
    {
        // Find the last visible child starting from the bottom of the list.
        int numChildren = pCurrentElement->ChildCount();
        for (int childIndex = numChildren - 1; childIndex >= 0; childIndex--)
        {
            rgEditorElement* pChild = pCurrentElement->GetChild(childIndex);
            assert(pChild != nullptr);
            if (pChild != nullptr)
            {
                // Only consider visible rows- skip hidden rows that are collapsed or filtered out.
                if (pChild->isVisible())
                {
                    // If the child has children, and is expanded, find the last visible descendant.
                    int childCount = pChild->ChildCount();
                    if (childCount > 0 && pChild->GetExpansionState() == rgRowExpansionState::Expanded)
                    {
                        // Search for the last descendant using the child as the root element.
                        pResult = FindLastVisibleDescendant(pChild);
                    }
                    else
                    {
                        // If there are no descendant rows, we have found the last descendant.
                        pResult = pChild;
                    }

                    // If the row was visible, something got selected. We're done searching.
                    break;
                }
            }
        }
    }

    return pResult;
}

rgEditorElement* rgEditorElement::GetLastVisibleDescendant()
{
    // Start searching for the descendant at this element. Returns itself if 0 children.
    rgEditorElement* pResult = FindLastVisibleDescendant(this);

    // If there aren't any visible rows above the element, this element is the last visible descendant.
    if (pResult == nullptr)
    {
        pResult = this;
    }

    return pResult;
}


rgPipelineStateTree* rgEditorElement::GetParentStateTree() const
{
    rgPipelineStateTree* pParentTree = nullptr;

    // Start with the current row.
    const rgEditorElement* pCurrentElement = this;

    // Step up through the row's parent elements.
    while (pCurrentElement->GetParentItem() != nullptr)
    {
        pCurrentElement = pCurrentElement->GetParentItem();
    }

    // Ensure that the root parent element's tree pointer is valid.
    assert(pCurrentElement->m_pParentTree != nullptr);
    if (pCurrentElement->m_pParentTree != nullptr)
    {
        pParentTree = pCurrentElement->m_pParentTree;
    }

    return pParentTree;
}

void rgEditorElement::SetParentStateTree(rgPipelineStateTree* pParentTree)
{
    assert(pParentTree != nullptr);
    if (pParentTree != nullptr)
    {
        m_pParentTree = pParentTree;
    }
}

void rgEditorElement::RemoveChild(rgEditorElement* pChildItem)
{
    auto childIter = std::find(m_childItems.begin(), m_childItems.end(), pChildItem);
    if (childIter != m_childItems.cend())
    {
        // Remove the child element from the children list layout.
        ui.childRowsLayout->removeWidget(pChildItem);

        // Erase the target item from the list of children and destroy it.
        m_childItems.erase(childIter);

        // Destroy the child element.
        RG_SAFE_DELETE(pChildItem);
    }
}

rgEditorElement* rgEditorElement::GetChild(int rowIndex) const
{
    return m_childItems[rowIndex];
}

bool rgEditorElement::GetIsEditable() const
{
    // All types, besides 'void', can be edited within the treeview.
    return m_type != rgEditorDataType::Void;
}

int rgEditorElement::GetRowHeight() const
{
    return ui.rowInfo->height();
}

uint32_t rgEditorElement::GetStyleFlags() const
{
    return m_styleFlags;
}

int rgEditorElement::ChildCount() const
{
    return static_cast<int>(m_childItems.size());
}

int rgEditorElement::ColumnCount() const
{
    // Always show only two columns: Member name & Value.
    return static_cast<int>(rgRowData::RowDataCount);
}

QVariant rgEditorElement::Data(int column) const
{
    QVariant result;

    if (column == static_cast<int>(rgRowData::RowDataMemberName))
    {
        result = QVariant(m_memberName.c_str());
    }

    return result;
}

rgPipelineStateEditorWidget* rgEditorElement::GetEditorWidget()
{
    return nullptr;
}

void rgEditorElement::AddStyleFlag(rgStyleFlags styleFlag)
{
    // Get the row's current style.
    uint32_t rowStyleFlags = GetStyleFlags();

    // Modify the row style flags by adding the given flag into the bitfield.
    rowStyleFlags |= static_cast<uint32_t>(styleFlag);

    // Apply the updated style flags to the row.
    SetStyleFlags(rowStyleFlags);
}

void rgEditorElement::RemoveStyleFlag(rgStyleFlags styleFlag)
{
    // Get the row's current style.
    uint32_t rowStyleFlags = GetStyleFlags();

    // Remove the given style flag.
    rowStyleFlags &= ~static_cast<uint32_t>(styleFlag);

    // Apply the updated style flags to the row.
    SetStyleFlags(rowStyleFlags);
}

rgEditorElement* rgEditorElement::GetParentItem() const
{
    return m_pParentItem;
}

rgEditorDataType rgEditorElement::GetType() const
{
    return m_type;
}

int rgEditorElement::GetRowIndex() const
{
    assert(m_pParentItem != nullptr);
    if (m_pParentItem != nullptr)
    {
        // Find this item within the parent's children.
        auto childIter = std::find(m_pParentItem->m_childItems.begin(), m_pParentItem->m_childItems.end(), this);
        if (childIter != m_pParentItem->m_childItems.end())
        {
            // Compute the child index for this item.
            return (childIter - m_pParentItem->m_childItems.begin());
        }
    }

    return 0;
}

void rgEditorElement::SetExpansionState(rgRowExpansionState state, bool updateChildren)
{
    // Update the row's expansion state.
    m_rowState = state;

    // Toggle the visibility of all children based on the row state.
    bool isRowExpanded = (m_rowState == rgRowExpansionState::Expanded);
    if (isRowExpanded)
    {
        ui.expandPushButton->setIcon(QIcon(gs_ICON_RESOURCE_EXPANDED_ROW));
    }
    else
    {
        ui.expandPushButton->setIcon(QIcon(gs_ICON_RESOURCE_COLLAPSED_ROW));
    }

    int numChildren = ChildCount();
    for (int childIndex = 0; childIndex < numChildren; ++childIndex)
    {
        rgEditorElement* pChildElement = m_childItems[childIndex];
        assert(pChildElement != nullptr);
        if (pChildElement != nullptr)
        {
            // If the row is not currently filtered out, make it visible.
            if (!pChildElement->m_isFilteredOut)
            {
                // Set the visibility of child rows.
                pChildElement->setVisible(isRowExpanded);

                // Recursively update children if necessary.
                if (updateChildren)
                {
                    m_childItems[childIndex]->SetExpansionState(state, updateChildren);
                }
            }
        }
    }
}

rgRowExpansionState rgEditorElement::GetExpansionState() const
{
    return m_rowState;
}

void rgEditorElement::SetStyleFlags(uint32_t styleFlags)
{
    // Don't change or repolish anything if the flags are not modified.
    if (styleFlags != m_styleFlags)
    {
        // Update the style flags.
        m_styleFlags = styleFlags;

        // Update the "selected" property in the row to highlight the currently-selected row.
        bool isSelectedRow = (m_styleFlags & static_cast<uint32_t>(rgStyleFlags::CurrentRow)) == static_cast<uint32_t>(rgStyleFlags::CurrentRow);
        ui.rowInfo->setProperty("selected", isSelectedRow);

        // Update the "currentResult" property in the row to highlight the current search result.
        bool isCurrentResult = (m_styleFlags & static_cast<uint32_t>(rgStyleFlags::SearchResultCurrent)) == static_cast<uint32_t>(rgStyleFlags::SearchResultCurrent);
        ui.rowInfo->setProperty("currentResult", isCurrentResult);

        // Update the "resultOccurrence" property in the row to indicate that this row includes a
        // search result- it's just not the current search result.
        bool isResultOccurrence = (m_styleFlags & static_cast<uint32_t>(rgStyleFlags::SearchResultOccurrence)) == static_cast<uint32_t>(rgStyleFlags::SearchResultOccurrence);
        ui.rowInfo->setProperty("resultOccurrence", isResultOccurrence);

        // Repolish the row's visual style.
        rgUtils::StyleRepolish(ui.rowInfo, true);

        // Update to repaint the row.
        update();
    }
}

void rgEditorElement::SetFocusedColumn(rgRowData column)
{
    // Set the focus on the member name label. Editor
    // widgets can be focused on separately using tab.
    if (column == rgRowData::RowDataMemberName)
    {
        ui.memberNameLabel->setFocus();
    }
}

void rgEditorElement::SetValueChangedCallback(std::function<void()> valueChangedCallback)
{
    assert(valueChangedCallback != nullptr);
    if (valueChangedCallback != nullptr)
    {
        m_valueChangedCallback = valueChangedCallback;
    }
}

void rgEditorElement::SetTooltipText()
{
    // Get the editor widget for the row.
    // Some rows are used exclusively as parent rows, and don't make use of editor widgets.
    rgPipelineStateEditorWidget* pEditorWidget = GetEditorWidget();
    if (pEditorWidget != nullptr)
    {
        // Find out if the element is editable.
        bool isEditable = GetIsEditable();

        // Check to see if the user is pointing to a valid cell.
        if (isEditable)
        {
            // Set the cursor to pointing hand cursor.
            pEditorWidget->setCursor(Qt::PointingHandCursor);

            // Set the tooltip text for the editor widget.
            rgUtils::SetToolAndStatusTip(s_EDITOR_ELEMENT_TOOLTIP, pEditorWidget);
        }
        else
        {
            // Set the cursor to arrow cursor.
            pEditorWidget->setCursor(Qt::ArrowCursor);
        }
    }
}

void rgEditorElement::InitializeControls()
{
    // Set the member name in the label.
    ui.memberNameLabel->setText(m_memberName.c_str());
}

void rgEditorElement::InitializeRows(rgEditorElement* pRootElement)
{
    if (pRootElement == nullptr)
    {
        pRootElement = this;
    }

    assert(pRootElement != nullptr);
    if (pRootElement != nullptr)
    {
        // Update the indentation for the row based on the number of ancestor parent rows.
        pRootElement->UpdateIndentation();

        // Set the tooltip text for the row.
        pRootElement->SetTooltipText();

        int numChildren = pRootElement->ChildCount();
        for (int childIndex = 0; childIndex < numChildren; ++childIndex)
        {
            rgEditorElement* pChild = pRootElement->GetChild(childIndex);
            InitializeRows(pChild);
        }
    }
}

void rgEditorElement::HandleValueChanged()
{
    ValueChangedHandler();
}

void rgEditorElement::ExpandTreeEntry()
{
    // Expand the tree entry.
    SetExpansionState(rgRowExpansionState::Expanded);
}

void rgEditorElement::HandleExpandClicked()
{
    // Toggle the expanded/collapsed state of the row.
    rgRowExpansionState newRowState = (m_rowState == rgRowExpansionState::Expanded) ?
        rgRowExpansionState::Collapsed : rgRowExpansionState::Expanded;

    // Update the expansion state.
    SetExpansionState(newRowState);
}

void rgEditorElement::HandleNameLabelFocusIn()
{
    // Get the parent tree that this element is connected to.
    rgPipelineStateTree* pParentTree = GetParentStateTree();
    assert(pParentTree != nullptr);
    if (pParentTree != nullptr)
    {
        // Set the current selection to this row and the member name column.
        pParentTree->SetCurrentSelection(this, static_cast<int>(rgRowData::RowDataMemberName));
    }
}

void rgEditorElement::HandleEditorFocusIn()
{
    // Get the parent tree that this element is connected to.
    rgPipelineStateTree* pParentTree = GetParentStateTree();
    assert(pParentTree != nullptr);
    if (pParentTree != nullptr)
    {
        // Set the current selection to this row and the value column.
        pParentTree->SetCurrentSelection(this, static_cast<int>(rgRowData::RowDataMemberValue));
    }
}

void rgEditorElement::HandleEditorFocusOut()
{
    // Get the parent tree that this element is connected to.
    rgPipelineStateTree* pParentTree = GetParentStateTree();
    assert(pParentTree != nullptr);
    if (pParentTree != nullptr)
    {
        // Select the current row only- no specific column.
        pParentTree->SetCurrentSelection(this);
    }
}

int rgEditorElement::ComputeIndentationWidth(rgEditorElement* pElement)
{
    if (pElement != nullptr)
    {
        rgEditorElement* pParent = pElement->GetParentItem();
        if (pParent != nullptr)
        {
            return s_INDENTATION_WIDTH + ComputeIndentationWidth(pParent);
        }
    }

    return 0;
}

void rgEditorElement::ConnectSignals()
{
    // Connect the row's expand/collapse button.
    bool isConnected = connect(ui.expandPushButton, &QPushButton::clicked, this, &rgEditorElement::HandleExpandClicked);
    assert(isConnected);

    // Connect the member name label's focus in signal.
    isConnected = connect(ui.memberNameLabel, &rgLabel::LabelFocusInEventSignal, this, &rgEditorElement::HandleNameLabelFocusIn);
    assert(isConnected);
}

void rgEditorElement::UpdateIndentation()
{
    // Adjust the row's indentation spacer width based on number of ancestors.
    int indentWidth = ComputeIndentationWidth(this);

    if (m_childItems.empty())
    {
        ui.expandPushButton->setVisible(false);
        indentWidth += ui.expandPushButton->width();
    }

    // Insert spacing at the beginning of the layout to indent.
    QHBoxLayout* pLayout = static_cast<QHBoxLayout*>(ui.rowInfo->layout());
    if (pLayout != nullptr && indentWidth > 0)
    {
        // If the spacer item has already been initialized,
        // remove it and destroy it before re-creating a new one.
        if (m_pSpacerItem != nullptr)
        {
            pLayout->removeItem(m_pSpacerItem);
            RG_SAFE_DELETE(m_pSpacerItem);
        }

        m_pSpacerItem = new QSpacerItem(indentWidth, 0, QSizePolicy::Fixed);
        pLayout->insertSpacerItem(0, m_pSpacerItem);
    }

    // Get the font metrics.
    QFontMetrics fontMetrics(ui.memberNameLabel->font());

    // Calculate the width of the member name label.
    const QRect boundingRect = fontMetrics.boundingRect(m_memberName.c_str());
    const int width = boundingRect.width();

    // Set the width of the value indent spacer. This aligns all editor widgets under the value
    // column to the same horizontal position.
    ui.valueIndentSpacer->changeSize(s_VALUE_COLUMN_POSITION - width, 0, QSizePolicy::Fixed);
}

void rgEditorElement::ClearSearchStringData()
{
    m_stringHighlightData.clear();
    UpdateButtonSubString();
    ResetLineEditSubString();
    ResetLabelSubString();
}

void rgEditorElement::ClearSearchStringDataVector()
{
    m_stringHighlightData.clear();
}

void rgEditorElement::HighlightButtonSubString(int startLocation, const std::string& searchString, bool isCurrentMatch)
{
    m_isCurrentMatch = isCurrentMatch;

    // Update search string location.
    UpdateStringMatchingLocation(startLocation, static_cast<int>(searchString.size()), searchString);
    UpdateButtonSubString();
}

void rgEditorElement::UpdateButtonSubString()
{
    QPushButton *pPushButton = ui.editorHost->findChild<QPushButton*>();
    ArrowIconWidget* pIconWidget = qobject_cast<ArrowIconWidget*>(pPushButton);
    if (pIconWidget != nullptr)
    {
        // Update the button string.
        pIconWidget->SetHighLightSubStringData(m_stringHighlightData);
        pIconWidget->SetHighLightSubString(true);
        pIconWidget->update();
    }
}

void rgEditorElement::UpdateStringMatchingLocation(int startLocation, int length, const std::string& searchString)
{
    bool isFound = false;

    // Remove any entries that do not match the search string anymore.
    int count = 0;
    std::vector<int> removeEntries;
    for (auto& stringData : m_stringHighlightData)
    {
        if (stringData.m_highlightString != searchString)
        {
            removeEntries.push_back(count);
        }
        count++;
    }
    for (std::vector<int>::reverse_iterator it = removeEntries.rbegin(); it != removeEntries.rend(); ++it)
    {
        m_stringHighlightData.remove(*it);
    }

    // Update existing locations, if any.
    for (auto& stringData : m_stringHighlightData)
    {
        if (stringData.m_startLocation == startLocation)
        {
            stringData.m_endLocation = startLocation + length;
            stringData.m_highlightString = searchString;
            isFound = true;
            SetHighlightColor(stringData);
            break;
        }
    }

    // Create a new entry if a matching entry not found.
    if (!isFound)
    {
        StringHighlightData stringHighlightData = {};
        stringHighlightData.m_startLocation = startLocation;
        stringHighlightData.m_endLocation = startLocation + length;
        stringHighlightData.m_highlightString = searchString;
        SetHighlightColor(stringHighlightData);
        m_stringHighlightData.push_back(stringHighlightData);
    }
}

void  rgEditorElement::SetHighlightColor(StringHighlightData& stringHighlightData)
{
    // The color in which the current match would be highlighted.
    static const QColor s_HIGHLIGHT_COLOR_MATCH_OTHER = QColor::fromRgba(qRgba(254, 206, 0, 200));

    // The color in which any match would be highlighted.
    static QColor s_HIGHLIGHT_COLOR_MATCH_CURRENT = QColor::fromRgba(qRgba(165, 175, 146, 200));

    if (m_isCurrentMatch)
    {
        stringHighlightData.m_highlightColor = s_HIGHLIGHT_COLOR_MATCH_CURRENT;
    }
    else
    {
        stringHighlightData.m_highlightColor = s_HIGHLIGHT_COLOR_MATCH_OTHER;
    }
}

void rgEditorElement::ResetButtonSubString()
{
    QPushButton *pPushButton = ui.editorHost->findChild<QPushButton*>();
    ArrowIconWidget* pIconWidget = qobject_cast<ArrowIconWidget*>(pPushButton);
    assert(pIconWidget != nullptr);
    if (pIconWidget != nullptr)
    {
        pIconWidget->ClearHighLightSubStringData();
        pIconWidget->SetHighLightSubString(false);
        pIconWidget->update();
    }
}

void rgEditorElement::ResetLineEditSubString()
{
    rgLineEdit* pLineEdit = ui.editorHost->findChild<rgLineEdit*>();
    if (pLineEdit != nullptr)
    {
        // Clear search string highlight.
        pLineEdit->SetHighlightSubString(false);
        pLineEdit->update();
    }
}

void rgEditorElement::HighlightLineEditSubString(int startLocation, const std::string& searchString, bool isCurrentMatch)
{
    m_isCurrentMatch = isCurrentMatch;

    rgLineEdit* pLineEdit = ui.editorHost->findChild<rgLineEdit*>();
    if (pLineEdit != nullptr)
    {
        // Update search string location.
        UpdateStringMatchingLocation(startLocation, static_cast<int>(searchString.size()), searchString);
        pLineEdit->SetHighlightSubStringData(m_stringHighlightData);
        pLineEdit->SetHighlightSubString(true);
        pLineEdit->update();
    }
}

void rgEditorElement::ResetLabelSubString()
{
    rgLabel* pLabel = ui.rowInfo->findChild<rgLabel*>();
    if (pLabel != nullptr)
    {
        // Clear search string highlight.
        pLabel->SetHighlightSubString(false);
        pLabel->update();
    }
}

void rgEditorElement::HighlightLabelSubString(int startLocation, const std::string& searchString, bool isCurrentMatch)
{
    m_isCurrentMatch = isCurrentMatch;

    rgLabel* pLabel = ui.rowInfo->findChild<rgLabel*>();
    if (pLabel != nullptr)
    {
        // Update search string location.
        UpdateStringMatchingLocation(startLocation, static_cast<int>(searchString.size()), searchString);
        pLabel->SetHighlightSubStringData(m_stringHighlightData);
        pLabel->SetHighlightSubString(true);
        pLabel->update();
    }
}