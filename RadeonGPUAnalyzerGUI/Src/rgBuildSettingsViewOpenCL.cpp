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
#include <Utils/Include/rgaCliDefs.h>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgBuildSettingsView.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgBuildSettingsViewOpenCL.h>
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

rgBuildSettingsViewOpenCL::rgBuildSettingsViewOpenCL(QWidget* pParent, const rgBuildSettingsOpenCL& buildSettings, bool isGlobalSettings) :
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

    // Connect focus in/out events for all of the checkboxes.
    ConnectCheckBoxClickedEvents();

    // Connect focus in event for the combobox.
    ConnectComboboxClickEvent();

    // Initialize the UI based on the incoming build settings.
    PushToWidgets(buildSettings);

    // Initialize the command line preview text.
    UpdateCommandLineText();

    // Set tooltips for general items.
    ui.targetGPUsLabel->setToolTip(STR_BUILD_SETTINGS_TARGET_GPUS_TOOLTIP);
    ui.predefinedMacrosLabel->setToolTip(STR_BUILD_SETTINGS_PREDEFINED_MACROS_TOOLTIP);
    ui.includeDirectoriesLabel->setToolTip(STR_BUILD_SETTINGS_ADDITIONAL_INC_DIRECTORY_TOOLTIP);

    // Set tooltips for alternative compiler components.
    ui.alternativeCompilerLabel->setToolTip(STR_BUILD_SETTINGS_ALTERNATIVE_COMPILER_TOOLTIP_GENERIC);
    ui.compilerBinariesLabel->setToolTip(CLI_DESC_ALTERNATIVE_ROCM_BIN_FOLDER);
    ui.compilerIncludesLabel->setToolTip(CLI_DESC_ALTERNATIVE_ROCM_INC_FOLDER);
    ui.compilerLibrariesLabel->setToolTip(CLI_DESC_ALTERNATIVE_ROCM_LIB_FOLDER);

    // Set tooltip for the optimization level item.
    ui.optimizationLevelLabel->setToolTip(STR_BUILD_SETTINGS_OPTIMIZATION_LEVEL);

    // Set tooltips for the Settings command line section.
    ui.settingsCommandLineHeaderLabel->setToolTip(STR_BUILD_SETTINGS_SETTINGS_CMDLINE_TOOLTIP);

    // Set tooltips for the Additional options line section.
    ui.additionalOptionsHeaderLabel->setToolTip(STR_BUILD_SETTINGS_CLANG_OPTIONS_TOOLTIP);

    // Set the tooltip for the General section of the build settings.
    ui.generalHeaderLabel->setToolTip(STR_BUILD_SETTINGS_GENERAL_TOOLTIP);

    // Set the tooltip for the command line section of the build settings.
    ui.settingsCommandLineHeaderLabel->setToolTip(STR_BUILD_SETTINGS_CMD_LINE_TOOLTIP);

    // Set the mouse cursor to the pointing hand cursor for various widgets.
    SetCursor();

    // Set the geometry of tool tip boxes.
    SetToolTipGeometry();

    // Set the event filter for "Additional Options" and "All Options" text edits.
    ui.additionalOptionsTextEdit->installEventFilter(this);
    ui.allOptionsTextEdit->installEventFilter(this);
}

