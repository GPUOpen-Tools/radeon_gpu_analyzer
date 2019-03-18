#pragma once

// Local.
#include <RadeonGPUAnalyzerGUI/Include/rgDataTypes.h>
#include <ui_rgMenuPipelineStateItem.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgMenuItem.h>

// Forward declarations:
class QPushButton;

class rgMenuPipelineStateItem
    : public rgMenuItem
{
    Q_OBJECT

public:
    explicit rgMenuPipelineStateItem(rgPipelineType pipelineType, rgMenu* pParent = nullptr);
    virtual ~rgMenuPipelineStateItem() = default;

    // Handler invoked when the user drags a file over.
    virtual void dragEnterEvent(QDragEnterEvent* pEvent) override;

    // Handler invoked when the user drops a dragged file.
    virtual void dropEvent(QDropEvent *pEvent) override;

    // Handler invoked when the user leaves the button.
    virtual void dragLeaveEvent(QDragLeaveEvent* pEvent);

    // Get a pointer to the pipeline state button within the item.
    QPushButton* GetPipelineStateButton() const;

    // Set the cursor to the specified type.
    void SetCursor(const QCursor& cursor);

    // Alter the visual style of the item if it is currently selected.
    void SetCurrent(bool isCurrent);

    // Get the current state.
    bool IsCurrent() const;

    // Simulate a click on the menu item.
    void ClickMenuItem() const;

signals:
    // A signal emitted when a PSO editor file is dragged and dropped.
    void DragAndDropExistingFile(const std::string& filePath);

    // Signal emitted when the pipeline state button is clicked.
    void PipelineStateButtonClicked(rgMenuPipelineStateItem* pItem);

private slots:
    // Handler invoked when the Pipeline State button is clicked.
    void HandlePipelineStateButtonClicked(bool checked);

private:
    // Connect signals.
    void ConnectSignals();

    // Set the text for the build settings item.
    void SetItemText(const std::string& itemText);

    // Change appearance to "focus in".
    void GotFocus();

    // Change appearance to "focus out".
    void LostFocus();

    // The Build Settings file item interface.
    Ui::rgMenuPipelineStateItem ui;

    // Flag to keep track of whether this item is currently selected.
    bool m_current = false;

    // The pipeline type.
    rgPipelineType m_pipelineType;
};