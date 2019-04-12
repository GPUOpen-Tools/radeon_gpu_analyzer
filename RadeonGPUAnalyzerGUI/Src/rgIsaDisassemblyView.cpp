#ifdef _WIN32
// This warning relates to a compiler limitation that restricts the length of type names.
// The warning doesn't impose any risk to the code, so it's disabled for this file.
#pragma warning(disable : 4503)
#endif

// C++.
#include <cassert>
#include <memory>
#include <sstream>

// Qt.
#include <QAction>
#include <QCheckBox>
#include <QListWidgetItem>

// Infra.
#include <QtCommon/CustomWidgets/ArrowIconWidget.h>
#include <QtCommon/CustomWidgets/ListWidget.h>
#include <QtCommon/Scaling/ScalingManager.h>
#include <QtCommon/Util/CommonDefinitions.h>
#include <QtCommon/Util/QtUtil.h>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgHideListWidgetEventFilter.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgIsaDisassemblyTableModel.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgIsaDisassemblyTableView.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgIsaDisassemblyTabView.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgIsaDisassemblyView.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgResourceUsageView.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgViewContainer.h>
#include <RadeonGPUAnalyzerGUI/Include/rgDataTypes.h>
#include <RadeonGPUAnalyzerGUI/Include/rgDefinitions.h>
#include <RadeonGPUAnalyzerGUI/Include/rgResourceUsageCsvFileParser.h>
#include <RadeonGPUAnalyzerGUI/Include/rgStringConstants.h>
#include <RadeonGPUAnalyzerGUI/Include/rgUtils.h>

// Names of special widgets within the disassembly view.
static const char* STR_DISASSEMBLY_COLUMN_VISIBILITY_LIST = "DisassemblyColumnVisibilityList";
static const char* STR_DISASSEMBLY_COLUMN_LIST_ITEM_CHECKBOX = "ListWidgetCheckBox";
static const char* STR_DISASSEMBLY_COLUMN_LIST_ITEM_ALL_CHECKBOX = "ListWidgetAllCheckBox";

// Object name associated with the target GPU dropdown list.
static const char* STR_DISASSEMBLY_TARGET_GPU_LIST = "TargetGpuList";

// Columns push button font size.
const int s_PUSH_BUTTON_FONT_SIZE = 11;

rgIsaDisassemblyView::rgIsaDisassemblyView(QWidget* pParent) :
    QWidget(pParent)
{
    ui.setupUi(this);

    ui.verticalSplitterWidget->setStretchFactor(0, 1);
    ui.verticalSplitterWidget->setStretchFactor(1, 0);
    ui.verticalSplitterWidget->handle(1)->setDisabled(true);

    // Create the widget used to control column visibility.
    CreateColumnVisibilityControls();

    // Fill the column visibility dropdown with all of the possible column names.
    PopulateColumnVisibilityList();

    // Create the dropdown list used to select the current target GPU.
    CreateTargetGpuListControls();

    // Connect the signals for the disassembly view.
    ConnectSignals();

    // Set mouse pointer to pointing hand cursor.
    SetCursor();

    // Block recursively repolishing child tables in the disassembly view.
    ui.disassemblyTableHostWidget->setProperty(gs_IS_REPOLISHING_BLOCKED, true);

    // Set the push button font sizes.
    SetFontSizes();

    // Set the focus proxy of the view maximize button to be the title bar.
    // This will cause the frame border to change to black when the maximize button loses focus.
    // This is because the view title bar already handles its own loss of focus event.
    ui.viewMaximizeButton->setFocusProxy(ui.viewTitlebar);
}

rgIsaDisassemblyView::~rgIsaDisassemblyView()
{
    // Remove the column dropdown focus event filters if they exist.
    if (m_pDisassemblyColumnsListWidget != nullptr && m_pDisassemblyColumnsListEventFilter != nullptr)
    {
        m_pDisassemblyColumnsListWidget->removeEventFilter(m_pDisassemblyColumnsListEventFilter);
        qApp->removeEventFilter(m_pDisassemblyColumnsListEventFilter);
    }

    // Remove the GPU dropdown event filter.
    if (m_pTargetGpusListWidget != nullptr && m_pTargetGpusListEventFilter != nullptr)
    {
        m_pTargetGpusListWidget->removeEventFilter(m_pTargetGpusListEventFilter);
        qApp->removeEventFilter(m_pTargetGpusListEventFilter);
    }
}

void rgIsaDisassemblyView::ClearBuildOutput()
{
    if (!m_gpuTabViews.empty())
    {
        // Destroy all existing GPU tabs.
        auto firstGpuTab = m_gpuTabViews.begin();
        auto lastGpuTab = m_gpuTabViews.end();
        for (auto gpuTabIter = firstGpuTab; gpuTabIter != lastGpuTab; ++gpuTabIter)
        {
            ui.disassemblyTableHostWidget->removeWidget(gpuTabIter->second);
        }
    }

    // Clear all state data in the view.
    m_gpuTabViews.clear();
    m_gpuResourceUsageViews.clear();
    m_resourceUsageText.clear();
}

void rgIsaDisassemblyView::GetResourceUsageTextBounds(QRect& textBounds) const
{
    QFontMetrics fontMetrics(m_resourceUsageFont);
    textBounds = fontMetrics.boundingRect(m_resourceUsageText.c_str());
}

bool rgIsaDisassemblyView::IsEmpty() const
{
    return m_gpuTabViews.empty();
}

void rgIsaDisassemblyView::RemoveInputFileEntries(const std::string& inputFilePath)
{
    // Clean up disassembly tables associated with the given file.
    DestroyDisassemblyViewsForFile(inputFilePath);

    // Clean up resource usage views associated with the given file.
    DestroyResourceUsageViewsForFile(inputFilePath);
}

void rgIsaDisassemblyView::HandleInputFileSelectedLineChanged(const std::string& targetGpu, const std::string& inputFilePath, std::string& entryName, int lineIndex)
{
    // Get the currently active stacked view.
    rgIsaDisassemblyTabView* pCurrentTabView = GetTargetGpuTabWidgetByTabName(targetGpu);

    if (pCurrentTabView != nullptr && pCurrentTabView->GetTableCount() > 0)
    {
        HandleSelectedEntrypointChanged(targetGpu, inputFilePath, entryName);
        pCurrentTabView->UpdateCorrelatedSourceFileLine(inputFilePath, lineIndex, entryName);
    }
}

