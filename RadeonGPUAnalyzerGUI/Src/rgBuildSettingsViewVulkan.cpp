// C++.
#include <cassert>
#include <sstream>
#include <algorithm>

// Qt.
#include <QWidget>
#include <QMessageBox>
#include <QDesktopWidget>
#include <QFileDialog>

// Infra.
#include <QtCommon/Util/RestoreCursorPosition.h>
#include <QtCommon/Scaling/ScalingManager.h>
#include <Utils/Include/rgaCliDefs.h>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgBuildSettingsView.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgBuildSettingsViewVulkan.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgCheckBox.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgIncludeDirectoriesView.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgLineEdit.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgPreprocessorDirectivesDialog.h>
#include <RadeonGPUAnalyzerGUI/Include/rgCliUtils.h>
#include <RadeonGPUAnalyzerGUI/Include/rgConfigManager.h>
#include <RadeonGPUAnalyzerGUI/Include/rgStringConstants.h>
#include <RadeonGPUAnalyzerGUI/Include/rgUtils.h>

// Checkbox tool tip stylesheets.
static const char* s_STR_FILEMENU_TITLE_BAR_TOOLTIP_WIDTH = "min-width: %1px; width: %2px;";
static const char* s_STR_FILEMENU_TITLE_BAR_TOOLTIP_HEIGHT = "min-height: %1px; height: %2px; max-height: %3px;";

// The delimiter to use to join and split a string of option items.
static const char* s_OPTIONS_LIST_DELIMITER = ";";

rgBuildSettingsViewVulkan::rgBuildSettingsViewVulkan(QWidget* pParent, const rgBuildSettingsVulkan& buildSettings, bool isGlobalSettings) :
    rgBuildSettingsView(pParent, isGlobalSettings),
    m_initialSettings(buildSettings)
{
    // Setup the UI.
    ui.setupUi(this);

    // Set the background to white.
    QPalette pal = palette();
    pal.setColor(QPalette::Background, Qt::white);
    this->setAutoFillBackground(true);
    this->setPalette(pal);

    // Create the include directories editor view.
    m_pIncludeDirectoriesView = new rgIncludeDirectoriesView(s_OPTIONS_LIST_DELIMITER, this);

    // Create the preprocessor directives editor dialog.
    m_pPreprocessorDirectivesDialog = new rgPreprocessorDirectivesDialog(s_OPTIONS_LIST_DELIMITER, this);

    // Connect the signals.
    ConnectSignals();

    // Connect focus in/out events for all of the line edits.
    ConnectLineEditFocusEvents();

    // Connect clicked events for check boxes.
    ConnectCheckBoxClickedEvents();

    // Initialize the UI based on the incoming build settings.
    PushToWidgets(buildSettings);

    // Initialize the command line preview text.
    UpdateCommandLineText();

    // Set tooltips for general items.
    ui.targetGPUsLabel->setToolTip(STR_BUILD_SETTINGS_TARGET_GPUS_TOOLTIP);
    ui.predefinedMacrosLabel->setToolTip(STR_BUILD_SETTINGS_PREDEFINED_MACROS_TOOLTIP);
    ui.includeDirectoriesLabel->setToolTip(STR_BUILD_SETTINGS_ADDITIONAL_INC_DIRECTORY_TOOLTIP);

    // Set tooltip for Vulkan runtime section.
    ui.vulkanOptionsHeaderLabel->setToolTip(STR_BUILD_SETTINGS_VULKAN_RUNTIME_TOOLTIP);

    // Set tooltip for ICD location item.
    ui.ICDLocationLabel->setToolTip(CLI_OPT_ICD_DESCRIPTION);

    // Set tooltips for alternative compiler (glslang) component.
    // Use the same tooltip for both the title and the item purposely.
    ui.alternativeCompilerHeaderLabel->setToolTip(STR_BUILD_SETTINGS_ALTERNATIVE_COMPILER_GLSLANG_TOOLTIP);
    ui.glslangOptionsLabel->setToolTip(CLI_OPT_GLSLANG_OPT_DESCRIPTION_A);
    ui.compilerBinariesLabel->setToolTip(CLI_DESC_ALTERNATIVE_VK_BIN_FOLDER);

    // Set tooltip for the General section.
    ui.generalHeaderLabel->setToolTip(STR_BUILD_SETTINGS_GENERAL_TOOLTIP);

    // Set the tooltip for the command line section of the build settings.
    ui.settingsCommandLineHeaderLabel->setToolTip(STR_BUILD_SETTINGS_CMD_LINE_TOOLTIP);

    // Set the mouse cursor to the pointing hand cursor for various widgets.
    SetCursor();

    // Set the event filter for "All Options" text edit.
    ui.allOptionsTextEdit->installEventFilter(this);

    // Hide HLSL options for now.
    HideHLSLOptions();
}

void rgBuildSettingsViewVulkan::HideHLSLOptions()
{
    // Hide HLSL options for now.
    ui.vulkanOptionsHeaderLabel->hide();
    ui.generateDebugInfoCheckBox->hide();
    ui.generateDebugInfoLabel->hide();
    ui.noExplicitBindingsCheckBox->hide();
    ui.noExplicitBindingsLabel->hide();
    ui.useHLSLBlockOffsetsCheckBox->hide();
    ui.useHLSLBlockOffsetsLabel->hide();
    ui.useHLSLIOMappingCheckBox->hide();
    ui.useHLSLIOMappingLabel->hide();
}

