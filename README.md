# PlasmaOS

A hobbyist x86-64 operating system built from scratch, featuring physical and virtual memory management, a filesystem driver, and userspace support.

## Features

- **Memory Management**
  - Physical memory manager (PMM)
  - Virtual memory manager (VMM)
  - Kernel and userspace address space separation
  
- **Filesytems**
  - ext2 filesystem driver with read support
  - ATA/IDE disk driver for storage access

- **Hardware Support**
  - GDT (Global Descriptor Table) configuration
  - IDT (Interrupt Descriptor Table) with interrupt handling
  - Serial port driver for debugging output

- **Userspace**
  - System call interface
  - Userspace program loading and execution
  - Basic threading support

- **Boot**
  - Limine bootloader integration
  - Boots on x86-64 systems in QEMU

## Building

### Prerequisites
- CMake 3.20+
- x86_64-elf cross-compiler toolchain (clang recommended)
- NASM assembler
- QEMU (for testing)
- xorriso (for creating bootable ISO)

### Build Instructions
```bash
git clone https://github.com/floofyplasma/PlasmaOS
cd PlasmaOS
mkdir build && cd build
cmake ..
make
```
### Creating Bootable ISO
```bash
cd tools
./make-iso.sh
```

### Running in QEMU
```bash
cd tools
./run-qemu.sh
```

For debugging with GDB:
```bash
cd tools
./debug-qemu.sh
```

## Current Status

**Working:**
- Boots successfully via Limine bootloader
- Physical and virtual memory management
- ext2 filesystem reading from root directory
- ATA disk I/O
- Interrupt handling and system calls
- Basic userspace program execution
- Serial output for debugging

**Limitations:**
- ext2 driver only supports reading files from root directory (no subdirectory traversal)
- No write support for filesystem
- Limited userspace functionality
- Cooperative multitasking based threading

**Future Goals:**
- Full ext2 directory traversal
- Filesystem write support
- Multi-tasking scheduler
- More comprehensive userspace API
- Additional filesystem support

## Technical Details

**Architecture:**

PlasmaOS is designed for x86-64 systems and uses the Limine boot protocol for initialization.

**Filesystem:**
- The ext2 driver parses superblocks, block groups, and inodes
- Currently supports reading regular files from the root directory
- ATA driver uses PIO mode for disk access

**Syscall Interface:**
- System calls transition from userspace (ring 3) to kernel (ring 0)
- Implemented in assembly with int 0x80

## Debugging

Serial output is available on COM1 (0x3F8) for debugging. When running in QEMU, serial output is redirected to the terminal.

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.
