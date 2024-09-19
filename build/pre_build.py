#! python3
# Copyright (c) 2020-2022 Advanced Micro Devices, Inc. All rights reserved.
#
# Script to perform all necessary pre build steps. This includes:
#
#   - Fetching all dependencies
#   - Creating output directories
#   - Calling CMake with appropriate parameters to generate the build files
#
import os
import sys
import importlib.util
import argparse
import shutil
import subprocess
import platform
import time

# prevent generation of .pyc file
sys.dont_write_bytecode = True

# Globals.
# Store script root folder to support running this script from anywhere.
SCRIPT_ROOT = os.path.dirname(os.path.realpath(__file__))

# Store the base script name.
SCRIPT_NAME = os.path.basename(__file__)

# Function definitions.

# Command line parser.
parser = None

def parse_arguments():
    global parser
    # initialize some paths for default argument values
    if sys.platform == "win32":
        output_root = os.path.join(SCRIPT_ROOT, "win")
    elif sys.platform == "darwin":
        output_root = os.path.join(SCRIPT_ROOT, "mac")
    else:
        output_root = os.path.join(SCRIPT_ROOT, "linux")

    # parse the command line arguments
    parser = argparse.ArgumentParser(description="A script that generates all the necessary build dependencies for a project")
    parser.add_argument("--qt", default="6.7.0", help="specify the version of QT to be used with the script (default: 6.7.0)" )
    parser.add_argument("--clean", action="store_true", help="delete any directories created by this script")
    parser.add_argument("--no-qt", action="store_true", help="build a headless version (not applicable for all products)")
    parser.add_argument("--no-vulkan", action="store_true", help="build a version without vulkan support")
    parser.add_argument("--no-dx10", action="store_true", help="build a version without dx10 support")
    parser.add_argument("--build-number", default="0",
                        help="specify the build number, primarily to be used by build machines to produce versioned builds")
    parser.add_argument("--internal", action="store_true", help="configure internal builds of the RGA applications (only used within AMD")
    parser.add_argument("--automation", action="store_true", help="configure builds for the automated GUI test application (only used within AMD)")
    parser.add_argument("--update", action="store_true", help="Instruct fetch_dependencies script to update all dependencies")
    parser.add_argument("--force", action="store_true", help="Instruct fetch_dependencies script to reclone any dependencies that don't match the dependency map origin URL")
    parser.add_argument("--output", default=output_root, help="specify the output location for generated cmake and build output files (default = OS specific subdirectory of location of PreBuild.py script)")
    parser.add_argument("--build", action="store_true", help="build all supported configurations on completion of prebuild step")
    parser.add_argument("--build-jobs", default="4", help="number of simultaneous jobs to run during a build (default = 4)")
    parser.add_argument("--analyze", action="store_true", help="perform static analysis of code on build")
    parser.add_argument("--vscode", action="store_true", help="generate CMake options into VsCode settings file for this project")
    parser.add_argument("--platform", default="x64", choices=["x64", "x86"], help="specify the platform (32 or 64 bit)")
    if sys.platform == "win32":
        parser.add_argument("--cppcheck", action="store_true", help="create a cppcheck project in the Visual Studio solution")
        parser.add_argument("--vs", default="2022", choices=["2017", "2019", "2022"], help="specify the version of Visual Studio to be used with this script (default: 2019)")
        parser.add_argument("--toolchain", default=None, choices=["2017", "2019", "2022"], help="specify the compiler toolchain to be used with this script (default: 2019)")
        parser.add_argument("--qt-root", default="C:\\Qt", help="specify the root directory for locating QT on this system (default: C:\\Qt\\)")
        parser.add_argument("--qt-libver", default="2019", choices=["2017", "2019"], help="specify the Qt lib version to be used with this script (default: 2019)")
        parser.add_argument("--vk-include", action="store", help="specify the Vulkan SDK include folder")
        parser.add_argument("--vk-lib", action="store", help="specify the Vulkan SDK lib folder")
    elif sys.platform == "darwin":
        parser.add_argument("--xcode", action="store_true", help="specify Xcode should be used as generator for CMake")
        parser.add_argument("--no-bundle", action="store_true", help="specify macOS application should be built as standard executable instead of app bundle")
        parser.add_argument("--qt-root", default="~/Qt", help="specify the root directory for locating QT on this system (default: ~/Qt) ")
    else:
        parser.add_argument("--qt-root", default="~/Qt", help="specify the root directory for locating QT on this system (default: ~/Qt) ")
        parser.add_argument("--disable-extra-qt-lib-deploy", action="store_true", help="prevent extra Qt library files (XCB and ICU libs) from being copied during post build step")
        parser.add_argument("--vk-include", help="specify the Vulkan SDK include folder")
        parser.add_argument("--vk-lib", help="specify the Vulkan SDK lib folder")
        parser.add_argument("--cppcheck", action="store_true", help="Add cppcheck step to make builds.")

    args = parser.parse_args()
    if sys.platform == "win32":
        if args.toolchain is None:
            args.toolchain = args.vs
    if args.force is True:
        if not args.update is True:
            log_error_and_exit("Invalid command line syntax: --force is only valid with --update")

    return args