void rgBuildSettingsViewVulkan::ConnectCheckBoxClickedEvents()
{
   bool isConnected = false;

    isConnected = connect(ui.generateDebugInfoCheckBox, &rgCheckBox::clicked, this, &rgBuildSettingsViewVulkan::HandleCheckBoxClickedEvent);
    assert(isConnected);

    isConnected = connect(ui.noExplicitBindingsCheckBox, &rgCheckBox::clicked, this, &rgBuildSettingsViewVulkan::HandleCheckBoxClickedEvent);
    assert(isConnected);

    isConnected = connect(ui.useHLSLBlockOffsetsCheckBox, &rgCheckBox::clicked, this, &rgBuildSettingsViewVulkan::HandleCheckBoxClickedEvent);
    assert(isConnected);

    isConnected = connect(ui.useHLSLIOMappingCheckBox, &rgCheckBox::clicked, this, &rgBuildSettingsViewVulkan::HandleCheckBoxClickedEvent);
    assert(isConnected);

    isConnected = connect(ui.enableValidationLayersCheckBox, &rgCheckBox::clicked, this, &rgBuildSettingsViewVulkan::HandleCheckBoxClickedEvent);
    assert(isConnected);
}

// Make the UI reflect the values in the supplied settings struct.
void rgBuildSettingsViewVulkan::PushToWidgets(const rgBuildSettingsVulkan& settings)
{
    // Disable any signals from the widgets while they're being populated.
    QSignalBlocker signalBlockerTargetGPUsLineEdit(ui.targetGPUsLineEdit);
    QSignalBlocker signalBlockerPredefinedMacrosLineEdit(ui.predefinedMacrosLineEdit);
    QSignalBlocker signalBlockerIncludeDirectoriesLineEdit(ui.includeDirectoriesLineEdit);
    QSignalBlocker signalBlockerGenerateDebugInfoCheckBox(ui.generateDebugInfoCheckBox);
    QSignalBlocker signalBlockerNoExplicitBindingsCheckBox(ui.noExplicitBindingsCheckBox);
    QSignalBlocker signalBlockerUseHLSLBlockOffsetsCheckBox(ui.useHLSLBlockOffsetsCheckBox);
    QSignalBlocker signalBlockerUseHLSLIOMappingCheckBox(ui.useHLSLIOMappingCheckBox);
    QSignalBlocker signalBlockerEnableValidationLayersCheckBox(ui.enableValidationLayersCheckBox);
    QSignalBlocker signalBlockerICDLocationLineEdit(ui.ICDLocationLineEdit);
    QSignalBlocker signalBlockerGlslangOptionsLineEdit(ui.glslangOptionsLineEdit);
    QSignalBlocker signalBlockerCompilerBinariesLineEdit(ui.compilerBinariesLineEdit);
    QSignalBlocker signalBlockerOutputFileBinaryNameLineEdit(ui.outputFileBinaryNameLineEdit);

    // The items below are common build settings for all API types.
    QString targetGpusList(rgUtils::BuildSemicolonSeparatedStringList(settings.m_targetGpus).c_str());
    ui.targetGPUsLineEdit->setText(targetGpusList);

    QString predefinedMacros(rgUtils::BuildSemicolonSeparatedStringList(settings.m_predefinedMacros).c_str());
    ui.predefinedMacrosLineEdit->setText(predefinedMacros);

    QString additionalIncludeDirs(rgUtils::BuildSemicolonSeparatedStringList(settings.m_additionalIncludeDirectories).c_str());
    ui.includeDirectoriesLineEdit->setText(additionalIncludeDirs);

    // Items below are Vulkan-specific build settings only.
    ui.generateDebugInfoCheckBox->setChecked(settings.m_isGenerateDebugInfoChecked);
    ui.noExplicitBindingsCheckBox->setChecked(settings.m_isNoExplicitBindingsChecked);
    ui.useHLSLBlockOffsetsCheckBox->setChecked(settings.m_isUseHlslBlockOffsetsChecked);
    ui.useHLSLIOMappingCheckBox->setChecked(settings.m_isUseHlslIoMappingChecked);
    ui.enableValidationLayersCheckBox->setChecked(settings.m_isEnableValidationLayersChecked);
    ui.ICDLocationLineEdit->setText(QString::fromStdString(settings.m_ICDLocation));
    ui.glslangOptionsLineEdit->setText(QString::fromStdString(settings.m_glslangOptions));
    ui.compilerBinariesLineEdit->setText(QString::fromStdString(std::get<CompilerFolderType::Bin>(settings.m_compilerPaths)));

    // Output binary file name.
    ui.outputFileBinaryNameLineEdit->setText(settings.m_binaryFileName.c_str());
}

rgBuildSettingsVulkan rgBuildSettingsViewVulkan::PullFromWidgets() const
{
    rgBuildSettingsVulkan settings;

    // Target GPUs.
    std::vector<std::string> targetGPUsVector;
    const std::string& commaSeparatedTargetGPUs = ui.targetGPUsLineEdit->text().toStdString();
    rgUtils::splitString(commaSeparatedTargetGPUs, rgConfigManager::RGA_LIST_DELIMITER, targetGPUsVector);
    settings.m_targetGpus = targetGPUsVector;

    // Predefined Macros.
    std::vector<std::string> predefinedMacrosVector;
    const std::string& commaSeparatedPredefinedMacros = ui.predefinedMacrosLineEdit->text().toStdString();
    rgUtils::splitString(commaSeparatedPredefinedMacros, rgConfigManager::RGA_LIST_DELIMITER, predefinedMacrosVector);
    settings.m_predefinedMacros = predefinedMacrosVector;

    // Additional Include Directories.
    std::vector<std::string> additionalIncludeDirectoriesVector;
    const std::string& commaSeparatedAdditionalIncludeDirectories = ui.includeDirectoriesLineEdit->text().toStdString();
    rgUtils::splitString(commaSeparatedAdditionalIncludeDirectories, rgConfigManager::RGA_LIST_DELIMITER, additionalIncludeDirectoriesVector);
    settings.m_additionalIncludeDirectories = additionalIncludeDirectoriesVector;

    // Vulkan-specific settings.
    settings.m_isGenerateDebugInfoChecked = ui.generateDebugInfoCheckBox->isChecked();
    settings.m_isNoExplicitBindingsChecked = ui.noExplicitBindingsCheckBox->isChecked();
    settings.m_isUseHlslBlockOffsetsChecked = ui.useHLSLBlockOffsetsCheckBox->isChecked();
    settings.m_isUseHlslIoMappingChecked = ui.useHLSLIOMappingCheckBox->isChecked();
    settings.m_isEnableValidationLayersChecked = ui.enableValidationLayersCheckBox->isChecked();
    settings.m_ICDLocation = ui.ICDLocationLineEdit->text().toStdString();
    settings.m_glslangOptions = ui.glslangOptionsLineEdit->text().toStdString();
    std::get<CompilerFolderType::Bin>(settings.m_compilerPaths) = ui.compilerBinariesLineEdit->text().toStdString();

    // Binary output file name.
    settings.m_binaryFileName = ui.outputFileBinaryNameLineEdit->text().toStdString();

    return settings;
}

