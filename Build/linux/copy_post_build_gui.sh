
QT_LIB_DIR=$1

# Copy the Qt5 dynamic libs
if [ -n "$QT_LIB_DIR" ]; then
  if [ ! -d "../../output/bin/Qt" ]; then
    mkdir ../../output/bin/Qt
  fi
  if [ ! -d "../../output/bin/Qt/platforms" ]; then
    mkdir ../../output/bin/Qt/platforms
  fi
  if [ ! -d "../../output/bin/Qt/platforms/imageformats" ]; then
    mkdir ../../output/bin/Qt/platforms/imageformats
  fi
  cp -d $QT_LIB_DIR/libQt5Core.so* ../../output/bin/Qt/
  cp -d $QT_LIB_DIR/libQt5DBus.so* ../../output/bin/Qt/
  cp -d $QT_LIB_DIR/libQt5Gui.so* ../../output/bin/Qt/
  cp -d $QT_LIB_DIR/libQt5Widgets.so* ../../output/bin/Qt/
  cp -d $QT_LIB_DIR/libQt5XcbQpa.so* ../../output/bin/Qt/
  cp -d $QT_LIB_DIR/libicui18n.so* ../../output/bin/Qt/
  cp -d $QT_LIB_DIR/libicudata.so* ../../output/bin/Qt/
  cp -d $QT_LIB_DIR/libicuuc.so* ../../output/bin/Qt/
  cp -d $QT_LIB_DIR/libQt5Svg.so.* ../../output/bin/Qt/
  cp -d $QT_LIB_DIR/../plugins/imageformats/libqsvg.so ../../output/bin/Qt/platforms/imageformats
  cp -d $QT_LIB_DIR/../plugins/platforms/libqxcb.so ../../output/bin/Qt/platforms
fi

# Copy the documentation files
if [ ! -d "../../output/bin/Documentation" ]; then
    mkdir ../../output/bin/Documentation
fi
if [ ! -d "../../output/bin/Documentation/html" ]; then
    mkdir ../../output/bin/Documentation/html
fi
cp -rf ../../Documentation/build/html ../../output/bin/Documentation

# Copy the GUI launch script.
cp ./RadeonGPUAnalyzerGUI ../../output/bin/
chmod +x ../../output/bin/RadeonGPUAnalyzerGUI

