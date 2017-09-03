# Try to find Theora
# Once done this will define
#  Theora_FOUND - System has Theora
#  Theora_INCLUDE_DIRS - The Theora include directories
#  Theora_LIBRARIES - The libraries needed to use Theora

find_package(PkgConfig)
pkg_check_modules(PC_Theora QUIET libtheora)

find_path(Theora_INCLUDE_DIR theora/theora.h
    HINTS ${PC_Theora_INCLUDEDIR} ${PC_Theora_INCLUDE_DIRS}
)

find_library(Theora_LIBRARY NAMES theora
    HINTS ${PC_Theora_LIBDIR} ${PC_Theora_LIBRARY_DIRS}
)

find_library(Theora_enc_LIBRARY NAMES theoraenc
    HINTS ${PC_Theora_LIBDIR} ${PC_Theora_LIBRARY_DIRS}
)

find_library(Theora_dec_LIBRARY NAMES theoradec
    HINTS ${PC_Theora_LIBDIR} ${PC_Theora_LIBRARY_DIRS}
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Theora DEFAULT_MSG Theora_LIBRARY Theora_enc_LIBRARY Theora_dec_LIBRARY  Theora_INCLUDE_DIR)

mark_as_advanced(Theora_INCLUDE_DIR Theora_LIBRARY Theora_enc_LIBRARY Theora_dec_LIBRARY)

set(Theora_LIBRARIES ${Theora_LIBRARY} ${Theora_enc_LIBRARY} ${Theora_dec_LIBRARY})
set(Theora_INCLUDE_DIRS ${Theora_INCLUDE_DIR})