void rgBuildSettingsViewOpenCL::PushToWidgets(const rgBuildSettingsOpenCL& buildSettings)
{
    // The items below are common build settings for all API types.
    QString targetGpusList(rgUtils::BuildSemicolonSeparatedStringList(buildSettings.m_targetGpus).c_str());
    ui.targetGPUsLineEdit->setText(targetGpusList);

    QString predefinedMacros(rgUtils::BuildSemicolonSeparatedStringList(buildSettings.m_predefinedMacros).c_str());
    ui.predefinedMacrosLineEdit->setText(predefinedMacros);

    QString additionalIncludeDirs(rgUtils::BuildSemicolonSeparatedStringList(buildSettings.m_additionalIncludeDirectories).c_str());
    ui.includeDirectoriesLineEdit->setText(additionalIncludeDirs);

    // Items below are OpenCL-specific build settings only.
    ui.optimizationLevelComboBox->setCurrentText(buildSettings.m_optimizationLevel.c_str());
    ui.doubleAsSingleCheckBox->setChecked(buildSettings.m_isTreatDoubleAsSingle);
    ui.flushDenormalizedCheckBox->setChecked(buildSettings.m_isDenormsAsZeros);
    ui.strictAliasingCheckBox->setChecked(buildSettings.m_isStrictAliasing);
    ui.enableMADCheckBox->setChecked(buildSettings.m_isEnableMAD);
    ui.ignoreZeroSignednessCheckBox->setChecked(buildSettings.m_isIgnoreZeroSignedness);
    ui.allowUnsafeOptimizationsCheckBox->setChecked(buildSettings.m_isUnsafeOptimizations);
    ui.assumeNoNanNorInfiniteCheckBox->setChecked(buildSettings.m_isNoNanNorInfinite);
    ui.aggressiveMathOptimizationsCheckBox->setChecked(buildSettings.m_isAggressiveMathOptimizations);
    ui.correctlyRoundSinglePercisionCheckBox->setChecked(buildSettings.m_isCorrectlyRoundDivSqrt);
    ui.additionalOptionsTextEdit->document()->setPlainText(buildSettings.m_additionalOptions.c_str());
    ui.compilerBinariesLineEdit->setText(std::get<CompilerFolderType::Bin>(buildSettings.m_compilerPaths).c_str());
    ui.compilerIncludesLineEdit->setText(std::get<CompilerFolderType::Include>(buildSettings.m_compilerPaths).c_str());
    ui.compilerLibrariesLineEdit->setText(std::get<CompilerFolderType::Lib>(buildSettings.m_compilerPaths).c_str());
}

rgBuildSettingsOpenCL rgBuildSettingsViewOpenCL::PullFromWidgets() const
{
    rgBuildSettingsOpenCL settings;

    // Target GPUs
    std::vector<std::string> targetGPUsVector;
    const std::string& commaSeparatedTargetGPUs = ui.targetGPUsLineEdit->text().toStdString();
    rgUtils::splitString(commaSeparatedTargetGPUs, rgConfigManager::RGA_LIST_DELIMITER, targetGPUsVector);
    settings.m_targetGpus = targetGPUsVector;

    // Predefined Macros
    std::vector<std::string> predefinedMacrosVector;
    const std::string& commaSeparatedPredefinedMacros = ui.predefinedMacrosLineEdit->text().toStdString();
    rgUtils::splitString(commaSeparatedPredefinedMacros, rgConfigManager::RGA_LIST_DELIMITER, predefinedMacrosVector);
    settings.m_predefinedMacros = predefinedMacrosVector;

    // Additional Include Directories
    std::vector<std::string> additionalIncludeDirectoriesVector;
    const std::string& commaSeparatedAdditionalIncludeDirectories = ui.includeDirectoriesLineEdit->text().toStdString();
    rgUtils::splitString(commaSeparatedAdditionalIncludeDirectories, rgConfigManager::RGA_LIST_DELIMITER, additionalIncludeDirectoriesVector);
    settings.m_additionalIncludeDirectories = additionalIncludeDirectoriesVector;

    // AdditionalOptions:
    settings.m_additionalOptions = ui.additionalOptionsTextEdit->document()->toPlainText().toStdString();

    // AlternativeCompilerBinDir:
    std::get<CompilerFolderType::Bin>(settings.m_compilerPaths) = ui.compilerBinariesLineEdit->text().toStdString();

    // AlternativeCompilerIncDir:
    std::get<CompilerFolderType::Include>(settings.m_compilerPaths) = ui.compilerIncludesLineEdit->text().toStdString();

    // AlternativeCompilerLibDir:
    std::get<CompilerFolderType::Lib>(settings.m_compilerPaths) = ui.compilerLibrariesLineEdit->text().toStdString();


    // OpenCL Specific Settings

    // OptimizationLevel:
    settings.m_optimizationLevel = ui.optimizationLevelComboBox->currentText().toStdString();

    // TreatDoubleFloatingPointAsSingle:
    settings.m_isTreatDoubleAsSingle = ui.doubleAsSingleCheckBox->isChecked();

    // FlushDenormalizedFloatsAsZero:
    settings.m_isDenormsAsZeros = ui.flushDenormalizedCheckBox->isChecked();

    // AssumeStrictAliasingRules:
    settings.m_isStrictAliasing = ui.strictAliasingCheckBox->isChecked();

    // EnableMADInstructions:
    settings.m_isEnableMAD = ui.enableMADCheckBox->isChecked();

    //IgnoreSignednessOfZeros:
    settings.m_isIgnoreZeroSignedness = ui.ignoreZeroSignednessCheckBox->isChecked();

    // AllowUnsafeOptimizations:
    settings.m_isUnsafeOptimizations = ui.allowUnsafeOptimizationsCheckBox->isChecked();

    // AssumeNoNaNNorInfinite:
    settings.m_isNoNanNorInfinite = ui.assumeNoNanNorInfiniteCheckBox->isChecked();

    // AgressiveMathOptimizations:
    settings.m_isAggressiveMathOptimizations = ui.aggressiveMathOptimizationsCheckBox->isChecked();

    // CorrectlyRoundSingleDivideAndSqrt:
    settings.m_isCorrectlyRoundDivSqrt = ui.correctlyRoundSinglePercisionCheckBox->isChecked();

    return settings;
}

