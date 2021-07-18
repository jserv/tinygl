include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/../include
  ${CMAKE_CURRENT_SOURCE_DIR}
  )

set(tinygl_srcs
	accum.c
	api.c
	arrays.c
	clear.c
	clip.c
	get.c
	image_util.c
	init.c
	light.c
	list.c
	matrix.c
	memory.c
	misc.c
	msghandling.c
	select.c
	specbuf.c
	texture.c
	vertex.c
	zbuffer.c
	zline.c
	zmath.c
	zpostprocess.c
	zraster.c
	ztext.c
	ztriangle.c
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