void rgIsaDisassemblyView::HandleSelectedEntrypointChanged(const std::string& targetGpu, const std::string& inputFilePath, const std::string& selectedEntrypointName)
{
    // Get the currently active stacked view.
    rgIsaDisassemblyTabView* pTargetTabView = GetTargetGpuTabWidgetByTabName(targetGpu);

    // Switch the target GPU tab if necessary.
    if (pTargetTabView != m_pCurrentTabView)
    {
        SetCurrentTargetGpuTabView(pTargetTabView);
    }

    if (pTargetTabView != nullptr)
    {
        // Switch the table to show the disassembly for the given entrypoint.
        pTargetTabView->SwitchToEntryPoint(inputFilePath, selectedEntrypointName);
    }

    // Get a reference to the map of input file path to the entry point names map.
    InputToEntrypointViews& inputFileToEntrypointMap = m_gpuResourceUsageViews[targetGpu];

    // Use the input file path to get a reference to a map of entry point names to resource usage views.
    EntrypointToResourcesView& entrypointMap = inputFileToEntrypointMap[inputFilePath];

    // Search the map to find the resource usage view associated with the given entrypoint.
    auto resourceViewIter = entrypointMap.find(selectedEntrypointName);
    if (resourceViewIter != entrypointMap.end())
    {
        // Display the resource usage view associated with the given entrypoint.
        rgResourceUsageView* pResourceUsageView = resourceViewIter->second;
        ui.resourceUsageHostStackedWidget->setCurrentWidget(pResourceUsageView);
    }
}

void rgIsaDisassemblyView::HandleColumnVisibilityButtonClicked(bool /*clicked*/)
{
    // Make the list widget appear and process user selection from the list widget.
    bool visible = m_pDisassemblyColumnsListWidget->isVisible();
    if (visible == true)
    {
        m_pDisassemblyColumnsListWidget->hide();

        // Change the up arrow to a down arrow.
        ui.columnVisibilityArrowPushButton->SetDirection(ArrowIconWidget::Direction::DownArrow);
    }
    else
    {
        // Update the check box values in case they were changed in global settings.
        rgConfigManager& configManager = rgConfigManager::Instance();
        std::shared_ptr<rgGlobalSettings> pGlobalSettings = configManager.GetGlobalConfig();
        ListWidget::SetColumnVisibilityCheckboxes(m_pDisassemblyColumnsListWidget, pGlobalSettings->m_visibleDisassemblyViewColumns);

        // Compute where to place the combo box relative to where the arrow button is.
        QWidget* pWidget = ui.columnVisibilityArrowPushButton;
        m_pDisassemblyColumnsListWidget->show();
        m_pDisassemblyColumnsListWidget->setFocus();
        QRect rect = pWidget->geometry();
        QPoint pos(0, 0);
        pos = pWidget->mapTo(this, pos);
        pos.setY(pos.y() + rect.height());
        int height = QtCommon::QtUtil::GetListWidgetHeight(m_pDisassemblyColumnsListWidget);
        int width = QtCommon::QtUtil::GetListWidgetWidth(m_pDisassemblyColumnsListWidget);
        m_pDisassemblyColumnsListWidget->setGeometry(pos.x(), pos.y(), width + s_CHECK_BOX_WIDTH, height);

        // Change the down arrow to an up arrow.
        ui.columnVisibilityArrowPushButton->SetDirection(ArrowIconWidget::Direction::UpArrow);
    }
}

void rgIsaDisassemblyView::HandleColumnVisibilityComboBoxItemClicked(const QString& text, const bool checked)
{
    rgConfigManager& configManager = rgConfigManager::Instance();
    std::shared_ptr<rgGlobalSettings> pGlobalSettings = configManager.GetGlobalConfig();

    const int firstColumn = rgIsaDisassemblyTableModel::GetTableColumnIndex(rgIsaDisassemblyTableColumns::Address);
    const int lastColumn = rgIsaDisassemblyTableModel::GetTableColumnIndex(rgIsaDisassemblyTableColumns::Count);

    // Get the current checked state of the UI.
    // This will include changes from the check/uncheck that triggered this callback.
    std::vector<bool> columnVisibility = ListWidget::GetColumnVisibilityCheckboxes(m_pDisassemblyColumnsListWidget);

    // Make sure at least one check box is still checked.
    bool isAtLeastOneChecked = std::any_of(columnVisibility.begin() + firstColumn, columnVisibility.begin() + lastColumn, [](bool b) { return b == true; });

    if (checked || isAtLeastOneChecked)
    {
        // If the user checked the "All" option, Step through each column and set to visible.
        if (text.compare(STR_DISASSEMBLY_TABLE_COLUMN_ALL) == 0 && (checked == true))
        {
            for (int columnIndex = firstColumn; columnIndex < lastColumn; ++columnIndex)
            {
                columnVisibility[columnIndex] = checked;
            }

            // Update the state of the dropdown check boxes to reflect that all options are checked.
            ListWidget::SetColumnVisibilityCheckboxes(m_pDisassemblyColumnsListWidget, columnVisibility);
        }

        // Save the changes.
        configManager.Instance().SetDisassemblyColumnVisibility(columnVisibility);
        configManager.SaveGlobalConfigFile();
    }
    else
    {
        // The user tried to uncheck the last check box, but at least one box
        // MUST be checked, so find that item in the ListWidget, and set it back to checked.
        for (int row = 0; row < m_pDisassemblyColumnsListWidget->count(); row++)
        {
            QListWidgetItem* pItem = m_pDisassemblyColumnsListWidget->item(row);
            QCheckBox* pCheckBox = (QCheckBox*)m_pDisassemblyColumnsListWidget->itemWidget(pItem);
            QString checkBoxText = pCheckBox->text();
            if (checkBoxText.compare(text) == 0)
            {
                QCheckBox* pCheckBox = (QCheckBox*)m_pDisassemblyColumnsListWidget->itemWidget(pItem);
                pCheckBox->setChecked(true);
            }
        }
    }

    // See if the "All" box needs checking/unchecking.
    ListWidget::UpdateAllCheckbox(m_pDisassemblyColumnsListWidget);

    // Update the "All" checkbox text color to grey or black.
    UpdateAllCheckBoxText();
}

void rgIsaDisassemblyView::HandleColumnVisibilityFilterStateChanged(bool checked)
{
    // Figure out the sender and process appropriately.
    QObject* pSender = QObject::sender();
    assert(pSender != nullptr);

    // Find out which entry caused the signal.
    QWidget* pItem = qobject_cast<QWidget*>(pSender);
    assert(pItem != nullptr);

    QCheckBox* pCheckBox = qobject_cast<QCheckBox*>(pSender);
    assert(pCheckBox != nullptr);

    // Process the click.
    HandleColumnVisibilityComboBoxItemClicked(pCheckBox->text(), checked);

    // Emit a signal to trigger a refresh of the disassembly table's filter.
    emit DisassemblyColumnVisibilityUpdated();
}

