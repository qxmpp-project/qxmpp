# SPDX-FileCopyrightText: 2008 Laurent Montel <montel@kde.org>
#
# SPDX-License-Identifier: LicenseRef-MIT-variant

# - Try to find the GLIB2 libraries
# Once done this will define
#
#  GLIB2_FOUND - system has glib2
#  GLIB2_INCLUDE_DIR - the glib2 include directory
#  GLIB2_LIBRARIES - glib2 library

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

if(GLIB2_INCLUDE_DIR AND GLIB2_LIBRARIES)
    # Already in cache, be silent
    set(GLIB2_FIND_QUIETLY TRUE)
endif(GLIB2_INCLUDE_DIR AND GLIB2_LIBRARIES)

if (NOT WIN32)
  find_package(PkgConfig QUIET)
  if(PKG_CONFIG_FOUND)
    pkg_check_modules(PKG_GLIB QUIET glib-2.0)
  endif()
endif(NOT WIN32)

find_path(GLIB2_MAIN_INCLUDE_DIR glib.h
          PATH_SUFFIXES glib-2.0
          HINTS ${PKG_GLIB_INCLUDE_DIRS} ${PKG_GLIB_INCLUDEDIR})

# search the glibconfig.h include dir under the same root where the library is found
find_library(GLIB2_LIBRARIES
             NAMES glib-2.0
             HINTS ${PKG_GLIB_LIBRARY_DIRS} ${PKG_GLIB_LIBDIR})

find_path(GLIB2_INTERNAL_INCLUDE_DIR glibconfig.h
          PATH_SUFFIXES glib-2.0/include ../lib/glib-2.0/include
          HINTS ${PKG_GLIB_INCLUDE_DIRS} ${PKG_GLIB_LIBRARIES} ${CMAKE_SYSTEM_LIBRARY_PATH})

set(GLIB2_INCLUDE_DIR ${GLIB2_MAIN_INCLUDE_DIR})

# not sure if this include dir is optional or required
# for now it is optional
if(GLIB2_INTERNAL_INCLUDE_DIR)
  set(GLIB2_INCLUDE_DIR ${GLIB2_INCLUDE_DIR} ${GLIB2_INTERNAL_INCLUDE_DIR})
endif(GLIB2_INTERNAL_INCLUDE_DIR)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GLIB2  DEFAULT_MSG  GLIB2_LIBRARIES GLIB2_MAIN_INCLUDE_DIR)

mark_as_advanced(GLIB2_INCLUDE_DIR GLIB2_LIBRARIES)


find_program(GLIB2_GENMARSHAL_UTIL glib-genmarshal)

macro(glib2_genmarshal output_name)
    file(REMOVE ${CMAKE_CURRENT_BINARY_DIR}/genmarshal_tmp)
    foreach(_declaration ${ARGN})
        file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/genmarshal_tmp "${_declaration}\n")
    endforeach()
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${output_name}.h ${CMAKE_CURRENT_BINARY_DIR}/${output_name}.c
        COMMAND ${GLIB2_GENMARSHAL_UTIL} --header genmarshal_tmp > ${output_name}.h
        COMMAND ${GLIB2_GENMARSHAL_UTIL} --body genmarshal_tmp > ${output_name}.c
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
endmacro()
