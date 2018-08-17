@echo off

rem Make all variables defined in this script local.
SETLOCAL

set OUTPUT_DIR=%1
set QT_LIB_DIR=%2

IF "%3"=="-debug" (
    set DEBUG=TRUE
)

rem Create the output folders:
IF NOT exist %OUTPUT_DIR% mkdir %OUTPUT_DIR%
IF NOT exist %OUTPUT_DIR%\platforms mkdir %OUTPUT_DIR%\platforms
IF NOT exist %OUTPUT_DIR%\iconengines mkdir %OUTPUT_DIR%\iconengines
IF NOT exist %OUTPUT_DIR%\imageformats mkdir %OUTPUT_DIR%\imageformats
IF NOT exist %OUTPUT_DIR%\Documentation mkdir %OUTPUT_DIR%\Documentation
IF NOT exist %OUTPUT_DIR%\Documentation\html mkdir %OUTPUT_DIR%\Documentation\html

rem Copy Qt5 dlls
IF NOT [%QT_LIB_DIR%]==[] (
    IF DEFINED DEBUG (
        XCopy /r /d /y "%QT_LIB_DIR%\Qt5Cored.dll" "%OUTPUT_DIR%\"
        XCopy /r /d /y "%QT_LIB_DIR%\Qt5Guid.dll" "%OUTPUT_DIR%\"
        XCopy /r /d /y "%QT_LIB_DIR%\Qt5Widgetsd.dll" "%OUTPUT_DIR%\"
        XCopy /r /d /y "%QT_LIB_DIR%\Qt5Svgd.dll" "%OUTPUT_DIR%\"
        XCopy /r /d /y "%QT_LIB_DIR%\..\plugins\platforms\qwindowsd.dll" "%OUTPUT_DIR%\platforms\"
        XCopy /r /d /y "%QT_LIB_DIR%\..\plugins\iconengines\qsvgicond.dll" "%OUTPUT_DIR%\iconengines\"
        XCopy /r /d /y "%QT_LIB_DIR%\..\plugins\imageformats\qsvgd.dll" "%OUTPUT_DIR%\imageformats\"
    ) ELSE (
        XCopy /r /d /y "%QT_LIB_DIR%\Qt5Core.dll" "%OUTPUT_DIR%\"
        XCopy /r /d /y "%QT_LIB_DIR%\Qt5Gui.dll" "%OUTPUT_DIR%\"
        XCopy /r /d /y "%QT_LIB_DIR%\Qt5Widgets.dll" "%OUTPUT_DIR%\"
        XCopy /r /d /y "%QT_LIB_DIR%\Qt5Svg.dll" "%OUTPUT_DIR%\"
        XCopy /r /d /y "%QT_LIB_DIR%\..\plugins\platforms\qwindows.dll" "%OUTPUT_DIR%\platforms\"
        XCopy /r /d /y "%QT_LIB_DIR%\..\plugins\iconengines\qsvgicon.dll" "%OUTPUT_DIR%\iconengines\"
        XCopy /r /d /y "%QT_LIB_DIR%\..\plugins\imageformats\qsvg.dll" "%OUTPUT_DIR%\imageformats\"
    )
)

rem Copy documentation files
XCopy /r /e /d /y "..\..\Documentation\build\html" "%OUTPUT_DIR%\Documentation\html\"