void rgBuildSettingsViewOpenCL::ConnectSignals()
{
    // Add target GPU button.
    bool isConnected = connect(this->ui.addTargetGPUsButton, &QPushButton::clicked, this, &rgBuildSettingsViewOpenCL::HandleAddTargetGpusButtonClick);
    assert(isConnected);

    // Add include directories editor dialog button.
    isConnected = connect(this->ui.includeDirsBrowseButton, &QPushButton::clicked, this, &rgBuildSettingsViewOpenCL::HandleIncludeDirsBrowseButtonClick);
    assert(isConnected);

    // Add preprocessor directives editor dialog button.
    isConnected = connect(this->ui.predefinedMacrosBrowseButton, &QPushButton::clicked, this, &rgBuildSettingsViewOpenCL::HandlePreprocessorDirectivesBrowseButtonClick);
    assert(isConnected);

    // Connect all textboxes within the view.
    isConnected = connect(this->ui.targetGPUsLineEdit, &QLineEdit::textChanged, this, &rgBuildSettingsViewOpenCL::HandleTextEditChanged);
    assert(isConnected);

    isConnected = connect(this->ui.predefinedMacrosLineEdit, &QLineEdit::textChanged, this, &rgBuildSettingsViewOpenCL::HandleTextEditChanged);
    assert(isConnected);

    isConnected = connect(this->ui.includeDirectoriesLineEdit, &QLineEdit::textChanged, this, &rgBuildSettingsViewOpenCL::HandleTextEditChanged);
    assert(isConnected);

    isConnected = connect(this->ui.optimizationLevelComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &rgBuildSettingsViewOpenCL::HandleComboboxIndexChanged);
    assert(isConnected);

    isConnected = connect(this->ui.doubleAsSingleCheckBox, &rgCheckBox::stateChanged, this, &rgBuildSettingsViewOpenCL::HandleCheckboxStateChanged);
    assert(isConnected);

    isConnected = connect(this->ui.flushDenormalizedCheckBox, &rgCheckBox::stateChanged, this, &rgBuildSettingsViewOpenCL::HandleCheckboxStateChanged);
    assert(isConnected);

    isConnected = connect(this->ui.strictAliasingCheckBox, &rgCheckBox::stateChanged, this, &rgBuildSettingsViewOpenCL::HandleCheckboxStateChanged);
    assert(isConnected);

    isConnected = connect(this->ui.enableMADCheckBox, &rgCheckBox::stateChanged, this, &rgBuildSettingsViewOpenCL::HandleCheckboxStateChanged);
    assert(isConnected);

    isConnected = connect(this->ui.ignoreZeroSignednessCheckBox, &rgCheckBox::stateChanged, this, &rgBuildSettingsViewOpenCL::HandleCheckboxStateChanged);
    assert(isConnected);

    isConnected = connect(this->ui.allowUnsafeOptimizationsCheckBox, &rgCheckBox::stateChanged, this, &rgBuildSettingsViewOpenCL::HandleCheckboxStateChanged);
    assert(isConnected);

    isConnected = connect(this->ui.assumeNoNanNorInfiniteCheckBox, &rgCheckBox::stateChanged, this, &rgBuildSettingsViewOpenCL::HandleCheckboxStateChanged);
    assert(isConnected);

    isConnected = connect(this->ui.aggressiveMathOptimizationsCheckBox, &rgCheckBox::stateChanged, this, &rgBuildSettingsViewOpenCL::HandleCheckboxStateChanged);
    assert(isConnected);

    isConnected = connect(this->ui.correctlyRoundSinglePercisionCheckBox, &rgCheckBox::stateChanged, this, &rgBuildSettingsViewOpenCL::HandleCheckboxStateChanged);
    assert(isConnected);

    // Connect "Additional Options" text box.
    isConnected = connect(this->ui.additionalOptionsTextEdit, &QPlainTextEdit::textChanged, this, &rgBuildSettingsViewOpenCL::HandleAdditionalOptionsTextChanged);
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

    // Connect the include directory editor dialog's "OK" button click.
    isConnected = connect(m_pIncludeDirectoriesView, &rgIncludeDirectoriesView::OKButtonClicked, this, &rgBuildSettingsViewOpenCL::HandleIncludeDirsUpdated);
    assert(isConnected);

    // Connect the preprocessor directives editor dialog's "OK" button click.
    isConnected = connect(m_pPreprocessorDirectivesDialog, &rgPreprocessorDirectivesDialog::OKButtonClicked, this, &rgBuildSettingsViewOpenCL::HandlePreprocessorDirectivesUpdated);
    assert(isConnected);
}

