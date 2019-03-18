// C++.
#include <cassert>

// Qt.
#include <QCheckBox>

// Infra.
#include <QtCommon/CustomWidgets/ArrowIconWidget.h>
#include <QtCommon/CustomWidgets/ListWidget.h>
#include <QtCommon/Util/CommonDefinitions.h>
#include <QtCommon/Util/QtUtil.h>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgPipelineStateEditorWidgetEnum.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgHideListWidgetEventFilter.h>
#include <RadeonGPUAnalyzerGUI/Include/rgUtils.h>

// Names of special widgets.
static const char* STR_ENUM_LIST = "EnumList";
static const char* STR_ENUM_LIST_ITEM_CHECKBOX = "EnumListWidgetCheckBox";
static const char* s_STR_ENUM_LIST_PUSH_BUTTON = "ListPushButton";

// Enumeration push button font size.
const int s_PUSH_BUTTON_FONT_SIZE = 11;

// Initialize the static enum push button count.
int rgPipelineStateEditorWidgetEnum::s_enumComboPushButtonCounter = 0;

rgPipelineStateEditorWidgetEnum::rgPipelineStateEditorWidgetEnum(bool isBitFlagsEnum, QWidget* pParent)
    : m_isBitFlagsEnum(isBitFlagsEnum)
    , m_pParent(pParent)
    , rgPipelineStateEditorWidget(pParent)
{
    ui.setupUi(this);

    // Create the enum drop down.
    CreateEnumControls();

    // Set the dropdown arrow button as the focus proxy widget.
    setFocusProxy(ui.enumComboPushButton);

    // Connect internal signals.
    ConnectSignals();
}

rgPipelineStateEditorWidgetEnum::~rgPipelineStateEditorWidgetEnum()
{
    // Remove the event filter from the main window.
    if (m_pEnumListWidget != nullptr)
    {
        qApp->removeEventFilter(m_pEnumListEventFilter);
    }
}

uint32_t rgPipelineStateEditorWidgetEnum::GetValue() const
{
    uint32_t result = 0;

    if (m_isBitFlagsEnum)
    {
        assert(m_pEnumListWidget != nullptr);
        if (m_pEnumListWidget != nullptr)
        {
            // Combine all flags into a single integer.
            for (int i = 0; i < m_pEnumListWidget->count(); i++)
            {
                QListWidgetItem* pItem = m_pEnumListWidget->item(i);
                assert(pItem != nullptr);
                if (pItem != nullptr)
                {
                    QCheckBox* pCheckBox = qobject_cast<QCheckBox*>(m_pEnumListWidget->itemWidget(pItem));
                    assert(pCheckBox != nullptr);
                    if (pCheckBox != nullptr)
                    {
                        if (pCheckBox->checkState() == Qt::CheckState::Checked)
                        {
                            uint32_t currentFlag = m_enumerators[i].m_value;
                            result |= currentFlag;
                        }
                    }
                }
            }
        }
    }
    else
    {
        // Read the push button text and return the appropriate value for it.
        assert(m_currentIndex >= 0 && m_currentIndex < m_enumerators.size());
        if (m_currentIndex >= 0 && m_currentIndex < m_enumerators.size())
        {
            result = m_enumerators[m_currentIndex].m_value;
        }
    }

    return result;
}

void rgPipelineStateEditorWidgetEnum::SetEnumerators(const rgEnumValuesVector& enumerators)
{
    m_enumerators = enumerators;

    // Set up the function pointer responsible for handling column visibility filter state change.
    using std::placeholders::_1;
    std::function<void(bool)> slotFunctionPointer = std::bind(&rgPipelineStateEditorWidgetEnum::HandleFlagCheckStateChanged, this, _1);

    assert(m_pEnumListWidget != nullptr);
    if (m_pEnumListWidget != nullptr)
    {
        // Add an item for each column in the table.
        for (const rgEnumNameValuePair& currentEnum : enumerators)
        {
            // Add an item for each possible enum.
            // If this item needs a check box, make the item checkable.
            if (m_isBitFlagsEnum)
            {
                ListWidget::AddListWidgetCheckboxItem(currentEnum.m_name.c_str(), m_pEnumListWidget, slotFunctionPointer, this, m_pEnumListWidget->objectName(), STR_ENUM_LIST_ITEM_CHECKBOX);
            }
            else
            {
                m_pEnumListWidget->addItem(currentEnum.m_name.c_str());
            }
        }
    }
}

