#!/bin/bash

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="$PROJECT_ROOT/build/Desktop-Debug"
PACKAGES_DIR="$SCRIPT_DIR/packages"
OUTPUT_FILE="$SCRIPT_DIR/Nomen-1.0.0-linux-x86_64.run"

echo "SCRIPT_DIR: $SCRIPT_DIR"
echo "PROJECT_ROOT: $PROJECT_ROOT"
echo "BUILD_DIR: $BUILD_DIR"
echo "PACKAGES_DIR: $PACKAGES_DIR"

echo ""
echo "Copying Nomen Binary from Debug Build..."
if [ -f "$BUILD_DIR/Nomen" ]; then
    mkdir -p "$PACKAGES_DIR/com.nomen.app/data/bin"
    cp "$BUILD_DIR/Nomen" "$PACKAGES_DIR/com.nomen.app/data/bin/"
    chmod +x "$PACKAGES_DIR/com.nomen.app/data/bin/Nomen"
    echo "OK - Binary Copied Successfully"
else
    echo "NOK - Binary not Found at $BUILD_DIR/Nomen"
    exit 1
fi

echo ""
echo "Copying Extension Libraries..."
if [ -d "$BUILD_DIR/extensions/ingestModules" ]; then
    mkdir -p "$PACKAGES_DIR/com.nomen.app/data/extensions"
    find "$BUILD_DIR/extensions/ingestModules" -name "*.so" -exec cp {} "$PACKAGES_DIR/com.nomen.app/data/extensions/" \;
    echo "OK - Extensions Copied Successfully"
else
    echo "NOK - Extensions directory not Found"
    exit 1
fi

echo ""
echo "Building Offline Installer: $(basename $OUTPUT_FILE)"
/usr/bin/binarycreator \
    --offline-only \
    -i com.nomen.app \
    -t /usr/bin/installerbase \
    -p "$PACKAGES_DIR" \
    -c "$SCRIPT_DIR/config/config.xml" \
    "$OUTPUT_FILE"

if [ $? -eq 0 ]; then
    chmod +x "$OUTPUT_FILE"
    echo "OK - Installer Created Successfully: $OUTPUT_FILE"
else
    echo "NOK - Installer Creation Failed"
    exit 1
fi
