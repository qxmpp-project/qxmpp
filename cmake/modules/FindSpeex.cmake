# Try to find Speex
# Once done this will define
#  Speex_FOUND - System has Speex
#  Speex_INCLUDE_DIRS - The Speex include directories
#  Speex_LIBRARIES - The libraries needed to use Speex

find_package(PkgConfig)
pkg_check_modules(PC_Speex QUIET libspeex)

find_path(Speex_INCLUDE_DIR speex/speex.h
    HINTS ${PC_Speex_INCLUDEDIR} ${PC_Speex_INCLUDE_DIRS}
)

find_library(Speex_LIBRARY NAMES speex
    HINTS ${PC_Speex_LIBDIR} ${PC_Speex_LIBRARY_DIRS}
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Speex DEFAULT_MSG Speex_LIBRARY Speex_INCLUDE_DIR)

mark_as_advanced(Speex_INCLUDE_DIR Speex_LIBRARY)

set(Speex_LIBRARIES ${Speex_LIBRARY})
set(Speex_INCLUDE_DIRS ${Speex_INCLUDE_DIR})

