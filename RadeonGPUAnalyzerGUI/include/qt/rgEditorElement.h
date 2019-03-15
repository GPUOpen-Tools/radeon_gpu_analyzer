#pragma once

// C++
#include <functional>

// Qt.
#include <QVariant>
#include <QWidget>

// Infra.
#include <QtCommon/CustomWidgets/ArrowIconWidget.h>

// Local.
#include <ui_rgEditorElement.h>

// The following style bit flags can be applied to each row in order to alter the background color.
enum class rgStyleFlags : char
{
    // No special style is applied. The row is drawn with default coloring.
    None = 0,

    // The row is displayed as "current," because the user selected the row.
    CurrentRow = 1 << 0,

    // The row is displayed as a "Search Result Occurrence," because it was flagged as containing
    // a search result, but it's not the current result being focused on.
    SearchResultOccurrence = 1 << 1,

    // The row is displayed as the "Current Search Result," because it's flagged as containing a
    // search result, and it's the current result being focused on.
    SearchResultCurrent = 1 << 2
};

// This enum identifies each row's data.
enum class rgRowData
{
    RowDataMemberName,
    RowDataMemberValue,
    RowDataCount
};

// An enumeration used to track the expanded/collapsed state of a row.
enum class rgRowExpansionState
{
    Expanded,
    Collapsed
};

// A list of value types that the user is able to edit.
enum class rgEditorDataType : char
{
    // This type is used for editor elements that cannot be edited.
    Void,

    // Edit true/false type.
    Bool,

    // Edit numeric primitive types.
    Int8,
    UInt8,
    Int16,
    UInt16,
    Int32,
    UInt32,
    Float,
    Double,

    // Edit elements with a limited number of possible values.
    Enum,

    // The type used for array root elements. These elements can be resized to alter
    // the dimension of an array of child element items.
    Array,

    // A single element of an array.
    ArrayElement,
};

// Forward declarations.
class rgPipelineStateEditorWidget;
class rgPipelineStateTree;

// The base class for typed editor elements.
class rgEditorElement : public QWidget
{
    Q_OBJECT

public:
    explicit rgEditorElement(QWidget* pParent, const std::string& memberName,
        rgEditorDataType dataType = rgEditorDataType::Void,
        std::function<void()> valueChangedCallback = nullptr);
    virtual ~rgEditorElement() = default;

    // Get the data held within this item.
    virtual QVariant Data(int column) const;

    // Retrieve the editor widget used by the row. If the row is not editable, it won't contain an
    // editor widget, and this function will return nullptr.
    virtual rgPipelineStateEditorWidget* GetEditorWidget();

    // Add the given style flag to the row's style bit field.
    void AddStyleFlag(rgStyleFlags styleFlag);

    // Remove the given style flag from the row's style bit field.
    void RemoveStyleFlag(rgStyleFlags styleFlag);

    // Append a new child element to the item.
    void AppendChildItem(rgEditorElement* pItem);

    // Remove all children from the element.
    void ClearChildren();

    // Recursively find the last visible descendant row.
    rgEditorElement* FindLastVisibleDescendant(rgEditorElement* pCurrentElement) const;

    // Get the next ancestor to the given element by walking up the tree of parent rows.
    rgEditorElement* FindNextAnscestor(rgEditorElement* pCurrentElement = nullptr);

    // Get the last descendant row of the element tree.
    // If there are no remaining descendants return this element.
    rgEditorElement* GetLastVisibleDescendant();

    // Get the parent rgPipelineStateTree that this widget hierarchy is rooted to.
    rgPipelineStateTree* GetParentStateTree() const;

    // Set this row's parent rgPipelineStateTree.
    void SetParentStateTree(rgPipelineStateTree* pParentTree);

    // Remove a child from the item.
    void RemoveChild(rgEditorElement* pChildItem);

    // Get a child item by row index.
    rgEditorElement* GetChild(int rowIndex) const;

    // Returns true if the value should be editable, and false if not.
    bool GetIsEditable() const;

    // Returns the height of the row itself- not including children.
    int GetRowHeight() const;

    // Return the style flags bit field for the row.
    uint32_t GetStyleFlags() const;

    // Get the number of children for this item.
    int ChildCount() const;

    // Get the number of columns displayed for an item.
    int ColumnCount() const;

    // Get the row index of this child relative to the parent's children list.
    int GetRowIndex() const;

    // Get the parent item for this item.
    rgEditorElement* GetParentItem() const;

    // Get the type of data being edited by this element.
    rgEditorDataType GetType() const;

