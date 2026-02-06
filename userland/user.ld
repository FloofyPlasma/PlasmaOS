OUTPUT_FORMAT(elf64-x86-64)
OUTPUT_ARCH(i386:x86-64)

ENTRY(_start)

/* Userspace programs load at standard location */
USER_BASE = 0x400000;

PHDRS
{
  text PT_LOAD FLAGS((1 << 0) | (1 << 2)) ; /* Execute + Read */
  rodata PT_LOAD FLAGS((1 << 2)) ; /* Read only */
  data PT_LOAD FLAGS((1 << 1) | (1 << 2)) ; /* Write + Read */
}

SECTIONS
{
  . = USER_BASE;

  .text : {
    KEEP(*(.text._start))
    *(.text.startup)
    *(.text*)
  } :text

  . = ALIGN(4K);

  .rodata : {
    *(.rodata*)
  } :rodata

  . = ALIGN(4K);

  .data : {
    *(.data*)
  } :data

  .bss : {
    *(COMMON)
    *(.bss*)
  } :data

  . = ALIGN(4K);

  /* Discard unnecessary sections */
  /DISCARD/ : {
    *(.eh_frame)
    *(.note*)
    *(.comment)
    *(.gnu*)
  }
}