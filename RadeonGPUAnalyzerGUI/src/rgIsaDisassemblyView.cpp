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
#include <QCheckBox>
#include <QListWidgetItem>

// Infra.
#include <QtCommon/CustomWidgets/ArrowIconWidget.h>
#include <QtCommon/CustomWidgets/ListWidget.h>
#include <QtCommon/Scaling/ScalingManager.h>
#include <QtCommon/Util/CommonDefinitions.h>
#include <QtCommon/Util/QtUtil.h>

// Local.
#include <RadeonGPUAnalyzerGUI/include/qt/rgHideListWidgetEventFilter.h>
#include <RadeonGPUAnalyzerGUI/include/qt/rgIsaDisassemblyTableModel.h>
#include <RadeonGPUAnalyzerGUI/include/qt/rgIsaDisassemblyTableView.h>
#include <RadeonGPUAnalyzerGUI/include/qt/rgIsaDisassemblyTabView.h>
#include <RadeonGPUAnalyzerGUI/include/qt/rgIsaDisassemblyView.h>
#include <RadeonGPUAnalyzerGUI/include/qt/rgResourceUsageView.h>
#include <RadeonGPUAnalyzerGUI/include/qt/rgViewContainer.h>
#include <RadeonGPUAnalyzerGUI/include/rgDataTypes.h>
#include <RadeonGPUAnalyzerGUI/include/rgDefinitions.h>
#include <RadeonGPUAnalyzerGUI/include/rgResourceUsageCsvFileParser.h>
#include <RadeonGPUAnalyzerGUI/include/rgStringConstants.h>
#include <RadeonGPUAnalyzerGUI/include/rgUtils.h>

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

bool rgIsaDisassemblyView::PopulateDisassemblyView(const std::vector<rgSourceFileInfo>& sourceFiles, const rgBuildOutputsMap& buildOutput)
{
    bool isProblemFound = false;

    // Iterate through each target GPU's output.
    for (auto gpuOutputsIter = buildOutput.begin(); gpuOutputsIter != buildOutput.end(); ++gpuOutputsIter)
    {
        const std::string& familyName = gpuOutputsIter->first;
        std::shared_ptr<rgCliBuildOutput> pGpuBuildOutput = gpuOutputsIter->second;
        bool isValidOutput = pGpuBuildOutput != nullptr;
        assert(isValidOutput);
        if (isValidOutput)
        {
            // Step through the outputs map, and load the disassembly data for each input file.
            for (auto outputIter = pGpuBuildOutput->m_perFileOutput.begin(); outputIter != pGpuBuildOutput->m_perFileOutput.end(); ++outputIter)
            {
                const std::string& sourceFilePath = outputIter->first;

                // Only load build outputs for files in the given list of source files.
                rgSourceFilePathSearcher sourceFileSearcher(sourceFilePath);
                auto sourceFileIter = std::find_if(sourceFiles.begin(), sourceFiles.end(), sourceFileSearcher);
                if (sourceFileIter != sourceFiles.end())
                {
                    // Get the list of outputs for the input file.
                    rgFileOutputs& fileOutputs = outputIter->second;
                    const std::vector<rgEntryOutput>& fileEntryOutputs = fileOutputs.m_outputs;

                    // Transform the disassembled entries into a map of GPU -> Entries.
                    // This will make populating the UI much simpler.
                    std::map<std::string, std::vector<rgEntryOutput>> gpuToDisassemblyCsvEntries;
                    std::map<std::string, std::vector<rgEntryOutput>> gpuToResourceUsageCsvEntries;
                    for (const rgEntryOutput& entry : fileOutputs.m_outputs)
                    {
                        for (const rgOutputItem& outputs : entry.m_outputs)
                        {
                            if (outputs.m_fileType == IsaDisassemblyCsv)
                            {
                                std::vector<rgEntryOutput>& disassemblyCsvFilePaths = gpuToDisassemblyCsvEntries[familyName];
                                disassemblyCsvFilePaths.push_back(entry);
                            }
                            else if (outputs.m_fileType == HwResourceUsageFile)
                            {
                                std::vector<rgEntryOutput>& entryCsvFilePaths = gpuToResourceUsageCsvEntries[familyName];
                                entryCsvFilePaths.push_back(entry);
                            }
                        }
                    }


                    // Load the disassembly CSV entries from the build output.
                    bool isDisassemblyDataLoaded = PopulateDisassemblyEntries(gpuToDisassemblyCsvEntries);
                    assert(isDisassemblyDataLoaded);
                    if (!isDisassemblyDataLoaded)
                    {
                        isProblemFound = true;
                    }

                    // Load the resource usage CSV entries from the build output.
                    bool isResourceUsageDataLoaded = PopulateResourceUsageEntries(gpuToResourceUsageCsvEntries);
                    assert(isResourceUsageDataLoaded);
                    if (!isResourceUsageDataLoaded)
                    {
                        isProblemFound = true;
                    }
                }
            }
        }
    }

    // If the disassembly results loaded correctly, add the target GPU to the dropdown.
    if (!isProblemFound)
    {
        // Populate the target GPU dropdown list with the targets from the build output.
        PopulateTargetGpuList(buildOutput);
    }

    return !isProblemFound;
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
        pTargetTabView->SwitchToEntryPoint(selectedEntrypointName);
    }

    // Get a reference to the map of input file path to the entrypoint names map.
    const auto& targetGpuResourceUsageViewsIter = m_gpuResourceUsageViews.find(targetGpu);
    assert(targetGpuResourceUsageViewsIter != m_gpuResourceUsageViews.end());
    if (targetGpuResourceUsageViewsIter != m_gpuResourceUsageViews.end())
    {
        InputToEntrypointViews& inputFileToEntrypointMap = targetGpuResourceUsageViewsIter->second;

        // Use the input file path to get a reference to a map of entrypoint names to resource usage views.
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
    else
    {
        std::string filenameOnly;
        bool isOk = rgUtils::ExtractFileName(inputFilePath, filenameOnly);
        assert(isOk);
        if (isOk)
        {
            std::stringstream errorStream;
            errorStream << STR_ERR_CANNOT_LOAD_RESOURCE_USAGE_CSV_FILE;
            errorStream << filenameOnly;
            rgUtils::ShowErrorMessageBox(errorStream.str().c_str());
        }
    }
}