void rgIsaDisassemblyView::HandleTargetGpuArrowClicked(bool clicked)
{
    // Make the list widget appear and process user selection from the list widget.
    bool visible = m_pTargetGpusListWidget->isVisible();
    if (visible == true)
    {
        m_pTargetGpusListWidget->hide();

        // Change the up arrow to a down arrow.
        ui.targetGpuPushButton->SetDirection(ArrowIconWidget::Direction::DownArrow);
    }
    else
    {
        // Compute where to place the combo box relative to where the arrow button is.
        QWidget* pWidget = ui.targetGpuPushButton;
        m_pTargetGpusListWidget->show();
        m_pTargetGpusListWidget->setFocus();
        m_pTargetGpusListWidget->setCursor(Qt::PointingHandCursor);
        QRect rect = pWidget->geometry();
        QPoint pos(0, 0);
        pos = pWidget->mapTo(this, pos);
        pos.setY(pos.y() + rect.height());
        int height = QtCommon::QtUtil::GetListWidgetHeight(m_pTargetGpusListWidget);
        int width = QtCommon::QtUtil::GetListWidgetWidth(m_pTargetGpusListWidget);
        m_pTargetGpusListWidget->setGeometry(pos.x(), pos.y(), width + s_CHECK_BOX_WIDTH, height);

        // Change the down arrow to an up arrow.
        ui.targetGpuPushButton->SetDirection(ArrowIconWidget::Direction::UpArrow);
    }
}

void rgIsaDisassemblyView::HandleTargetGpuChanged(int currentIndex)
{
    assert(m_pTargetGpusListWidget != nullptr);
    if (m_pTargetGpusListWidget != nullptr)
    {
        auto pTargetGpuItem = m_pTargetGpusListWidget->item(currentIndex);
        assert(pTargetGpuItem != nullptr);
        if (pTargetGpuItem != nullptr)
        {
            // Change the target GPU if it differs from the current target GPU.
            std::string currentTargetGpu = ui.targetGpuPushButton->text().toStdString();
            std::string newTargetGpu = pTargetGpuItem->text().toStdString();
            if (currentTargetGpu.compare(newTargetGpu) != 0)
            {
                // Use the dropdown list's selection model to change the currently selected target GPU.
                QItemSelectionModel* pSelectionModel = m_pTargetGpusListWidget->selectionModel();
                assert(pSelectionModel != nullptr);
                if (pSelectionModel != nullptr)
                {
                    // Select the new target GPU within the dropdown list widget.
                    QAbstractItemModel* pListModel = m_pTargetGpusListWidget->model();

                    assert(pListModel != nullptr);
                    if (pListModel != nullptr)
                    {
                        QModelIndex modelIndex = pListModel->index(currentIndex, 0);
                        pSelectionModel->setCurrentIndex(modelIndex, QItemSelectionModel::SelectionFlag::Select);
                    }
                }

                // Change the target GPU to the newly selected item.
                SetTargetGpu(newTargetGpu);

                // Strip the GPU name from the architecture if needed.
                std::string strippedGpuName;
                size_t bracketPos = newTargetGpu.find("(");
                if (bracketPos != std::string::npos && newTargetGpu.size() > 2)
                {
                    strippedGpuName = newTargetGpu.substr(0, bracketPos - 1);
                }
                else
                {
                    strippedGpuName = newTargetGpu;
                }

                // Strip gfx notation if needed.
                size_t slashPos = strippedGpuName.find("/");
                if (slashPos != std::string::npos)
                {
                    strippedGpuName = strippedGpuName.substr(slashPos + 1);
                }

                // Emit a signal with the name of the target GPU to switch to.
                emit SelectedTargetGpuChanged(strippedGpuName);
            }
        }
    }

}

void rgIsaDisassemblyView::ClearListWidget(ListWidget* &pListWidget)
{
    assert(pListWidget != nullptr);

    // Disconnect slot/signal connection for each check box
    for (int row = 0; row < pListWidget->count(); row++)
    {
        QListWidgetItem* pItem = pListWidget->item(row);
        QCheckBox* pCheckBox = (QCheckBox*)pListWidget->itemWidget(pItem);

        if (pListWidget->objectName().compare(STR_DISASSEMBLY_COLUMN_VISIBILITY_LIST) == 0)
        {
            bool isDisconnected = disconnect(pCheckBox, &QCheckBox::clicked, this, &rgIsaDisassemblyView::HandleColumnVisibilityFilterStateChanged);
            assert(isDisconnected);
        }
        else
        {
            assert(false);
        }
    }

    // Clear the list widget. This also deletes each item.
    pListWidget->clear();
}

void rgIsaDisassemblyView::ConnectDisassemblyTabViewSignals(rgIsaDisassemblyTabView* pEntryView)
{
    // Connect the new disassembly GPU tab's source input file highlight line changed signal.
    bool isConnected = connect(pEntryView, &rgIsaDisassemblyTabView::InputSourceHighlightedLineChanged, this, &rgIsaDisassemblyView::InputSourceHighlightedLineChanged);
    assert(isConnected);

    // Connect the disassembly table's resized event handler.
    isConnected = connect(pEntryView, &rgIsaDisassemblyTabView::DisassemblyTableWidthResizeRequested, this, &rgIsaDisassemblyView::DisassemblyTableWidthResizeRequested);
    assert(isConnected);

    // Connect the disassembly view's column visibility updated signal.
    isConnected = connect(this, &rgIsaDisassemblyView::DisassemblyColumnVisibilityUpdated, pEntryView, &rgIsaDisassemblyTabView::HandleColumnVisibilityFilterStateChanged);
    assert(isConnected);

    // Connect the disassembly view's set frame border red signal.
    isConnected = connect(pEntryView, &rgIsaDisassemblyTabView::FrameFocusInSignal, this, &rgIsaDisassemblyView::HandleDisassemblyTabViewClicked);
    assert(isConnected);

    // Connect the disassembly view's set frame border black signal.
    isConnected = connect(pEntryView, &rgIsaDisassemblyTabView::FrameFocusOutSignal, this, &rgIsaDisassemblyView::HandleDisassemblyTabViewLostFocus);
    assert(isConnected);

    // Connect the disassembly view's title bar's set frame border red signal.
    isConnected = connect(ui.viewTitlebar, &rgIsaDisassemblyViewTitlebar::FrameFocusInSignal, this, &rgIsaDisassemblyView::HandleDisassemblyTabViewClicked);
    assert(isConnected);

    // Connect the disassembly view's enable scroll bar signal.
    isConnected = connect(this, &rgIsaDisassemblyView::EnableScrollbarSignals, pEntryView, &rgIsaDisassemblyTabView::EnableScrollbarSignals);
    assert(isConnected);

    // Connect the disassembly view's disable scroll bar signal.
    isConnected = connect(this, &rgIsaDisassemblyView::DisableScrollbarSignals, pEntryView, &rgIsaDisassemblyTabView::DisableScrollbarSignals);
    assert(isConnected);

    // Connect the disassembly table's target GPU push button focus in signal.
    isConnected = connect(pEntryView, &rgIsaDisassemblyTabView::FocusTargetGpuPushButton, this, &rgIsaDisassemblyView::HandleFocusTargetGpuPushButton);
    assert(isConnected);

    // Connect the disassembly table's switch disassembly view size signal.
    isConnected = connect(pEntryView, &rgIsaDisassemblyTabView::SwitchDisassemblyContainerSize, this, &rgIsaDisassemblyView::SwitchDisassemblyContainerSize);
    assert(isConnected);

    // Connect the disassembly table's columns push button focus in signal.
    isConnected = connect(pEntryView, &rgIsaDisassemblyTabView::FocusColumnPushButton, this, &rgIsaDisassemblyView::HandleFocusColumnsPushButton);
    assert(isConnected);

    // Connect the disassembly table's cli output window focus in signal.
    isConnected = connect(pEntryView, &rgIsaDisassemblyTabView::FocusSourceWindow, this, &rgIsaDisassemblyView::FocusSourceWindow);
    assert(isConnected);

    // Connect the disassembly view's update current sub widget signal.
    isConnected = connect(this, &rgIsaDisassemblyView::UpdateCurrentSubWidget, pEntryView, &rgIsaDisassemblyTabView::UpdateCurrentSubWidget);
    assert(isConnected);
}

