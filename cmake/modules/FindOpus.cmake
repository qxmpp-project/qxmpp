# Try to find Opus
# Once done this will define
#  Opus_FOUND - System has Opus
#  Opus_INCLUDE_DIRS - The Opus include directories
#  Opus_LIBRARIES - The libraries needed to use Opus

find_package(PkgConfig)
pkg_check_modules(PC_Opus QUIET libopus)

find_path(Opus_INCLUDE_DIR opus/opus.h
    HINTS ${PC_Opus_INCLUDEDIR} ${PC_Opus_INCLUDE_DIRS}
)

find_library(Opus_LIBRARY NAMES opus
    HINTS ${PC_Opus_LIBDIR} ${PC_Opus_LIBRARY_DIRS}
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Opus DEFAULT_MSG Opus_LIBRARY Opus_INCLUDE_DIR)

mark_as_advanced(Opus_INCLUDE_DIR Opus_LIBRARY)

set(Opus_LIBRARIES ${Opus_LIBRARY})
set(Opus_INCLUDE_DIRS ${Opus_INCLUDE_DIR})