void rgIsaDisassemblyView::HandleColumnVisibilityArrowClicked(bool /*clicked*/)
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

    // Make sure that the user didn't uncheck the only checked box,
    // nor did they uncheck the "All" button, since this will uncheck
    // every single check box, which is something we do not want.
    if (checked || QtCommon::QtUtil::VerifyOneCheckboxChecked(pGlobalSettings->m_visibleDisassemblyViewColumns, firstColumn, lastColumn))
    {
        // Process the selected text accordingly
        if (text.compare(STR_DISASSEMBLY_TABLE_COLUMN_ALL) == 0 && (checked == true))
        {
            // Step through each column and set to visible.
            for (int columnIndex = firstColumn; columnIndex < lastColumn; ++columnIndex)
            {
                pGlobalSettings->m_visibleDisassemblyViewColumns[columnIndex] = checked;
                configManager.SaveGlobalConfigFile();
            }

            // Update the state of the dropdown checkboxes based on the visibility in the global settings.
            ListWidget::SetColumnVisibilityCheckboxes(m_pDisassemblyColumnsListWidget, pGlobalSettings->m_visibleDisassemblyViewColumns);
        }
        else
        {
            // Step through each column and set the visibility if the user changed the check state.
            for (int columnIndex = firstColumn; columnIndex < lastColumn; ++columnIndex)
            {
                rgIsaDisassemblyTableColumns column = static_cast<rgIsaDisassemblyTableColumns>(columnIndex);
                std::string columnName = GetDisassemblyColumnName(column);
                if (text.compare(columnName.c_str()) == 0)
                {
                    pGlobalSettings->m_visibleDisassemblyViewColumns[columnIndex] = checked;
                    configManager.SaveGlobalConfigFile();
                }
            }
        }
    }
    else
    {
        // Uncheck the check box since this is the only checked check box.
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

    // See if the "All" box needs checking/unchecking
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
        // The selected GPU hasn't been changed yet- we're only altering the visibility of the GPU
        // dropdown in this handler. Block signals from being emitted from the dropdown while
        // setting the focus. The "Target GPU Changed" handler will only be invoked when the user
        // changes the selected row in the GPU dropdown list.
        QSignalBlocker selectedGpuChangedBlocker(m_pTargetGpusListWidget);

        // Compute where to place the combo box relative to where the arrow button is.
        QWidget* pWidget = ui.targetGpuPushButton;
        m_pTargetGpusListWidget->show();
        m_pTargetGpusListWidget->setFocus();
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

                // Emit a signal with the name of the target GPU to switch to.
                emit SelectedTargetGpuChanged(newTargetGpu);
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
}

