#pragma once

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgMenuFileItem.h>
#include <RadeonGPUAnalyzerGUI/Include/rgStringConstants.h>
#include <ui_rgMenuFileItemOpenCL.h>

// Forward declarations.
class QMenu;
class QStandardItemModel;
class rgEntrypointItemStyleDelegate;
struct rgEntryOutput;

// A model responsible for managing a file item's entry point list.
// Must be declared here to allow a Qt interface to be generated for it.
class rgMenuItemEntryListModel : public QObject
{
    Q_OBJECT

public:
    rgMenuItemEntryListModel(QWidget* pParent = nullptr);
    virtual ~rgMenuItemEntryListModel() = default;

    // Get the item model for the entry point list.
    QStandardItemModel* GetEntryItemModel() const;

    // Add a new entry point to the file's entry point list.
    void AddEntry(const std::string& entrypointName);

    // Clear all entries from the list model.
    void ClearEntries();

    // Set the width of the entry point widget.
    void SetEntryPointWidgetWidth(int width);

    // Set the entry point label widget.
    void SetEntryPointTreeWidget(rgMenuEntryPointTree* pTree);

    // Get the entry point names.
    void GetEntryPointNames(std::vector<std::string>& entrypointNames);

    // Get the entry point name at a given index.
    std::string GetEntryPointName(const int index) const;

    // Get the entry point name from the display name.
    std::string GetEntryPointName(const std::string& displayEntrypointName) const;

signals:
    // A signal emitted when the user changes the selected entry point in the entry point list.
    void SelectedEntrypointChanged(const std::string& inputFilePath, int selectedIndex);

private:
    // A model responsible for holding the entry point item list.
    QStandardItemModel* m_pEntrypointItemModel = nullptr;

    // The width of the entry point widget.
    int m_entryPointWidgetWidth = 0;

    // The entry point tree.
    rgMenuEntryPointTree* m_pEntryPointTree = nullptr;

    // Keep track of entry point name.
    std::vector<std::string> m_entryPointNames;

    //Keep track of the display name.
    std::vector<std::string> m_displayNames;
};

// An object used to represent a single shader file within the file menu.
class rgMenuFileItemOpenCL : public rgMenuFileItem
{
    Q_OBJECT

public:
    explicit rgMenuFileItemOpenCL(const std::string& fileFullPath, rgMenu* pParent = nullptr);
    virtual ~rgMenuFileItemOpenCL() = default;

    // Handler for mouse hover enter events.
    virtual void enterEvent(QEvent* pEvent) override;

    // Handler for mouse hover leave events.
    virtual void leaveEvent(QEvent* pEvent) override;

    // Handler for a double-click on the item.
    virtual void mouseDoubleClickEvent(QMouseEvent* pEvent) override;

    // Handler invoked when the user clicks this menu item.
    virtual void mousePressEvent(QMouseEvent* pEvent) override;

    // Handler invoked when this item is resized.
    virtual void resizeEvent(QResizeEvent* pEvent) override;

    // Handler invoked when this item is shown.
    virtual void showEvent(QShowEvent* pEvent) override;

    // Handler invoked when the user hits a key.
    virtual void keyPressEvent(QKeyEvent* pEvent) override;

    // Remove all entrypoints from the item's entry point list.
    void ClearEntrypointsList();

    // Retrieve the list of entry point names for this input file.
    void GetEntrypointNames(std::vector<std::string>& entrypointNames) const;

    // Get the name of the currently selected entrypoint, if there is one.
    // Returns "false" if no entry is currently selected or the selected one is not valid any more.
    bool GetSelectedEntrypointName(std::string& entrypointName) const;

    // Alter the visual style of the item if it is hovered or not.
    virtual void SetHovered(bool isHovered) override;

    // Alter the visual style of the item if it is currently selected.
    virtual void SetCurrent(bool isCurrent) override;

    // Set the visibility of the entrypoints list for the file.
    void ShowEntrypointsList(bool showList);

    // Switch to the given entry point in the item's dropdown.
    void SwitchToEntrypointByName(const std::string& entrypointName);

    // Update the file item with the latest build outputs.
    void UpdateBuildOutputs(const std::vector<rgEntryOutput>& entryOutputs);

signals:
    // Signal emitted when the item's close button is clicked.
    void MenuItemCloseButtonClicked(const std::string& fullPath);

    // Signal emitted when the user changes the selected entrypoint.
    void SelectedEntrypointChanged(const std::string& filePath, const std::string& selectedEntrypointName);

private slots:
    // Handler invoked when the user would like to remove a file.
    void HandleRemoveItemRequest();

    // Handler invoked when the user clicks an item in the entry point list.
    void HandleEntrypointClicked();

    // Handler invoked when the user enters the entry point list tree view
    void HandleTableItemEntered(const QModelIndex& modelIndex);

public slots:
    // Handler invoked when the project build is successful.
    void HandleProjectBuildSuccess();

protected:
    // Initialize signals.
    void ConnectSignals();

    // Create the file's entry point list.
    void InitializeEntrypointsList();

    // Update the item's file label text.
    virtual void UpdateFilenameLabelText() override;

    // Get the rename item line edit widget.
    virtual QLineEdit* GetRenameLineEdit() override;

    // Get the item text label widget.
    virtual QLabel* GetItemLabel() override;

    // Update the file path for this item.
    void UpdateFilepath(const std::string& newFilepath);

    // Set the cursor to pointing hand cursor for various widgets.
    void SetCursor();

    // A model responsible for the data within a file menu item's "entrypoint list" view.
    rgMenuItemEntryListModel* m_pEntryListModel = nullptr;

    // The style delegate used to paint the entry point list correctly.
    rgEntrypointItemStyleDelegate* m_pEntrypointStyleDelegate = nullptr;

    // The name of last selected entry point name.
    std::string m_lastSelectedEntryName;

    // The generated view object.
    Ui::rgMenuFileItemOpenCL ui;
};