void rgBuildSettingsViewVulkan::ConnectSignals()
{
    // Add target GPU button.
    bool isConnected = connect(this->ui.addTargetGPUsButton, &QPushButton::clicked, this, &rgBuildSettingsViewVulkan::HandleAddTargetGpusButtonClick);
    assert(isConnected);

    // Add include directories editor dialog button.
    isConnected = connect(this->ui.includeDirsBrowseButton, &QPushButton::clicked, this, &rgBuildSettingsViewVulkan::HandleIncludeDirsBrowseButtonClick);
    assert(isConnected);

    // Add preprocessor directives editor dialog button.
    isConnected = connect(this->ui.predefinedMacrosBrowseButton, &QPushButton::clicked, this, &rgBuildSettingsViewVulkan::HandlePreprocessorDirectivesBrowseButtonClick);
    assert(isConnected);

    // Connect all textboxes within the view.
    isConnected = connect(this->ui.targetGPUsLineEdit, &QLineEdit::textChanged, this, &rgBuildSettingsViewVulkan::HandleTextEditChanged);
    assert(isConnected);

    // Handle changes to the Predefined Macros setting.
    isConnected = connect(this->ui.predefinedMacrosLineEdit, &QLineEdit::textChanged, this, &rgBuildSettingsViewVulkan::HandleTextEditChanged);
    assert(isConnected);

    // Handle changes to the Include Directories setting.
    isConnected = connect(this->ui.includeDirectoriesLineEdit, &QLineEdit::textChanged, this, &rgBuildSettingsViewVulkan::HandleTextEditChanged);
    assert(isConnected);

    // Connect the include directory editor dialog's "OK" button click.
    isConnected = connect(m_pIncludeDirectoriesView, &rgIncludeDirectoriesView::OKButtonClicked, this, &rgBuildSettingsViewVulkan::HandleIncludeDirsUpdated);
    assert(isConnected);

    // Connect the preprocessor directives editor dialog's "OK" button click.
    isConnected = connect(m_pPreprocessorDirectivesDialog, &rgPreprocessorDirectivesDialog::OKButtonClicked, this, &rgBuildSettingsViewVulkan::HandlePreprocessorDirectivesUpdated);
    assert(isConnected);

    // Handle changes to the Generate Debug Info checkbox.
    isConnected = connect(this->ui.generateDebugInfoCheckBox, &rgCheckBox::stateChanged, this, &rgBuildSettingsViewVulkan::HandleCheckboxStateChanged);
    assert(isConnected);

    // Handle changes to the No Explicit Bindings checkbox.
    isConnected = connect(this->ui.noExplicitBindingsCheckBox, &rgCheckBox::stateChanged, this, &rgBuildSettingsViewVulkan::HandleCheckboxStateChanged);
    assert(isConnected);

    // Handle changes to the Use HLSL Block Offsets checkbox.
    isConnected = connect(this->ui.useHLSLBlockOffsetsCheckBox, &rgCheckBox::stateChanged, this, &rgBuildSettingsViewVulkan::HandleCheckboxStateChanged);
    assert(isConnected);

    // Handle changes to the Use HLSL IO Mapping checkbox.
    isConnected = connect(this->ui.useHLSLIOMappingCheckBox, &rgCheckBox::stateChanged, this, &rgBuildSettingsViewVulkan::HandleCheckboxStateChanged);
    assert(isConnected);

    // Handle changes to the Enable Validation Layers checkbox.
    isConnected = connect(this->ui.enableValidationLayersCheckBox, &rgCheckBox::stateChanged, this, &rgBuildSettingsViewVulkan::HandleCheckboxStateChanged);
    assert(isConnected);

    // Handle clicking of ICD location browse button.
    isConnected = connect(this->ui.ICDLocationBrowseButton, &QPushButton::clicked, this, &rgBuildSettingsViewVulkan::HandleICDLocationBrowseButtonClick);
    assert(isConnected);

    // Handle changes to the ICD location line edit.
    isConnected = connect(this->ui.ICDLocationLineEdit, &QLineEdit::textChanged, this, &rgBuildSettingsViewVulkan::HandleICDLocationLineEditChanged);
    assert(isConnected);

    // Handle changes to the glslang options line edit.
    isConnected = connect(this->ui.glslangOptionsLineEdit, &QLineEdit::textChanged, this, &rgBuildSettingsViewVulkan::HandleGlslangOptionsLineEditChanged);
    assert(isConnected);

    // Handle clicking of the Alternative Compiler path browse button.
    isConnected = connect(this->ui.compilerBrowseButton, &QPushButton::clicked, this, &rgBuildSettingsViewVulkan::HandleAlternativeCompilerBrowseButtonClicked);
    assert(isConnected);

    // Handle changes to the Alternative Compiler path line edit.
    isConnected = connect(this->ui.compilerBinariesLineEdit, &QLineEdit::textChanged, this, &rgBuildSettingsViewVulkan::HandleAlternativeCompilerLineEditChanged);
    assert(isConnected);

    // Binary Output File name textChanged signal.
    isConnected = connect(this->ui.outputFileBinaryNameLineEdit, &QLineEdit::textChanged, this, &rgBuildSettingsViewVulkan::HandleOutputBinaryEditBoxChanged);
    assert(isConnected);

    // Binary Output File name editingFinished signal.
    isConnected = connect(this->ui.outputFileBinaryNameLineEdit, &QLineEdit::editingFinished, this, &rgBuildSettingsViewVulkan::HandleOutputBinaryFileEditingFinished);
    assert(isConnected);
}

