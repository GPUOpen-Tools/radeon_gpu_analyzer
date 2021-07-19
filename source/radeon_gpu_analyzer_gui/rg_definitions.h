#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_RG_DEFINITIONS_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_RG_DEFINITIONS_H_

// The mode string used for OpenCL mode.
static const char* kStrModeStringOpencl = "opencl";

// The mode string used for Vulkan mode.
static const char* kStrModeStringVulkan = "vulkan";

// The default entry point name for Vulkan GLSL shaders.
static const char* kStrDefaultVulkanGlslEntrypointName = "main";

// Standard truncation lengths.
static const int kTextTruncateLengthFront       = 10;
static const int kTextTruncateLengthBack        = 10;
static const int kTextTruncateLengthBackOpencl  = 10;
static const int kTextTruncateLengthBackVulkan  = 2;

// The constant used when the selected line in the source file doesn't have any correlated disassembly lines.
static const int kInvalidCorrelationLineIndex = -1;

// The duration, in milliseconds, of how long that the status bar text will remain before being cleared.
const int kStatusBarNotificationTimeoutMs = 3000;

// The font size in point for various RGA buttons.
const int kButtonPointFontSize = 8;

// The font size for build view.
static const int kBuildViewFontSize = 10;

// The property name used to block recursive style repolishing.
static const char* kIsRepolishingBlocked = "isRepolishingBlocked";

// Home page definitions.
static const char* kActionHotkeyOpenProject   = "Ctrl+Alt+O";
static const char* kActionHotkeyBackToHome    = "Ctrl+Alt+H";
static const char* kActionHotkeyExit          = "Alt+F4";
static const char* kActionHotkeyAbout         = "Ctrl+F1";
static const char* kActionHotkeyHelpManual    = "F1";

// Focus navigation hotkey definitions.
static const char* kActionHotkeyNextView       = "Ctrl+Tab";
static const char* kActionHotkeyPreviousView   = "Shift+Ctrl+Tab";

// File menu hotkey definitions.
static const char* kActionHotkeyFileMenuNextItem          = "Down";
static const char* kActionHotkeyFileMenuPrevItem          = "Up";
static const char* kActionHotkeyFileMenuActivateSpace     = "Space";
static const char* kActionHotkeyFileMenuContextMenu       = "Shift+F10";
static const char* kActionHotkeyFileMenuActivateReturn    = "Return";
static const char* kActionHotkeyFileMenuActivateEnter     = "Enter";
static const char* kActionHotkeyFileMenuRename             = "F2";
static const char* kActionHotkeyFileMenuActivateTab       = "Tab";
static const char* kActionHotkeyFileMenuActivateShiftTab = "Shift+Tab";

// Edit menu hotkey definitions.
static const char* kActionHotkeyGoToLine      = "Ctrl+G";
static const char* kActionHotkeyFind            = "Ctrl+F";
static const char* kActionHotkeyFindNext       = "F3";
static const char* kActionHotkeyFindPrevious   = "Shift+F3";

// Build menu hotkey definitions.
static const char* kActionHotkeyBuildProject   = "Ctrl+Shift+B";
static const char* kActionHotkeyBuildSettings  = "F8";
static const char* kActionHotkeyPipelineState  = "F9";
static const char* kActionHotkeyBuildCancel    = "Ctrl+Shift+X";

// Source view hotkey definitions.
static const char* kSourceEditorHotkeyContextMenuCut = "Ctrl+X";
static const char* kSourceEditorHotkeyContextMenuCopy = "Ctrl+C";
static const char* kSourceEditorHotkeyContextMenuPaste = "Ctrl+V";
static const char* kSourceEditorHotkeyContextMenuSelectAll = "Ctrl+A";

// Disassembly view hot key definitions.
static const char* kDisassemblyViewHotkeyGpuSelection = "Ctrl+T";

// View container hot key definitions.
static const char* kSwitchContainerSize = "Ctrl+R";

// Restore default settings hot key.
static const char* kRestoreDefaultSettings = "Ctrl+R";

// Icon resource paths.
static const char* kIconResourceRgaLogo                = ":/icons/rga_icon.png";
static const char* kIconResourceFindMagnifyingGlass    = ":/icons/magnifying_glass_icon.svg";
static const char* kIconResourceExpandedRow            = ":/icons/expand_file_item.svg";
static const char* kIconResourceCollapsedRow           = ":/icons/collapsed_arrow.svg";
static const char* kIconResourceRemoveNotification     = ":/icons/correlation_warning_icon.svg";

// Build settings options list delimiter.
static const char* kOptionsListDelimiter = ";";

// Macros.
#define RG_SAFE_DELETE(ptr) { delete ptr; ptr = nullptr;}

#ifdef WIN32
    #define STRCPY(dst, size, src) strcpy_s(dst, size, src)
#else
    #define STRCPY(dst, size, src) strncpy(dst, src, size)
#endif
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_RG_DEFINITIONS_H_
