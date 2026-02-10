include(kernel)

set(KERNEL_SOURCES
        main.c
        limine.c
        serial.c
        thread.c
        gdt.c
        idt.c
        pit.c
        pmm.c
        vmm.c
        syscall.c
        ata.c
        ext2.c
)

set(KERNEL_ASM_SOURCES
        entry.asm
        switch.asm
        gdt_asm.asm
        idt_asm.asm
        usermode.asm
        syscall_asm.asm
)

add_executable(plasma.elf
        ${KERNEL_SOURCES}
        ${KERNEL_ASM_SOURCES}
)

set_source_files_properties(${KERNEL_ASM_SOURCES}
        PROPERTIES
        COMPILE_FLAGS "-f elf64"
        LANGUAGE ASM_NASM
)

configure_kernel_target(plasma.elf)

target_include_directories(plasma.elf PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/limine-protocol/include
)

set_target_properties(plasma.elf PROPERTIES
        OUTPUT_NAME plasma
        SUFFIX .elf
)

add_custom_command(TARGET plasma.elf POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "Kernel build: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/plasma.elf"
)
