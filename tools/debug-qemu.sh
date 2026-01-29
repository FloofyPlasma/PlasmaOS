#!/bin/bash

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="${BUILD_DIR:-$PROJECT_ROOT/build}"

ISO_PATH="$BUILD_DIR/plasma.iso"

if [ ! -f "$ISO_PATH" ]; then
  echo "Error: ISO not found at $ISO_PATH"
  echo "Please build the ISO first with: make iso"
  exit 1
fi

echo "== Running PlasmaOS in QEMU (Debug Mode) =="
echo "ISO: $ISO_PATH"
echo ""
echo "GDB stub listening on localhost:1234"
echo "To attach GDB, run in another terminal:"
echo "  gdb $BUILD_DIR/bin/plasma.elf"
echo "  (gdb) target remote :1234"
echo "  (gdb) continue"
echo ""

QEMU_OPTS=(
  -cdrom "$ISO_PATH"
  -serial stdio
  -m 512M
  -smp 2
  -no-reboot
  -no-shutdown
  -s
  -S
)

if [ -e /dev/kvm ] && [ -r /dev/kvm ] && [ -w /dev/kvm ]; then
  echo "KVM acceleration enabled"
  QEMU_OPTS+=(-enable-kvm)
fi

exec qemu-system-x86_64 "${QEMU_OPTS[@]}"
