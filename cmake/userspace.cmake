set(USERSPACE_C_FLAGS
        -ffreestanding
        -nostdlib
        -fno-builtin
        -fno-stack-protector
        -fno-pic
        -fno-pie
        -static
        -m64
)

set(USERSPACE_WARNING_FLAGS
        -Wall
        -Wextra
        -Werror=implicit-function-declaration
        -Werror=return-type
        -Wno-unused-parameter
)

set(USERSPACE_DEBUG_FLAGS
        -g
        -gdwarf-4
)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(USERSPACE_OPT_FLAGS -O0)
else ()
    set(USERSPACE_OPT_FLAGS -O2)
endif ()

set(USERSPACE_COMPILE_FLAGS
        ${USERSPACE_C_FLAGS}
        ${USERSPACE_WARNING_FLAGS}
        ${USERSPACE_DEBUG_FLAGS}
        ${USERSPACE_OPT_FLAGS}
)

set(USERSPACE_LINK_FLAGS
        -nostdlib
        -static
        -no-pie
)

function(configure_userspace_target target)
    target_compile_options(${target} PRIVATE
            $<$<COMPILE_LANGUAGE:C>:${USERSPACE_COMPILE_FLAGS}>
            $<$<COMPILE_LANGUAGE:CXX>:${USERSPACE_COMPILE_FLAGS}>
    )

    target_link_options(${target} PRIVATE ${USERSPACE_LINK_FLAGS})

    target_link_options(${target} PRIVATE
            -T ${CMAKE_SOURCE_DIR}/userland/user.ld
    )

    set_target_properties(${target} PROPERTIES
            LINK_DEPENDS ${CMAKE_SOURCE_DIR}/userland/user.ld
    )

    target_include_directories(${target} PRIVATE
            ${CMAKE_SOURCE_DIR}/kernel/include
    )
endfunction()

# Helper function to create a userspace binary
function(add_userspace_binary name)
    set(elf_target ${name}.elf)
    set(bin_target ${name}.bin)

    add_executable(${elf_target} ${ARGN})
    configure_userspace_target(${elf_target})

    # Convert ELF to flat binary
    add_custom_command(
            OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${bin_target}
            COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${elf_target}> ${CMAKE_CURRENT_BINARY_DIR}/${bin_target}
            DEPENDS ${elf_target}
            COMMENT "Creating flat binary ${bin_target}"
            VERBATIM
    )

    add_custom_target(${name}_bin ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${bin_target})

    # Export path for ISO building
    set(${name}_BIN_PATH ${CMAKE_CURRENT_BINARY_DIR}/${bin_target} PARENT_SCOPE)
endfunction()