# Print a message to the console with appropriate pre-amble
def log_print(message):
    print ("\n" + SCRIPT_NAME + ": " + message)


# Print an error message to the console with appropriate pre-amble then exit
def log_error_and_exit(message):
    print ("\nERROR: " + SCRIPT_NAME + ": " + message)
    sys.stdout.flush()
    sys.exit(-1)


# Remove a directory and all subdirectories - printing relevant status
def rmdir_print(dir):
    log_print ("Removing directory - " + dir)
    if os.path.exists(dir):
        try:
            shutil.rmtree(dir)
        except Exception as e:
            log_error_and_exit ("Failed to delete directory - " + dir + ": " + str(e))
    else:
        log_print ("    " + dir + " doesn't exist!")


# Make a directory if it doesn't exist - print information
def mkdir_print(dir):
    if not os.path.exists(dir):
        log_print ("Creating Directory: " + dir)
        os.makedirs(dir)


# Generate the full path to QT, converting path to OS specific form
# Look for Qt path in specified Qt root directory
# Example:
# --qt-root=C:\\Qt
# --qt=6.7.0
# Look first for C:\\Qt\\Qt6.7.0\\6.7.0
#  (if not found..)
# Look next for C:\\Qt\\6.7.0
#
# If neither of those can be found AND we are using the default
# qt-root dir (i.e. the user did not specify --qt-root), then also
# go up one directory from qt-root and check both permutations
# again. This allows the default Qt install path on Linux to be
# found without needing to specify a qt-root
#
# Returns a tuple, containing a boolean and a string.
# If the boolean is True, then the string is the found path to Qt.
# If the boolean is False, then the string is an error message indicating which paths were searched
def check_qt_path(qt_root, qt_root_arg, qt_arg):
    qt_path_not_found_error = "Unable to find Qt root dir. Use --qt-root to specify\n    Locations checked:"
    qt_path = os.path.normpath(os.path.join(qt_root, "Qt" + qt_arg, qt_arg))
    if not os.path.exists(qt_path):
        qt_path_not_found_error = qt_path_not_found_error + "\n      " + qt_path
        qt_path = os.path.normpath(os.path.join(qt_root, qt_arg))
        if not os.path.exists(qt_path):
            qt_path_not_found_error = qt_path_not_found_error + "\n      " + qt_path
            # if there is no user-specified qt-root, then check additional locations
            # used by the various Qt installers
            if qt_root_arg == parser.get_default('qt-root'):
                qt_path = os.path.normpath(os.path.join(qt_root, "..", "Qt" + qt_arg, qt_arg))
                if not os.path.exists(qt_path):
                    qt_path_not_found_error = qt_path_not_found_error + "\n      " + qt_path
                    qt_path = os.path.normpath(os.path.join(qt_root, "..", qt_arg))
                    if not os.path.exists(qt_path):
                        qt_path_not_found_error = qt_path_not_found_error + "\n      " + qt_path
                        return False, qt_path_not_found_error
            else:
                return False, qt_path_not_found_error
    return True, qt_path


