set(KERNEL_C_FLAGS
  -ffreestanding
  -nostdlib
  -fno-builtin
  -fno-stack-protector
  -fno-pic
  -fno-pie
  -mno-red-zone
  -mcmodel=kernel
  -mno-sse
  -mno-sse2
  -mno-mmx
  -mno-80387
)

set(KERNEL_WARNING_FLAGS
  -Wall
  -Wextra
  -Werror=implicit-function-declaration
  -Werror=return-type
  -Wno-unused-parameter
)

set(KERNEL_DEBUG_FLAGS
  -g
  -gdwarf-4
)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(KERNEL_OPT_FLAGS -O0)
else()
  set(KERNEL_OPT_FLAGS -O2)
endif()

set(KERNEL_COMPILE_FLAGS
  ${KERNEL_C_FLAGS}
  ${KERNEL_WARNING_FLAGS}
  ${KERNEL_DEBUG_FLAGS}
  ${KERNEL_OPT_FLAGS}
)

set(KERNEL_LINK_FLAGS
  -nostdlib
  -static
  -no-pie
  -z max-page-size=0x1000
)

function(configure_kernel_target target)
  target_compile_options(${target} PRIVATE
    $<$<COMPILE_LANGUAGE:C>:${KERNEL_COMPILE_FLAGS}>
    $<$<COMPILE_LANGUAGE:CXX>:${KERNEL_COMPILE_FLAGS}>
  )

  target_link_options(${target} PRIVATE ${KERNEL_LINK_FLAGS})

  target_link_options(${target} PRIVATE
    -T ${CMAKE_SOURCE_DIR}/kernel/linker.ld
  )

  set_target_properties(${target} PROPERTIES
    LINK_DEPENDS ${CMAKE_SOURCE_DIR}/kernel/linker.ld
  )
endfunction()