void rgBuildSettingsViewVulkan::HandleOutputBinaryFileEditingFinished()
{
    // Verify that the output binary file text is not empty before losing the focus.
    if (this->ui.outputFileBinaryNameLineEdit->text().trimmed().isEmpty() || !rgUtils::IsValidFileName(ui.outputFileBinaryNameLineEdit->text().toStdString()))
    {
        // Initialize the binary output file name edit line.
        std::shared_ptr<rgBuildSettings> pDefaultSettings = rgConfigManager::Instance().GetUserGlobalBuildSettings(rgProjectAPI::Vulkan);
        auto pVkDefaultSettings = std::dynamic_pointer_cast<rgBuildSettingsVulkan>(pDefaultSettings);

        assert(pVkDefaultSettings != nullptr);
        if (pVkDefaultSettings != nullptr)
        {
            this->ui.outputFileBinaryNameLineEdit->setText(pVkDefaultSettings->m_binaryFileName.c_str());
        }
        this->ui.outputFileBinaryNameLineEdit->setFocus();
    }

    // Signal to any listeners that the values in the UI have changed.
    HandlePendingChangesStateChanged(GetHasPendingChanges());

    // Update the command line preview text.
    UpdateCommandLineText();
}

void rgBuildSettingsViewVulkan::HandleOutputBinaryEditBoxChanged(const QString& text)
{
    // Update the tooltip.
    ui.outputFileBinaryNameLineEdit->setToolTip(text);

    // Restore the cursor to the original position when the text has changed.
    QtCommon::QtUtil::RestoreCursorPosition cursorPosition(ui.outputFileBinaryNameLineEdit);

    // Signal to any listeners that the values in the UI have changed.
    HandlePendingChangesStateChanged(GetHasPendingChanges());

    // Update the command line preview text.
    UpdateCommandLineText();
}

void rgBuildSettingsViewVulkan::HandleICDLocationBrowseButtonClick(bool /* checked */)
{
    QString selectedFile = QFileDialog::getOpenFileName(this, tr(STR_ICD_LOCATION_DIALOG_SELECT_FILE_TITLE),
                                                        ui.ICDLocationLineEdit->text(),
                                                        tr(STR_DIALOG_FILTER_ICD));
    if (!selectedFile.isEmpty())
    {
        ui.ICDLocationLineEdit->setText(selectedFile);

        // Inform the UI of a possible change to the pending state.
        HandlePendingChangesStateChanged(GetHasPendingChanges());

        // Update the command line preview text.
        UpdateCommandLineText();
    }
}

void rgBuildSettingsViewVulkan::HandleICDLocationLineEditChanged(const QString& text)
{
    // Restore the cursor to the original position when the text has changed.
    QtCommon::QtUtil::RestoreCursorPosition cursorPosition(ui.ICDLocationLineEdit);

    // Set the text value.
    ui.ICDLocationLineEdit->setText(text);

    // Inform the UI of a possible change to the pending state.
    HandlePendingChangesStateChanged(GetHasPendingChanges());

    // Update the command line preview text.
    UpdateCommandLineText();
}

void rgBuildSettingsViewVulkan::HandleGlslangOptionsLineEditChanged(const QString& text)
{
    // Restore the cursor to the original position when the text has changed.
    QtCommon::QtUtil::RestoreCursorPosition cursorPosition(ui.glslangOptionsLineEdit);

    // Set the text value.
    ui.glslangOptionsLineEdit->setText(text);

    // Inform the UI of a possible change to the pending state.
    HandlePendingChangesStateChanged(GetHasPendingChanges());

    // Update the command line preview text.
    UpdateCommandLineText();
}

void rgBuildSettingsViewVulkan::HandleAlternativeCompilerBrowseButtonClicked()
{
    QFileDialog fileDialog;
    QLineEdit* pLineEdit = ui.compilerBinariesLineEdit;
    assert(pLineEdit != nullptr);
    if (pLineEdit != nullptr)
    {
        QString currentDir = (pLineEdit->text().isEmpty() ?
            QString::fromStdString(rgConfigManager::Instance().GetLastSelectedFolder()) : pLineEdit->text());

        QString selectedDirectory = QFileDialog::getExistingDirectory(this, tr(STR_INCLUDE_DIR_DIALOG_SELECT_DIR_TITLE),
            currentDir, QFileDialog::ShowDirsOnly);
        if (!selectedDirectory.isEmpty())
        {
            ui.compilerBinariesLineEdit->setText(selectedDirectory);

            // Inform the UI of a possible change to the pending state.
            HandlePendingChangesStateChanged(GetHasPendingChanges());

            // Update the command line preview text.
            UpdateCommandLineText();
        }
    }
}

void rgBuildSettingsViewVulkan::HandleAlternativeCompilerLineEditChanged(const QString& text)
{
    // Restore the cursor to the original position when the text has changed.
    QtCommon::QtUtil::RestoreCursorPosition cursorPosition(ui.compilerBinariesLineEdit);

    // Set the text value.
    ui.compilerBinariesLineEdit->setText(text);

    // Inform the UI of a possible change to the pending state.
    HandlePendingChangesStateChanged(GetHasPendingChanges());

    // Update the command line preview text.
    UpdateCommandLineText();
}

