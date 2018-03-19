// C++.
#include <cassert>
#include <sstream>

// Qt.
#include <QWidget>
#include <QMessageBox>
#include <QDesktopWidget>
#include <QFileDialog>

// Infra.
#include <QtCommon/Util/RestoreCursorPosition.h>
#include <QtCommon/Scaling/ScalingManager.h>
#include <Utils/include/rgaCliDefs.h>

// Local.
#include <RadeonGPUAnalyzerGUI/include/qt/rgBuildSettingsView.h>
#include <RadeonGPUAnalyzerGUI/include/qt/rgCheckBox.h>
#include <RadeonGPUAnalyzerGUI/include/qt/rgIncludeDirectoriesView.h>
#include <RadeonGPUAnalyzerGUI/include/qt/rgLineEdit.h>
#include <RadeonGPUAnalyzerGUI/include/qt/rgOpenCLBuildSettingsModel.h>
#include <RadeonGPUAnalyzerGUI/include/qt/rgOpenCLBuildSettingsView.h>
#include <RadeonGPUAnalyzerGUI/include/qt/rgPreprocessorDirectivesDialog.h>
#include <RadeonGPUAnalyzerGUI/include/rgCliUtils.h>
#include <RadeonGPUAnalyzerGUI/include/rgConfigManager.h>
#include <RadeonGPUAnalyzerGUI/include/rgStringConstants.h>
#include <RadeonGPUAnalyzerGUI/include/rgUtils.h>

// Checkbox tool tip stylesheets.
static const char* s_STR_FILEMENU_TITLE_BAR_TOOLTIP_WIDTH = "min-width: %1px; width: %2px;";
static const char* s_STR_FILEMENU_TITLE_BAR_TOOLTIP_HEIGHT = "min-height: %1px; height: %2px; max-height: %3px;";

// The delimiter to use to join and split a string of option items.
static const char* s_OPTIONS_LIST_DELIMITER = ";";

rgOpenCLBuildSettingsView::rgOpenCLBuildSettingsView(QWidget* pParent, std::shared_ptr<rgCLBuildSettings> pBuildSettings, bool isGlobalSettings) :
    rgBuildSettingsView(pParent, isGlobalSettings),
    m_isGlobalSettings(isGlobalSettings)
{
    // Setup the UI.
    ui.setupUi(this);

    // Set the background to white.
    QPalette pal = palette();
    pal.setColor(QPalette::Background, Qt::white);
    this->setAutoFillBackground(true);
    this->setPalette(pal);

    // Make sure that the build settings that we received are valid.
    assert(pBuildSettings);

    // Create the include directories editor view.
    m_pIncludeDirectoriesView = new rgIncludeDirectoriesView(s_OPTIONS_LIST_DELIMITER, this);

    // Create the preprocessor directives editor dialog.
    m_pPreprocessorDirectivesDialog = new rgPreprocessorDirectivesDialog(s_OPTIONS_LIST_DELIMITER, this);

    m_pSettingsModel = new rgOpenCLBuildSettingsModel(pBuildSettings, rgOpenCLBuildSettingsControls::Count);

    // Initialize the model, bind it to the view, and populate default values.
    InitializeModel();

    // Connect the signals.
    ConnectSignals();

    // Connect focus in/out events for all of the line edits.
    ConnectLineEditFocusEvents();

    // Connect focus in/out events for all of the checkboxes.
    ConnectCheckBoxClickedEvents();

    // Connect focus in event for the combobox.
    ConnectComboboxClickEvent();

    // Initialize the model with sensible default values based on the incoming build settings.
    m_pSettingsModel->InitializeModelValues();

    // Initialize the command line preview text.
    UpdateCommandLineText();

    // Initialize the "Additional options" content.
    SetAdditionalOptionsText(pBuildSettings->m_additionalOptions);

    // Set tooltips for general items.
    ui.targetGPUsLabel->setToolTip(STR_BUILD_SETTINGS_TARGET_GPUS_TOOLTIP);
    ui.predefinedMacrosLabel->setToolTip(STR_BUILD_SETTINGS_PREDEFINED_MACROS_TOOLTIP);
    ui.includeDirectoriesDevicesLabel->setToolTip(STR_BUILD_SETTINGS_ADDITIONAL_INC_DIRECTORY_TOOLTIP);

    // Set tooltips for alternative compiler components.
    ui.alternativeCompilerLabel->setToolTip(STR_BUILD_SETTINGS_ALTERNATIVE_COMPILER_TOOLTIP);
    ui.compilerBinariesLabel->setToolTip(CLI_DESC_ALTERNATIVE_BIN_FOLDER);
    ui.compilerIncludesLabel->setToolTip(CLI_DESC_ALTERNATIVE_INC_FOLDER);
    ui.compilerLibrariesLabel->setToolTip(CLI_DESC_ALTERNATIVE_LIB_FOLDER);

    // Set tooltip for the optimization level item.
    ui.optimizationLevelLabel->setToolTip(STR_BUILD_SETTINGS_OPTIMIZATION_LEVEL);

    // Set tooltips for the Settings command line section.
    ui.settingsCommandLineHeaderLabel->setToolTip(STR_BUILD_SETTINGS_SETTINGS_CMDLINE_TOOLTIP);

    // Set tooltips for the Additional options line section.
    ui.additionalOptionsHeaderLabel->setToolTip(STR_BUILD_SETTINGS_CLANG_OPTIONS_TOOLTIP);

    // Set the mouse cursor to the pointing hand cursor for various widgets.
    SetCursor();

    // Set the geometry of tool tip boxes.
    SetToolTipGeometry();

    // Set the event filter for "Additional Options" and "All Options" text edits.
    ui.additionalOptionsTextEdit->installEventFilter(this);
    ui.allOptionsTextEdit->installEventFilter(this);
}