void rgIsaDisassemblyView::ConnectSignals()
{
    // Connect the column visibility selector arrow button.
    bool isConnected = connect(ui.columnVisibilityArrowPushButton, &QPushButton::clicked, this, &rgIsaDisassemblyView::HandleColumnVisibilityButtonClicked);
    assert(isConnected);

    // Connect the handler to show/hide the target GPU list when the arrow button is clicked.
    isConnected = connect(ui.targetGpuPushButton, &QPushButton::clicked, this, &rgIsaDisassemblyView::HandleTargetGpuArrowClicked);
    assert(isConnected);

    // Connect the handler to give focus to frame on view maximize button click.
    isConnected = connect(ui.viewMaximizeButton, &QPushButton::clicked, this, &rgIsaDisassemblyView::HandleDisassemblyTabViewClicked);
    assert(isConnected);

    // Connect the handler to give focus to frame on disassembly column list widget's gain of focus.
    isConnected = connect(m_pDisassemblyColumnsListWidget, &ListWidget::FocusInEvent, this, &rgIsaDisassemblyView::HandleListWidgetFocusInEvent);
    assert(isConnected);

    // Connect the handler to remove focus from frame on disassembly column list widget's loss of focus.
    isConnected = connect(m_pDisassemblyColumnsListWidget, &ListWidget::FocusOutEvent, this, &rgIsaDisassemblyView::HandleListWidgetFocusOutEvent);
    assert(isConnected);

    // Connect the handler to give focus to frame on target GPUs list widget's gain of focus.
    isConnected = connect(m_pTargetGpusListWidget, &ListWidget::FocusInEvent, this, &rgIsaDisassemblyView::HandleListWidgetFocusInEvent);
    assert(isConnected);

    // Connect the handler to give focus to frame on target GPUs list widget's loss of focus.
    isConnected = connect(m_pTargetGpusListWidget, &ListWidget::FocusOutEvent, this, &rgIsaDisassemblyView::HandleListWidgetFocusOutEvent);
    assert(isConnected);

    // Connect the handler to give focus to frame on columns push button click.
    isConnected = connect(ui.columnVisibilityArrowPushButton, &QPushButton::clicked, this, &rgIsaDisassemblyView::HandleDisassemblyTabViewClicked);
    assert(isConnected);

    // Connect the handler to give focus to frame on target GPUs push button click.
    isConnected = connect(ui.targetGpuPushButton, &QPushButton::clicked, this, &rgIsaDisassemblyView::HandleDisassemblyTabViewClicked);
    assert(isConnected);

    // Connect the handler to remove focus from frame on columns push button loss of focus.
    isConnected = connect(ui.columnVisibilityArrowPushButton, &ArrowIconWidget::FocusOutEvent, this, &rgIsaDisassemblyView::HandleListWidgetFocusOutEvent);
    assert(isConnected);

    // Connect the handler to give focus to frame on target GPUs push button loss of focus.
    isConnected = connect(ui.targetGpuPushButton, &ArrowIconWidget::FocusOutEvent, this, &rgIsaDisassemblyView::HandleListWidgetFocusOutEvent);
    assert(isConnected);

    // Select next GPU device action.
    m_pSelectNextGPUTarget = new QAction(this);
    m_pSelectNextGPUTarget->setShortcutContext(Qt::ApplicationShortcut);
    m_pSelectNextGPUTarget->setShortcut(QKeySequence(gs_DISASSEMBLY_VIEW_HOTKEY_GPU_SELECTION));
    addAction(m_pSelectNextGPUTarget);

    isConnected = connect(m_pSelectNextGPUTarget, &QAction::triggered, this, &rgIsaDisassemblyView::HandleSelectNextGPUTargetAction);
    assert(isConnected);
}

void rgIsaDisassemblyView::CreateColumnVisibilityControls()
{
    // Setup the list widget that opens when the user clicks the column visibility arrow.
    rgUtils::SetupComboList(this, m_pDisassemblyColumnsListWidget, ui.columnVisibilityArrowPushButton, m_pDisassemblyColumnsListEventFilter, false);
    m_pDisassemblyColumnsListWidget->setObjectName(STR_DISASSEMBLY_COLUMN_VISIBILITY_LIST);

    // Handle the open gpu list widget signal and,
    // the update current sub widget signal from the hide list widget event filter object.
    if (m_pDisassemblyColumnsListEventFilter != nullptr)
    {
        rgHideListWidgetEventFilter* pEventFilter = static_cast<rgHideListWidgetEventFilter*>(m_pDisassemblyColumnsListEventFilter);
        if (pEventFilter != nullptr)
        {
            bool isConnected = connect(pEventFilter, &rgHideListWidgetEventFilter::OpenGpuListWidget, this, &rgIsaDisassemblyView::HandleOpenGpuListWidget);
            assert(isConnected);

            isConnected = connect(pEventFilter, &rgHideListWidgetEventFilter::UpdateCurrentSubWidget, this, &rgIsaDisassemblyView::UpdateCurrentSubWidget);
            assert(isConnected);

            isConnected = connect(pEventFilter, &rgHideListWidgetEventFilter::FocusCliOutputWindow, this, &rgIsaDisassemblyView::FocusCliOutputWindow);
            assert(isConnected);
        }
    }

    // Update scale factor for widgets.
    QFont font = ui.columnVisibilityArrowPushButton->font();
    double scaleFactor = ScalingManager::Get().GetScaleFactor();
    font.setPointSize(s_PUSH_BUTTON_FONT_SIZE * scaleFactor);
    m_pDisassemblyColumnsListWidget->setStyleSheet(s_LIST_WIDGET_STYLE.arg(font.pointSize()));

    // Reset the current selection in the column visibility list.
    m_pDisassemblyColumnsListWidget->setCurrentRow(0);
}

