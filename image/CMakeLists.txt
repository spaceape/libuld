#uld::image

set(IMAGE_SRC_DIR ${ULD_SRC_DIR}/image)

set(inc
  page.h pool.h data.h segment.h table.h string_table.h symbol_table.h
)

if(SDK)
  file(MAKE_DIRECTORY ${IMAGE_SRC_DIR})
  install(
    FILES
      ${inc}
    DESTINATION
      ${IMAGE_SRC_DIR}
  )
endif(SDK)