void rgOpenCLBuildSettingsView::ConnectSignals()
{
    // Add target GPU button.
    bool isConnected = connect(this->ui.addTargetGPUsButton, &QPushButton::clicked, this, &rgOpenCLBuildSettingsView::HandleAddTargetGpusButtonClick);
    assert(isConnected);

    // Add include directories editor dialog button.
    isConnected = connect(this->ui.includeDirsBrowseButton, &QPushButton::clicked, this, &rgOpenCLBuildSettingsView::HandleIncludeDirsBrowseButtonClick);
    assert(isConnected);

    // Add preprocessor directives editor dialog button.
    isConnected = connect(this->ui.predefinedMacrosBrowseButton, &QPushButton::clicked, this, &rgOpenCLBuildSettingsView::HandlePreprocessorDirectivesBrowseButtonClick);
    assert(isConnected);

    // Connect all textboxes within the view.
    isConnected = connect(this->ui.targetGPUsLineEdit, &QLineEdit::textChanged, this, &rgOpenCLBuildSettingsView::HandleTextEditChanged);
    assert(isConnected);

    isConnected = connect(this->ui.predefinedMacrosLineEdit, &QLineEdit::textChanged, this, &rgOpenCLBuildSettingsView::HandleTextEditChanged);
    assert(isConnected);

    isConnected = connect(this->ui.includeDirectoriesLineEdit, &QLineEdit::textChanged, this, &rgOpenCLBuildSettingsView::HandleTextEditChanged);
    assert(isConnected);

    isConnected = connect(this->ui.optimizationLevelComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &rgOpenCLBuildSettingsView::HandleComboboxIndexChanged);
    assert(isConnected);

    isConnected = connect(this->ui.doubleAsSingleCheckBox, &rgCheckBox::stateChanged, this, &rgOpenCLBuildSettingsView::HandleCheckboxStateChanged);
    assert(isConnected);

    isConnected = connect(this->ui.flushDenormalizedCheckBox, &rgCheckBox::stateChanged, this, &rgOpenCLBuildSettingsView::HandleCheckboxStateChanged);
    assert(isConnected);

    isConnected = connect(this->ui.strictAliasingCheckBox, &rgCheckBox::stateChanged, this, &rgOpenCLBuildSettingsView::HandleCheckboxStateChanged);
    assert(isConnected);

    isConnected = connect(this->ui.enableMADCheckBox, &rgCheckBox::stateChanged, this, &rgOpenCLBuildSettingsView::HandleCheckboxStateChanged);
    assert(isConnected);

    isConnected = connect(this->ui.ignoreZeroSignednessCheckBox, &rgCheckBox::stateChanged, this, &rgOpenCLBuildSettingsView::HandleCheckboxStateChanged);
    assert(isConnected);

    isConnected = connect(this->ui.allowUnsafeOptimizationsCheckBox, &rgCheckBox::stateChanged, this, &rgOpenCLBuildSettingsView::HandleCheckboxStateChanged);
    assert(isConnected);

    isConnected = connect(this->ui.assumeNoNanNorInfiniteCheckBox, &rgCheckBox::stateChanged, this, &rgOpenCLBuildSettingsView::HandleCheckboxStateChanged);
    assert(isConnected);

    isConnected = connect(this->ui.aggressiveMathOptimizationsCheckBox, &rgCheckBox::stateChanged, this, &rgOpenCLBuildSettingsView::HandleCheckboxStateChanged);
    assert(isConnected);

    isConnected = connect(this->ui.correctlyRoundSinglePercisionCheckBox, &rgCheckBox::stateChanged, this, &rgOpenCLBuildSettingsView::HandleCheckboxStateChanged);
    assert(isConnected);

    // Connect "Additional Options" text box.
    isConnected = connect(this->ui.additionalOptionsTextEdit, &QPlainTextEdit::textChanged, this, &rgOpenCLBuildSettingsView::HandleAdditionalOptionsTextChanged);
    assert(isConnected);

    // Connect Alternative Compiler folders browse buttons.
    isConnected = connect(this->ui.compilerBinariesBrowseButton, &QPushButton::clicked, this, [&]() { HandleCompilerFolderBrowseButtonClick(CompilerFolderType::Bin); });
    assert(isConnected);
    isConnected = connect(this->ui.compilerIncludesBrowseButton, &QPushButton::clicked, this, [&]() { HandleCompilerFolderBrowseButtonClick(CompilerFolderType::Include); });
    assert(isConnected);
    isConnected = connect(this->ui.compilerLibrariesBrowseButton, &QPushButton::clicked, this, [&]() { HandleCompilerFolderBrowseButtonClick(CompilerFolderType::Lib); });
    assert(isConnected);

    // Connect Alternative Compiler folders text edits.
    isConnected = connect(this->ui.compilerBinariesLineEdit, &QLineEdit::textChanged, this, [&]() { HandleCompilerFolderEditChanged(CompilerFolderType::Bin); });
    assert(isConnected);
    isConnected = connect(this->ui.compilerIncludesLineEdit, &QLineEdit::textChanged, this, [&]() { HandleCompilerFolderEditChanged(CompilerFolderType::Include); });
    assert(isConnected);
    isConnected = connect(this->ui.compilerLibrariesLineEdit, &QLineEdit::textChanged, this, [&]() { HandleCompilerFolderEditChanged(CompilerFolderType::Lib); });
    assert(isConnected);

    // Connect the model's pending changes state handler.
    assert(m_pSettingsModel != nullptr);
    if (m_pSettingsModel != nullptr)
    {
        isConnected = connect(m_pSettingsModel, &rgOpenCLBuildSettingsModel::PendingChangesStateChanged, this, &rgOpenCLBuildSettingsView::HandlePendingChangesStateChanged);
        assert(isConnected);
    }

    // Connect the include directory editor dialog's "OK" button click.
    isConnected = connect(m_pIncludeDirectoriesView, &rgIncludeDirectoriesView::OKButtonClicked, this, &rgOpenCLBuildSettingsView::HandleIncludeDirsUpdated);
    assert(isConnected);

    // Connect the preprocessor directives editor dialog's "OK" button click.
    isConnected = connect(m_pPreprocessorDirectivesDialog, &rgPreprocessorDirectivesDialog::OKButtonClicked, this, &rgOpenCLBuildSettingsView::HandlePreprocessorDirectivesUpdated);
    assert(isConnected);
}