void rgIsaDisassemblyView::HandleOpenColumnListWidget()
{
    if (m_pDisassemblyColumnsListWidget != nullptr)
    {
        ui.columnVisibilityArrowPushButton->clicked();
    }
}

void rgIsaDisassemblyView::CreateTargetGpuListControls()
{
    // Setup the list widget used to select the current target GPU.
    rgUtils::SetupComboList(this, m_pTargetGpusListWidget, ui.targetGpuPushButton, m_pTargetGpusListEventFilter, false);
    m_pTargetGpusListWidget->setObjectName(STR_DISASSEMBLY_TARGET_GPU_LIST);

    if (m_pTargetGpusListEventFilter != nullptr)
    {
        rgHideListWidgetEventFilter* pEventFilter = static_cast<rgHideListWidgetEventFilter*>(m_pTargetGpusListEventFilter);
        if (pEventFilter != nullptr)
        {
            bool isConnected = connect(pEventFilter, &rgHideListWidgetEventFilter::OpenColumnListWidget, this, &rgIsaDisassemblyView::HandleOpenColumnListWidget);
            assert(isConnected);

            isConnected = connect(pEventFilter, &rgHideListWidgetEventFilter::UpdateCurrentSubWidget, this, &rgIsaDisassemblyView::UpdateCurrentSubWidget);
            assert(isConnected);
        }
    }

    // Update scale factor for widgets.
    QFont font = ui.targetGpuPushButton->font();
    double scaleFactor = ScalingManager::Get().GetScaleFactor();
    font.setPointSize(s_PUSH_BUTTON_FONT_SIZE * scaleFactor);
    m_pTargetGpusListWidget->setStyleSheet(s_LIST_WIDGET_STYLE.arg(font.pointSize()));

    // Reset the current selection in the target GPU dropdown list.
    m_pTargetGpusListWidget->setCurrentRow(0);

    // Connect the signal used to handle a change in the selected target GPU.
    bool isConnected = connect(m_pTargetGpusListWidget, &QListWidget::currentRowChanged, this, &rgIsaDisassemblyView::HandleTargetGpuChanged);
    assert(isConnected);
}

void rgIsaDisassemblyView::HandleOpenGpuListWidget()
{
    if (m_pTargetGpusListWidget != nullptr)
    {
        ui.targetGpuPushButton->clicked();
    }
}

std::string rgIsaDisassemblyView::GetDisassemblyColumnName(rgIsaDisassemblyTableColumns column) const
{
    std::string result;

    static std::map<rgIsaDisassemblyTableColumns, std::string> kColumnNameMap =
    {
        { rgIsaDisassemblyTableColumns::Address,        STR_DISASSEMBLY_TABLE_COLUMN_ADDRESS },
        { rgIsaDisassemblyTableColumns::Opcode,         STR_DISASSEMBLY_TABLE_COLUMN_OPCODE },
        { rgIsaDisassemblyTableColumns::Operands,       STR_DISASSEMBLY_TABLE_COLUMN_OPERANDS },
        { rgIsaDisassemblyTableColumns::FunctionalUnit, STR_DISASSEMBLY_TABLE_COLUMN_FUNCTIONAL_UNIT },
        { rgIsaDisassemblyTableColumns::Cycles,         STR_DISASSEMBLY_TABLE_COLUMN_CYCLES },
        { rgIsaDisassemblyTableColumns::BinaryEncoding, STR_DISASSEMBLY_TABLE_COLUMN_BINARY_ENCODING },
    };

    auto columnNameIter = kColumnNameMap.find(column);
    if (columnNameIter != kColumnNameMap.end())
    {
        result = columnNameIter->second;
    }
    else
    {
        // The incoming column doesn't have a name string mapped to it.
        assert(false);
    }

    return result;
}

rgIsaDisassemblyTabView* rgIsaDisassemblyView::GetTargetGpuTabWidgetByTabName(const std::string& tabText) const
{
    rgIsaDisassemblyTabView* pResult = nullptr;

    // If a matching view exists return it.
    auto gpuTabIter = m_gpuTabViews.find(tabText);
    if (gpuTabIter != m_gpuTabViews.end())
    {
        pResult = gpuTabIter->second;
    }

    return pResult;
}

void rgIsaDisassemblyView::PopulateTargetGpuList(const rgBuildOutputsMap& buildOutput)
{
    // Get a mapping of the compute capability to architecture.
    std::map<std::string, std::string> computeCapabilityToArch;
    bool hasArchMapping = rgUtils::GetComputeCapabilityToArchMapping(computeCapabilityToArch);

    // Block signals to stop updates when each new GPU is added to the list.
    m_pTargetGpusListWidget->blockSignals(true);

    m_pTargetGpusListWidget->clear();

    for (auto targetGpuIter = buildOutput.rbegin(); targetGpuIter != buildOutput.rend(); ++targetGpuIter)
    {
        if (targetGpuIter->second != nullptr)
        {
            // Add each target GPU from the build outputs to the dropdown list.
            auto targetGpu = targetGpuIter->first;

            // Construct the presented name.
            std::stringstream presentedName;

            // If applicable, prepend the gfx notation (for example, "gfx802/Tonga" for "Tonga").
            std::string gfxNotation;
            bool hasGfxNotation = rgUtils::GetGfxNotation(targetGpu, gfxNotation);
            if (hasGfxNotation && !gfxNotation.empty())
            {
                presentedName << gfxNotation << "/";
            }
            presentedName << targetGpu;

            // If we have a mapping, let's construct a name that includes
            // the GPU architecture as well: <compute capability> (<architecture>).
            if (hasArchMapping)
            {
                auto iter = computeCapabilityToArch.find(targetGpu);
                if (iter != computeCapabilityToArch.end())
                {
                    presentedName << " (" << iter->second << ")";
                }
            }

            // Add the name to the list.
            m_pTargetGpusListWidget->addItem(presentedName.str().c_str());

            // Sort in descending order so that the newer targets are at the top.
            m_pTargetGpusListWidget->sortItems(Qt::SortOrder::DescendingOrder);
        }
    }

    // Switch to the first target GPU.
    HandleTargetGpuChanged(0);

    // Re-enable signals emitted from the target GPU list.
    m_pTargetGpusListWidget->blockSignals(false);
}

