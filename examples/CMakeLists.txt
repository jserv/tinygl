set(ui_srcs
  glx.c
  x11.c
  )

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/../include
  ${CMAKE_CURRENT_SOURCE_DIR}/../src
  ${CMAKE_CURRENT_SOURCE_DIR}
  )

find_package(X11 REQUIRED)

add_executable(mech mech.c glu.c ${ui_srcs})
target_link_libraries(mech tinygl ${X11_LIBRARIES} ${M_LIBRARY})
add_executable(texobj texobj.c ${ui_srcs})
target_link_libraries(texobj tinygl ${X11_LIBRARIES} ${M_LIBRARY})
add_executable(gears gears.c ${ui_srcs})
target_link_libraries(gears tinygl ${X11_LIBRARIES} ${M_LIBRARY})
add_executable(spin spin.c ${ui_srcs})
target_link_libraries(spin tinygl ${X11_LIBRARIES} ${X11_Xext_LIB} ${M_LIBRARY})

# Local Variables:
# tab-width: 8
# mode: cmake
# indent-tabs-mode: t
# End:
# ex: shiftwidth=2 tabstop=8