void rgPipelineStateEditorWidgetEnum::GetFlagBitsString(std::string& flagBits)
{
    uint32_t currentValue = GetValue();

    uint32_t flagsValue = 0;
    assert(m_pEnumListWidget != nullptr);
    if (m_pEnumListWidget != nullptr)
    {
        int itemCount = m_pEnumListWidget->count();

        // Bitwise OR together all the checked values and display that number for push button text.
        for (int i = 0; i < m_enumerators.size(); ++i)
        {
            bool isChecked = (m_enumerators[i].m_value & currentValue) == m_enumerators[i].m_value;
            if (isChecked)
            {
                assert(i >= 0 && i < itemCount);
                if (i >= 0 && i < itemCount)
                {
                    QListWidgetItem* pItem = m_pEnumListWidget->item(i);
                    assert(pItem != nullptr);
                    if (pItem != nullptr)
                    {
                        QCheckBox* pCheckBoxItem = static_cast<QCheckBox*>(m_pEnumListWidget->itemWidget(pItem));
                        assert(pCheckBoxItem != nullptr);
                        if (pCheckBoxItem != nullptr)
                        {
                            uint32_t currentFlag = m_enumerators[i].m_value;
                            flagsValue |= currentFlag;
                        }
                    }
                }
            }
        }
    }

    // Return a string that displays individual flag bits.
    int numEnumerators = static_cast<int>(m_enumerators.size());
    QString outputFlagBits = "(" + QString::number(flagsValue) + ") " + QString::number(flagsValue, 2).rightJustified(numEnumerators, '0');
    flagBits = outputFlagBits.toStdString();
}

void rgPipelineStateEditorWidgetEnum::GetTooltipString(std::string& tooltipText)
{
    uint32_t currentValue = GetValue();

    // Bitwise OR together all the checked values and display that number for push button text.
    QString tooltip;
    assert(m_pEnumListWidget != nullptr);
    if (m_pEnumListWidget != nullptr)
    {
        int itemCount = m_pEnumListWidget->count();
        for (int i = 0; i < m_enumerators.size(); ++i)
        {
            bool isChecked = (m_enumerators[i].m_value & currentValue) == m_enumerators[i].m_value;
            if (isChecked)
            {
                assert(i >= 0 && i < itemCount);
                if (i >= 0 && i < itemCount)
                {
                    // Append a bitwise OR pipe between each enumerator.
                    if (!tooltip.isEmpty())
                    {
                        tooltip += " | ";
                    }

                    QListWidgetItem* pItem = m_pEnumListWidget->item(i);
                    assert(pItem != nullptr);
                    if (pItem != nullptr)
                    {
                        QCheckBox* pCheckBoxItem = static_cast<QCheckBox*>(m_pEnumListWidget->itemWidget(pItem));
                        assert(pCheckBoxItem != nullptr);
                        if (pCheckBoxItem != nullptr)
                        {
                            const QString& enumString = pCheckBoxItem->text();
                            tooltip += enumString;
                        }
                    }
                }
            }
        }
    }

    // Return the tooltip text used to show each enumerator name.
    tooltipText = tooltip.toStdString();
}

void rgPipelineStateEditorWidgetEnum::HandleUpdateEnumButtonText(const QString& text, bool checked)
{
    std::string tooltipText;
    GetTooltipString(tooltipText);

    std::string flagBits;
    GetFlagBitsString(flagBits);

    // Set the tooltip to display all the checked values as a text.
    ui.enumComboPushButton->setToolTip(tooltipText.c_str());

    // Set the flag bits string.
    UpdateSelectedEnum(flagBits);

    // The user altered the current value- signal to the parent row that editing is finished.
    emit EditingFinished();
}

