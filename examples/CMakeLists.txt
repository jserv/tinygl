include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/../include
  ${CMAKE_CURRENT_SOURCE_DIR}/../src
  ${CMAKE_CURRENT_SOURCE_DIR}
  )

find_package(SDL REQUIRED)

add_executable(texobj texobj.c)
target_link_libraries(texobj tinygl ${SDL_LIBRARY} ${M_LIBRARY})
add_executable(gears gears.c)
target_link_libraries(gears tinygl ${SDL_LIBRARY} ${M_LIBRARY})
add_executable(triangle triangle.c)
target_link_libraries(triangle tinygl ${SDL_LIBRARY} ${M_LIBRARY})

# Local Variables:
# tab-width: 8
# mode: cmake
# indent-tabs-mode: t
# End:
# ex: shiftwidth=2 tabstop=8