void rgOpenCLBuildSettingsView::ConnectLineEditFocusEvents()
{
    bool isConnected = false;

    isConnected = connect(this->ui.includeDirectoriesLineEdit, &rgLineEdit::LineEditFocusInEvent, this, &rgOpenCLBuildSettingsView::HandleLineEditFocusInEvent);
    assert(isConnected);

    isConnected = connect(this->ui.includeDirectoriesLineEdit, &rgLineEdit::LineEditFocusOutEvent, this, &rgOpenCLBuildSettingsView::HandleLineEditFocusOutEvent);
    assert(isConnected);

    isConnected = connect(this->ui.predefinedMacrosLineEdit, &rgLineEdit::LineEditFocusInEvent, this, &rgOpenCLBuildSettingsView::HandleLineEditFocusInEvent);
    assert(isConnected);

    isConnected = connect(this->ui.predefinedMacrosLineEdit, &rgLineEdit::LineEditFocusOutEvent, this, &rgOpenCLBuildSettingsView::HandleLineEditFocusOutEvent);
    assert(isConnected);

    isConnected = connect(this->ui.targetGPUsLineEdit, &rgLineEdit::LineEditFocusInEvent, this, &rgOpenCLBuildSettingsView::HandleLineEditFocusInEvent);
    assert(isConnected);

    isConnected = connect(this->ui.targetGPUsLineEdit, &rgLineEdit::LineEditFocusOutEvent, this, &rgOpenCLBuildSettingsView::HandleLineEditFocusOutEvent);
    assert(isConnected);

    isConnected = connect(this->ui.compilerBinariesLineEdit, &rgLineEdit::LineEditFocusInEvent, this, &rgOpenCLBuildSettingsView::HandleLineEditFocusInEvent);
    assert(isConnected);

    isConnected = connect(this->ui.compilerBinariesLineEdit, &rgLineEdit::LineEditFocusOutEvent, this, &rgOpenCLBuildSettingsView::HandleLineEditFocusOutEvent);
    assert(isConnected);

    isConnected = connect(this->ui.compilerIncludesLineEdit, &rgLineEdit::LineEditFocusInEvent, this, &rgOpenCLBuildSettingsView::HandleLineEditFocusInEvent);
    assert(isConnected);

    isConnected = connect(this->ui.compilerIncludesLineEdit, &rgLineEdit::LineEditFocusOutEvent, this, &rgOpenCLBuildSettingsView::HandleLineEditFocusOutEvent);
    assert(isConnected);

    isConnected = connect(this->ui.compilerLibrariesLineEdit, &rgLineEdit::LineEditFocusInEvent, this, &rgOpenCLBuildSettingsView::HandleLineEditFocusInEvent);
    assert(isConnected);

    isConnected = connect(this->ui.compilerLibrariesLineEdit, &rgLineEdit::LineEditFocusOutEvent, this, &rgOpenCLBuildSettingsView::HandleLineEditFocusOutEvent);
    assert(isConnected);
}

