set -e
OUTPUT_DIR=$1
QT_LIB_DIR=$2
QT_PLUGINS_DIR=$3
AUTOMATION=$4

# Copy the Qt5 dynamic libs
if [ -n "$QT_LIB_DIR" ]; then
  if [ ! -d "$OUTPUT_DIR/lib" ]; then
    mkdir -p $OUTPUT_DIR/lib
  fi
  if [ ! -d "$OUTPUT_DIR/plugins/platforms" ]; then
    mkdir -p $OUTPUT_DIR/plugins/platforms
  fi
  if [ ! -d "$OUTPUT_DIR/plugins/imageformats" ]; then
    mkdir -p $OUTPUT_DIR/plugins/imageformats
  fi
  cp $QT_LIB_DIR/libQt5Core.so.5 $OUTPUT_DIR/lib/
  cp $QT_LIB_DIR/libQt5DBus.so.5 $OUTPUT_DIR/lib/
  cp $QT_LIB_DIR/libQt5Gui.so.5 $OUTPUT_DIR/lib/
  cp $QT_LIB_DIR/libQt5Widgets.so.5 $OUTPUT_DIR/lib/
  cp $QT_LIB_DIR/libQt5XcbQpa.so.5 $OUTPUT_DIR/lib/
  cp $QT_LIB_DIR/libicui18n.so.50 $OUTPUT_DIR/lib/
  cp $QT_LIB_DIR/libicudata.so.50 $OUTPUT_DIR/lib/
  cp $QT_LIB_DIR/libicuuc.so.50 $OUTPUT_DIR/lib/
  cp $QT_LIB_DIR/libQt5Svg.so.5 $OUTPUT_DIR/lib/
  cp $QT_PLUGINS_DIR/imageformats/libqsvg.so $OUTPUT_DIR/plugins/imageformats/
  cp $QT_PLUGINS_DIR/platforms/libqxcb.so $OUTPUT_DIR/plugins/platforms/
  if [ "$AUTOMATION" = "-automation" ]; then
    cp $QT_LIB_DIR/libQt5Test.so.5 $OUTPUT_DIR/lib/
  fi
fi

# Copy the GUI launch script.
cp ./RadeonGPUAnalyzer $OUTPUT_DIR/
chmod +x $OUTPUT_DIR/RadeonGPUAnalyzer

# Copy the automation files/folders.
if [ "$AUTOMATION" = "-automation" ]; then
  cp -rf ../../../../RGA-Internal/tests/data $OUTPUT_DIR/
  cp ../../../../RGA-Internal/tests_gui/run.py $OUTPUT_DIR/
  chmod +x $OUTPUT_DIR/run.py
fi

# Copy the Radeon Tools Download Assistant.
cp ../../../../Common/Src/update_check_api/rtda/linux/rtda $OUTPUT_DIR/
chmod +x $OUTPUT_DIR/rtda

