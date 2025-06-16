//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Implementation for the view object used to edit pipeline state configuration.
//=============================================================================

// C++.
#include <cassert>

// Qt.
#include <QMimeData>
#include <QDragEnterEvent>

// QtCommon.
#include "qt_common/utils/qt_util.h"

// Local.
#include "radeon_gpu_analyzer_gui/qt/rg_pipeline_state_model.h"
#include "radeon_gpu_analyzer_gui/qt/rg_pipeline_state_view.h"
#include "radeon_gpu_analyzer_gui/rg_factory_vulkan.h"
#include "radeon_gpu_analyzer_gui/rg_pipeline_state_searcher.h"
#include "radeon_gpu_analyzer_gui/rg_string_constants.h"

RgPipelineStateView::RgPipelineStateView(QWidget* parent) :
    RgSettingsView(parent)
{
    // Initialize the generated view object.
    ui_.setupUi(this);

    // Block recursively repolishing all child widgets within the PSO tree.
    ui_.settingsTree->setProperty(kIsRepolishingBlocked, true);

    // Connect interface signals.
    ConnectSignals();

    // Set button cursor to pointing hand cursor.
    ui_.saveButton->setCursor(Qt::PointingHandCursor);
    ui_.loadButton->setCursor(Qt::PointingHandCursor);

    // Enable drag and drop.
    setAcceptDrops(true);

    // Set focus policy.
    setFocusPolicy(Qt::FocusPolicy::StrongFocus);

    // Set the font size.
    ui_.pipelineStateHeaderLabel->setStyleSheet("font-size: 12pt");
}

void RgPipelineStateView::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);

    emit EditorResized();
}

void RgPipelineStateView::hideEvent(QHideEvent* event)
{
    QWidget::hideEvent(event);

    emit EditorHidden();
}

RgPipelineStateSearcher* RgPipelineStateView::GetSearcher() const
{
    return pipeline_state_searcher_;
}

bool RgPipelineStateView::GetSelectedText(std::string& selected_text_string) const
{
    bool ret = false;

    // Is there a current selection in the table?
    const RgPipelineStateTree::CurrentSelection& selected_row = ui_.settingsTree->GetCurrentSelection();
    if (selected_row.selected_row != nullptr)
    {
        int column_index = selected_row.focused_column;

        if (column_index != -1)
        {
            // Return the data stored in the first selected model index.
            QVariant selected_data = selected_row.selected_row->Data(column_index);
            selected_text_string = selected_data.toString().toStdString();

            // If the user selected the left column containing member names, treat it differently.
            // In this case, we'll trim off the leading array index from the output string.
            if (selected_row.selected_row->GetType() == RgEditorDataType::kArrayElement && column_index == static_cast<int>(RgRowData::kRowDataMemberName))
            {

                // Find the first space in the string- the array index number directly precedes it.
                size_t space_offset = selected_text_string.find_first_of(' ');
                if (space_offset != std::string::npos)
                {
                    selected_text_string = selected_text_string.substr(space_offset + 1);
                }
            }

            // Return true since data was extracted from the selected index.
            ret = true;
        }
    }

    return ret;
}

void RgPipelineStateView::InitializeModel(RgPipelineStateModel* pipeline_state_model)
{
    pipeline_type_ = pipeline_state_model->GetPipelineType();

    // Bind the provided pipeline state model to the settings tree view.
    assert(pipeline_state_model != nullptr);
    if (pipeline_state_model != nullptr)
    {
        // Set the model root element into the tree.
        RgEditorElement* model_root_item = pipeline_state_model->GetRootElement();
        ui_.settingsTree->SetRootItem(model_root_item);

        // Create the pipeline state tree searcher instance.
        pipeline_state_searcher_ = new RgPipelineStateSearcher();

        // Attach a pipeline searcher instance to be used by the find widget.
        assert(pipeline_state_searcher_ != nullptr);
        if (pipeline_state_searcher_ != nullptr)
        {
            pipeline_state_searcher_->SetTargetModel(pipeline_state_model);
            pipeline_state_searcher_->SetTargetView(ui_.settingsTree);
        }

        // Connect the expand node handler.
        [[maybe_unused]] bool is_connected = connect(pipeline_state_model, &RgPipelineStateModel::ExpandNode, this, &RgPipelineStateView::HandleNodeExpanded);
        assert(is_connected);

        // Update the label depending on pipeline type.
        if (pipeline_type_ == RgPipelineType::kGraphics)
        {
            ui_.labelPipelineType->setText(QString(kStrPipelineStateEditorLabelGraphics) + QString(kStrPipelineStateEditorLabelHelp));
        }
        else if (pipeline_type_ == RgPipelineType::kCompute)
        {
            ui_.labelPipelineType->setText(QString(kStrPipelineStateEditorLabelCompute) + QString(kStrPipelineStateEditorLabelHelp));
        }
        else
        {
            // Should not get here.
            assert(false);
        }
    }
}

void RgPipelineStateView::ResetSearch()
{
    assert(pipeline_state_searcher_ != nullptr);
    if (pipeline_state_searcher_ != nullptr)
    {
        pipeline_state_searcher_->ResetSearch();
    }
}

void RgPipelineStateView::HandleNodeExpanded(RgEditorElementArrayElementAdd* array_root)
{
    assert(array_root != nullptr);
    if (array_root != nullptr)
    {
        // Expand the given row.
        array_root->SetExpansionState(RgRowExpansionState::kExpanded);
    }
}

