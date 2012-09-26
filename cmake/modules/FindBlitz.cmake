# - Try to find Blitz
# Once done, this will define
#
#  Blitz_FOUND - system has Blitz
#  Blitz_INCLUDE_DIRS - the Blitz include directories
#  Blitz_LIBRARIES - link these to use Blitz
#
# this file is modeled after http://www.cmake.org/Wiki/CMake:How_To_Find_Libraries

include(LibFindMacros)

# Use pkg-config to get hints about paths
libfind_pkg_check_modules(Blitz_PKGCONF Blitz)

# Include dir
find_path(Blitz_INCLUDE_DIR
  NAMES blitz/array.h
  PATHS ${Blitz_PKGCONF_INCLUDE_DIRS}
)

# Finally the library itself
find_library(Blitz_LIBRARY
  NAMES blitz
  PATHS ${Blitz_PKGCONF_LIBRARY_DIRS}
)

# Set the include dir variables and the libraries and let libfind_process do the rest.
# NOTE: Singular variables for this library, plural for libraries this this lib depends on.
set(Blitz_PROCESS_INCLUDES Blitz_INCLUDE_DIR)
set(Blitz_PROCESS_LIBS Blitz_LIBRARY)
libfind_process(Blitz)
