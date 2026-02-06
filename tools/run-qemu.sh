#!/bin/bash

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="${BUILD_DIR:-$PROJECT_ROOT/build}"

ISO_PATH="$BUILD_DIR/plasma.iso"
DISK_PATH="$BUILD_DIR/disk.img"

if [ ! -f "$ISO_PATH" ]; then
  echo "Error: ISO not found at $ISO_PATH"
  echo "Please build the ISO first with: ninja iso"
  exit 1
fi

if [ ! -f "$DISK_PATH" ]; then
  echo "Creating virtual disk image"
  dd if=/dev/zero of="$DISK_PATH" bs=1M count=10 2>/dev/null
  echo "Virtual disk screated at $DISK_PATH"
fi

echo "== Running PlasmaOS in QEMU =="
echo "ISO: $ISO_PATH"
echo ""

QEMU_OPTS=(
  -cdrom "$ISO_PATH"
  -serial stdio
  -m 512M
  -smp 2
  -no-reboot
  -no-shutdown
)

if [ -e /dev/kvm ] && [ -r /dev/kvm ] && [ -w /dev/kvm ]; then
  echo "KVM acceleration enabled"
  QEMU_OPTS+=(-enable-kvm)
fi

exec qemu-system-x86_64 "${QEMU_OPTS[@]}" -d int,cpu_reset -D qemu.log  -drive file="$DISK_PATH",format=raw,if=ide