void rgOpenCLBuildSettingsView::ConnectCheckBoxClickedEvents()
{
    bool isConnected = false;

    isConnected = connect(this->ui.aggressiveMathOptimizationsCheckBox, &rgCheckBox::clicked, this, &rgOpenCLBuildSettingsView::HandleCheckBoxClickedEvent);
    assert(isConnected);

    isConnected = connect(this->ui.allowUnsafeOptimizationsCheckBox, &rgCheckBox::clicked, this, &rgOpenCLBuildSettingsView::HandleCheckBoxClickedEvent);
    assert(isConnected);

    isConnected = connect(this->ui.assumeNoNanNorInfiniteCheckBox, &rgCheckBox::clicked, this, &rgOpenCLBuildSettingsView::HandleCheckBoxClickedEvent);
    assert(isConnected);

    isConnected = connect(this->ui.correctlyRoundSinglePercisionCheckBox, &rgCheckBox::clicked, this, &rgOpenCLBuildSettingsView::HandleCheckBoxClickedEvent);
    assert(isConnected);

    isConnected = connect(this->ui.doubleAsSingleCheckBox, &rgCheckBox::clicked, this, &rgOpenCLBuildSettingsView::HandleCheckBoxClickedEvent);
    assert(isConnected);

    isConnected = connect(this->ui.enableMADCheckBox, &rgCheckBox::clicked, this, &rgOpenCLBuildSettingsView::HandleCheckBoxClickedEvent);
    assert(isConnected);

    isConnected = connect(this->ui.flushDenormalizedCheckBox, &rgCheckBox::clicked, this, &rgOpenCLBuildSettingsView::HandleCheckBoxClickedEvent);
    assert(isConnected);

    isConnected = connect(this->ui.ignoreZeroSignednessCheckBox, &rgCheckBox::clicked, this, &rgOpenCLBuildSettingsView::HandleCheckBoxClickedEvent);
    assert(isConnected);

    isConnected = connect(this->ui.strictAliasingCheckBox, &rgCheckBox::clicked, this, &rgOpenCLBuildSettingsView::HandleCheckBoxClickedEvent);
    assert(isConnected);
}

void rgOpenCLBuildSettingsView::ConnectComboboxClickEvent()
{
    bool isConnected = connect(this->ui.optimizationLevelComboBox, &rgComboBox::ComboBoxFocusInEvent, this, &rgOpenCLBuildSettingsView::HandleComboBoxFocusInEvent);
    assert(isConnected);
}

bool rgOpenCLBuildSettingsView::eventFilter(QObject * obj, QEvent * evnt)
{
    // Intercept events for "Additional "Options" widget.
    if (evnt != nullptr)
    {
        if (evnt->type() == QEvent::FocusIn)
        {
            HandleLineEditFocusInEvent();
        }
        else if (evnt->type() == QEvent::FocusOut)
        {
            HandleLineEditFocusOutEvent();
        }
    }

    // Continue default processing.
    return QObject::eventFilter(obj, evnt);
}

void rgOpenCLBuildSettingsView::HandleAddTargetGpusButtonClick()
{
    // Trim out any spaces within the target GPUs string.
    QString selectedGPUs = this->ui.targetGPUsLineEdit->text();

    // Create a new Target GPU Selection dialog instance.
    m_pTargetGpusDialog = new rgTargetGpusDialog(selectedGPUs, this);

    // Present the dialog to the user.
    m_pTargetGpusDialog->setModal(true);
    int dialogResult = m_pTargetGpusDialog->exec();

    // If the user clicked "OK", extract the Checked items into a comma-separated string,
    // and put the string in the Target GPUs textbox.
    if (dialogResult == 1)
    {
        // After the dialog is hidden, extract the list of families selected by the user.
        std::vector<std::string> selectedFamilies = m_pTargetGpusDialog->GetSelectedCapabilityGroups();

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

        // Set the target GPUs text by updating the model.
        QVariant gpuFamilyValue = QVariant::fromValue<QString>(familyListString.str().c_str());
        m_pSettingsModel->UpdateModelValue(rgOpenCLBuildSettingsControls::TargetGPUs, gpuFamilyValue, false);
    }
}

void rgOpenCLBuildSettingsView::HandleTextEditChanged()
{
    // Determine which control's text has been updated.
    QLineEdit* pLineEdit = static_cast<QLineEdit*>(QObject::sender());
    assert(pLineEdit);
    if (pLineEdit)
    {
        // Restore the cursor to the original position when the text has changed.
        QtCommon::QtUtil::RestoreCursorPosition cursorPosition(pLineEdit);

        // Find the control's ID, and then send the updated value.
        int id = m_pSettingsModel->GetMappedWidgetId(pLineEdit);
        rgOpenCLBuildSettingsControls objectId = static_cast<rgOpenCLBuildSettingsControls>(id);
        m_pSettingsModel->UpdateModelValue(objectId, pLineEdit->text(), false);

        // Update the command line preview text.
        UpdateCommandLineText();
    }
}

void rgOpenCLBuildSettingsView::HandleComboboxIndexChanged(int index)
{
    // Determine which control's text has been updated.
    QComboBox *pComboBox = static_cast<QComboBox*>(QObject::sender());
    assert(pComboBox);
    if (pComboBox)
    {
        // Find the control's ID, and then send the updated value.
        int id = m_pSettingsModel->GetMappedWidgetId(pComboBox);
        rgOpenCLBuildSettingsControls objectId = static_cast<rgOpenCLBuildSettingsControls>(id);
        const QString& selectedText = pComboBox->itemText(index);
        m_pSettingsModel->UpdateModelValue(objectId, selectedText, false);

        // Update the command line preview text.
        UpdateCommandLineText();
    }
}