void rgBuildSettingsViewOpenCL::ConnectLineEditFocusEvents()
{
    bool isConnected = false;

    isConnected = connect(this->ui.includeDirectoriesLineEdit, &rgLineEdit::LineEditFocusInEvent, this, &rgBuildSettingsViewOpenCL::HandleLineEditFocusInEvent);
    assert(isConnected);

    isConnected = connect(this->ui.includeDirectoriesLineEdit, &rgLineEdit::LineEditFocusOutEvent, this, &rgBuildSettingsViewOpenCL::HandleLineEditFocusOutEvent);
    assert(isConnected);

    isConnected = connect(this->ui.predefinedMacrosLineEdit, &rgLineEdit::LineEditFocusInEvent, this, &rgBuildSettingsViewOpenCL::HandleLineEditFocusInEvent);
    assert(isConnected);

    isConnected = connect(this->ui.predefinedMacrosLineEdit, &rgLineEdit::LineEditFocusOutEvent, this, &rgBuildSettingsViewOpenCL::HandleLineEditFocusOutEvent);
    assert(isConnected);

    isConnected = connect(this->ui.targetGPUsLineEdit, &rgLineEdit::LineEditFocusInEvent, this, &rgBuildSettingsViewOpenCL::HandleLineEditFocusInEvent);
    assert(isConnected);

    isConnected = connect(this->ui.targetGPUsLineEdit, &rgLineEdit::LineEditFocusOutEvent, this, &rgBuildSettingsViewOpenCL::HandleLineEditFocusOutEvent);
    assert(isConnected);

    isConnected = connect(this->ui.compilerBinariesLineEdit, &rgLineEdit::LineEditFocusInEvent, this, &rgBuildSettingsViewOpenCL::HandleLineEditFocusInEvent);
    assert(isConnected);

    isConnected = connect(this->ui.compilerBinariesLineEdit, &rgLineEdit::LineEditFocusOutEvent, this, &rgBuildSettingsViewOpenCL::HandleLineEditFocusOutEvent);
    assert(isConnected);

    isConnected = connect(this->ui.compilerIncludesLineEdit, &rgLineEdit::LineEditFocusInEvent, this, &rgBuildSettingsViewOpenCL::HandleLineEditFocusInEvent);
    assert(isConnected);

    isConnected = connect(this->ui.compilerIncludesLineEdit, &rgLineEdit::LineEditFocusOutEvent, this, &rgBuildSettingsViewOpenCL::HandleLineEditFocusOutEvent);
    assert(isConnected);

    isConnected = connect(this->ui.compilerLibrariesLineEdit, &rgLineEdit::LineEditFocusInEvent, this, &rgBuildSettingsViewOpenCL::HandleLineEditFocusInEvent);
    assert(isConnected);

    isConnected = connect(this->ui.compilerLibrariesLineEdit, &rgLineEdit::LineEditFocusOutEvent, this, &rgBuildSettingsViewOpenCL::HandleLineEditFocusOutEvent);
    assert(isConnected);
}

