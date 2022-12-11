#uld::bfd
#binary file decoder 
set(BFD_SRC_DIR ${ULD_SRC_DIR}/bfd)

set(inc
  raw.h ar.h bin.h elf32.h elf64.h
)

add_subdirectory(util)

if(SDK)
  file(MAKE_DIRECTORY ${BFD_SRC_DIR})
  install(
    FILES
      ${inc}
    DESTINATION
      ${BFD_SRC_DIR}
  )
endif(SDK)

