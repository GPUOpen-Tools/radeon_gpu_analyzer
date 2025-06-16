//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header for a Qt validator used to ensure that a given value remains between a specific range of unsigned integers.
//=============================================================================
#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_UNSIGNED_INT_VALIDATOR_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_UNSIGNED_INT_VALIDATOR_H_

// Qt.
#include <QValidator>

// A Qt validator used to ensure that a given value remains
// between a specific range of unsigned integers.
class RgUnsignedIntValidator : public QValidator
{
    Q_OBJECT
public:
    // The default constructor clamps the given value between the entire unsigned integer range.
    explicit RgUnsignedIntValidator(QObject* parent = nullptr);

    // A constructor used to clamp the given value between the given lower and upper bounds.
    explicit RgUnsignedIntValidator(uint32_t bottom, uint32_t top, QObject* parent = nullptr);
    virtual ~RgUnsignedIntValidator() = default;

    // Validate the current string against the minimum and maximum allowed values.
    virtual State validate(QString &, int &) const override;

private:
    // The minimum allowed value to clamp the valid range to.
    uint32_t minimum_value_;

    // The maximum allowed value to clamp the valid range to.
    uint32_t maximum_value_;
};
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_UNSIGNED_INT_VALIDATOR_H_
