// C++.
#include <cassert>

// Qt.
#include <QMimeData>
#include <QDragEnterEvent>

// Infra.
#include <QtCommon/Util/QtUtil.h>
#include <QtCommon/Scaling/ScalingManager.h>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgPipelineStateModel.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgPipelineStateView.h>
#include <RadeonGPUAnalyzerGUI/Include/rgFactoryVulkan.h>
#include <RadeonGPUAnalyzerGUI/Include/rgPipelineStateSearcher.h>
#include <RadeonGPUAnalyzerGUI/Include/rgStringConstants.h>

rgPipelineStateView::rgPipelineStateView(QWidget* pParent) :
    rgSettingsView(pParent)
{
    // Initialize the generated view object.
    ui.setupUi(this);

    // Block recursively repolishing all child widgets within the PSO tree.
    ui.settingsTree->setProperty(gs_IS_REPOLISHING_BLOCKED, true);

    // Connect interface signals.
    ConnectSignals();

    // Set button cursor to pointing hand cursor.
    ui.saveButton->setCursor(Qt::PointingHandCursor);
    ui.loadButton->setCursor(Qt::PointingHandCursor);

    // Enable drag and drop.
    setAcceptDrops(true);

    // Set focus policy.
    setFocusPolicy(Qt::FocusPolicy::StrongFocus);
}

void rgPipelineStateView::ScaleSettingsTree()
{
    // Unregister/Register this object with the scaling manager.
    ScalingManager::Get().UnregisterObject(this);
    ScalingManager::Get().RegisterObject(this);
}

void rgPipelineStateView::resizeEvent(QResizeEvent* pEvent)
{
    QWidget::resizeEvent(pEvent);

    emit EditorResized();
}

void rgPipelineStateView::hideEvent(QHideEvent* pEvent)
{
    QWidget::hideEvent(pEvent);

    emit EditorHidden();
}

rgPipelineStateSearcher* rgPipelineStateView::GetSearcher() const
{
    return m_pPipelineStateSearcher;
}

bool rgPipelineStateView::GetSelectedText(std::string& selectedTextString) const
{
    bool ret = false;

    // Is there a current selection in the table?
    const rgPipelineStateTree::CurrentSelection& selectedRow = ui.settingsTree->GetCurrentSelection();
    if (selectedRow.m_pSelectedRow != nullptr)
    {
        int columnIndex = selectedRow.m_focusedColumn;

        if (columnIndex != -1)
        {
            // Return the data stored in the first selected model index.
            QVariant selectedData = selectedRow.m_pSelectedRow->Data(columnIndex);
            selectedTextString = selectedData.toString().toStdString();

            // If the user selected the left column containing member names, treat it differently.
            // In this case, we'll trim off the leading array index from the output string.
            if (selectedRow.m_pSelectedRow->GetType() == rgEditorDataType::ArrayElement && columnIndex == static_cast<int>(rgRowData::RowDataMemberName))
            {

                // Find the first space in the string- the array index number directly precedes it.
                size_t spaceOffset = selectedTextString.find_first_of(' ');
                if (spaceOffset != std::string::npos)
                {
                    selectedTextString = selectedTextString.substr(spaceOffset + 1);
                }
            }

            // Return true since data was extracted from the selected index.
            ret = true;
        }
    }

    return ret;
}

void rgPipelineStateView::InitializeModel(rgPipelineStateModel* pPipelineStateModel)
{
    m_pipelineType = pPipelineStateModel->GetPipelineType();

    // Bind the provided pipeline state model to the settings tree view.
    assert(pPipelineStateModel != nullptr);
    if (pPipelineStateModel != nullptr)
    {
        // Set the model root element into the tree.
        rgEditorElement* pModelRootItem = pPipelineStateModel->GetRootElement();
        ui.settingsTree->SetRootItem(pModelRootItem);

        // Create the pipeline state tree searcher instance.
        m_pPipelineStateSearcher = new rgPipelineStateSearcher();

        // Attach a pipeline searcher instance to be used by the find widget.
        assert(m_pPipelineStateSearcher != nullptr);
        if (m_pPipelineStateSearcher != nullptr)
        {
            m_pPipelineStateSearcher->SetTargetModel(pPipelineStateModel);
            m_pPipelineStateSearcher->SetTargetView(ui.settingsTree);
        }

        // Connect the expand node handler.
        bool isConnected = connect(pPipelineStateModel, &rgPipelineStateModel::ExpandNode, this, &rgPipelineStateView::HandleNodeExpanded);
        assert(isConnected);

        // Update the label depending on pipeline type.
        if (m_pipelineType == rgPipelineType::Graphics)
        {
            ui.labelPipelineType->setText(QString(STR_PIPELINE_STATE_EDITOR_LABEL_GRAPHICS) + QString(STR_PIPELINE_STATE_EDITOR_LABEL_HELP));
        }
        else if (m_pipelineType == rgPipelineType::Compute)
        {
            ui.labelPipelineType->setText(QString(STR_PIPELINE_STATE_EDITOR_LABEL_COMPUTE) + QString(STR_PIPELINE_STATE_EDITOR_LABEL_HELP));
        }
        else
        {
            // Should not get here.
            assert(false);
        }
    }
}

void rgPipelineStateView::ResetSearch()
{
    assert(m_pPipelineStateSearcher != nullptr);
    if (m_pPipelineStateSearcher != nullptr)
    {
        m_pPipelineStateSearcher->ResetSearch();
    }
}

void rgPipelineStateView::HandleNodeExpanded(rgEditorElementArrayElementAdd* pArrayRoot)
{
    assert(pArrayRoot != nullptr);
    if (pArrayRoot != nullptr)
    {
        // Expand the given row.
        pArrayRoot->SetExpansionState(rgRowExpansionState::Expanded);
    }
}