void rgBuildSettingsViewOpenCL::ConnectCheckBoxClickedEvents()
{
    bool isConnected = false;

    isConnected = connect(this->ui.aggressiveMathOptimizationsCheckBox, &rgCheckBox::clicked, this, &rgBuildSettingsViewOpenCL::HandleCheckBoxClickedEvent);
    assert(isConnected);

    isConnected = connect(this->ui.allowUnsafeOptimizationsCheckBox, &rgCheckBox::clicked, this, &rgBuildSettingsViewOpenCL::HandleCheckBoxClickedEvent);
    assert(isConnected);

    isConnected = connect(this->ui.assumeNoNanNorInfiniteCheckBox, &rgCheckBox::clicked, this, &rgBuildSettingsViewOpenCL::HandleCheckBoxClickedEvent);
    assert(isConnected);

    isConnected = connect(this->ui.correctlyRoundSinglePercisionCheckBox, &rgCheckBox::clicked, this, &rgBuildSettingsViewOpenCL::HandleCheckBoxClickedEvent);
    assert(isConnected);

    isConnected = connect(this->ui.doubleAsSingleCheckBox, &rgCheckBox::clicked, this, &rgBuildSettingsViewOpenCL::HandleCheckBoxClickedEvent);
    assert(isConnected);

    isConnected = connect(this->ui.enableMADCheckBox, &rgCheckBox::clicked, this, &rgBuildSettingsViewOpenCL::HandleCheckBoxClickedEvent);
    assert(isConnected);

    isConnected = connect(this->ui.flushDenormalizedCheckBox, &rgCheckBox::clicked, this, &rgBuildSettingsViewOpenCL::HandleCheckBoxClickedEvent);
    assert(isConnected);

    isConnected = connect(this->ui.ignoreZeroSignednessCheckBox, &rgCheckBox::clicked, this, &rgBuildSettingsViewOpenCL::HandleCheckBoxClickedEvent);
    assert(isConnected);

    isConnected = connect(this->ui.strictAliasingCheckBox, &rgCheckBox::clicked, this, &rgBuildSettingsViewOpenCL::HandleCheckBoxClickedEvent);
    assert(isConnected);
}

void rgBuildSettingsViewOpenCL::ConnectComboboxClickEvent()
{
    bool isConnected = connect(this->ui.optimizationLevelComboBox, &rgComboBox::ComboBoxFocusInEvent, this, &rgBuildSettingsViewOpenCL::HandleComboBoxFocusInEvent);
    assert(isConnected);
}

bool rgBuildSettingsViewOpenCL::eventFilter(QObject* pObject, QEvent* pEvent)
{
    // Intercept events for "Additional "Options" widget.
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

void rgBuildSettingsViewOpenCL::HandleAddTargetGpusButtonClick()
{
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

void rgBuildSettingsViewOpenCL::HandleTextEditChanged()
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

void rgBuildSettingsViewOpenCL::HandleComboboxIndexChanged(int index)
{
    // Determine which control's text has been updated.
    QComboBox *pComboBox = static_cast<QComboBox*>(QObject::sender());
    assert(pComboBox != nullptr);
    if (pComboBox != nullptr)
    {
        // Inform the UI of a possible change to the pending state.
        HandlePendingChangesStateChanged(GetHasPendingChanges());

        // Update the command line preview text.
        UpdateCommandLineText();
    }
}

void rgBuildSettingsViewOpenCL::HandleCheckboxStateChanged()
{
    // Determine which control's text has been updated.
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

std::string rgBuildSettingsViewOpenCL::GetTitleString()
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

const std::string rgBuildSettingsViewOpenCL::GetTitleTooltipString() const
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

void rgBuildSettingsViewOpenCL::HandlePendingChangesStateChanged(bool hasPendingChanges)
{
    // Let the base class determine if there is a need to signal listeners
    // about the pending changes state.
    rgBuildSettingsView::SetHasPendingChanges(hasPendingChanges);
}

void rgBuildSettingsViewOpenCL::UpdateCommandLineText()
{
    rgBuildSettingsOpenCL apiBuildSetting = PullFromWidgets();

    // Generate a command line string from the build settings structure.
    std::string buildSettings;
    bool ret = rgCliUtils::GenerateOpenClBuildSettingsString(apiBuildSetting, buildSettings, false);
    assert(ret);
    if (ret)
    {
        ui.allOptionsTextEdit->setPlainText(buildSettings.c_str());
    }
}

bool rgBuildSettingsViewOpenCL::IsTargetGpusStringValid(std::vector<std::string>& errors) const
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

            // Add an error field indicating that the GPU name is invalid.
            errors.push_back(errorStream.str());
            isValid = false;
        }
    }

    return isValid;
}

