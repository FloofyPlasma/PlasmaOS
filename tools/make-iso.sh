#!/bin/bash

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="${BUILD_DIR:-$PROJECT_ROOT/build}"

echo "== PlasmaOS ISO Generator =="
echo "Build directory: $BUILD_DIR"

if [ ! -f "$BUILD_DIR/bin/plasma.elf" ]; then
  echo "Error: Kernel not found at $BUILD_DIR/bin/plasma.elf"
  echo "Please build the kernel first with: cmake --build $BUILD_DIR"
  exit 1
fi

ISO_ROOT="$BUILD_DIR/iso_root"
rm -rf "$ISO_ROOT"
mkdir -p "$ISO_ROOT/boot"

LIMINE_DIR="$PROJECT_ROOT/kernel/limine"
if [ ! -d "$LIMINE_DIR" ]; then
  echo "Error: Limine submodule not found at $LIMINE_DIR"
  echo "Please run: git submodule update --init --recursive"
  exit 1
fi

if [ ! -f "$LIMINE_DIR/limine" ]; then
  echo "Building limine utility..."
  make -C "$LIMINE_DIR" limine >/dev/null 2>&1 || cc -O2 "$LIMINE_DIR/limine.c" -o "$LIMINE_DIR/limine"
fi

echo "Copying kernel..."
cp "$BUILD_DIR/bin/plasma.elf" "$ISO_ROOT/boot/"

echo "Copying userland programs..."
if [ -f "$BUILD_DIR/userland/init.bin" ]; then
  cp "$BUILD_DIR/userland/init.bin" "$ISO_ROOT/boot/"
  echo "  init.bin copied"
else
  echo "Error: init.bin not found at $BUILD_DIR/userland/init.bin"
  echo "Userland may not have built correctly"
  exit 1
fi

echo "Creating Limine configuration..."
cat >"$ISO_ROOT/limine.conf" <<'EOF'
timeout: 1

/PlasmaOS
  protocol: limine
  kernel_path: boot():/boot/plasma.elf
  module_path: boot():/boot/init.bin
EOF

echo "Copying Limine bootloader files..."
cp "$LIMINE_DIR/limine-bios.sys" "$ISO_ROOT/boot/"
cp "$LIMINE_DIR/limine-bios-cd.bin" "$ISO_ROOT/boot/"
cp "$LIMINE_DIR/limine-uefi-cd.bin" "$ISO_ROOT/boot/"

mkdir -p "$ISO_ROOT/EFI/BOOT"
cp "$LIMINE_DIR/BOOTX64.EFI" "$ISO_ROOT/EFI/BOOT/"
cp "$LIMINE_DIR/BOOTIA32.EFI" "$ISO_ROOT/EFI/BOOT/"

echo "Generating ISO image..."
xorriso -as mkisofs \
  -b boot/limine-bios-cd.bin \
  -no-emul-boot -boot-load-size 4 -boot-info-table \
  --efi-boot boot/limine-uefi-cd.bin \
  -efi-boot-part --efi-boot-image --protective-msdos-label \
  "$ISO_ROOT" -o "$BUILD_DIR/plasma.iso" 2>/dev/null

echo "Installing Limine to ISO..."
"$LIMINE_DIR/limine" bios-install "$BUILD_DIR/plasma.iso" 2>/dev/null

echo ""
echo "ISO image created $BUILD_DIR/plasma.iso"
echo ""
