# Minimum required version of CMake
cmake_minimum_required(VERSION 3.12)

# Set CMake project name
project(TINYGL)

if (NOT BIN_DIR)
  set(BIN_DIR bin)
endif (NOT BIN_DIR)

if (NOT INCLUDE_DIR)
  set(INCLUDE_DIR include)
endif (NOT INCLUDE_DIR)

if (NOT DEFINED LIB_DIR)
  set(LIB_DIR lib)
endif (NOT DEFINED LIB_DIR)

include(CheckLibraryExists)
check_library_exists(m cos "" HAVE_M_LIBRARY)
if (HAVE_M_LIBRARY)
  set(M_LIBRARY m)
endif (HAVE_M_LIBRARY)

find_package(X11 REQUIRED)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${X11_INCLUDE_DIRS}
  )

set(tinygl_srcs
  src/api.c
  src/arrays.c
  src/clear.c
  src/clip.c
  src/error.c
  src/get.c
  src/image_util.c
  src/init.c
  src/light.c
  src/list.c
  src/matrix.c
  src/memory.c
  src/misc.c
  src/msghandling.c
  src/select.c
  src/specbuf.c
  src/texture.c
  src/vertex.c
  src/zbuffer.c
  src/zdither.c
  src/zline.c
  src/zmath.c
  src/ztriangle.c
  )

add_library(tinygl SHARED ${tinygl_srcs})
install(TARGETS tinygl
  RUNTIME DESTINATION ${BIN_DIR}
  LIBRARY DESTINATION ${LIB_DIR}
  ARCHIVE DESTINATION ${LIB_DIR})

if (BUILD_STATIC_LIBS)
  add_library(tinygl-static STATIC ${tinygl_srcs})
  install(TARGETS tinygl-static
    RUNTIME DESTINATION ${BIN_DIR}
    LIBRARY DESTINATION ${LIB_DIR}
    ARCHIVE DESTINATION ${LIB_DIR})
endif (BUILD_STATIC_LIBS)

set(ui_srcs
  src/glx.c
  examples/x11.c 
  )

add_executable(mech examples/mech.c examples/glu.c ${ui_srcs})
target_link_libraries(mech tinygl ${X11_LIBRARIES} ${M_LIBRARY})
add_executable(texobj examples/texobj.c ${ui_srcs})
target_link_libraries(texobj tinygl ${X11_LIBRARIES} ${M_LIBRARY})
add_executable(gears examples/gears.c ${ui_srcs})
target_link_libraries(gears tinygl ${X11_LIBRARIES} ${M_LIBRARY})
add_executable(spin examples/spin.c ${ui_srcs})
target_link_libraries(spin tinygl ${X11_LIBRARIES} ${X11_Xext_LIB} ${M_LIBRARY})

# Local Variables:
# tab-width: 8
# mode: cmake
# indent-tabs-mode: t
# End:
# ex: shiftwidth=2 tabstop=8
