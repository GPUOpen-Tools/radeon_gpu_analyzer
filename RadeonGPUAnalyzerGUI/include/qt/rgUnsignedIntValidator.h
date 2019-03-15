#pragma once

// Qt.
#include <QValidator>

// A Qt validator used to ensure that a given value remains
// between a specific range of unsigned integers.
class rgUnsignedIntValidator : public QValidator
{
    Q_OBJECT
public:
    // The default constructor clamps the given value between the entire unsigned integer range.
    explicit rgUnsignedIntValidator(QObject* pParent = nullptr);

    // A constructor used to clamp the given value between the given lower and upper bounds.
    explicit rgUnsignedIntValidator(uint32_t bottom, uint32_t top, QObject* pParent = nullptr);
    virtual ~rgUnsignedIntValidator() = default;

    // Validate the current string against the minimum and maximum allowed values.
    virtual State validate(QString &, int &) const override;

private:
    // The minimum allowed value to clamp the valid range to.
    uint32_t m_minimumValue;

    // The maximum allowed value to clamp the valid range to.
    uint32_t m_maximumValue;
};