bool rgBuildSettingsViewOpenCL::ValidatePendingSettings()
{
    std::vector<std::string> errors;
    bool isValid = IsTargetGpusStringValid(errors);
    if (!isValid)
    {
        std::stringstream errorStream;
        errorStream << STR_ERR_INVALID_PENDING_SETTING;
        errorStream << std::endl;

        // Loop through all errors and append them to the stream.
        for (const std::string errorString : errors)
        {
            errorStream << errorString;
            errorStream << std::endl;
        }

        // Display an error message box to the user telling them what to fix.
        rgUtils::ShowErrorMessageBox(errorStream.str().c_str(), this);
    }

    return isValid;
}

void rgBuildSettingsViewOpenCL::SetCursor()
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

void rgBuildSettingsViewOpenCL::HandleIncludeDirsBrowseButtonClick()
{
    // Position the window in the middle of the screen.
    m_pIncludeDirectoriesView->setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, m_pIncludeDirectoriesView->size(), qApp->desktop()->availableGeometry()));

    // Set the current include dirs.
    m_pIncludeDirectoriesView->SetListItems(ui.includeDirectoriesLineEdit->text());

    // Show the window.
    m_pIncludeDirectoriesView->exec();
}

void rgBuildSettingsViewOpenCL::HandleIncludeDirsUpdated(QStringList includeDirs)
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

void rgBuildSettingsViewOpenCL::HandlePreprocessorDirectivesBrowseButtonClick()
{
    // Position the window in the middle of the screen.
    m_pPreprocessorDirectivesDialog->setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, m_pPreprocessorDirectivesDialog->size(), qApp->desktop()->availableGeometry()));

    // Set the current preprocessor directives in the dialog.
    m_pPreprocessorDirectivesDialog->SetListItems(ui.predefinedMacrosLineEdit->text());

    // Show the dialog.
    m_pPreprocessorDirectivesDialog->exec();
}

void rgBuildSettingsViewOpenCL::HandlePreprocessorDirectivesUpdated(QStringList preprocessorDirectives)
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

void rgBuildSettingsViewOpenCL::HandleAdditionalOptionsTextChanged()
{
    // Inform the UI of a possible change to the pending state.
    HandlePendingChangesStateChanged(GetHasPendingChanges());

    // Update the command line preview text.
    UpdateCommandLineText();
}

void rgBuildSettingsViewOpenCL::SetToolTipGeometry()
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

void rgBuildSettingsViewOpenCL::HandleLineEditFocusInEvent()
{
    emit SetFrameBorderGreenSignal();
}

void rgBuildSettingsViewOpenCL::HandleLineEditFocusOutEvent()
{
    emit SetFrameBorderBlackSignal();
}

void rgBuildSettingsViewOpenCL::HandleCheckBoxClickedEvent()
{
    emit SetFrameBorderGreenSignal();
}

void rgBuildSettingsViewOpenCL::HandleComboBoxFocusInEvent()
{
    emit SetFrameBorderGreenSignal();
}

