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

add_subdirectory(src)
add_subdirectory(examples)

# Local Variables:
# tab-width: 8
# mode: cmake
# indent-tabs-mode: t
# End:
# ex: shiftwidth=2 tabstop=8
