# - macro find_gstreamer_library
#
# Copyright (c) 2010, Collabora Ltd.
#   @author George Kiagiadakis <george.kiagiadakis@collabora.co.uk>
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# 1. Redistributions of source code must retain the copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. The name of the author may not be used to endorse or promote products
#    derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
# OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
# IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
# NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
# THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

macro(find_gstreamer_library _name _header _abi_version _quiet)
    string(TOLOWER ${_name} _lower_name)
    string(TOUPPER ${_name} _upper_name)

    if (GSTREAMER_${_upper_name}_LIBRARY AND GSTREAMER_${_upper_name}_INCLUDE_DIR)
        set(_GSTREAMER_${_upper_name}_QUIET TRUE)
    else()
        set(_GSTREAMER_${_upper_name}_QUIET FALSE)
    endif()

    if (PKG_CONFIG_FOUND)
        pkg_check_modules(PKG_GSTREAMER_${_upper_name} QUIET gstreamer-${_lower_name}-${_abi_version})
    endif()

    find_library(GSTREAMER_${_upper_name}_LIBRARY
                 NAMES gst${_lower_name}-${_abi_version}
                 HINTS ${PKG_GSTREAMER_${_upper_name}_LIBRARY_DIRS}
                       ${PKG_GSTREAMER_${_upper_name}_LIBDIR}
    )

    find_path(GSTREAMER_${_upper_name}_INCLUDE_DIR
              gst/${_lower_name}/${_header}
              HINTS ${PKG_GSTREAMER_${_upper_name}_INCLUDE_DIRS}
                    ${PKG_GSTREAMER_${_upper_name}_INCLUDEDIR}
              PATH_SUFFIXES gstreamer-${_abi_version}
    )

    if (GSTREAMER_${_upper_name}_LIBRARY AND GSTREAMER_${_upper_name}_INCLUDE_DIR)
        set(GSTREAMER_${_upper_name}_LIBRARY_FOUND TRUE)
    else()
        set(GSTREAMER_${_upper_name}_LIBRARY_FOUND FALSE)
    endif()

    if (NOT _GSTREAMER_${_upper_name}_QUIET AND NOT _quiet)
        if (GSTREAMER_${_upper_name}_LIBRARY)
            message(STATUS "Found GSTREAMER_${_upper_name}_LIBRARY: ${GSTREAMER_${_upper_name}_LIBRARY}")
        else()
            message(STATUS "Could NOT find GSTREAMER_${_upper_name}_LIBRARY")
        endif()

        if (GSTREAMER_${_upper_name}_INCLUDE_DIR)
            message(STATUS "Found GSTREAMER_${_upper_name}_INCLUDE_DIR: ${GSTREAMER_${_upper_name}_INCLUDE_DIR}")
        else()
            message(STATUS "Could NOT find GSTREAMER_${_upper_name}_INCLUDE_DIR")
        endif()
    endif()

    mark_as_advanced(GSTREAMER_${_upper_name}_LIBRARY GSTREAMER_${_upper_name}_INCLUDE_DIR)
endmacro()