bool rgIsaDisassemblyView::PopulateDisassemblyEntries(const GpuToEntryVector& gpuToDisassemblyCsvEntries)
{
    bool ret = true;

    if (!gpuToDisassemblyCsvEntries.empty())
    {
        // Step through each GPU and insert a new rgIsaDisassemblyTabView into a new tab.
        for (auto gpuEntryIter = gpuToDisassemblyCsvEntries.begin(); gpuEntryIter != gpuToDisassemblyCsvEntries.end(); ++gpuEntryIter)
        {
            const std::string& gpuName = gpuEntryIter->first;
            const std::vector<rgEntryOutput>& gpuEntries = gpuEntryIter->second;

            // Does a tab already exist for the target GPU we're loading results for?
            rgIsaDisassemblyTabView* pEntryView = GetTargetGpuTabWidgetByTabName(gpuName);

            // Create a new tab for the target GPU, and add a new disassembly table viewer.
            if (pEntryView == nullptr)
            {
                // Create a new entry view for each unique GPU.
                pEntryView = new rgIsaDisassemblyTabView();

                // Connect signals for the new tab.
                ConnectDisassemblyTabViewSignals(pEntryView);

                // Add the new entry view to the map.
                m_gpuTabViews[gpuName] = pEntryView;

                // Add the new disassembly table as a tab page in the array of GPU results.
                ui.disassemblyTableHostWidget->addWidget(pEntryView);
            }

            // Send the CSV file paths to the GPU-specific entry viewer.
            bool isTablePopulated = pEntryView->PopulateEntries(gpuEntries);

            // Verify that the table was populated correctly.
            assert(isTablePopulated);
            if (!isTablePopulated)
            {
                ret = false;
            }
        }
    }

    return ret;
}

bool rgIsaDisassemblyView::PopulateResourceUsageEntries(const GpuToEntryVector& gpuToResourceUsageCsvEntries)
{
    bool isLoadFailed = false;

    if (!gpuToResourceUsageCsvEntries.empty())
    {
        // Step through each GPU and insert a new rgResourceView underneath the disassembly table.
        for (auto gpuEntryIter = gpuToResourceUsageCsvEntries.begin(); gpuEntryIter != gpuToResourceUsageCsvEntries.end(); ++gpuEntryIter)
        {
            const std::string& gpuName = gpuEntryIter->first;
            const std::vector<rgEntryOutput>& gpuEntries = gpuEntryIter->second;

            // Create a resource usage disassembly table for each entry, and add it to the layout.
            // Only a single entry point table will be visible at a time, and the user can switch between the current entry.
            for (const rgEntryOutput& entry : gpuEntries)
            {
                OutputFileTypeFinder outputFileTypeSearcher(rgCliOutputFileType::HwResourceUsageFile);
                auto csvFileIter = std::find_if(entry.m_outputs.begin(), entry.m_outputs.end(), outputFileTypeSearcher);
                if (csvFileIter != entry.m_outputs.end())
                {
                    // Create a CSV parser to read the resource usage file.
                    rgResourceUsageCsvFileParser resourceUsageFileParser(csvFileIter->m_filePath);

                    // Attempt to parse the file.
                    std::string parseErrorString;
                    bool parsedSuccessfully = resourceUsageFileParser.Parse(parseErrorString);
                    if (parsedSuccessfully)
                    {
                        // Extract the parsed data, and populate the resource usage view.
                        const rgResourceUsageData& resourceUsageData = resourceUsageFileParser.GetData();
                        rgResourceUsageView* pResourceUsageView = new rgResourceUsageView();
                        pResourceUsageView->PopulateView(resourceUsageData);
                        m_resourceUsageText = pResourceUsageView->GetResourceUsageText();
                        m_resourceUsageFont = pResourceUsageView->GetResourceUsageFont();

                        // Register the resource usage view with the scaling manager.
                        ScalingManager::Get().RegisterObject(pResourceUsageView);

                        // Connect resource usage view signals.
                        ConnectResourceUsageViewSignals(pResourceUsageView);

                        // Add the new resource usage view to the host widget.
                        ui.resourceUsageHostStackedWidget->addWidget(pResourceUsageView);
                        ui.resourceUsageHostStackedWidget->setContentsMargins(0, 10, 0, 0);

                        // Get a reference to the entry point views associated with the parsed device.
                        InputToEntrypointViews& inputFileToEntrypointMap = m_gpuResourceUsageViews[gpuName];

                        // Get a reference to the resource views map for the source file.
                        EntrypointToResourcesView& entrypointMap = inputFileToEntrypointMap[entry.m_inputFilePath];

                        // Associate the entrypoint's name with the new rgResourceView.
                        entrypointMap[entry.m_entrypointName] = pResourceUsageView;
                    }
                    else
                    {
                        isLoadFailed = true;
                    }
                }
            }
        }
    }

    return !isLoadFailed;
}

void rgIsaDisassemblyView::ConnectResourceUsageViewSignals(rgResourceUsageView * pResourceUsageView)
{
    // Connect to the resource usage view's mouse press event.
    bool isConnected = connect(pResourceUsageView, &rgResourceUsageView::ResourceUsageViewClickedSignal, this, &rgIsaDisassemblyView::HandleDisassemblyTabViewClicked);
    assert(isConnected);

    // Connect to the resource usage view's focus out event.
    isConnected = connect(pResourceUsageView, &rgResourceUsageView::ResourceUsageViewFocusOutEventSignal, this, &rgIsaDisassemblyView::HandleResourceUsageViewFocusOutEvent);
    assert(isConnected);
}

void rgIsaDisassemblyView::PopulateColumnVisibilityList()
{
    // Set up the function pointer responsible for handling column visibility filter state change.
    using std::placeholders::_1;
    std::function<void(bool)> slotFunctionPointer = std::bind(&rgIsaDisassemblyView::HandleColumnVisibilityFilterStateChanged, this, _1);

    // Remove the existing items first
    ClearListWidget(m_pDisassemblyColumnsListWidget);

    // Add the "All" entry
    ListWidget::AddListWidgetCheckboxItem(STR_DISASSEMBLY_TABLE_COLUMN_ALL, m_pDisassemblyColumnsListWidget, slotFunctionPointer, this, STR_DISASSEMBLY_COLUMN_VISIBILITY_LIST, STR_DISASSEMBLY_COLUMN_LIST_ITEM_ALL_CHECKBOX);

    // Loop through each column enum member.
    int startColumn = rgIsaDisassemblyTableModel::GetTableColumnIndex(rgIsaDisassemblyTableColumns::Address);
    int endColumn   = rgIsaDisassemblyTableModel::GetTableColumnIndex(rgIsaDisassemblyTableColumns::Count);

    // Add an item for each column in the table.
    for (int columnIndex = startColumn; columnIndex < endColumn; ++columnIndex)
    {
        // Add an item for each possible column in the table.
        std::string columnName = GetDisassemblyColumnName(static_cast<rgIsaDisassemblyTableColumns>(columnIndex));
        ListWidget::AddListWidgetCheckboxItem(columnName.c_str(), m_pDisassemblyColumnsListWidget, slotFunctionPointer, this, STR_DISASSEMBLY_COLUMN_VISIBILITY_LIST, STR_DISASSEMBLY_COLUMN_LIST_ITEM_CHECKBOX);
    }

    // Populate the check box items by reading the global settings.
    rgConfigManager& configManager = rgConfigManager::Instance();
    std::shared_ptr<rgGlobalSettings> pGlobalSettings = configManager.GetGlobalConfig();
    ListWidget::SetColumnVisibilityCheckboxes(m_pDisassemblyColumnsListWidget, pGlobalSettings->m_visibleDisassemblyViewColumns);

    // Update the "All" checkbox text color to grey or black.
    UpdateAllCheckBoxText();

    // Set list widget's check box's focus proxy to be the frame.
    SetCheckBoxFocusProxies(m_pDisassemblyColumnsListWidget);
}