void rgPipelineStateEditorWidgetEnum::SetValue(uint32_t value)
{
    if (m_isBitFlagsEnum)
    {
        for (int enumeratorIndex = 0; enumeratorIndex < static_cast<int>(m_enumerators.size()); ++enumeratorIndex)
        {
            // Set the push button's text to the item with the given value.
            auto pEnumItem = m_pEnumListWidget->item(enumeratorIndex);
            assert(pEnumItem != nullptr);
            if (pEnumItem != nullptr)
            {
                bool isChecked = (value & m_enumerators[enumeratorIndex].m_value) == m_enumerators[enumeratorIndex].m_value;

                QCheckBox* pCheckbox = qobject_cast<QCheckBox*>(m_pEnumListWidget->itemWidget(pEnumItem));
                assert(pCheckbox != nullptr);
                if (pCheckbox != nullptr)
                {
                    pCheckbox->setCheckState(isChecked ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
                }
            }
        }

        // Set the tooltip to display all the checked values as a text.
        std::string tooltipText;
        GetTooltipString(tooltipText);
        ui.enumComboPushButton->setToolTip(tooltipText.c_str());

        // Set the flag bits string.
        std::string flagBits;
        GetFlagBitsString(flagBits);
        UpdateSelectedEnum(flagBits);
    }
    else
    {
        // Find the given value in the list of enumerators, and set it as the
        // current value in the combo box containing all enumerator options.
        rgEnumeratorSearcher searcher(value);
        auto enumeratorIter = std::find_if(m_enumerators.begin(), m_enumerators.end(), searcher);
        if (enumeratorIter != m_enumerators.end())
        {
            // Set the push button's text to the item with the given value.
            int itemIndex = enumeratorIter - m_enumerators.begin();
            auto pEnumItem = m_pEnumListWidget->item(itemIndex);
            assert(pEnumItem != nullptr);
            if (pEnumItem != nullptr)
            {
                // Set the push button's text to the item with the given value.
                int itemIndex = enumeratorIter - m_enumerators.begin();
                auto pEnumItem = m_pEnumListWidget->item(itemIndex);
                assert(pEnumItem != nullptr);
                if (pEnumItem != nullptr)
                {
                    std::string enumString = pEnumItem->text().toStdString();
                    UpdateSelectedEnum(enumString);

                    // Update the current index member variable.
                    SetSelectedListRow(itemIndex);
                }
            }
        }

        // Update the button text with the selected entry.
        auto pEnumItem = m_pEnumListWidget->item(m_currentIndex);
        assert(pEnumItem != nullptr);
        if (pEnumItem != nullptr)
        {
            std::string enumString = pEnumItem->text().toStdString();
            UpdateSelectedEnum(enumString);
        }
    }
}

void rgPipelineStateEditorWidgetEnum::ConnectSignals()
{
    // Connect the enum arrow widget button clicked handler.
    bool isConnected = connect(this->ui.enumComboPushButton, &QPushButton::clicked, this, &rgPipelineStateEditorWidgetEnum::HandleEnumPushButtonClick);
    assert(isConnected);

    // Connect the arrow widget button focus in handler.
    isConnected = connect(ui.enumComboPushButton, &ArrowIconWidget::FocusInEvent, this, &rgPipelineStateEditorWidget::FocusInSignal);
    assert(isConnected);

    // Connect the arrow widget button focus out handler.
    isConnected = connect(ui.enumComboPushButton, &ArrowIconWidget::FocusOutEvent, this, &rgPipelineStateEditorWidgetEnum::HandleArrowButtonFocusOutEvent);
    assert(isConnected);

    // Connect the signal used to handle a change in the selected enum.
    isConnected = connect(m_pEnumListWidget, &QListWidget::currentRowChanged, this, &rgPipelineStateEditorWidgetEnum::HandleEnumChanged);
    assert(isConnected);

    // Connect the handler to close list widget on application's loss of focus.
    isConnected = connect(qApp, &QGuiApplication::applicationStateChanged, this, &rgPipelineStateEditorWidgetEnum::HandleApplicationFocusOutEvent);
    assert(isConnected);
}

void rgPipelineStateEditorWidgetEnum::HandleArrowButtonFocusOutEvent()
{
    HideListWidget();
}

void rgPipelineStateEditorWidgetEnum::HideListWidget()
{
    assert(m_pEnumListWidget != nullptr);
    if (m_pEnumListWidget != nullptr)
    {
        // List widget push button lost focus so hide the list widget.
        if (!m_pEnumListWidget->isHidden())
        {
            m_pEnumListWidget->hide();

            // Change the up arrow to a down arrow.
            ui.enumComboPushButton->SetDirection(ArrowIconWidget::Direction::DownArrow);

            // Emit the list widget status signal.
            emit EnumListWidgetStatusSignal(false);
        }
    }
}

void rgPipelineStateEditorWidgetEnum::HandleApplicationFocusOutEvent(Qt::ApplicationState state)
{
    assert(m_pEnumListWidget != nullptr);
    if (m_pEnumListWidget != nullptr && state != Qt::ApplicationState::ApplicationActive)
    {
        HideListWidget();
    }
}

void rgPipelineStateEditorWidgetEnum::HandleEnumChanged(int currentIndex)
{
    assert(m_pEnumListWidget != nullptr);
    if (m_pEnumListWidget != nullptr)
    {
        auto pEnumItem = m_pEnumListWidget->item(currentIndex);
        assert(pEnumItem != nullptr);
        if (pEnumItem != nullptr)
        {
            // Change the enum value if it differs from the current enum value.
            std::string currentEnum = ui.enumComboPushButton->text().toStdString();
            std::string newEnum = pEnumItem->text().toStdString();
            if (currentEnum.compare(newEnum) != 0)
            {
                // Use the dropdown list's selection model to change the currently selected enum.
                QItemSelectionModel* pSelectionModel = m_pEnumListWidget->selectionModel();
                assert(pSelectionModel != nullptr);
                if (pSelectionModel != nullptr)
                {
                    // Select the new enum within the dropdown list widget.
                    QAbstractItemModel* pListModel = m_pEnumListWidget->model();

                    assert(pListModel != nullptr);
                    if (pListModel != nullptr)
                    {
                        QModelIndex modelIndex = pListModel->index(currentIndex, 0);
                        pSelectionModel->setCurrentIndex(modelIndex, QItemSelectionModel::SelectionFlag::Select);
                    }
                }

                // Change the push button to the newly selected item.
                UpdateSelectedEnum(newEnum);

                // Update the current index member variable.
                m_currentIndex = currentIndex;

                emit EditingFinished();
            }
        }
    }
}

void rgPipelineStateEditorWidgetEnum::UpdateSelectedEnum(const std::string& newValue)
{
    static const int g_ARROW_WIDGET_EXTRA_WIDTH = 100;

    // Update the button text.
    ui.enumComboPushButton->setText(newValue.c_str());

    // Measure the width of the enum text, and add extra space to account for the width of the arrow.
    int scaledArrowWidth = static_cast<int>(g_ARROW_WIDGET_EXTRA_WIDTH * ScalingManager::Get().GetScaleFactor());
    int textWidth = QtCommon::QtUtil::GetTextWidth(ui.enumComboPushButton->font(), newValue.c_str());
    ui.enumComboPushButton->setMinimumWidth(scaledArrowWidth + textWidth);
}

void rgPipelineStateEditorWidgetEnum::HandleEnumPushButtonClick(bool /* checked */)
{
    // Make the list widget appear and process user selection from the list widget.
    bool visible = m_pEnumListWidget->isVisible();
    if (visible)
    {
        m_pEnumListWidget->hide();

        // Change the up arrow to a down arrow.
        ui.enumComboPushButton->SetDirection(ArrowIconWidget::Direction::DownArrow);

        // Emit the list widget status signal.
        emit EnumListWidgetStatusSignal(false);
    }
    else
    {
        // Compute where to place the combo box relative to where the arrow button is.
        QWidget* pWidget = ui.enumComboPushButton;
        QRect rect = pWidget->geometry();
        QPoint pos(0, 0);
        QMainWindow* pMainWindow = qobject_cast<QMainWindow*>(QApplication::activeWindow());
        pos = pWidget->mapTo(pMainWindow, pos);
        pos.setY(pos.y() + rect.height());
        int height = QtCommon::QtUtil::GetListWidgetHeight(m_pEnumListWidget);
        int width = QtCommon::QtUtil::GetListWidgetWidth(m_pEnumListWidget);
        m_pEnumListWidget->setGeometry(pos.x(), pos.y(), width + s_CHECK_BOX_WIDTH, height);
        m_pEnumListWidget->show();

        // Change the down arrow to an up arrow.
        ui.enumComboPushButton->SetDirection(ArrowIconWidget::Direction::UpArrow);

        // Emit the list widget status signal.
        emit EnumListWidgetStatusSignal(true);
    }
}

void rgPipelineStateEditorWidgetEnum::SetSelectedListRow(int rowIndex)
{
    // Update the current index member variable.
    m_currentIndex = rowIndex;

    // Reset the current selection in the enum list.
    m_pEnumListWidget->setCurrentRow(m_currentIndex);
}

void rgPipelineStateEditorWidgetEnum::CreateEnumControls()
{
    // Limit the maximum height of the popup ListWidget.
    // A vertical scrollbar will automatically be inserted if it's needed.
    static const int s_MAX_LIST_HEIGHT = 250;

    // Setup the list widget that opens when the user clicks the enum arrow.
    QMainWindow* pMainWindow = qobject_cast<QMainWindow*>(QApplication::activeWindow());
    rgUtils::SetupComboList(m_pParent, m_pEnumListWidget, ui.enumComboPushButton, m_pEnumListEventFilter, false);
    m_pEnumListWidget->setMaximumHeight(s_MAX_LIST_HEIGHT);
    m_pEnumListWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    // Set a unique object name for the enum combo push button.
    QString enumComboPushButtonObjectName = s_STR_ENUM_LIST_PUSH_BUTTON + QString::number(s_enumComboPushButtonCounter);
    ui.enumComboPushButton->setObjectName(enumComboPushButtonObjectName);

    // Add the enum combo push button object to the list widget event filter.
    rgHideListWidgetEventFilter* pEventFilter = qobject_cast<rgHideListWidgetEventFilter*>(m_pEnumListEventFilter);
    assert(pEventFilter != nullptr);
    if (pEventFilter != nullptr)
    {
        pEventFilter->AddObjectName(enumComboPushButtonObjectName);

        // Connect to event filter's list widget status signal.
        bool isConnected = connect(pEventFilter, &rgHideListWidgetEventFilter::EnumListWidgetStatusSignal, this, &rgPipelineStateEditorWidgetEnum::EnumListWidgetStatusSignal);
        assert(isConnected);
    }

    // Bump up the enum combo push button counter.
    UpdateEnumPushButtonCounter();

    // Update scale factor for widgets.
    QFont font = ui.enumComboPushButton->font();
    double scaleFactor = ScalingManager::Get().GetScaleFactor();
    font.setPointSize(s_PUSH_BUTTON_FONT_SIZE * scaleFactor);
    m_pEnumListWidget->setStyleSheet(s_LIST_WIDGET_STYLE.arg(font.pointSize()));

    SetSelectedListRow(0);

    // Set the list cursor to pointing hand cursor.
    m_pEnumListWidget->setCursor(Qt::PointingHandCursor);
}

void rgPipelineStateEditorWidgetEnum::HandleFlagCheckStateChanged(bool checked)
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
    if (pCheckBox != nullptr)
    {
        HandleUpdateEnumButtonText(pCheckBox->text(), checked);
    }
}

void rgPipelineStateEditorWidgetEnum::UpdateEnumPushButtonCounter()
{
    s_enumComboPushButtonCounter++;
}

int rgPipelineStateEditorWidgetEnum::GetEnumPushButtonCounter()
{
    return s_enumComboPushButtonCounter;
}