void rgBuildSettingsViewVulkan::ConnectLineEditFocusEvents()
{
    bool isConnected = false;

    isConnected = connect(this->ui.includeDirectoriesLineEdit, &rgLineEdit::LineEditFocusInEvent, this, &rgBuildSettingsViewVulkan::HandleLineEditFocusInEvent);
    assert(isConnected);

    isConnected = connect(this->ui.includeDirectoriesLineEdit, &rgLineEdit::LineEditFocusOutEvent, this, &rgBuildSettingsViewVulkan::HandleLineEditFocusOutEvent);
    assert(isConnected);

    isConnected = connect(this->ui.predefinedMacrosLineEdit, &rgLineEdit::LineEditFocusInEvent, this, &rgBuildSettingsViewVulkan::HandleLineEditFocusInEvent);
    assert(isConnected);

    isConnected = connect(this->ui.predefinedMacrosLineEdit, &rgLineEdit::LineEditFocusOutEvent, this, &rgBuildSettingsViewVulkan::HandleLineEditFocusOutEvent);
    assert(isConnected);

    isConnected = connect(this->ui.targetGPUsLineEdit, &rgLineEdit::LineEditFocusInEvent, this, &rgBuildSettingsViewVulkan::HandleLineEditFocusInEvent);
    assert(isConnected);

    isConnected = connect(this->ui.targetGPUsLineEdit, &rgLineEdit::LineEditFocusOutEvent, this, &rgBuildSettingsViewVulkan::HandleLineEditFocusOutEvent);
    assert(isConnected);

    isConnected = connect(this->ui.ICDLocationLineEdit, &rgLineEdit::LineEditFocusInEvent, this, &rgBuildSettingsViewVulkan::HandleLineEditFocusInEvent);
    assert(isConnected);

    isConnected = connect(this->ui.ICDLocationLineEdit, &rgLineEdit::LineEditFocusOutEvent, this, &rgBuildSettingsViewVulkan::HandleLineEditFocusOutEvent);
    assert(isConnected);

    isConnected = connect(this->ui.glslangOptionsLineEdit, &rgLineEdit::LineEditFocusInEvent, this, &rgBuildSettingsViewVulkan::HandleLineEditFocusInEvent);
    assert(isConnected);

    isConnected = connect(this->ui.glslangOptionsLineEdit, &rgLineEdit::LineEditFocusOutEvent, this, &rgBuildSettingsViewVulkan::HandleLineEditFocusOutEvent);
    assert(isConnected);

    isConnected = connect(this->ui.compilerBinariesLineEdit, &rgLineEdit::LineEditFocusInEvent, this, &rgBuildSettingsViewVulkan::HandleLineEditFocusInEvent);
    assert(isConnected);

    isConnected = connect(this->ui.compilerBinariesLineEdit, &rgLineEdit::LineEditFocusOutEvent, this, &rgBuildSettingsViewVulkan::HandleLineEditFocusOutEvent);
    assert(isConnected);

    isConnected = connect(this->ui.addTargetGPUsButton, &rgBrowseButton::BrowseButtonFocusInEvent, this, &rgBuildSettingsViewVulkan::HandleBrowseButtonFocusInEvent);
    assert(isConnected);

    isConnected = connect(this->ui.addTargetGPUsButton, &rgBrowseButton::BrowseButtonFocusOutEvent, this, &rgBuildSettingsViewVulkan::HandleBrowseButtonFocusOutEvent);
    assert(isConnected);

    isConnected = connect(this->ui.predefinedMacrosBrowseButton, &rgBrowseButton::BrowseButtonFocusInEvent, this, &rgBuildSettingsViewVulkan::HandleBrowseButtonFocusInEvent);
    assert(isConnected);

    isConnected = connect(this->ui.predefinedMacrosBrowseButton, &rgBrowseButton::BrowseButtonFocusOutEvent, this, &rgBuildSettingsViewVulkan::HandleBrowseButtonFocusOutEvent);
    assert(isConnected);

    isConnected = connect(this->ui.includeDirsBrowseButton, &rgBrowseButton::BrowseButtonFocusInEvent, this, &rgBuildSettingsViewVulkan::HandleBrowseButtonFocusInEvent);
    assert(isConnected);

    isConnected = connect(this->ui.includeDirsBrowseButton, &rgBrowseButton::BrowseButtonFocusOutEvent, this, &rgBuildSettingsViewVulkan::HandleBrowseButtonFocusOutEvent);
    assert(isConnected);

    isConnected = connect(this->ui.ICDLocationBrowseButton, &rgBrowseButton::BrowseButtonFocusInEvent, this, &rgBuildSettingsViewVulkan::HandleBrowseButtonFocusInEvent);
    assert(isConnected);

    isConnected = connect(this->ui.ICDLocationBrowseButton, &rgBrowseButton::BrowseButtonFocusOutEvent, this, &rgBuildSettingsViewVulkan::HandleBrowseButtonFocusOutEvent);
    assert(isConnected);

    isConnected = connect(this->ui.compilerBrowseButton, &rgBrowseButton::BrowseButtonFocusInEvent, this, &rgBuildSettingsViewVulkan::HandleBrowseButtonFocusInEvent);
    assert(isConnected);

    isConnected = connect(this->ui.compilerBrowseButton, &rgBrowseButton::BrowseButtonFocusOutEvent, this, &rgBuildSettingsViewVulkan::HandleBrowseButtonFocusOutEvent);
    assert(isConnected);

    isConnected = connect(this->ui.enableValidationLayersCheckBox, &rgCheckBox::CheckBoxFocusInEvent, this, &rgBuildSettingsViewVulkan::HandleCheckBoxFocusInEvent);
    assert(isConnected);

    isConnected = connect(this->ui.enableValidationLayersCheckBox, &rgCheckBox::CheckBoxFocusOutEvent, this, &rgBuildSettingsViewVulkan::HandleCheckBoxFocusOutEvent);
    assert(isConnected);
}

