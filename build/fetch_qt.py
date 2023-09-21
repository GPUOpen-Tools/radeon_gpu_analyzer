#! python3
# Script to download and unzip/untar Qt
#
#   fetch_qt.py
#

import argparse
import os
import platform
import shutil
import sys
import tarfile
import zipfile

# prevent generation of .pyc file
sys.dont_write_bytecode = True

# Check for the python 3.x name and import it as the 2.x name
try:
    import urllib.request as urllib
# if that failed, then try the 2.x name
except ImportError:
    import urllib

# to allow the script to be run from anywhere - not just the cwd - store the absolute path to the script file
SCRIPT_ROOT = os.path.dirname(os.path.realpath(__file__))

# also store the basename of the file
SCRIPT_NAME = os.path.basename(__file__)

# define the Qt archive locations on artifactory
# TODO: this will change to use the "Releases/Stable" tree in artifactory once we promote Qt builds to that tree
QT_PACKAGE_BASE_URL = "http://bdcartifactory.amd.com:80/artifactory/DevToolsBDC/Builds/qtBuild/BuildQt/66/"

if sys.platform == "win32":
    qt_package_url = QT_PACKAGE_BASE_URL + "Windows/Qt5.15.2-win.zip"
elif sys.platform == "darwin":
    qt_package_url = QT_PACKAGE_BASE_URL + "Mac/Qt5.15.2-mac.tgz"
elif sys.platform.startswith('linux') == True:
    qt_package_url = QT_PACKAGE_BASE_URL + "Centos/Qt5.15.2-centos.tgz"
else:
    log_print("Unsupported Platform")
    sys.exit(-1)

# Print a message to the console with appropriate pre-amble
def log_print(message):
    print ("\n%s: %s"%(SCRIPT_NAME, message))
    sys.stdout.flush()

# Download the Qt zip or tgz file from a URL and unzip into the directory defined by target_qt_dir.
# The destination directory will be created if it doesn't exist
# TODO - this function needs to handle errors gracefully when URL is incorrect or inaccessible
def do_fetch_qt(target_qt_dir, retry_count = 10):
    # convert targetPath to OS specific format
    tmp_path = os.path.join(SCRIPT_ROOT, target_qt_dir)

    # clean up path, collapsing any ../ and converting / to \ for Windows
    target_path = os.path.normpath(tmp_path)

    # make target directory if it doesn't exist
    if not os.path.isdir(target_path):
        os.makedirs(target_path)

    # generate the target zip file name from the source URL filename and the target path
    # note - this rule currently handles URLs that contain # and ? characters at the end
    # those currently used by Jenkins don't have this style
    zip_file_name = qt_package_url.split('/')[-1].split('#')[0].split('?')[0]
    zip_path = os.path.join(target_path, zip_file_name)

    if os.path.isfile(zip_path):
        # File exists - print message and continue
        log_print("URL Dependency %s found and not updated" % zip_path)
    else:
        # File doesn't exist - download and unpack it
        log_print("Downloading " + qt_package_url + " into " + zip_path)
        try:
            urllib.urlretrieve(qt_package_url, zip_path)
        except urllib.ContentTooShortError:
            os.remove(zip_path)
            if retry_count > 0:
                log_print("URL content too short. Retrying. Retries remaining: %d" % retry_count)
                do_fetch_qt(target_qt_dir, retry_count - 1)
            return;

        # Unpack the downloaded file into the target directory
        if os.path.splitext(zip_path)[1] == ".zip":
            # if file extension is .zip then unzip it
            log_print("Extracting in " + target_path)
            zipfile.ZipFile(zip_path).extractall(target_path)
        elif os.path.splitext(zip_path)[1] == ".tgz":
            # if file extension is .tgz then untar it
            log_print("Extracting in " + target_path)
            tarfile.open(zip_path).extractall(target_path)

        # After unzipping, delete the archive (.zip or .tgz) so as not to take extra space on disk
        os.remove(zip_path)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="A script that fetches Qt from Artifactory")
    if sys.platform == "win32":
        parser.add_argument("--qt-root", default="C:\\Qt", help="specify the root directory in which to fetch QT on this system (default: C:\\Qt\\)")
    else:
        # Linux and Mac use this path
        parser.add_argument("--qt-root", default="~/Qt", help="specify the root directory in which to fetch QT on this system (default: ~/Qt) ")
    parser.add_argument("--qt", default="5.15.2", help="specify the version of QT to be used with the script (default: 5.15.2)" )
    parser.add_argument("--force", action="store_true", help="always fetch Qt, forcibly replacing an existing version found on disk")

    args = parser.parse_args()

    qt_expanded_root = os.path.expanduser(args.qt_root)
    qt_path = os.path.normpath(qt_expanded_root + "/" + "Qt" + args.qt + "/" + args.qt)

    do_fetch = True
    qt_found = os.path.exists(qt_path)
    if qt_found:
        delete_existing_qt = args.force
        if delete_existing_qt == False:
            user_confirm = input("Existing Qt build found. Remove it and fetch Qt again? (type 'yes' to confirm): ")
            if user_confirm == 'yes':
               delete_existing_qt = True
        if delete_existing_qt:
            log_print("**************************************")
            log_print("***** Deleting existing Qt build *****")
            log_print("**************************************")
            shutil.rmtree(qt_path)
            qt_found = False
        else:
            log_print("Aborting fetch of Qt -- Qt already found.")


    if qt_found == False:
        do_fetch_qt(qt_expanded_root, retry_count = 10)
