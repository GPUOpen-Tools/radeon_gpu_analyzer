#pragma once

// Standard truncation lengths.
// @TODO - Determine appropriate values.
static const int gs_TEXT_TRUNCATE_LENGTH_FRONT = 10;
static const int gs_TEXT_TRUNCATE_LENGTH_BACK = 10;

// The constant used when the selected line in the source file doesn't have any correlated disassembly lines.
static const int kInvalidCorrelationLineIndex = -1;

// The duration, in milliseconds, of how long that the status bar text will remain before being cleared.
const int gs_STATUS_BAR_NOTIFICATION_TIMEOUT_MS = 3000;

// The font size in point for various RGA buttons.
const int gs_BUTTON_POINT_FONT_SIZE = 8;

// The property name used to block recursive style repolishing.
static const char* gs_IS_REPOLISHING_BLOCKED = "isRepolishingBlocked";

// Home page definitions.
static const char* gs_ACTION_HOTKEY_OPEN_PROJECT    = "Ctrl+Alt+O";
static const char* gs_ACTION_HOTKEY_BACK_TO_HOME    = "Ctrl+Alt+H";
static const char* gs_ACTION_HOTKEY_EXIT            = "Alt+F4";
static const char* gs_ACTION_HOTKEY_ABOUT           = "Ctrl+F1";
static const char* gs_ACTION_HOTKEY_HELP_MANUAL     = "F1";

// Focus navigation hotkey definitions.
static const char* gs_ACTION_HOTKEY_NEXT_VIEW       = "Ctrl+Tab";
static const char* gs_ACTION_HOTKEY_PREVIOUS_VIEW   = "Shift+Ctrl+Tab";

// File menu hotkey definitions.
static const char* gs_ACTION_HOTKEY_FILE_MENU_NEXT_ITEM         = "Down";
static const char* gs_ACTION_HOTKEY_FILE_MENU_PREV_ITEM         = "Up";
static const char* gs_ACTION_HOTKEY_FILE_MENU_CONTEXT_MENU      = "Shift+F10";
static const char* gs_ACTION_HOTKEY_FILE_MENU_ACTIVATE_RETURN   = "Return";
static const char* gs_ACTION_HOTKEY_FILE_MENU_ACTIVATE_ENTER    = "Enter";
static const char* gs_ACTION_HOTKEY_FILE_MENU_RENAME            = "F2";

// Edit menu hotkey definitions.
static const char* gs_ACTION_HOTKEY_GO_TO_LINE  = "Ctrl+G";
static const char* gs_ACTION_HOTKEY_FIND        = "Ctrl+F";

// Build menu hotkey definitions.
static const char* gs_ACTION_HOTKEY_BUILD_PROJECT   = "Ctrl+Shift+B";
static const char* gs_ACTION_HOTKEY_BUILD_SETTINGS  = "F8";
static const char* gs_ACTION_HOTKEY_BUILD_CANCEL = "Ctrl+Shift+C";

// Icon resource paths.
static const char* gs_ICON_RESOURCE_RGA_LOGO                = ":/icons/rgaIcon.svg";
static const char* gs_ICON_RESOURCE_FIND_MAGNIFYING_GLASS   = ":/icons/magnifyingGlassIcon.svg";

// Macros.
#define RG_SAFE_DELETE(ptr) { delete ptr; ptr = nullptr;}