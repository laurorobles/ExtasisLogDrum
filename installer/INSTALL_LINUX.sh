#!/bin/bash
set -e

echo "============================================================"
echo " ExtasisLogDrum v1.0.0 - Linux Installer (Free & Open Source)"
echo "============================================================"
echo ""

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

if [ -d "$SCRIPT_DIR/../ExtasisLogDrum.vst3" ] || [ -f "$SCRIPT_DIR/../ExtasisLogDrum" ]; then
    SRC_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
else
    SRC_DIR="$SCRIPT_DIR"
fi

VST3_DIR="$HOME/.vst3"
BIN_DIR="$HOME/.local/bin"

mkdir -p "$VST3_DIR" "$BIN_DIR"

INSTALLED=0

if [ -d "$SRC_DIR/ExtasisLogDrum.vst3" ]; then
    echo "[1/2] Installing VST3 to $VST3_DIR..."
    rm -rf "$VST3_DIR/ExtasisLogDrum.vst3"
    cp -R "$SRC_DIR/ExtasisLogDrum.vst3" "$VST3_DIR/"
    INSTALLED=1
else
    echo "[1/2] ExtasisLogDrum.vst3 not found in $SRC_DIR (skipping)"
fi

if [ -f "$SRC_DIR/ExtasisLogDrum" ]; then
    echo "[2/2] Installing Standalone App to $BIN_DIR..."
    cp "$SRC_DIR/ExtasisLogDrum" "$BIN_DIR/ExtasisLogDrum"
    chmod +x "$BIN_DIR/ExtasisLogDrum"
    INSTALLED=1
else
    echo "[2/2] ExtasisLogDrum standalone binary not found in $SRC_DIR (skipping)"
fi

echo ""
if [ "$INSTALLED" -eq 1 ]; then
    echo "============================================================"
    echo " Installation Complete! Open your DAW and rescan VST3 plugins."
    echo " Run 'ExtasisLogDrum' from ~/.local/bin for Standalone mode."
    echo " Licensed under GPL-3.0 | Extasis Records (@laurorobles)"
    echo "============================================================"
else
    echo "ERROR: No plugin bundles found next to the installer."
    exit 1
fi