void rgOpenCLBuildSettingsView::HandleCheckboxStateChanged()
{
    // Determine which control's text has been updated.
    QCheckBox* pCheckBox = static_cast<QCheckBox*>(QObject::sender());
    assert(pCheckBox);
    if (pCheckBox)
    {
        // Find the control's ID, and then send the updated value.
        int id = m_pSettingsModel->GetMappedWidgetId(pCheckBox);
        rgOpenCLBuildSettingsControls objectId = static_cast<rgOpenCLBuildSettingsControls>(id);

        m_pSettingsModel->UpdateModelValue(objectId, pCheckBox->isChecked(), false);

        // Update the command line preview text.
        UpdateCommandLineText();
    }
}

const std::string rgOpenCLBuildSettingsView::GetTitleString() const
{
    std::stringstream titleString;

    // Build the title string.
    if (m_isGlobalSettings)
    {
        // For the global settings.
        titleString << STR_BUILD_SETTINGS_DEFAULT_TITLE;
        titleString << " ";
        titleString << STR_API_NAME_OPENCL;
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

const std::string rgOpenCLBuildSettingsView::GetTitleTooltipString() const
{
    std::stringstream tooltipString;

    if (m_isGlobalSettings)
    {
        tooltipString << STR_BUILD_SETTINGS_GLOBAL_TOOLTIP_A;
        tooltipString << STR_API_NAME_OPENCL;
        tooltipString << STR_BUILD_SETTINGS_GLOBAL_TOOLTIP_B;
    }
    else
    {
        tooltipString << STR_BUILD_SETTINGS_PROJECT_TOOLTIP_A;
        tooltipString << STR_API_NAME_OPENCL;
        tooltipString << STR_BUILD_SETTINGS_PROJECT_TOOLTIP_B;
    }

    return tooltipString.str();
}

void rgOpenCLBuildSettingsView::HandlePendingChangesStateChanged(bool hasPendingChanges)
{
    emit PendingChangesStateChanged(hasPendingChanges);
}

void rgOpenCLBuildSettingsView::InitializeModel()
{
    // Associate a widget with an identifying enum, and specify which property of that widget is registered to the model's data.
    m_pSettingsModel->InitializeModel(ui.targetGPUsLineEdit, rgOpenCLBuildSettingsControls::TargetGPUs, "text");
    m_pSettingsModel->InitializeModel(ui.predefinedMacrosLineEdit, rgOpenCLBuildSettingsControls::PredefinedMacros, "text");
    m_pSettingsModel->InitializeModel(ui.includeDirectoriesLineEdit, rgOpenCLBuildSettingsControls::AdditionalIncludeDirectories, "text");
    m_pSettingsModel->InitializeModel(ui.optimizationLevelComboBox, rgOpenCLBuildSettingsControls::OptimizationLevel, "currentText");
    m_pSettingsModel->InitializeModel(ui.doubleAsSingleCheckBox, rgOpenCLBuildSettingsControls::TreatDoubleFloatingPointAsSingle, "checked");
    m_pSettingsModel->InitializeModel(ui.flushDenormalizedCheckBox, rgOpenCLBuildSettingsControls::FlushDenormalizedFloatsAsZero, "checked");
    m_pSettingsModel->InitializeModel(ui.strictAliasingCheckBox, rgOpenCLBuildSettingsControls::AssumeStrictAliasingRules, "checked");
    m_pSettingsModel->InitializeModel(ui.enableMADCheckBox, rgOpenCLBuildSettingsControls::EnableMADInstructions, "checked");
    m_pSettingsModel->InitializeModel(ui.ignoreZeroSignednessCheckBox, rgOpenCLBuildSettingsControls::IgnoreSignednessOfZeros, "checked");
    m_pSettingsModel->InitializeModel(ui.allowUnsafeOptimizationsCheckBox, rgOpenCLBuildSettingsControls::AllowUnsafeOptimizations, "checked");
    m_pSettingsModel->InitializeModel(ui.assumeNoNanNorInfiniteCheckBox, rgOpenCLBuildSettingsControls::AssumeNoNaNNorInfinite, "checked");
    m_pSettingsModel->InitializeModel(ui.aggressiveMathOptimizationsCheckBox, rgOpenCLBuildSettingsControls::AgressiveMathOptimizations, "checked");
    m_pSettingsModel->InitializeModel(ui.correctlyRoundSinglePercisionCheckBox, rgOpenCLBuildSettingsControls::CorrectlyRoundSingleDivideAndSqrt, "checked");
    m_pSettingsModel->InitializeModel(ui.additionalOptionsTextEdit, rgOpenCLBuildSettingsControls::AdditionalOptions, "text");
    m_pSettingsModel->InitializeModel(ui.compilerBinariesLineEdit, rgOpenCLBuildSettingsControls::AlternativeCompilerBinDir, "text");
    m_pSettingsModel->InitializeModel(ui.compilerIncludesLineEdit, rgOpenCLBuildSettingsControls::AlternativeCompilerIncDir, "text");
    m_pSettingsModel->InitializeModel(ui.compilerLibrariesLineEdit, rgOpenCLBuildSettingsControls::AlternativeCompilerLibDir, "text");
}

void rgOpenCLBuildSettingsView::UpdateCommandLineText()
{
    std::shared_ptr<rgCLBuildSettings> pBuildSettings = m_pSettingsModel->GetBuildSettings();
    if (m_pSettingsModel->HasPendingChanges())
    {
        pBuildSettings = m_pSettingsModel->GetPendingBuildSettings();
    }

    // Generate a command line string from the build settings structure.
    std::string buildSettings;
    bool ret = rgCliUtils::GenerateBuildSettingsString(pBuildSettings, buildSettings, false);
    assert(ret);
    if (ret)
    {
        ui.allOptionsTextEdit->setPlainText(buildSettings.c_str());
    }
}

void rgOpenCLBuildSettingsView::SetAdditionalOptionsText(const std::string& text)
{
    std::shared_ptr<rgCLBuildSettings> pBuildSettings = m_pSettingsModel->GetBuildSettings();
    if (pBuildSettings != nullptr)
    {
        ui.additionalOptionsTextEdit->setPlainText(QString(text.c_str()));
    }
}

bool rgOpenCLBuildSettingsView::ValidatePendingSettings()
{
    std::vector<rgInvalidFieldInfo> errorFields;
    bool isValid = m_pSettingsModel->ValidatePendingSettings(errorFields);
    if (!isValid)
    {
        std::stringstream errorStream;
        errorStream << STR_ERR_INVALID_PENDING_SETTING;
        errorStream << std::endl;

        // Loop through all errors and append them to the stream.
        for (const rgInvalidFieldInfo fieldInfo : errorFields)
        {
            errorStream << fieldInfo.m_errorString;
            errorStream << std::endl;
        }

        // Display an error message box to the user telling them what to fix.
        rgUtils::ShowErrorMessageBox(errorStream.str().c_str());
    }

    return isValid;
}

void rgOpenCLBuildSettingsView::SetCursor()
{
    // Set the cursor to pointing hand cursor.
    ui.addTargetGPUsButton->setCursor(Qt::PointingHandCursor);
    ui.aggressiveMathOptimizationsCheckBox->setCursor(Qt::PointingHandCursor);
    ui.allowUnsafeOptimizationsCheckBox->setCursor(Qt::PointingHandCursor);
    ui.assumeNoNanNorInfiniteCheckBox->setCursor(Qt::PointingHandCursor);
    ui.correctlyRoundSinglePercisionCheckBox->setCursor(Qt::PointingHandCursor);
    ui.doubleAsSingleCheckBox->setCursor(Qt::PointingHandCursor);
    ui.enableMADCheckBox->setCursor(Qt::PointingHandCursor);
    ui.flushDenormalizedCheckBox->setCursor(Qt::PointingHandCursor);
    ui.ignoreZeroSignednessCheckBox->setCursor(Qt::PointingHandCursor);
    ui.strictAliasingCheckBox->setCursor(Qt::PointingHandCursor);
    ui.optimizationLevelComboBox->setCursor(Qt::PointingHandCursor);
    ui.includeDirsBrowseButton->setCursor(Qt::PointingHandCursor);
    ui.predefinedMacrosBrowseButton->setCursor(Qt::PointingHandCursor);

    // Set the cursor for alternative compiler buttons.
    ui.compilerBinariesBrowseButton->setCursor(Qt::PointingHandCursor);
    ui.compilerIncludesBrowseButton->setCursor(Qt::PointingHandCursor);
    ui.compilerLibrariesBrowseButton->setCursor(Qt::PointingHandCursor);
}

void rgOpenCLBuildSettingsView::HandleIncludeDirsBrowseButtonClick()
{
    // Position the window in the middle of the screen.
    m_pIncludeDirectoriesView->setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, m_pIncludeDirectoriesView->size(), qApp->desktop()->availableGeometry()));

    // Set the current include dirs.
    m_pIncludeDirectoriesView->SetListItems(ui.includeDirectoriesLineEdit->text());

    // Show the window.
    m_pIncludeDirectoriesView->exec();
}