void rgBuildSettingsViewVulkan::HandleAddTargetGpusButtonClick()
{
    // Set the frame border color to red.
    emit SetFrameBorderRedSignal();

    // Trim out any spaces within the target GPUs string.
    QString selectedGPUs = this->ui.targetGPUsLineEdit->text();

    // Create a new Target GPU Selection dialog instance.
    m_pTargetGpusDialog = new rgTargetGpusDialog(selectedGPUs, this);

    // Register the target gpu dialog box with the scaling manager.
    ScalingManager::Get().RegisterObject(m_pTargetGpusDialog);

    // Center the dialog on the view (registering with the scaling manager
    // shifts it out of the center so we need to manually center it).
    rgUtils::CenterOnWidget(m_pTargetGpusDialog, this);

    // Present the dialog to the user.
    m_pTargetGpusDialog->setModal(true);
    int dialogResult = m_pTargetGpusDialog->exec();

    // If the user clicked "OK", extract the Checked items into a comma-separated string,
    // and put the string in the Target GPUs textbox.
    if (dialogResult == 1)
    {
        // After the dialog is hidden, extract the list of families selected by the user.
        std::vector<std::string> selectedFamilies = m_pTargetGpusDialog->GetSelectedCapabilityGroups();

        // Remove gfx notation if needed.
        std::transform(selectedFamilies.begin(), selectedFamilies.end(), selectedFamilies.begin(),
            [&](std::string& family)
        {
            return rgUtils::RemoveGfxNotation(family);
        });

        // Create a comma-separated list of GPU families that the user selected.
        std::stringstream familyListString;
        size_t numFamilies = selectedFamilies.size();
        for (size_t familyIndex = 0; familyIndex < numFamilies; ++familyIndex)
        {
            // Append the family name to the string.
            familyListString << selectedFamilies[familyIndex];

            // Append a comma between each family name, until the last one.
            if ((familyIndex + 1) < numFamilies)
            {
                familyListString << rgConfigManager::RGA_LIST_DELIMITER;
            }
        }

        // Set the target GPUs text.
        ui.targetGPUsLineEdit->setText(familyListString.str().c_str());

        // Inform the UI of a possible change to the pending state.
        HandlePendingChangesStateChanged(GetHasPendingChanges());
    }
}

void rgBuildSettingsViewVulkan::HandleTextEditChanged()
{
    // Determine which control's text has been updated.
    QLineEdit* pLineEdit = static_cast<QLineEdit*>(QObject::sender());
    assert(pLineEdit != nullptr);
    if (pLineEdit != nullptr)
    {
        // Inform the UI of a possible change to the pending state.
        HandlePendingChangesStateChanged(GetHasPendingChanges());

        // Update the command line preview text.
        UpdateCommandLineText();
    }
}

std::string rgBuildSettingsViewVulkan::GetTitleString()
{
    std::stringstream titleString;

    // Build the title string.
    if (m_isGlobalSettings)
    {
        // For the global settings.
        titleString << STR_BUILD_SETTINGS_DEFAULT_TITLE;
        titleString << " ";
        titleString << STR_API_NAME_VULKAN;
        titleString << " ";
    }
    else
    {
        // For project-specific settings.
        titleString << STR_BUILD_SETTINGS_PROJECT_TITLE;
        titleString << " ";
    }
    titleString << STR_MENU_BUILD_SETTINGS_LOWER;

    return titleString.str();
}

const std::string rgBuildSettingsViewVulkan::GetTitleTooltipString() const
{
    std::stringstream tooltipString;

    if (m_isGlobalSettings)
    {
        tooltipString << STR_BUILD_SETTINGS_GLOBAL_TOOLTIP_A;
        tooltipString << STR_API_NAME_VULKAN;
        tooltipString << STR_BUILD_SETTINGS_GLOBAL_TOOLTIP_B;
    }
    else
    {
        tooltipString << STR_BUILD_SETTINGS_PROJECT_TOOLTIP_A;
        tooltipString << STR_API_NAME_VULKAN;
        tooltipString << STR_BUILD_SETTINGS_PROJECT_TOOLTIP_B;
    }

    return tooltipString.str();
}

void rgBuildSettingsViewVulkan::UpdateCommandLineText()
{
    rgBuildSettingsVulkan apiBuildSetting = PullFromWidgets();

    // Generate a command line string from the build settings structure.
    std::string buildSettings;
    bool ret = rgCliUtils::GenerateVulkanBuildSettingsString(apiBuildSetting, buildSettings);
    assert(ret);
    if (ret)
    {
        ui.allOptionsTextEdit->setPlainText(buildSettings.c_str());
    }
}

void rgBuildSettingsViewVulkan::HandlePendingChangesStateChanged(bool hasPendingChanges)
{
    // Let the base class determine if there is a need to signal listeners
    // about the pending changes state.
    rgBuildSettingsView::SetHasPendingChanges(hasPendingChanges);
}

