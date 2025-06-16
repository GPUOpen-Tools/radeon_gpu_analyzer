//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Implementation for the unsaved items dialog.
//=============================================================================

// C++.
#include <limits>

// Local.
#include "radeon_gpu_analyzer_gui/qt/rg_unsigned_int_validator.h"

RgUnsignedIntValidator::RgUnsignedIntValidator(QObject* parent)
    : QValidator(parent)
    , minimum_value_(0)
    , maximum_value_(std::numeric_limits<uint32_t>::max())
{
}

RgUnsignedIntValidator::RgUnsignedIntValidator(uint32_t bottom, uint32_t top, QObject* parent)
    : QValidator(parent)
    , minimum_value_(bottom)
    , maximum_value_(top)
{
}

QValidator::State RgUnsignedIntValidator::validate(QString& value_string, int&) const
{
    QValidator::State is_valid_state = QValidator::State::Invalid;

    if (value_string.isEmpty())
    {
        // If the string is empty, it's probably still being edited.
        is_valid_state = QValidator::State::Intermediate;
    }
    else
    {
        // Attempt to parse the given string as an unsigned integer.
        bool is_ok = false;
        uint current_value = value_string.toUInt(&is_ok);

        if (is_ok)
        {
            // Does the given string fall between the allowable value range?
            if (current_value >= minimum_value_ && current_value <= maximum_value_)
            {
                is_valid_state = QValidator::State::Acceptable;
            }
        }
    }

    return is_valid_state;
}