void rgOpenCLBuildSettingsView::HandleIncludeDirsUpdated(QStringList includeDirs)
{
    QString includeDirsText;

    // Create a delimiter-separated string.
    if (!includeDirs.isEmpty())
    {
        includeDirsText = includeDirs.join(s_OPTIONS_LIST_DELIMITER);
    }

    // Update the text box.
    ui.includeDirectoriesLineEdit->setText(includeDirsText);
}

void rgOpenCLBuildSettingsView::HandlePreprocessorDirectivesBrowseButtonClick()
{
    // Position the window in the middle of the screen.
    m_pPreprocessorDirectivesDialog->setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, m_pPreprocessorDirectivesDialog->size(), qApp->desktop()->availableGeometry()));

    // Set the current preprocessor directives in the dialog.
    m_pPreprocessorDirectivesDialog->SetListItems(ui.predefinedMacrosLineEdit->text());

    // Show the dialog.
    m_pPreprocessorDirectivesDialog->exec();
}

void rgOpenCLBuildSettingsView::HandlePreprocessorDirectivesUpdated(QStringList preprocessorDirectives)
{
    QString preprocessorDirectivesText;

    // Create a delimiter-separated string.
    if (!preprocessorDirectives.isEmpty())
    {
        preprocessorDirectivesText = preprocessorDirectives.join(s_OPTIONS_LIST_DELIMITER);
    }

    // Update the text box.
    ui.predefinedMacrosLineEdit->setText(preprocessorDirectivesText);
}