void rgBuildSettingsViewOpenCL::HandleCompilerFolderBrowseButtonClick(CompilerFolderType folderType)
{
    QFileDialog fileDialog;

    // If existing text in the line edit is not empty, pass it to the File Dialog.
    // Otherwise, get the latest entry from the directories list and open the dialog there.
    QLineEdit* pLineEdit = (folderType == CompilerFolderType::Bin ? ui.compilerBinariesLineEdit :
                           (folderType == CompilerFolderType::Include ? ui.compilerIncludesLineEdit : ui.compilerLibrariesLineEdit));
    assert(pLineEdit != nullptr);
    if (pLineEdit != nullptr)
    {
        QString currentDir = (pLineEdit->text().isEmpty() ? QString::fromStdString(rgConfigManager::Instance().GetLastSelectedFolder()) : pLineEdit->text());

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

void rgBuildSettingsViewOpenCL::HandleCompilerFolderEditChanged(CompilerFolderType folderType)
{
    auto pTextEdit = qobject_cast<QLineEdit*>(QObject::sender());
    assert(pTextEdit != nullptr);
    if (pTextEdit != nullptr)
    {
        // Inform the UI of a possible change to the pending state.
        HandlePendingChangesStateChanged(GetHasPendingChanges());
    }

    // Update the command line preview text.
    UpdateCommandLineText();
}

bool rgBuildSettingsViewOpenCL::GetHasPendingChanges() const
{
    bool ret = false;

    rgBuildSettingsOpenCL apiBuildSettings = PullFromWidgets();

    ret = !m_initialSettings.HasSameSettings(apiBuildSettings);

    return ret;
}

bool rgBuildSettingsViewOpenCL::RevertPendingChanges()
{
    PushToWidgets(m_initialSettings);

    // Inform the UI of a possible change to the pending state.
    HandlePendingChangesStateChanged(GetHasPendingChanges());

    return false;
}

void rgBuildSettingsViewOpenCL::RestoreDefaultSettings()
{
    bool isRestored = false;

    if (m_isGlobalSettings)
    {
        // If this is for the global settings, then restore to the hard-coded defaults.

        // Get the hard-coded default build settings.
        std::shared_ptr<rgBuildSettings> pDefaultBuildSettings = rgConfigManager::GetDefaultBuildSettings(rgProjectAPI::OpenCL);

        std::shared_ptr<rgBuildSettingsOpenCL> pApiBuildSettings = std::dynamic_pointer_cast<rgBuildSettingsOpenCL>(pDefaultBuildSettings);
        assert(pApiBuildSettings != nullptr);
        if (pApiBuildSettings != nullptr)
        {
            // Reset our initial settings back to the defaults.
            m_initialSettings = *pApiBuildSettings;

            // Update the UI to reflect the new initial settings.
            PushToWidgets(m_initialSettings);

            // Update the ConfigManager to use the new settings.
            rgConfigManager::Instance().SetApiBuildSettings(STR_API_NAME_OPENCL, &m_initialSettings);

            // Save the settings file
            isRestored = rgConfigManager::Instance().SaveGlobalConfigFile();

            // Inform the rest of the UI that the settings have been changed.
            HandlePendingChangesStateChanged(GetHasPendingChanges());
        }
    }
    else
    {
        // This view is showing project-specific settings, so restore back to the stored settings in the project.
        std::shared_ptr<rgBuildSettings> pDefaultSettings = rgConfigManager::Instance().GetUserGlobalBuildSettings(rgProjectAPI::OpenCL);
        auto pCLDefaultSettings = std::dynamic_pointer_cast<rgBuildSettingsOpenCL>(pDefaultSettings);

        m_initialSettings = *pCLDefaultSettings;

        PushToWidgets(m_initialSettings);

        // Inform the rest of the UI that the settings have been changed.
        HandlePendingChangesStateChanged(GetHasPendingChanges());

        // Let the rgBuildView know that the build settings have been updated.
        emit ProjectBuildSettingsSaved(pCLDefaultSettings);
        isRestored = true;
    }

    // Show an error dialog if the settings failed to be reset.
    if (!isRestored)
    {
        rgUtils::ShowErrorMessageBox(STR_ERR_CANNOT_RESTORE_DEFAULT_SETTINGS, this);
    }
}

bool rgBuildSettingsViewOpenCL::SaveSettings()
{
    bool isValid = ValidatePendingSettings();
    if (isValid)
    {
        // Reset the initial settings to match what the UI shows.
        m_initialSettings = PullFromWidgets();

        if (m_isGlobalSettings)
        {
            // Update the config manager to use these new settings.
            rgConfigManager& configManager = rgConfigManager::Instance();
            configManager.SetApiBuildSettings(STR_API_NAME_OPENCL, &m_initialSettings);

            // Save the global config settings.
            isValid = configManager.SaveGlobalConfigFile();
        }
        else
        {
            // Save the project settings.
            std::shared_ptr<rgBuildSettingsOpenCL> pTmpPtr = std::make_shared<rgBuildSettingsOpenCL>(m_initialSettings);
            emit ProjectBuildSettingsSaved(pTmpPtr);
        }

        if (isValid)
        {
            // Make sure the rest of the UI knows that the settings have been saved.
            HandlePendingChangesStateChanged(false);
        }
    }

    return isValid;
}

void rgBuildSettingsViewOpenCL::mousePressEvent(QMouseEvent* pEvent)
{
    Q_UNUSED(pEvent);

    emit SetFrameBorderGreenSignal();
}

void rgBuildSettingsViewOpenCL::SetInitialWidgetFocus()
{
    ui.addTargetGPUsButton->setFocus();
}