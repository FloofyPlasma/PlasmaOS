OUTPUT_FORMAT(elf64-x86-64)
OUTPUT_ARCH(i386:x86-64)

ENTRY(_start)

/* Kernel is loaded at -2GB (higher half) */
KERNEL_VMA = 0xFFFFFFFF80000000;

PHDRS
{
  text PT_LOAD FLAGS((1 << 0) | (1 << 2)) ; /* Execute + Read */
  rodata PT_LOAD FLAGS((1 << 2)) ; /* Read only */
  data PT_LOAD FLAGS((1 << 1) | (1 << 2)) ; /* Write + Read */
}

SECTIONS
{
  . = KERNEL_VMA;

  .text : {
    *(.text.entry)
    *(.text .text.*)
  } : text

  . = ALIGN(4K);

  .rodata : {
    *(.rodata .rodata.*)
  } :rodata

  . = ALIGN(4K);

  .data : {
    *(.data .data.*)
  } :data

  .bss : {
    *(COMMON)
    *(.bss .bss.*)
  } :data

  . = ALIGN(4K);

  . = ALIGN(8);

    /* Limine requests section */
  .limine_requests : {
    KEEP(*(.limine_requests))
  } :data

    /* Discard unnecessary sections */
  /DISCARD/ : {
    *(.eh_frame)
    *(.note .note.*)
    *(.comment)
  }
}
