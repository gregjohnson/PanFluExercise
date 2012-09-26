# - Try to find NetCDF
# Once done, this will define
#
#  NetCDF_FOUND - system has NetCDF
#  NetCDF_INCLUDE_DIRS - the NetCDF include directories
#  NetCDF_LIBRARIES - link these to use NetCDF
#
# this file is modeled after http://www.cmake.org/Wiki/CMake:How_To_Find_Libraries

include(LibFindMacros)

# Use pkg-config to get hints about paths
libfind_pkg_check_modules(NetCDF_PKGCONF NetCDF)

# Include dir
find_path(NetCDF_INCLUDE_DIR
  NAMES netcdfcpp.h
  PATHS ${NetCDF_PKGCONF_INCLUDE_DIRS}
)

# Finally the library itself
find_library(NetCDF_LIBRARY
  NAMES netcdf_c++
  PATHS ${NetCDF_PKGCONF_LIBRARY_DIRS}
)

# Set the include dir variables and the libraries and let libfind_process do the rest.
# NOTE: Singular variables for this library, plural for libraries this this lib depends on.
set(NetCDF_PROCESS_INCLUDES NetCDF_INCLUDE_DIR)
set(NetCDF_PROCESS_LIBS NetCDF_LIBRARY)
libfind_process(NetCDF)
