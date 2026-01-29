cmake_minimum_required(VERSION 3.20)

project(PlasmaOS
  VERSION 0.1.0
  LANGUAGES C CXX ASM_NASM
)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "PlasmaOS version: ${PROJECT_VERSION}")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

add_subdirectory(kernel)

add_custom_target(iso
  COMMAND ${CMAKE_SOURCE_DIR}/tools/make-iso.sh
  DEPENDS plasma.elf
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  COMMENT "Generating bootable ISO image"
)

add_custom_target(run
  COMMAND ${CMAKE_SOURCE_DIR}/tools/run-qemu.sh
  DEPENDS iso
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  COMMENT "Running PlasmaOS in QEMU"
)

add_custom_target(debug
  COMMAND ${CMAKE_SOURCE_DIR}/tools/debug-qemu.sh
  DEPENDS iso
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  COMMENT "Running PlasmaOS in QEMU with GDB stub"
)
