#pragma once

// The mode string used for OpenCL mode.
static const char* STR_MODE_STRING_OPENCL = "rocm-cl";

// The mode string used for Vulkan mode.
static const char* STR_MODE_STRING_VULKAN = "vulkan";

// The default entry point name for Vulkan GLSL shaders.
static const char* STR_DEFAULT_VULKAN_GLSL_ENTRYPOINT_NAME = "main";

// Standard truncation lengths.
static const int gs_TEXT_TRUNCATE_LENGTH_FRONT       = 10;
static const int gs_TEXT_TRUNCATE_LENGTH_BACK        = 10;
static const int gs_TEXT_TRUNCATE_LENGTH_BACK_OPENCL = 10;
static const int gs_TEXT_TRUNCATE_LENGTH_BACK_VULKAN = 2;

// The constant used when the selected line in the source file doesn't have any correlated disassembly lines.
static const int kInvalidCorrelationLineIndex = -1;

// The duration, in milliseconds, of how long that the status bar text will remain before being cleared.
const int gs_STATUS_BAR_NOTIFICATION_TIMEOUT_MS = 3000;

// The font size in point for various RGA buttons.
const int gs_BUTTON_POINT_FONT_SIZE = 8;

// The font size for build view.
static const int gs_BUILD_VIEW_FONT_SIZE = 10;

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
static const char* gs_ACTION_HOTKEY_FILE_MENU_NEXT_ITEM          = "Down";
static const char* gs_ACTION_HOTKEY_FILE_MENU_PREV_ITEM          = "Up";
static const char* gs_ACTION_HOTKEY_FILE_MENU_ACTIVATE_SPACE     = "Space";
static const char* gs_ACTION_HOTKEY_FILE_MENU_CONTEXT_MENU       = "Shift+F10";
static const char* gs_ACTION_HOTKEY_FILE_MENU_ACTIVATE_RETURN    = "Return";
static const char* gs_ACTION_HOTKEY_FILE_MENU_ACTIVATE_ENTER     = "Enter";
static const char* gs_ACTION_HOTKEY_FILE_MENU_RENAME             = "F2";
static const char* gs_ACTION_HOTKEY_FILE_MENU_ACTIVATE_TAB       = "Tab";
static const char* gs_ACTION_HOTKEY_FILE_MENU_ACTIVATE_SHIFT_TAB = "Shift+Tab";

// Edit menu hotkey definitions.
static const char* gs_ACTION_HOTKEY_GO_TO_LINE      = "Ctrl+G";
static const char* gs_ACTION_HOTKEY_FIND            = "Ctrl+F";
static const char* gs_ACTION_HOTKEY_FIND_NEXT       = "F3";
static const char* gs_ACTION_HOTKEY_FIND_PREVIOUS   = "Shift+F3";

// Build menu hotkey definitions.
static const char* gs_ACTION_HOTKEY_BUILD_PROJECT   = "Ctrl+Shift+B";
static const char* gs_ACTION_HOTKEY_BUILD_SETTINGS  = "F8";
static const char* gs_ACTION_HOTKEY_PIPELINE_STATE  = "F9";
static const char* gs_ACTION_HOTKEY_BUILD_CANCEL    = "Ctrl+Shift+C";

// Source view hotkey definitions.
static const char* gs_SOURCE_EDITOR_HOTKEY_CONTEXT_MENU_CUT = "Ctrl+X";
static const char* gs_SOURCE_EDITOR_HOTKEY_CONTEXT_MENU_COPY = "Ctrl+C";
static const char* gs_SOURCE_EDITOR_HOTKEY_CONTEXT_MENU_PASTE = "Ctrl+V";
static const char* gs_SOURCE_EDITOR_HOTKEY_CONTEXT_MENU_SELECT_ALL = "Ctrl+A";

// Disassembly view hot key definitions.
static const char* gs_DISASSEMBLY_VIEW_HOTKEY_GPU_SELECTION = "Ctrl+T";

// View container hot key definitions.
static const char* gs_SWITCH_CONTAINER_SIZE = "Ctrl+R";

// Icon resource paths.
static const char* gs_ICON_RESOURCE_RGA_LOGO                = ":/icons/rgaIcon.svg";
static const char* gs_ICON_RESOURCE_FIND_MAGNIFYING_GLASS   = ":/icons/magnifyingGlassIcon.svg";
static const char* gs_ICON_RESOURCE_EXPANDED_ROW            = ":/icons/expandFileItem.svg";
static const char* gs_ICON_RESOURCE_COLLAPSED_ROW           = ":/icons/collapsedArrow.svg";
static const char* gs_ICON_RESOURCE_REMOVE_NOTIFICATION     = ":/icons/correlationWarningIcon.svg";

// Macros.
#define RG_SAFE_DELETE(ptr) { delete ptr; ptr = nullptr;}

#ifdef WIN32
    #define STRCPY(dst, size, src) strcpy_s(dst, size, src)
#else
    #define STRCPY(dst, size, src) strncpy(dst, src, size)
#endif