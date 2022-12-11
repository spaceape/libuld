#hld::bfd::util

set(UTIL_SRC_DIR ${BFD_SRC_DIR}/util)

set(inc
  cache.h
)

if(SDK)
  file(MAKE_DIRECTORY ${UTIL_SRC_DIR})
  install(
    FILES
      ${inc}
    DESTINATION
      ${UTIL_SRC_DIR}
  )
endif(SDK)