bool rgBuildSettingsViewVulkan::IsTargetGpusStringValid(std::vector<std::string>& errors) const
{
    bool isValid = true;

    // The target GPUs string must be non-empty.
    std::string targetGpusString = ui.targetGPUsLineEdit->text().toStdString();
    if (targetGpusString.empty())
    {
        // The Target GPUs field is invalid since it is empty.
        errors.push_back(STR_ERR_TARGET_GPUS_CANNOT_BE_EMPTY);
        isValid = false;
    }
    else
    {
        // Split the comma-separated GPUs string.
        std::vector<std::string> targetGPUsVector;
        rgUtils::splitString(targetGpusString, rgConfigManager::RGA_LIST_DELIMITER, targetGPUsVector);

        // Use the Config Manager to verify that the specified GPUs are valid.
        std::vector<std::string> invalidGpus;
        rgConfigManager& configManager = rgConfigManager::Instance();
        for (const std::string& targetGpuFamilyName : targetGPUsVector)
        {
            std::string trimmedName;
            rgUtils::TrimLeadingAndTrailingWhitespace(targetGpuFamilyName, trimmedName);

            // Is the given target GPU family name supported?
            if (!configManager.IsGpuFamilySupported(trimmedName))
            {
                // Add the GPU to a list of invalid names if it's not supported.
                invalidGpus.push_back(trimmedName);
            }
        }

        if (!invalidGpus.empty())
        {
            // Build an error string indicating the invalid GPUs.
            std::stringstream errorStream;
            errorStream << STR_ERR_INVALID_GPUS_SPECIFIED;

            int numInvalid = static_cast<int>(invalidGpus.size());
            for (int gpuIndex = 0; gpuIndex < numInvalid; ++gpuIndex)
            {
                errorStream << invalidGpus[gpuIndex];

                if (gpuIndex != (numInvalid - 1))
                {
                    errorStream << ", ";
                }
            }

            // Add an error indicating that the GPU name is invalid.
            errors.push_back(errorStream.str());
            isValid = false;
        }
    }

    return isValid;
}

bool rgBuildSettingsViewVulkan::ValidatePendingSettings()
{
    std::vector<std::string> errorFields;
    bool isValid = IsTargetGpusStringValid(errorFields);
    if (!isValid)
    {
        std::stringstream errorStream;
        errorStream << STR_ERR_INVALID_PENDING_SETTING;
        errorStream << std::endl;

        // Loop through all errors and append them to the stream.
        for (const std::string error : errorFields)
        {
            errorStream << error;
            errorStream << std::endl;
        }

        // Display an error message box to the user telling them what to fix.
        rgUtils::ShowErrorMessageBox(errorStream.str().c_str(), this);
    }

    return isValid;
}

void rgBuildSettingsViewVulkan::SetCursor()
{
    // Set the cursor for buttons to pointing hand cursor.
    ui.addTargetGPUsButton->setCursor(Qt::PointingHandCursor);
    ui.includeDirsBrowseButton->setCursor(Qt::PointingHandCursor);
    ui.predefinedMacrosBrowseButton->setCursor(Qt::PointingHandCursor);
    ui.ICDLocationBrowseButton->setCursor(Qt::PointingHandCursor);
    ui.compilerBrowseButton->setCursor(Qt::PointingHandCursor);

    // Set the cursor for check boxes to pointing hand cursor.
    ui.generateDebugInfoCheckBox->setCursor(Qt::PointingHandCursor);
    ui.noExplicitBindingsCheckBox->setCursor(Qt::PointingHandCursor);
    ui.useHLSLBlockOffsetsCheckBox->setCursor(Qt::PointingHandCursor);
    ui.useHLSLIOMappingCheckBox->setCursor(Qt::PointingHandCursor);
    ui.enableValidationLayersCheckBox->setCursor(Qt::PointingHandCursor);
}

void rgBuildSettingsViewVulkan::HandleIncludeDirsBrowseButtonClick()
{
    // Set the frame border color to red.
    emit SetFrameBorderRedSignal();

    // Position the window in the middle of the screen.
    m_pIncludeDirectoriesView->setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, m_pIncludeDirectoriesView->size(), qApp->desktop()->availableGeometry()));

    // Set the current include dirs.
    m_pIncludeDirectoriesView->SetListItems(ui.includeDirectoriesLineEdit->text());

    // Show the window.
    m_pIncludeDirectoriesView->exec();
}

void rgBuildSettingsViewVulkan::HandleIncludeDirsUpdated(QStringList includeDirs)
{
    QString includeDirsText;

    // Create a delimiter-separated string.
    if (!includeDirs.isEmpty())
    {
        includeDirsText = includeDirs.join(s_OPTIONS_LIST_DELIMITER);
    }

    // Update the text box.
    ui.includeDirectoriesLineEdit->setText(includeDirsText);

    // Inform the rest of the UI that the settings have been changed.
    HandlePendingChangesStateChanged(GetHasPendingChanges());
}

void rgBuildSettingsViewVulkan::HandlePreprocessorDirectivesBrowseButtonClick()
{
    // Set the frame border color to red.
    emit SetFrameBorderRedSignal();

    // Position the window in the middle of the screen.
    m_pPreprocessorDirectivesDialog->setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, m_pPreprocessorDirectivesDialog->size(), qApp->desktop()->availableGeometry()));

    // Set the current preprocessor directives in the dialog.
    m_pPreprocessorDirectivesDialog->SetListItems(ui.predefinedMacrosLineEdit->text());

    // Show the dialog.
    m_pPreprocessorDirectivesDialog->exec();
}

void rgBuildSettingsViewVulkan::HandlePreprocessorDirectivesUpdated(QStringList preprocessorDirectives)
{
    QString preprocessorDirectivesText;

    // Create a delimiter-separated string.
    if (!preprocessorDirectives.isEmpty())
    {
        preprocessorDirectivesText = preprocessorDirectives.join(s_OPTIONS_LIST_DELIMITER);
    }

    // Update the text box.
    ui.predefinedMacrosLineEdit->setText(preprocessorDirectivesText);

    // Inform the rest of the UI that the settings have been changed.
    HandlePendingChangesStateChanged(GetHasPendingChanges());
}

void rgBuildSettingsViewVulkan::HandleLineEditFocusInEvent()
{
    emit SetFrameBorderRedSignal();
}

void rgBuildSettingsViewVulkan::HandleLineEditFocusOutEvent()
{
    emit SetFrameBorderBlackSignal();
}

void rgBuildSettingsViewVulkan::HandleBrowseButtonFocusInEvent()
{
    emit SetFrameBorderRedSignal();
}

void rgBuildSettingsViewVulkan::HandleBrowseButtonFocusOutEvent()
{
    emit SetFrameBorderBlackSignal();
}