void rgOpenCLBuildSettingsView::HandleAdditionalOptionsTextChanged()
{
    m_pSettingsModel->UpdateModelValue(rgOpenCLBuildSettingsControls::AdditionalOptions, ui.additionalOptionsTextEdit->document()->toPlainText(), false);

    // Update the command line preview text.
    UpdateCommandLineText();
}

void rgOpenCLBuildSettingsView::SetToolTipGeometry()
{
    // Get the font metrics.
    QFontMetrics fontMetrics(ui.doubleAsSingleCheckBox->font());

    // Use the width of an edit box as width of the tooltip string.
    const int width = ui.includeDirectoriesLineEdit->width();

    // Calculate the height of the tooltip string.
    const int height = fontMetrics.height() * ScalingManager::Get().GetScaleFactor();

    // Create a width and a height string.
    const QString widthString = QString(s_STR_FILEMENU_TITLE_BAR_TOOLTIP_WIDTH).arg(width).arg(width);
    const QString heightString = QString(s_STR_FILEMENU_TITLE_BAR_TOOLTIP_HEIGHT).arg(height).arg(height).arg(height);

    // Set the stylesheet.
    ui.doubleAsSingleCheckBox->setStyleSheet("QToolTip {" + widthString + heightString + "}");
    ui.flushDenormalizedCheckBox->setStyleSheet("QToolTip {" + widthString + "}");
    ui.aggressiveMathOptimizationsCheckBox->setStyleSheet("QToolTip {" + widthString + "}");
    ui.allowUnsafeOptimizationsCheckBox->setStyleSheet("QToolTip {" + widthString + "}");
    ui.assumeNoNanNorInfiniteCheckBox->setStyleSheet("QToolTip {" + widthString + "}");
    ui.correctlyRoundSinglePercisionCheckBox->setStyleSheet("QToolTip {" + widthString + "}");
    ui.doubleAsSingleCheckBox->setStyleSheet("QToolTip {" + widthString + "}");
    ui.enableMADCheckBox->setStyleSheet("QToolTip {" + widthString + "}");
    ui.ignoreZeroSignednessCheckBox->setStyleSheet("QToolTip {" + widthString + "}");
    ui.strictAliasingCheckBox->setStyleSheet("QToolTip {" + widthString + "}");
    ui.additionalOptionsHeaderLabel->setStyleSheet("QToolTip {" + widthString + "}");
}

void rgOpenCLBuildSettingsView::HandleLineEditFocusInEvent()
{
    emit SetFrameBorderRedSignal();
}

void rgOpenCLBuildSettingsView::HandleLineEditFocusOutEvent()
{
    emit SetFrameBorderBlackSignal();
}

void rgOpenCLBuildSettingsView::HandleCheckBoxClickedEvent()
{
    emit SetFrameBorderRedSignal();
}

void rgOpenCLBuildSettingsView::HandleComboBoxFocusInEvent()
{
    emit SetFrameBorderRedSignal();
}

void rgOpenCLBuildSettingsView::HandleCompilerFolderBrowseButtonClick(CompilerFolderType folderType)
{
    QFileDialog fileDialog;

    // If existing text in the line edit is not empty, pass it to the File Dialog.
    // Otherwise, get the latest entry from the directories list and open the dialog there.
    QLineEdit* pLineEdit = (folderType == CompilerFolderType::Bin ? ui.compilerBinariesLineEdit :
                           (folderType == CompilerFolderType::Include ? ui.compilerIncludesLineEdit : ui.compilerLibrariesLineEdit));
    assert(pLineEdit != nullptr);
    if (pLineEdit != nullptr)
    {
        QString currentDir = (pLineEdit->text().isEmpty() ? QString::fromStdString(rgConfigManager::GetLastSelectedFolder()) : pLineEdit->text());

        QString selectedDirectory = QFileDialog::getExistingDirectory(this, tr(STR_INCLUDE_DIR_DIALOG_SELECT_DIR_TITLE),
                                                                      currentDir, QFileDialog::ShowDirsOnly);
        if (!selectedDirectory.isEmpty())
        {
            switch (folderType)
            {
            case CompilerFolderType::Bin:      ui.compilerBinariesLineEdit->setText(selectedDirectory);  break;
            case CompilerFolderType::Include:  ui.compilerIncludesLineEdit->setText(selectedDirectory);  break;
            case CompilerFolderType::Lib:      ui.compilerLibrariesLineEdit->setText(selectedDirectory); break;
            default:                           assert(false);
            }
        }
    }
}