void RgPipelineStateView::HandlePsoFileLoaded()
{
    RgEditorElement* root_element = ui_.settingsTree->GetRootItem();

    assert(root_element != nullptr);
    if (root_element != nullptr)
    {
        // Recursively expand all nodes after loading the new file.
        root_element->SetExpansionState(RgRowExpansionState::kExpanded, true);
    }
}

void RgPipelineStateView::ConnectSignals()
{
    // Connect the save button handler.
    bool is_connected = connect(ui_.saveButton, &QPushButton::clicked, this, &RgPipelineStateView::SaveButtonClicked);
    assert(is_connected);

    // Connect the load button handler.
    is_connected = connect(ui_.loadButton, &QPushButton::clicked, this, &RgPipelineStateView::LoadButtonClicked);
    assert(is_connected);

    // Connect the settings tree in focus signals.
    is_connected = connect(ui_.settingsTree, &RgPipelineStateTree::PipelineStateTreeFocusIn, this, &RgPipelineStateView::PipelineStateTreeFocusIn);
    assert(is_connected);

    // Connect the settings tree out of focus signals.
    is_connected = connect(ui_.settingsTree, &RgPipelineStateTree::PipelineStateTreeFocusOut, this, &RgPipelineStateView::PipelineStateTreeFocusOut);
    assert(is_connected);
}

void RgPipelineStateView::focusInEvent(QFocusEvent* event)
{
    Q_UNUSED(event);

    emit PipelineStateTreeFocusIn();
}

void RgPipelineStateView::focusOutEvent(QFocusEvent* event)
{
    Q_UNUSED(event);

    emit PipelineStateTreeFocusOut();
}

void RgPipelineStateView::SetInitialWidgetFocus()
{
    assert(ui_.settingsTree != nullptr);
    if (ui_.settingsTree != nullptr)
    {
        // Focus on the PSO tree widget within the PSO editor view.
        ui_.settingsTree->setFocus();

        // Is there already a row selected in the tree? If not, select the first row.
        const RgPipelineStateTree::CurrentSelection& selection = ui_.settingsTree->GetCurrentSelection();
        if (selection.selected_row == nullptr)
        {
            // Get the root element of the tree in order to select the first row.
            RgEditorElement* root_element = ui_.settingsTree->GetRootItem();
            assert(root_element != nullptr);
            if (root_element != nullptr)
            {
                // Set the selection in the PSO tree.
                ui_.settingsTree->SetCurrentSelection(root_element);
            }
        }
    }
}

void RgPipelineStateView::dragEnterEvent(QDragEnterEvent* event)
{
    assert(event != nullptr);
    if (event != nullptr)
    {
        const QMimeData* mime_data = event->mimeData();
        assert(mime_data != nullptr);
        if (mime_data != nullptr)
        {
            const int num_files = mime_data->urls().size();

            // Make sure the drop data has only one file url, and is a valid file.
            if (mime_data->hasUrls() && (num_files == 1))
            {
                // Check to make sure the file is valid.
                QUrl url = mime_data->urls().at(0);

                // Verify we have the correct file for the current pipeline type.
                bool valid_file = false;
                QString extension;
                const QString file_path = url.toLocalFile();
                QStringList name_extension = file_path.split(kStrFileExtensionDelimiter);
                assert(name_extension.size() == 2);
                if (name_extension.size() == 2)
                {
                    extension = file_path.split(".").at(1);
                    if (pipeline_type_ == RgPipelineType::kGraphics && extension.compare(kStrDefaultPipelineFileExtensionNameGraphics) == 0)
                    {
                        valid_file = true;
                    }
                    else if (pipeline_type_ == RgPipelineType::kCompute && extension.compare(kStrDefaultPipelineFileExtensionNameCompute) == 0)
                    {
                        valid_file = true;
                    }

                    if (url.isLocalFile() && valid_file)
                    {
                        // Accept the action, making it so we receive a dropEvent when the items are released.
                        event->setDropAction(Qt::DropAction::CopyAction);
                        event->accept();
                    }
                    else
                    {
                        event->ignore();
                    }
                }
                else
                {
                    event->ignore();
                }
            }
            else
            {
                event->ignore();
            }
        }
    }
}

void RgPipelineStateView::dropEvent(QDropEvent* event)
{
    assert(event != nullptr);
    if (event != nullptr)
    {
        const QMimeData* mime_data = event->mimeData();
        assert(mime_data != nullptr);
        if (mime_data != nullptr)
        {
            // Make sure the drop data has a file.
            if (mime_data->hasUrls())
            {
                // Check to make sure the file is valid.
                QUrl url = mime_data->urls().at(0);
                if (url.isLocalFile())
                {
                    // Get the file path.
                    std::string file_path = url.toLocalFile().toStdString();

                    // Emit a signal to open an existing PSO file.
                    emit DragAndDropExistingFile(file_path);
                }
            }
            else
            {
                event->ignore();
            }
        }
    }
}

void RgPipelineStateView::InsertFindWidget(QWidget* widget)
{
    ui_.gridLayout->addWidget(widget, 0, 1, Qt::AlignTop);
}

void RgPipelineStateView::SetEnumListWidgetStatus(bool is_open)
{
    ui_.settingsTree->SetEnumListWidgetStatus(is_open);
}
