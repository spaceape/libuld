#uld
#ELF Loader support library
project(uld)
set(NAME uld)
set(PREFIX)

add_definitions(
  ${GLOBAL_DEFS}
)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${GLOBAL_LIBRARY_DIR}/fat
  ${GLOBAL_LIBRARY_DIR}/fat/ff14a
  ${GLOBAL_LIBRARY_DIR}/fat/sd_driver
)

set(ULD_SDK_DIR ${GLOBAL_SDK_DIR}/${NAME})

set(inc
  config.h error.h
)

set(srcs
  bfd/util/file.cpp bfd/util/cache.cpp
  bfd/raw.cpp bfd/ar.cpp bfd/bin.cpp bfd/elf32.cpp bfd/elf64.cpp
  image/segment.cpp image/string_table.cpp image/symbol_table.cpp image/program_table.cpp
  target.cpp image.cpp elf32.cpp elf64.cpp
  uld.cpp
)

set(libs pico_stdlib host fat)

add_subdirectory(bfd)

add_library(${NAME} STATIC ${srcs})
set_target_properties(${NAME} PROPERTIES PREFIX "${PREFIX}")
target_link_libraries(${NAME} ${libs})

if(SDK)
  file(MAKE_DIRECTORY ${ULD_SDK_DIR})
  install(
    FILES
      ${inc}
    DESTINATION
      ${ULD_SDK_DIR}
  )
endif(SDK)
