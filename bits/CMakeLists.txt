#uld::bits
set(BITS_SRC_DIR ${ULD_SRC_DIR}/bits)

set(inc
  common.h arm.h
)

if(SDK)
  file(MAKE_DIRECTORY ${BITS_SRC_DIR})
  install(
    FILES
      ${inc}
    DESTINATION
      ${BITS_SRC_DIR}
  )
endif(SDK)

