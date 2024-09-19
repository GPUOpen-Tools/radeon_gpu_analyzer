#######################################################################################################################
### Copyright (c) 2019-2024 Advanced Micro Devices, Inc. All rights reserved.
### @author AMD Developer Tools Team
#######################################################################################################################

cmake_minimum_required(VERSION 3.10)

# Attempt to automatically find Qt on the local machine
if (LINUX)
    find_package(Qt6 QUIET COMPONENTS Core Widgets Network Gui Svg Test GuiPrivate)
else ()
    find_package(Qt6 QUIET COMPONENTS Core Widgets Network Gui Svg Test)
endif ()

if (Qt6_DIR)
    message(STATUS "Qt6 cmake package found at ${Qt6_DIR}")
endif ()

if (NOT Qt6_DIR)
    # Attempt to query Qt 5
    if (LINUX)
        find_package(Qt5 QUIET COMPONENTS Core Widgets Network Gui Test X11Extras)
    else ()
        find_package(Qt5 QUIET COMPONENTS Core Widgets Network Gui Test)
    endif ()

    if (Qt5_DIR)
        message(STATUS "Qt5 cmake package found at ${Qt5_DIR}")
    else ()
        message(WARNING "Qt5 cmake package not found. Please specify Qt5_DIR manually or in the CMAKE_PREFIX_PATH")
    endif ()
endif ()

if (Qt5_DIR OR Qt6_DIR)
    #######################################################################################################################
    # Setup the INSTALL target to include Qt DLLs
    ##
    # Logic used to find and use Qt's internal deployment tool to copy Qt-based dependencies and DLLs upon building, so
    # that we can run the application from an IDE and so that we just have a simple build directory with all dependencies
    # already deployed that we can easily distribute
    #######################################################################################################################
    get_target_property(_qmake_executable Qt::qmake IMPORTED_LOCATION)
    get_filename_component(_qt_bin_dir "${_qmake_executable}" DIRECTORY)
    if (WIN32)
        find_program(DEPLOYQT_EXECUTABLE windeployqt HINTS "${_qt_bin_dir}")
    elseif (LINUX)
        find_program(QT_QMAKE_EXECUTABLE qmake HINTS "${_qt_bin_dir}")
        set(DEPLOYQT_EXECUTABLE ${LINUXDEPLOYQT})
    endif ()

    function(deploy_qt_build target)

        if (DEPLOYQT_EXECUTABLE)
            if (WIN32)
                if (Qt6_DIR)
                    set(DEPLOYQT_POST_BUILD_COMMAND
                            ${DEPLOYQT_EXECUTABLE} $<TARGET_FILE:${target}> -verbose 0 --no-compiler-runtime --no-translations --no-system-d3d-compiler --no-system-dxc-compiler --no-opengl-sw --no-network
                            WORKING_DIRECTORY ${PROJECT_BINARY_DIR})
                else ()
                    set(DEPLOYQT_POST_BUILD_COMMAND
                            ${DEPLOYQT_EXECUTABLE} $<TARGET_FILE:${target}> -verbose 0 --no-compiler-runtime --no-translations --no-angle --no-system-d3d-compiler --no-opengl-sw
                            WORKING_DIRECTORY ${PROJECT_BINARY_DIR})
                endif ()
            elseif (LINUX)
                set(DEPLOYQT_POST_BUILD_COMMAND
                        ${CMAKE_COMMAND} -E env LD_LIBRARY_PATH=${EXTERNAL_DIR}/libtraceevent/lib:${EXTERNAL_DIR}/libtracefs/lib
                        ${DEPLOYQT_EXECUTABLE} $<TARGET_FILE:${target}> -qmake=${QT_QMAKE_EXECUTABLE} -verbose=0 -unsupported-allow-new-glibc -no-translations 
                        WORKING_DIRECTORY ${PROJECT_BINARY_DIR})
            endif ()

            # Deploy Qt to build directory after a successful build
            add_custom_command(
                    TARGET ${target} POST_BUILD
                    COMMAND ${DEPLOYQT_POST_BUILD_COMMAND}
            )
        endif ()
    endfunction()

    function(deploy_qt_install_hook target component)
        if (DEPLOYQT_EXECUTABLE)
            deploy_qt_build(${target})

            set(target_file_dir "$<TARGET_FILE_DIR:${target}>")

            # Handle installation of Qt dependencies
            if (WIN32)
                # Debug dlls end with a `d.dll`
                set(qt_suffix "$<$<CONFIG:Debug>:d>.dll")

                # Due to windows requiring DLLs be shipped adjacent we must be explicit here...
                # TODO: Maybe eventually we could look into some sort of manifest file?
                if (Qt5_DIR)
                    install(FILES
                            ${target_file_dir}/Qt5Core${qt_suffix}
                            ${target_file_dir}/Qt5Gui${qt_suffix}
                            ${target_file_dir}/Qt5Svg${qt_suffix}
                            ${target_file_dir}/Qt5Widgets${qt_suffix}
                            DESTINATION . COMPONENT ${component})

                    install(FILES ${target_file_dir}/Qt5Network${qt_suffix} DESTINATION . COMPONENT ${component} OPTIONAL)
                else ()
                    install(FILES
                            ${target_file_dir}/Qt6Core${qt_suffix}
                            ${target_file_dir}/Qt6Gui${qt_suffix}
                            ${target_file_dir}/Qt6Svg${qt_suffix}
                            ${target_file_dir}/Qt6Widgets${qt_suffix}
                            DESTINATION . COMPONENT ${component})

                    install(FILES ${target_file_dir}/Qt6Network${qt_suffix} DESTINATION . COMPONENT ${component} OPTIONAL)
                endif ()

                install(DIRECTORY ${target_file_dir}/iconengines DESTINATION . COMPONENT ${component})
                install(DIRECTORY ${target_file_dir}/imageformats DESTINATION . COMPONENT ${component})
                install(DIRECTORY ${target_file_dir}/platforms DESTINATION . COMPONENT ${component})
                install(DIRECTORY ${target_file_dir}/styles DESTINATION . COMPONENT ${component})
            elseif (LINUX)
                install(FILES
                        ${target_file_dir}/qt.conf
                        DESTINATION . COMPONENT ${component})
                install(DIRECTORY ${target_file_dir}/lib DESTINATION . COMPONENT ${component})
                install(DIRECTORY ${target_file_dir}/plugins DESTINATION . COMPONENT ${component})
            endif ()

        else ()
            message(FATAL_ERROR "Qt deployment executable not found.")
        endif ()
    endfunction()
endif ()