# Common code related to generating a build configuration
def generate_config(config, args):
    if (config != ""):
        cmake_dir = os.path.join(args.output, config)
        mkdir_print(cmake_dir)
    else:
        cmake_dir = args.output

    cmakelist_path = os.path.join(SCRIPT_ROOT, os.path.normpath(".."))

    release_output_dir = os.path.join(args.output, "release")
    debug_output_dir = os.path.join(args.output, "debug")

    # Specify the type of Build files to generate
    cmake_generator = None
    if sys.platform == "win32":
        if args.vs == "2022":
            cmake_generator="Visual Studio 17 2022"
        elif args.vs == "2019":
            cmake_generator="Visual Studio 16 2019"
        else:
            cmake_generator="Visual Studio 15 2017"
            if not support_32_bit_build or args.platform != "x86":
                cmake_generator = cmake_generator + " Win64"

    elif sys.platform == "darwin":
        if args.xcode:
            cmake_generator="Xcode"
        else:
            cmake_generator="Unix Makefiles"
    else:
        cmake_generator="Unix Makefiles"

    # Configure Qt dependency.
    if not args.no_qt:
        # locate the relevant QT libraries
        # generate the platform specific portion of the QT path name
        if sys.platform == "win32":
            qt_leaf = "msvc" + args.qt_libver + "_64"
        elif sys.platform == "darwin":
            qt_leaf = "clang_64"
        else:
            qt_leaf = "gcc_64"

        qt_expanded_root = os.path.expanduser(args.qt_root)
        qt_found,qt_path = check_qt_path(qt_expanded_root, args.qt_root, args.qt)

        if qt_found == False:
            log_error_and_exit(qt_path)

        qt_path = os.path.normpath(qt_path + "/"  + qt_leaf)

        if not os.path.exists(qt_path) and not args.no_qt:
            log_error_and_exit ("QT Path does not exist - " + qt_path)

        cmake_args = ["cmake", cmakelist_path, "-DCMAKE_PREFIX_PATH=" + qt_path, "-DQT_PACKAGE_ROOT=" + qt_path, "-G", cmake_generator]
    else:
        cmake_args = ["cmake", cmakelist_path, "-DHEADLESS=TRUE", "-DBUILD_CLI_ONLY=ON"]

    if args.no_dx10:
        cmake_args.extend(["-DRGA_DISABLE_DX10=ON"])

    if sys.platform == "win32":
        if args.vs != "2017":
            if args.platform != "x86":
                cmake_args.extend(["-A" + "x64"])
            if args.toolchain == "2019":
                cmake_args.extend(["-Tv142"])
            elif args.toolchain == "2017":
                cmake_args.extend(["-Tv141"])

    # Use build number.
    cmake_args.extend(["-DRGA_BUILD_NUMBER=" + str(args.build_number)])

    # Add Vulkan include and lib paths
    if not args.vk_include is None:
        cmake_args.extend(["-DVULKAN_SDK_INC_DIR=" + str(args.vk_include)])
    if not args.vk_lib is None:
        cmake_args.extend(["-DVULKAN_SDK_LIB_DIR=" + str(args.vk_lib)])

    if args.no_vulkan:
        cmake_args.extend(["-DRGA_ENABLE_VULKAN=OFF"])

    if sys.platform.startswith('linux'):
        if args.disable_extra_qt_lib_deploy:
            cmake_args.extend(["-DDISABLE_EXTRA_QT_LIB_DEPLOY:BOOL=TRUE"])

    cmake_args.extend(["-DCMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE=" + release_output_dir])
    cmake_args.extend(["-DCMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE=" + release_output_dir])
    cmake_args.extend(["-DCMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG=" + debug_output_dir])
    cmake_args.extend(["-DCMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG=" + debug_output_dir])

    if sys.platform != "win32":
        if "RELEASE" in config.upper():
            cmake_args.extend(["-DCMAKE_BUILD_TYPE=Release"])
        elif "DEBUG" in config.upper():
            cmake_args.extend(["-DCMAKE_BUILD_TYPE=Debug"])
        else:
            log_error_and_exit("unknown configuration: " + config)

    if sys.platform == "darwin":
        cmake_args.extend(["-DNO_APP_BUNDLE=" + str(args.no_bundle)])


    if args.vscode:
        # Generate data into VSCode Settings file

        # only generate this file once - even though parent function is called twice on linux platforms
        if (config == "") or (config == "debug"):
            import json

            vscode_json_path = cmakelist_path + "/.vscode"
            vscode_json_file = vscode_json_path + "/settings.json"

            log_print ("Updating VSCode settings file: " + vscode_json_file)

            if os.path.isfile(vscode_json_file):
                # if file exists load the contents as a JSON data blob
                with open(vscode_json_file) as f:
                    json_data = json.load(f)
            else:
                # file doesn't exist
                if not os.path.isdir(vscode_json_path):
                    # .vscode directory doesn't exist - create it
                    os.mkdir(vscode_json_path)
                # initialize dictionary with no data
                json_data = {}

            cmake_configure_args = []
            for index, arg in enumerate(cmake_args):
                # only include cmake arguments starting with -D.
                # Ignore -DCMAKE_BUILD_TYPE as it's not relevant for VSCode builds
                if not arg.startswith("-DCMAKE_BUILD_TYPE") and arg.startswith("-D"):
                    cmake_configure_args.append(arg)
                if arg.startswith("-G"):
                    # next argument is the generator type to be captured
                    json_data['cmake.generator'] = cmake_args[index + 1]
            json_data['cmake.configureArgs'] = cmake_configure_args

            # define the build directory used by cmake
            cmake_build_directory = cmake_dir
            if (config == "debug"):
                # On linux - use the build type from vscode to define the config
                cmake_build_directory = cmake_build_directory.replace("debug", "${buildType}", 1)
            json_data['cmake.buildDirectory'] = cmake_build_directory

            with open(vscode_json_file, 'w') as f:
                json.dump(json_data, f, indent=4)

    if not shutil.which(cmake_args[0]):
        log_error_and_exit("cmake not found")

    log_print("DEBUG: cmake command: %s"%cmake_args)
    p = subprocess.Popen(cmake_args, cwd=cmake_dir, stderr=subprocess.STDOUT)
    p.wait()
    sys.stdout.flush()
    if(p.returncode != 0):
        log_error_and_exit("cmake failed with %d" % p.returncode)


