include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/../include
  ${CMAKE_CURRENT_SOURCE_DIR}
  )

set(tinygl_srcs
  api.c
  arrays.c
  bitmap.c
  blend.c
  clear.c
  clip.c
  enable.c
  get.c
  image_util.c
  init.c
  light.c
  matrix.c
  memory.c
  misc.c
  msghandling.c
  specbuf.c
  texture.c
  vertex.c
  zbuffer.c
  dither.c
  line.c
  math.c
  triangle.c
  )

add_library(tinygl SHARED ${tinygl_srcs})
install(TARGETS tinygl
  RUNTIME DESTINATION ${BIN_DIR}
  LIBRARY DESTINATION ${LIB_DIR}
  ARCHIVE DESTINATION ${LIB_DIR})
target_compile_options(tinygl PRIVATE "-Wall")

if (BUILD_STATIC_LIBS)
  add_library(tinygl-static STATIC ${tinygl_srcs})
  install(TARGETS tinygl-static
    RUNTIME DESTINATION ${BIN_DIR}
    LIBRARY DESTINATION ${LIB_DIR}
    ARCHIVE DESTINATION ${LIB_DIR})
endif (BUILD_STATIC_LIBS)

# Local Variables:
# tab-width: 8
# mode: cmake
# indent-tabs-mode: t
# End:
# ex: shiftwidth=2 tabstop=8
