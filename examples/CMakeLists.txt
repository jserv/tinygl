include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/../include
  ${CMAKE_CURRENT_SOURCE_DIR}/../src
  ${CMAKE_CURRENT_SOURCE_DIR}
  )

find_package(SDL REQUIRED)

add_executable(game game.c)
target_link_libraries(game tinygl ${SDL_LIBRARY} ${M_LIBRARY})

add_executable(gears gears.c)
target_link_libraries(gears tinygl ${SDL_LIBRARY} ${M_LIBRARY})

add_executable(helloworld helloworld.c)
target_link_libraries(helloworld tinygl ${SDL_LIBRARY} ${M_LIBRARY})

add_executable(menu menu.c)
target_link_libraries(menu tinygl ${SDL_LIBRARY} ${M_LIBRARY})

add_executable(model model.c)
target_link_libraries(model tinygl ${SDL_LIBRARY} ${M_LIBRARY})

add_executable(texture texture.c)
target_link_libraries(texture tinygl ${SDL_LIBRARY} ${M_LIBRARY})

# Local Variables:
# tab-width: 8
# mode: cmake
# indent-tabs-mode: t
# End:
# ex: shiftwidth=2 tabstop=8

