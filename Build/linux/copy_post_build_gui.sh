
QT_LIB_DIR=$1

# Copy the Qt5 dynamic libs
if [ -n "$QT_LIB_DIR" ]; then
  cp $QT_LIB_DIR/libQt5Core.so* ../../output/bin
  cp $QT_LIB_DIR/libQt5Gui.so* ../../output/bin
  cp $QT_LIB_DIR/libQt5Widgets.so* ../../output/bin
fi
