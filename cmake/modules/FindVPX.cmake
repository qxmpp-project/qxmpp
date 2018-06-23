# Try to find VPX
# Once done this will define
#  VPX_FOUND - System has VPX
#  VPX_INCLUDE_DIRS - The VPX include directories
#  VPX_LIBRARIES - The libraries needed to use VPX

find_package(PkgConfig)
pkg_check_modules(PC_VPX QUIET libvpx)

find_path(VPX_INCLUDE_DIR vpx/vp8.h
    HINTS ${PC_VPX_INCLUDEDIR} ${PC_VPX_INCLUDE_DIRS}
)

find_library(VPX_LIBRARY NAMES vpx
    HINTS ${PC_VPX_LIBDIR} ${PC_VPX_LIBRARY_DIRS}
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(VPX DEFAULT_MSG VPX_LIBRARY VPX_INCLUDE_DIR)

mark_as_advanced(VPX_INCLUDE_DIR VPX_LIBRARY)

set(VPX_LIBRARIES ${VPX_LIBRARY})
set(VPX_INCLUDE_DIRS ${VPX_INCLUDE_DIR})