void rgPipelineStateView::HandlePsoFileLoaded()
{
    rgEditorElement* pRootElement = ui.settingsTree->GetRootItem();

    assert(pRootElement != nullptr);
    if (pRootElement != nullptr)
    {
        // Recursively expand all nodes after loading the new file.
        pRootElement->SetExpansionState(rgRowExpansionState::Expanded, true);
    }
}

void rgPipelineStateView::ConnectSignals()
{
    // Connect the save button handler.
    bool isConnected = connect(ui.saveButton, &QPushButton::clicked, this, &rgPipelineStateView::SaveButtonClicked);
    assert(isConnected);

    // Connect the load button handler.
    isConnected = connect(ui.loadButton, &QPushButton::clicked, this, &rgPipelineStateView::LoadButtonClicked);
    assert(isConnected);

    // Connect the settings tree in focus signals.
    isConnected = connect(ui.settingsTree, &rgPipelineStateTree::PipelineStateTreeFocusIn, this, &rgPipelineStateView::PipelineStateTreeFocusIn);
    assert(isConnected);

    // Connect the settings tree out of focus signals.
    isConnected = connect(ui.settingsTree, &rgPipelineStateTree::PipelineStateTreeFocusOut, this, &rgPipelineStateView::PipelineStateTreeFocusOut);
    assert(isConnected);
}

void rgPipelineStateView::focusInEvent(QFocusEvent* pEvent)
{
    emit PipelineStateTreeFocusIn();
}

void rgPipelineStateView::focusOutEvent(QFocusEvent* pEvent)
{
    emit PipelineStateTreeFocusOut();
}

void rgPipelineStateView::SetInitialWidgetFocus()
{
    assert(ui.settingsTree != nullptr);
    if (ui.settingsTree != nullptr)
    {
        // Focus on the PSO tree widget within the PSO editor view.
        ui.settingsTree->setFocus();

        // Is there already a row selected in the tree? If not, select the first row.
        const rgPipelineStateTree::CurrentSelection& selection = ui.settingsTree->GetCurrentSelection();
        if (selection.m_pSelectedRow == nullptr)
        {
            // Get the root element of the tree in order to select the first row.
            rgEditorElement* pRootElement = ui.settingsTree->GetRootItem();
            assert(pRootElement != nullptr);
            if (pRootElement != nullptr)
            {
                // Set the selection in the PSO tree.
                ui.settingsTree->SetCurrentSelection(pRootElement);
            }
        }
    }
}

void rgPipelineStateView::dragEnterEvent(QDragEnterEvent* pEvent)
{
    assert(pEvent != nullptr);
    if (pEvent != nullptr)
    {
        const QMimeData* pMimeData = pEvent->mimeData();
        assert(pMimeData != nullptr);
        if (pMimeData != nullptr)
        {
            const int numFiles = pMimeData->urls().size();

            // Make sure the drop data has only one file url, and is a valid file.
            if (pMimeData->hasUrls() && (numFiles == 1))
            {
                // Check to make sure the file is valid.
                QUrl url = pMimeData->urls().at(0);

                // Verify we have the correct file for the current pipeline type.
                bool validFile = false;
                QString extension;
                const QString filePath = url.toLocalFile();
                QStringList nameExtension = filePath.split(STR_FILE_EXTENSION_DELIMITER);
                assert(nameExtension.size() == 2);
                if (nameExtension.size() == 2)
                {
                    extension = filePath.split(".").at(1);
                    if (m_pipelineType == rgPipelineType::Graphics && extension.compare(STR_DEFAULT_PIPELINE_FILE_EXTENSION_NAME_GRAPHICS) == 0)
                    {
                        validFile = true;
                    }
                    else if (m_pipelineType == rgPipelineType::Compute && extension.compare(STR_DEFAULT_PIPELINE_FILE_EXTENSION_NAME_COMPUTE) == 0)
                    {
                        validFile = true;
                    }

                    if (url.isLocalFile() && validFile)
                    {
                        // Accept the action, making it so we receive a dropEvent when the items are released.
                        pEvent->setDropAction(Qt::DropAction::CopyAction);
                        pEvent->accept();
                    }
                    else
                    {
                        pEvent->ignore();
                    }
                }
                else
                {
                    pEvent->ignore();
                }
            }
            else
            {
                pEvent->ignore();
            }
        }
    }
}

void rgPipelineStateView::dropEvent(QDropEvent* pEvent)
{
    assert(pEvent != nullptr);
    if (pEvent != nullptr)
    {
        const QMimeData* pMimeData = pEvent->mimeData();
        assert(pMimeData != nullptr);
        if (pMimeData != nullptr)
        {
            // Make sure the drop data has a file.
            if (pMimeData->hasUrls())
            {
                // Check to make sure the file is valid.
                QUrl url = pMimeData->urls().at(0);
                if (url.isLocalFile())
                {
                    // Get the file path.
                    std::string filePath = url.toLocalFile().toStdString();

                    // Emit a signal to open an existing PSO file.
                    emit DragAndDropExistingFile(filePath);
                }
            }
            else
            {
                pEvent->ignore();
            }
        }
    }
}

void rgPipelineStateView::InsertFindWidget(QWidget* pWidget)
{
    ui.gridLayout->addWidget(pWidget, 0, 1, Qt::AlignTop);
}

void rgPipelineStateView::SetEnumListWidgetStatus(bool isOpen)
{
    ui.settingsTree->SetEnumListWidgetStatus(isOpen);
}