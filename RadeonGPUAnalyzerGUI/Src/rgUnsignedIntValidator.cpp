// C++.
#include <limits>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgUnsignedIntValidator.h>

rgUnsignedIntValidator::rgUnsignedIntValidator(QObject* pParent)
    : QValidator(pParent)
    , m_minimumValue(0)
    , m_maximumValue(std::numeric_limits<uint32_t>::max())
{
}

rgUnsignedIntValidator::rgUnsignedIntValidator(uint32_t bottom, uint32_t top, QObject* pParent)
    : QValidator(pParent)
    , m_minimumValue(bottom)
    , m_maximumValue(top)
{
}

QValidator::State rgUnsignedIntValidator::validate(QString& valueString, int&) const
{
    QValidator::State isValidState = QValidator::State::Invalid;

    if (valueString.isEmpty())
    {
        // If the string is empty, it's probably still being edited.
        isValidState = QValidator::State::Intermediate;
    }
    else
    {
        // Attempt to parse the given string as an unsigned integer.
        bool isOk = false;
        uint currentValue = valueString.toUInt(&isOk);

        if (isOk)
        {
            // Does the given string fall between the allowable value range?
            if (currentValue >= m_minimumValue && currentValue <= m_maximumValue)
            {
                isValidState = QValidator::State::Acceptable;
            }
        }
    }

    return isValidState;
}