void rgOpenCLBuildSettingsView::HandleCompilerFolderEditChanged(CompilerFolderType folderType)
{
    auto pBuildSettings = m_pSettingsModel->GetBuildSettings();
    auto pTextEdit = qobject_cast<QLineEdit*>(QObject::sender());
    const std::string& value = pTextEdit->text().toStdString();
    assert(pBuildSettings != nullptr && pTextEdit != nullptr);
    if (pBuildSettings != nullptr && pTextEdit != nullptr)
    {
        switch (folderType)
        {
        case CompilerFolderType::Bin:
        {
            QtCommon::QtUtil::RestoreCursorPosition cursorPosition(ui.compilerBinariesLineEdit);
            m_pSettingsModel->UpdateModelValue(rgOpenCLBuildSettingsControls::AlternativeCompilerBinDir, value.c_str(), false);
            break;
        }
        case CompilerFolderType::Include:
        {
            QtCommon::QtUtil::RestoreCursorPosition cursorPosition(ui.compilerIncludesLineEdit);
            m_pSettingsModel->UpdateModelValue(rgOpenCLBuildSettingsControls::AlternativeCompilerIncDir, value.c_str(), false);
            break;
        }
        case CompilerFolderType::Lib:
        {
            QtCommon::QtUtil::RestoreCursorPosition cursorPosition(ui.compilerLibrariesLineEdit);
            m_pSettingsModel->UpdateModelValue(rgOpenCLBuildSettingsControls::AlternativeCompilerLibDir, value.c_str(), false);
            break;
        }
        default:
            assert(false);
            break;
        }
    }

    // Update the command line preview text.
    UpdateCommandLineText();
}

void rgOpenCLBuildSettingsView::SetScrollAreaFixedHeight(const int minHeight)
{
    ui.scrollArea->setFixedHeight(minHeight);
}

bool rgOpenCLBuildSettingsView::GetHasPendingChanges() const
{
    bool ret = false;
    assert(m_pSettingsModel != nullptr);
    if (m_pSettingsModel != nullptr)
    {
        ret = m_pSettingsModel->HasPendingChanges();
    }

    return ret;
}

bool rgOpenCLBuildSettingsView::RevertPendingChanges()
{
    m_pSettingsModel->RevertPendingChanges();
    ui.additionalOptionsTextEdit->document()->setPlainText(m_pSettingsModel->GetBuildSettings()->m_additionalOptions.c_str());
    return false;
}

void rgOpenCLBuildSettingsView::RestoreDefaultSettings()
{
    assert(m_pSettingsModel != nullptr);
    if (m_pSettingsModel != nullptr)
    {
        m_pSettingsModel->RevertPendingChanges();
    }

    bool isRestored = false;

    if (m_isGlobalSettings)
    {
        rgConfigManager& configManager = rgConfigManager::Instance();

        // Revert the global default OpenCL build settings.
        configManager.RevertToDefaultBuildSettings(rgProjectAPI::OpenCL);

        // Get a pointer to the new default build settings.
        std::shared_ptr<rgBuildSettings> pGlobalCLBuildSettings = configManager.GetUserGlobalBuildSettings(rgProjectAPI::OpenCL);

        // Set the default settings in the model.
        auto pCLDefaultSettings = std::dynamic_pointer_cast<rgCLBuildSettings>(pGlobalCLBuildSettings);
        m_pSettingsModel->SetBuildSettings(pCLDefaultSettings);

        // Save the configuration file with the default settings.
        isRestored = configManager.SaveGlobalConfigFile();
    }
    else
    {
        std::shared_ptr<rgBuildSettings> pDefaultSettings = rgConfigManager::GetDefaultBuildSettings(rgProjectAPI::OpenCL);
        auto pCLDefaultSettings = std::dynamic_pointer_cast<rgCLBuildSettings>(pDefaultSettings);
        m_pSettingsModel->SetBuildSettings(pCLDefaultSettings);

        // Let the rgBuildView know that the build settings have been updated.
        emit ProjectBuildSettingsSaved(pCLDefaultSettings);
        isRestored = true;
    }

    // TODO: This is a workaround for resetting the content of "Additional Options" text box.
    // It should be replaced by the appropriate fix on the Model side.
    ui.additionalOptionsTextEdit->document()->clear();

    // Show an error dialog if the settings failed to be reset.
    if (!isRestored)
    {
        rgUtils::ShowErrorMessageBox(STR_ERR_CANNOT_RESTORE_DEFAULT_SETTINGS);
    }
}

void rgOpenCLBuildSettingsView::SaveSettings()
{
    bool isValid = ValidatePendingSettings();
    if (isValid)
    {
        assert(m_pSettingsModel != nullptr);
        if (m_pSettingsModel != nullptr)
        {
            // Confirm all pending changes within the view.
            m_pSettingsModel->SubmitPendingChanges();

            // Either save the global settings, or an individual project with altered settings.
            if (m_isGlobalSettings)
            {
                rgConfigManager::Instance().SaveGlobalConfigFile();
            }
            else
            {
                std::shared_ptr<rgBuildSettings> pBuildSettings = m_pSettingsModel->GetBuildSettings();
                emit ProjectBuildSettingsSaved(pBuildSettings);
            }
        }
    }
}