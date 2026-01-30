#!/bin/bash

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="${BUILD_DIR:-$PROJECT_ROOT/build}"

ISO_PATH="$BUILD_DIR/plasma.iso"

if [ ! -f "$ISO_PATH" ]; then
  echo "Error: ISO not found at $ISO_PATH"
  echo "Please build the ISO first with: ninja iso"
  exit 1
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

exec qemu-system-x86_64 "${QEMU_OPTS[@]}" -d int,cpu_reset -D qemu.log
