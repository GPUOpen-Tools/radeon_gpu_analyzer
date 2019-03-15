
OUTPUT_DIR=$1
QT_LIB_DIR=$2
QT_PLUGINS_DIR=$3
AUTOMATION=$4

# Copy the Qt5 dynamic libs
if [ -n "$QT_LIB_DIR" ]; then
  if [ ! -d "$OUTPUT_DIR/Qt" ]; then
    mkdir $OUTPUT_DIR/Qt
  fi
  if [ ! -d "$OUTPUT_DIR/Qt/platforms" ]; then
    mkdir $OUTPUT_DIR/Qt/platforms
  fi
  if [ ! -d "$OUTPUT_DIR/Qt/platforms/imageformats" ]; then
    mkdir $OUTPUT_DIR/Qt/platforms/imageformats
  fi
  cp -d $QT_LIB_DIR/libQt5Core.so* $OUTPUT_DIR/Qt/
  cp -d $QT_LIB_DIR/libQt5DBus.so* $OUTPUT_DIR/Qt/
  cp -d $QT_LIB_DIR/libQt5Gui.so* $OUTPUT_DIR/Qt/
  cp -d $QT_LIB_DIR/libQt5Widgets.so* $OUTPUT_DIR/Qt/
  cp -d $QT_LIB_DIR/libQt5XcbQpa.so* $OUTPUT_DIR/Qt/
  cp -d $QT_LIB_DIR/libicui18n.so* $OUTPUT_DIR/Qt/
  cp -d $QT_LIB_DIR/libicudata.so* $OUTPUT_DIR/Qt/
  cp -d $QT_LIB_DIR/libicuuc.so* $OUTPUT_DIR/Qt/
  cp -d $QT_LIB_DIR/libQt5Svg.so.* $OUTPUT_DIR/Qt/
  cp -d $QT_PLUGINS_DIR/imageformats/libqsvg.so $OUTPUT_DIR/Qt/platforms/imageformats
  cp -d $QT_PLUGINS_DIR/platforms/libqxcb.so $OUTPUT_DIR/Qt/platforms
  if [ "$AUTOMATION" = "-automation" ]; then
    cp -d $QT_LIB_DIR/libQt5Test.so* $OUTPUT_DIR/Qt/
  fi
fi

# Copy the documentation files
if [ ! -d "$OUTPUT_DIR/Documentation" ]; then
    mkdir $OUTPUT_DIR/Documentation
fi
if [ ! -d "$OUTPUT_DIR/Documentation/html" ]; then
    mkdir $OUTPUT_DIR/Documentation/html
fi
cp -rf ../../../Documentation/build/html $OUTPUT_DIR/Documentation

# Copy the GUI launch script.
cp ./RadeonGPUAnalyzerGUI $OUTPUT_DIR/
chmod +x $OUTPUT_DIR/RadeonGPUAnalyzerGUI

# Copy the automation files/folders.
if [ "$AUTOMATION" = "-automation" ]; then
  cp -rf ../../../../RGA-Internal/Tests/data $OUTPUT_DIR/
  cp ../../../../RGA-Internal/Tests-GUI/run.py $OUTPUT_DIR/
fi

# Copy the AMDToolsDownloader.
cp ../../../../Common/Src/UpdateCheckAPI/AMDToolsDownloader/Linux/AMDToolsDownloader $OUTPUT_DIR/
chmod +x $OUTPUT_DIR/AMDToolsDownloader

