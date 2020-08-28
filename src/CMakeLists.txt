include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/../include
  ${CMAKE_CURRENT_SOURCE_DIR}
  )

set(tinygl_srcs
  tgl_api.c
  tgl_arrays.c
  tgl_bitmap.c
  tgl_blend.c
  tgl_clear.c
  tgl_clip.c
  tgl_enable.c
  tgl_get.c
  tgl_image_util.c
  tgl_init.c
  tgl_light.c
  tgl_matrix.c
  tgl_memory.c
  tgl_misc.c
  tgl_msghandling.c
  tgl_specbuf.c
  tgl_texture.c
  tgl_vertex.c
  tgl_zbuffer.c
  tgl_dither.c
  tgl_line.c
  tgl_math.c
  tgl_triangle.c
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
