# - Try to find DisplayCluster library
# Once done, this will define
#
#  DisplayCluster_FOUND - system has DisplayCluster library
#  DisplayCluster_INCLUDE_DIRS - the DisplayCluster include directories
#  DisplayCluster_LIBRARIES - link these to use DisplayCluster library
#
# this file is modeled after http://www.cmake.org/Wiki/CMake:How_To_Find_Libraries

include(LibFindMacros)

# Use pkg-config to get hints about paths
libfind_pkg_check_modules(DisplayCluster_PKGCONF DisplayCluster)

# Include dir
find_path(DisplayCluster_INCLUDE_DIR
  NAMES dcStream.h
  PATHS ${DisplayCluster_PKGCONF_INCLUDE_DIRS}
)

# Finally the library itself
find_library(DisplayCluster_LIBRARY
  NAMES DisplayCluster
  PATHS ${DisplayCluster_PKGCONF_LIBRARY_DIRS}
)

# Set the include dir variables and the libraries and let libfind_process do the rest.
# NOTE: Singular variables for this library, plural for libraries this this lib depends on.
set(DisplayCluster_PROCESS_INCLUDES DisplayCluster_INCLUDE_DIR)
set(DisplayCluster_PROCESS_LIBS DisplayCluster_LIBRARY)
libfind_process(DisplayCluster)