void rgIsaDisassemblyView::SetCheckBoxFocusProxies(const ListWidget* pListWidget) const
{
    for (int i = 0; i < pListWidget->count(); i++)
    {
        QListWidgetItem* pItem = pListWidget->item(i);
        assert(pItem != nullptr);

        QCheckBox* pCheckBox = qobject_cast<QCheckBox*>(pListWidget->itemWidget(pItem));
        assert(pCheckBox != nullptr);

        pCheckBox->setFocusProxy(ui.frame);
    }
}

void rgIsaDisassemblyView::UpdateAllCheckBoxText()
{
    bool areAllItemsChecked = true;

    // Check to see if all of the boxes are checked.
    for (int i = 1; i < m_pDisassemblyColumnsListWidget->count(); i++)
    {
        QListWidgetItem* pItem = m_pDisassemblyColumnsListWidget->item(i);
        assert(pItem != nullptr);

        QCheckBox* pCheckBox = qobject_cast<QCheckBox*>(m_pDisassemblyColumnsListWidget->itemWidget(pItem));
        assert(pCheckBox != nullptr);

        if (pCheckBox->checkState() == Qt::CheckState::Unchecked)
        {
            areAllItemsChecked = false;
            break;
        }
    }

    // If all boxes are checked, update the text color of the All check box.
    QListWidgetItem* pItem = m_pDisassemblyColumnsListWidget->item(0);
    if (pItem != nullptr)
    {
        QCheckBox* pCheckBox = qobject_cast<QCheckBox*>(m_pDisassemblyColumnsListWidget->itemWidget(pItem));
        if (pCheckBox != nullptr)
        {
            if (areAllItemsChecked)
            {
                pCheckBox->setStyleSheet("QCheckBox#ListWidgetAllCheckBox {color: grey;}");
            }
            else
            {
                pCheckBox->setStyleSheet("QCheckBox#ListWidgetAllCheckBox {color: black;}");
            }
        }
    }
}

void rgIsaDisassemblyView::DestroyDisassemblyViewsForFile(const std::string& inputFilePath)
{
    // Keep a list of tabs that should be destroyed after removing the input file.
    std::vector<std::string> gpuTabsToRemove;

    // Step through each GPU tab and try to remove the entries associated with the given input file.
    auto startTab = m_gpuTabViews.begin();
    auto endTab = m_gpuTabViews.end();
    for (auto tabIter = startTab; tabIter != endTab; ++tabIter)
    {
        // Search the tab for entries to remove.
        rgIsaDisassemblyTabView* pGpuTab = tabIter->second;
        assert(pGpuTab != nullptr);
        if (pGpuTab != nullptr)
        {
            // Attempt to remove entries associated with the input file from each GPU tab.
            pGpuTab->RemoveInputFileEntries(inputFilePath);

            // Does the GPU tab have any tables left in it? If not, destroy the tab too.
            int numTablesInTab = pGpuTab->GetTableCount();
            if (numTablesInTab == 0)
            {
                const std::string& gpuName = tabIter->first;
                gpuTabsToRemove.push_back(gpuName);
            }
        }
    }

    // Destroy all tabs that were marked for destruction.
    for (auto pGpuTab : gpuTabsToRemove)
    {
        auto tabIter = m_gpuTabViews.find(pGpuTab);
        if (tabIter != m_gpuTabViews.end())
        {
            // Destroy the GPU tab.
            rgIsaDisassemblyTabView* pGpuTabView = tabIter->second;

            // Remove the GPU tab from the view.
            ui.disassemblyTableHostWidget->removeWidget(pGpuTabView);

            // Remove the GPU from the view.
            m_gpuTabViews.erase(tabIter);
        }
    }
}

void rgIsaDisassemblyView::DestroyResourceUsageViewsForFile(const std::string& inputFilePath)
{
    // Destroy resource views for all GPUs.
    for (auto gpuIter = m_gpuResourceUsageViews.begin(); gpuIter != m_gpuResourceUsageViews.end(); ++gpuIter)
    {
        // Find all resource views related to the given input file.
        InputToEntrypointViews& inputFileToViewsIter = gpuIter->second;
        auto entrypointResourceUsageviewsIter = inputFileToViewsIter.find(inputFilePath);
        if (entrypointResourceUsageviewsIter != inputFileToViewsIter.end())
        {
            // Step through each entrypoint's resource usage view, and destroy it.
            EntrypointToResourcesView& entrypointResourceViews = entrypointResourceUsageviewsIter->second;
            for (auto entrypointIter = entrypointResourceViews.begin(); entrypointIter != entrypointResourceViews.end(); ++entrypointIter)
            {
                // Destroy the resource usage view.
                rgResourceUsageView* pResourceUsageView = entrypointIter->second;

                // Remove the resource usage widget from the view.
                ui.resourceUsageHostStackedWidget->removeWidget(pResourceUsageView);
            }

            inputFileToViewsIter.erase(entrypointResourceUsageviewsIter);
        }
    }
}

void rgIsaDisassemblyView::SetCurrentResourceUsageView(rgResourceUsageView* pResourceUsageView)
{
    // Set the current widget in the stack.
    ui.resourceUsageHostStackedWidget->setCurrentWidget(pResourceUsageView);

    // Use the given resource usage view as the focus proxy for this view.
    setFocusProxy(pResourceUsageView);
}

void rgIsaDisassemblyView::SetCurrentTargetGpuTabView(rgIsaDisassemblyTabView* pTabView)
{
    // Set the current widget in the stack.
    ui.disassemblyTableHostWidget->setCurrentWidget(pTabView);

    // Store the current target GPU tab being viewed.
    m_pCurrentTabView = pTabView;

    // Use the current tab view as the focus proxy for this view.
    setFocusProxy(pTabView);
}

void rgIsaDisassemblyView::SetCursor()
{
    ui.columnVisibilityArrowPushButton->setCursor(Qt::PointingHandCursor);
    ui.targetGpuPushButton->setCursor(Qt::PointingHandCursor);
    ui.viewMaximizeButton->setCursor(Qt::PointingHandCursor);
}