void rgBuildSettingsViewVulkan::HandleCheckBoxFocusInEvent()
{
    emit SetFrameBorderRedSignal();
}

void rgBuildSettingsViewVulkan::HandleCheckBoxFocusOutEvent()
{
    emit SetFrameBorderBlackSignal();
}

void rgBuildSettingsViewVulkan::HandleCheckBoxClickedEvent()
{
    emit SetFrameBorderRedSignal();
}

void rgBuildSettingsViewVulkan::HandleCheckboxStateChanged()
{
    // Make sure it was a checkbox that caused this state change (just to be sure).
    QCheckBox* pCheckBox = static_cast<QCheckBox*>(QObject::sender());
    assert(pCheckBox != nullptr);
    if (pCheckBox != nullptr)
    {
        // Inform the UI of a possible change to the pending state.
        HandlePendingChangesStateChanged(GetHasPendingChanges());

        // Update the command line preview text.
        UpdateCommandLineText();
    }
}

bool rgBuildSettingsViewVulkan::GetHasPendingChanges() const
{
    rgBuildSettingsVulkan currentSettings = PullFromWidgets();

    bool hasChanges = !m_initialSettings.HasSameSettings(currentSettings);

    return hasChanges;
}

bool rgBuildSettingsViewVulkan::RevertPendingChanges()
{
    PushToWidgets(m_initialSettings);

    // Make sure the rest of the UI knows that the settings don't need to be saved.
    HandlePendingChangesStateChanged(false);

    return false;
}

void rgBuildSettingsViewVulkan::RestoreDefaultSettings()
{
    bool isRestored = false;

    if (m_isGlobalSettings)
    {
        // If this is for the global settings, then restore to the hard-coded defaults.

        // Get the hardcoded default build settings.
        std::shared_ptr<rgBuildSettings> pDefaultBuildSettings = rgConfigManager::GetDefaultBuildSettings(rgProjectAPI::Vulkan);

        std::shared_ptr<rgBuildSettingsVulkan> pApiBuildSettings = std::dynamic_pointer_cast<rgBuildSettingsVulkan>(pDefaultBuildSettings);
        assert(pApiBuildSettings != nullptr);
        if (pApiBuildSettings != nullptr)
        {
            // Reset our initial settings back to the defaults.
            m_initialSettings = *pApiBuildSettings;

            // Update the UI to reflect the new initial settings.
            PushToWidgets(m_initialSettings);

            // Update the ConfigManager to use the new settings.
            rgConfigManager::Instance().SetApiBuildSettings(STR_API_NAME_VULKAN, &m_initialSettings);

            // Save the settings file.
            isRestored = rgConfigManager::Instance().SaveGlobalConfigFile();

            // Inform the rest of the UI that the settings have been changed.
            HandlePendingChangesStateChanged(GetHasPendingChanges());
        }
    }
    else
    {
        // This view is showing project-specific settings, so restore back to the stored settings in the project.
        std::shared_ptr<rgBuildSettings> pDefaultSettings = rgConfigManager::Instance().GetUserGlobalBuildSettings(rgProjectAPI::Vulkan);
        auto pVkDefaultSettings = std::dynamic_pointer_cast<rgBuildSettingsVulkan>(pDefaultSettings);

        m_initialSettings = *pVkDefaultSettings;

        PushToWidgets(m_initialSettings);

        // Inform the rest of the UI that the settings have been changed.
        HandlePendingChangesStateChanged(GetHasPendingChanges());

        // Let the rgBuildView know that the build settings have been updated.
        emit ProjectBuildSettingsSaved(pVkDefaultSettings);
        isRestored = true;
    }

    // Show an error dialog if the settings failed to be reset.
    if (!isRestored)
    {
        rgUtils::ShowErrorMessageBox(STR_ERR_CANNOT_RESTORE_DEFAULT_SETTINGS, this);
    }
}

bool rgBuildSettingsViewVulkan::SaveSettings()
{
    bool canBeSaved = ValidatePendingSettings();
    if (canBeSaved)
    {
        // Reset the initial settings to match what the UI shows.
        m_initialSettings = PullFromWidgets();

        if (m_isGlobalSettings)
        {
            // Update the config manager to use these new settings.
            rgConfigManager& configManager = rgConfigManager::Instance();
            configManager.SetApiBuildSettings(STR_API_NAME_VULKAN, &m_initialSettings);

            // Save the global config settings.
            canBeSaved = configManager.SaveGlobalConfigFile();
        }
        else
        {
            // Save the project settings.
            std::shared_ptr<rgBuildSettingsVulkan> pTmpPtr = std::make_shared<rgBuildSettingsVulkan>(m_initialSettings);
            emit ProjectBuildSettingsSaved(pTmpPtr);
        }

        if (canBeSaved)
        {
            // Make sure the rest of the UI knows that the settings have been saved.
            HandlePendingChangesStateChanged(false);
        }
    }

    // Set focus to target GPUs browse button.
    ui.addTargetGPUsButton->setFocus();

    return canBeSaved;
}

void rgBuildSettingsViewVulkan::mousePressEvent(QMouseEvent* pEvent)
{
    Q_UNUSED(pEvent);

    emit SetFrameBorderRedSignal();
}

bool rgBuildSettingsViewVulkan::eventFilter(QObject* pObject, QEvent* pEvent)
{
    // Intercept events for "All Options" widget.
    if (pEvent != nullptr)
    {
        if (pEvent->type() == QEvent::FocusIn)
        {
            HandleLineEditFocusInEvent();
        }
        else if (pEvent->type() == QEvent::FocusOut)
        {
            HandleLineEditFocusOutEvent();
        }
    }

    // Continue default processing.
    return QObject::eventFilter(pObject, pEvent);
}

void rgBuildSettingsViewVulkan::SetInitialWidgetFocus()
{
    ui.addTargetGPUsButton->setFocus();
}