def main():
    # prevent pre_build.py script from leaving a compiled .pyc file in the script directory
    sys.dont_write_bytecode = True

    # Remember the start time
    start_time = time.time()

    # Enable/Disable options supported by this project by manually editing the value.

    # Parse command line arguments.
    script_args = parse_arguments()

    # only import fetch_dependencies.py if the script exists
    can_fetch = True
    if importlib.util.find_spec("fetch_dependencies") is not None:
        import fetch_dependencies
    else:
        can_fetch = False

    # Define the build configurations that will be generated
    configs = ["debug", "release"]

    if script_args.analyze and not script_args.build:
        log_error_and_exit("--analyze option requires the --build option to also be specified")

    # check that the default output directory exists
    mkdir_print(script_args.output)

    # Define the output directory for CMake generated files
    cmake_output_dir = None
    test_suffix = ""
    if script_args.automation:
        test_suffix = "_test"

    if sys.platform == "win32":
        toolchain_suffix = ""
        if script_args.toolchain != script_args.vs:
            if script_args.toolchain == "2017":
                toolchain_suffix = "_msvc141"
            if script_args.toolchain == "2019":
                toolchain_suffix = "_msvc142"
        cmake_output_dir = os.path.join(script_args.output, "vs" + script_args.vs + toolchain_suffix + test_suffix)
    else:
        cmake_output_dir = os.path.join(script_args.output, "make" + test_suffix)

    # Clean all files generated by this script or the build process
    if (script_args.clean):
        log_print ("Cleaning build ...\n")
        # delete the CMake output directory
        rmdir_print(cmake_output_dir)
        # delete the build output directories
        for config in configs:
            dir = os.path.join(script_args.output, config)
            rmdir_print(dir)
        sys.exit(0)

    # Create the CMake output directory
    script_args.output = cmake_output_dir
    mkdir_print(cmake_output_dir)

    # Call fetch_dependencies script
    if can_fetch:
        log_print ("Fetching project dependencies ...\n")
        if (fetch_dependencies.do_fetch_dependencies(script_args.update, script_args.internal, script_args.force) == False):
            log_error_and_exit("Unable to retrieve dependencies")

    log_print("Generating build files ...")
    if sys.platform == "win32":
        # On Windows always generates both Debug and Release configurations in a single solution file
        generate_config("", script_args)
    else:
        # For Linux & Mac - generate both Release and Debug configurations
        for config in configs:
            generate_config(config, script_args)

    if script_args.cppcheck:
        cmake_args.append("-DCMAKE_CXX_CPPCHECK")

    # Optionally, the user can choose to build all configurations on conclusion of the prebuild job
    if (script_args.build):
        for config in configs:
            log_print( "\nBuilding " + config + " configuration\n")
            build_dir = ""

            cmake_args_docs = ""
            if sys.platform == "win32":
                build_dir = cmake_output_dir

                # For Visual Studio, specify the config to build
                cmake_args = ["cmake", "--build", build_dir, "--config", config, "--target", "ALL_BUILD", "--",  "/m:" + script_args.build_jobs]
                if script_args.analyze:
    #                cmake_args.append("/p:CodeAnalysisTreatWarningsAsErrors=true")
    #                cmake_args.append("/p:CodeAnalysisRuleSet=NativeRecommendedRules.ruleset")
                    cmake_args.append("/p:CodeAnalysisRuleSet=NativeMinimumRules.ruleset")
                    cmake_args.append("/p:RunCodeAnalysis=true")

                cmake_args_docs = ["cmake", "--build", build_dir, "--config", config, "--target", "Documentation", "--", "/m:" + script_args.build_jobs]
            else:
                # linux & mac use the same commands
                # generate the path to the config specific makefile
                build_dir = os.path.join(cmake_output_dir, config + config_suffix)

                cmake_args = ["cmake", "--build", build_dir, "--parallel", script_args.build_jobs]

                cmake_args_docs = ["cmake", "--build", build_dir, "--config", config, "--target", "Documentation", "--parallel", script_args.build_jobs]

            p = subprocess.Popen(cmake_args, cwd=cmake_output_dir, stderr=subprocess.STDOUT)
            p.wait()
            sys.stdout.flush()

            if(p.returncode != 0):
                log_error_and_exit("CMake build failed with %d" % p.returncode)

            log_print( "\nBuilding Documentation\n")

            p = subprocess.Popen(cmake_args_docs, cwd=cmake_output_dir, stderr=subprocess.STDOUT)
            p.wait()
            sys.stdout.flush()

    minutes, seconds = divmod(time.time() - start_time, 60)
    log_print("Successfully completed in {0:.0f} minutes, {1:.1f} seconds".format(minutes,seconds))
    sys.exit(0)


if __name__ == '__main__':
    main()