void rgIsaDisassemblyView::SetTargetGpu(const std::string& targetGpu)
{
    static const int g_ARROW_WIDGET_EXTRA_WIDTH = 30;

    // Update the button text.
    ui.targetGpuPushButton->setText(targetGpu.c_str());

    // Measure the width of the Target GPU text, and add extra space to account for the width of the arrow.
    int scaledArrowWidth = static_cast<int>(g_ARROW_WIDGET_EXTRA_WIDTH * ScalingManager::Get().GetScaleFactor());
    int textWidth = QtCommon::QtUtil::GetTextWidth(ui.targetGpuPushButton->font(), targetGpu.c_str());
    ui.targetGpuPushButton->setMinimumWidth(scaledArrowWidth + textWidth);
}

void rgIsaDisassemblyView::SetFontSizes()
{
    // Set column visibility push button font.
    ArrowIconWidget* pArrowWidget = dynamic_cast<ArrowIconWidget*>(ui.columnVisibilityArrowPushButton);
    if (pArrowWidget != nullptr)
    {
        pArrowWidget->SetFontSize(s_PUSH_BUTTON_FONT_SIZE);
    }

    // Set ISA list push button font.
    pArrowWidget = dynamic_cast<ArrowIconWidget*>(ui.targetGpuPushButton);
    if (pArrowWidget != nullptr)
    {
        pArrowWidget->SetFontSize(s_PUSH_BUTTON_FONT_SIZE);
    }
}

bool rgIsaDisassemblyView::IsLineCorrelatedInEntry(const std::string& inputFilePath, const std::string& targetGpu, const std::string& entrypoint, int srcLine) const
{
    bool  ret = false;

    rgIsaDisassemblyTabView* pTargetGpuTab = GetTargetGpuTabWidgetByTabName(targetGpu.c_str());

    assert(pTargetGpuTab != nullptr);
    if (pTargetGpuTab != nullptr)
    {
        ret = pTargetGpuTab->IsSourceLineCorrelatedForEntry(inputFilePath, entrypoint, srcLine);
    }

    return ret;
}

void rgIsaDisassemblyView::HandleDisassemblyTabViewClicked()
{
    // Emit a signal to indicate that disassembly view was clicked.
    emit DisassemblyViewClicked();

    // Highlight the frame in the correct API color (give it focus).
    SetBorderStylesheet();

    // Remove the button focus in the file menu.
    emit RemoveFileMenuButtonFocus();
}

void rgIsaDisassemblyView::HandleDisassemblyTabViewLostFocus()
{
    // Highlight the frame in black.
    ui.frame->setStyleSheet(STR_DISASSEMBLY_FRAME_BORDER_BLACK_STYLESHEET);

    // Remove the button focus in the file menu.
    emit RemoveFileMenuButtonFocus();
}

void rgIsaDisassemblyView::HandleResourceUsageViewFocusOutEvent()
{
    // Highlight the frame in black.
    ui.frame->setStyleSheet(STR_DISASSEMBLY_FRAME_BORDER_BLACK_STYLESHEET);

    // Remove the button focus in the file menu.
    emit RemoveFileMenuButtonFocus();
}

void rgIsaDisassemblyView::HandleTitlebarClickedEvent(QMouseEvent* pEvent)
{
    // Emit a signal to indicate that disassembly view was clicked.
    emit DisassemblyViewClicked();

    // Highlight the frame in the correct API color (give it focus).
    SetBorderStylesheet();

    // Remove the button focus in the file menu.
    emit RemoveFileMenuButtonFocus();
}

void rgIsaDisassemblyView::HandleListWidgetFocusInEvent()
{
    // Emit a signal to indicate that disassembly view was clicked.
    emit DisassemblyViewClicked();

    // Highlight the frame in the correct API color (give it focus).
    SetBorderStylesheet();

    // Remove the button focus in the file menu.
    emit RemoveFileMenuButtonFocus();
}

void rgIsaDisassemblyView::HandleListWidgetFocusOutEvent()
{
    ui.frame->setStyleSheet(STR_DISASSEMBLY_FRAME_BORDER_BLACK_STYLESHEET);
}

void rgIsaDisassemblyView::HandleFocusOutEvent()
{
    ui.frame->setStyleSheet(STR_DISASSEMBLY_FRAME_BORDER_BLACK_STYLESHEET);

    // Remove the button focus in the file menu.
    emit RemoveFileMenuButtonFocus();
}

void rgIsaDisassemblyView::HandleFocusTargetGpuPushButton()
{
    ui.targetGpuPushButton->clicked(false);
}

void rgIsaDisassemblyView::HandleFocusColumnsPushButton()
{
    ui.columnVisibilityArrowPushButton->clicked(false);
}

void rgIsaDisassemblyView::ConnectTitleBarDoubleClick(const rgViewContainer* pDisassemblyViewContainer)
{
    assert(pDisassemblyViewContainer != nullptr);
    if (pDisassemblyViewContainer != nullptr)
    {
        bool isConnected = connect(ui.viewTitlebar, &rgIsaDisassemblyViewTitlebar::ViewTitleBarDoubleClickedSignal, pDisassemblyViewContainer, &rgViewContainer::MaximizeButtonClicked);
        assert(isConnected);
    }
}

bool rgIsaDisassemblyView::ReplaceInputFilePath(const std::string& oldFilePath, const std::string& newFilePath)
{
    bool result = true;

    // Replace the file path in all disassembly tab views for all devices.
    for (auto& gpuAndTabView : m_gpuTabViews)
    {
        rgIsaDisassemblyTabView* pTabView = gpuAndTabView.second;
        if (!pTabView->ReplaceInputFilePath(oldFilePath, newFilePath))
        {
            result = false;
            break;
        }
    }

    // Replace the file path in the resource usage map.
    if (result)
    {
        for (auto& gpuAndResourceUsage : m_gpuResourceUsageViews)
        {
            auto& fileAndResourceUsage = gpuAndResourceUsage.second;
            auto it = fileAndResourceUsage.find(oldFilePath);
            if ((result = (it != fileAndResourceUsage.end())) == true)
            {
                std::map<std::string, rgResourceUsageView*> resUsageView = it->second;
                fileAndResourceUsage.erase(oldFilePath);
                fileAndResourceUsage[newFilePath] = resUsageView;
            }
        }
    }

    return result;
}

void rgIsaDisassemblyView::HandleSelectNextGPUTargetAction()
{
    int currentRow = m_pTargetGpusListWidget->currentRow();

    if (currentRow < m_pTargetGpusListWidget->count() - 1)
    {
        currentRow++;
    }
    else
    {
        currentRow = 0;
    }
    m_pTargetGpusListWidget->setCurrentRow(currentRow);
}