void rgIsaDisassemblyView::ConnectSignals()
{
    // Connect the column visibility selector arrow button.
    bool isConnected = connect(ui.columnVisibilityArrowPushButton, &QPushButton::clicked, this, &rgIsaDisassemblyView::HandleColumnVisibilityArrowClicked);
    assert(isConnected);

    // Connect the handler to show/hide the target GPU list when the arrow button is clicked.
    isConnected = connect(ui.targetGpuPushButton, &QPushButton::clicked, this, &rgIsaDisassemblyView::HandleTargetGpuArrowClicked);
    assert(isConnected);
}

void rgIsaDisassemblyView::CreateColumnVisibilityControls()
{
    // Setup the list widget that opens when the user clicks the column visibility arrow.
    rgUtils::SetupComboList(this, m_pDisassemblyColumnsListWidget, ui.columnVisibilityArrowPushButton, m_pDisassemblyColumnsListEventFilter, false);
    m_pDisassemblyColumnsListWidget->setObjectName(STR_DISASSEMBLY_COLUMN_VISIBILITY_LIST);

    // Update scale factor for widgets.
    QFont font = ui.columnVisibilityArrowPushButton->font();
    double scaleFactor = ScalingManager::Get().GetScaleFactor();
    font.setPointSize(s_PUSH_BUTTON_FONT_SIZE * scaleFactor);
    m_pDisassemblyColumnsListWidget->setStyleSheet(s_LIST_WIDGET_STYLE.arg(font.pointSize()));

    // Reset the current selection in the column visibility list.
    m_pDisassemblyColumnsListWidget->setCurrentRow(0);
}

void rgIsaDisassemblyView::CreateTargetGpuListControls()
{
    // Setup the list widget used to select the current target GPU.
    rgUtils::SetupComboList(this, m_pTargetGpusListWidget, ui.targetGpuPushButton, m_pTargetGpusListEventFilter, false);
    m_pTargetGpusListWidget->setObjectName(STR_DISASSEMBLY_TARGET_GPU_LIST);

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
    // Block signals to stop updates when each new GPU is added to the list.
    m_pTargetGpusListWidget->blockSignals(true);

    m_pTargetGpusListWidget->clear();

    for (auto targetGpuIter = buildOutput.begin(); targetGpuIter != buildOutput.end(); ++targetGpuIter)
    {
        if (targetGpuIter->second != nullptr)
        {
            // Add each target GPU from the build outputs to the dropdown list.
            auto targetGpu = targetGpuIter->first;
            m_pTargetGpusListWidget->addItem(targetGpu.c_str());
        }
    }

    // Switch to the last target GPU, which is the most recently released.
    HandleTargetGpuChanged(static_cast<int>(buildOutput.size()) - 1);

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
            // Only a single entrypoint table will be visible at a time, and the user can switch between the current entry.
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

                        // Add the new resource usage view to the host widget.
                        ui.resourceUsageHostStackedWidget->addWidget(pResourceUsageView);

                        // Get a reference to the entrypoint views associated with the parsed device.
                        InputToEntrypointViews& inputFileToEntrypointMap = m_gpuResourceUsageViews[gpuName];

                        // Get a reference to the resource views map for the source file.
                        EntrypointToResourcesView& entrypointMap = inputFileToEntrypointMap[entry.m_inputFilePath];

                        // Associate the entrypoint's name with the new rgResourceView.
                        entrypointMap[entry.m_kernelName] = pResourceUsageView;
                    }
                    else
                    {
                        // Display an error explaining why parsing failed.
                        rgUtils::ShowErrorMessageBox(parseErrorString.c_str());
                        isLoadFailed = true;
                    }
                }
            }
        }
    }

    return !isLoadFailed;
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

bool rgIsaDisassemblyView::IsLineCorrelatedInEntry(const std::string& targetGpu, const std::string& entrypoint, int srcLine) const
{
    bool  ret = false;

    rgIsaDisassemblyTabView* pTargetGpuTab = GetTargetGpuTabWidgetByTabName(targetGpu.c_str());

    assert(pTargetGpuTab != nullptr);
    if (pTargetGpuTab != nullptr)
    {
        ret = pTargetGpuTab->IsSourceLineCorrelatedForEntry(entrypoint, srcLine);

    }

    return ret;
}