    // Set the expansion state of the row.
    void SetExpansionState(rgRowExpansionState state, bool updateChildren = false);

    // Get the expanded/collapsed state.
    rgRowExpansionState GetExpansionState() const;

    // Set the element's tooltip text.
    void SetTooltipText();

    // Initialize the row controls.
    void InitializeControls();

    // Recursively initialize tree rows, starting at the given element.
    void InitializeRows(rgEditorElement* pRootElement = nullptr);

    // Retrieve the member name.
    std::string GetMemberName() const { return m_memberName; }

    // Clear the substring search data.
    // This method clears any stored data related to
    // substring start location, end location, string value
    // and the color to be used to highlight the substring.
    void ClearSearchStringData();

    // Clear the substring search data storage vector.
    // This method clears the substring information
    // in between highlighting substring between
    // widgets within the same row.
    void ClearSearchStringDataVector();

    // Highlight the line edit matching substring.
    void HighlightLineEditSubString(int startLocation, const std::string& searchString, bool isCurrentMatch);

    // Reset the line edit matching substring.
    void ResetLineEditSubString();

    // Reset the label matching substring.
    void ResetLabelSubString();

    // Highlight the requested substring in the button.
    void HighlightButtonSubString(int characterLocation, const std::string& searchString, bool isCurrentMatch);

    // Reset the requested substring in the button.
    void ResetButtonSubString();

    // Highlight the requested substring in the label.
    void HighlightLabelSubString(int characterLocation, const std::string& searchString, bool isCurrentMatch);

    // Reset the requested substring in the label.
    void ResetLabelSubString(int characterLocation, const std::string& searchString);

    // Focus on the given column widget.
    void SetFocusedColumn(rgRowData column);

    // Set the callback that's invoked when the value is changed.
    void SetValueChangedCallback(std::function<void()> valueChangedCallback);

    // A flag indicating if the row is currently filtered out due to a user search.
    bool m_isFilteredOut;

public slots:
    // Handler invoked when the user clicks an editor widget in the value column of the row.
    void HandleEditorFocusIn();

    // Handler invoked when editor widget loses focus.
    void HandleEditorFocusOut();

    // Handler invoked when the user clicks the row's Expand/Collapse button.
    void HandleExpandClicked();

    // A handler invoked when an element's value has changed.
    void HandleValueChanged();

protected slots:
    // Handler invoked when the user clicks the row's name label in the left column.
    void HandleNameLabelFocusIn();

signals:
    // A signal emitted to indicate list widget status change.
    void EnumListWidgetStatusSignal(bool isOpen);

protected:
    // A handler invoked when the user has changed the editable value.
    virtual void ValueChangedHandler() {};

    // Compute the indent width for the given element.
    int ComputeIndentationWidth(rgEditorElement* pElement);

    // Connect internal signals to slots within the row element.
    void ConnectSignals();

    // Expand the tree entry.
    void ExpandTreeEntry();

    // Set the highlight color for the matched string.
    void SetHighlightColor(StringHighlightData& stringHighlightData);

    // Set the style flag bits for the row.
    void SetStyleFlags(uint32_t styleFlags);

    // Compute the indentation for each row in the tree.
    void UpdateIndentation();

    // Update the button substring to highlight.
    void UpdateButtonSubString();

    // Update the location for the matching substring location.
    void UpdateStringMatchingLocation(int startLocation, int length, const std::string& searchString);

    // A callback invoked when the element value is changed.
    std::function<void()> m_valueChangedCallback = nullptr;

    // A vector of child nodes for this item.
    std::vector<rgEditorElement*> m_childItems;

    // The name of the member being edited.
    std::string m_memberName;

    // The type of value being edited.
    rgEditorDataType m_type;

    // The current expanded/collapsed state of the row.
    rgRowExpansionState m_rowState;

    // The parent item for this item.
    rgEditorElement* m_pParentItem = nullptr;

    // The parent rgPipelineStateTree that this instance is rooted to.
    // This pointer will only be valid if it's the root element in the tree.
    rgPipelineStateTree* m_pParentTree = nullptr;

    // A bit field of style flags for the row.
    uint32_t m_styleFlags = 0;

    // A vector to store substring highlight data.
    QVector<StringHighlightData> m_stringHighlightData = {};

    // Boolean to indicate if this is the current match.
    bool m_isCurrentMatch = false;

    // A spacer that can be inserted to indent the row.
    QSpacerItem* m_pSpacerItem = nullptr;

    // The generated view object.
    Ui::rgEditorElement ui;
};
