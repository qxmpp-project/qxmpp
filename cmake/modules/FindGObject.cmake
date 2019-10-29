# - Try to find GObject
# Once done this will define
#
#  GOBJECT_FOUND - system has GObject
#  GOBJECT_INCLUDE_DIR - the GObject include directory
#  GOBJECT_LIBRARIES - the libraries needed to use GObject
#  GOBJECT_DEFINITIONS - Compiler switches required for using GObject

# Copyright (c) 2006, Tim Beaulen <tbscope@gmail.com>
# Copyright (c) 2008 Helio Chissini de Castro, <helio@kde.org>
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

IF (GOBJECT_INCLUDE_DIR AND GOBJECT_LIBRARIES)
   # in cache already
   SET(GObject_FIND_QUIETLY TRUE)
ELSE (GOBJECT_INCLUDE_DIR AND GOBJECT_LIBRARIES)
   SET(GObject_FIND_QUIETLY FALSE)
ENDIF (GOBJECT_INCLUDE_DIR AND GOBJECT_LIBRARIES)

IF (NOT WIN32)
   FIND_PACKAGE(PkgConfig REQUIRED)
   # use pkg-config to get the directories and then use these values
   # in the FIND_PATH() and FIND_LIBRARY() calls
   PKG_CHECK_MODULES(PKG_GOBJECT2 REQUIRED gobject-2.0)
   SET(GOBJECT_DEFINITIONS ${PKG_GOBJECT2_CFLAGS})
ENDIF (NOT WIN32)

FIND_PATH(GOBJECT_INCLUDE_DIR gobject/gobject.h
   HINTS ${PKG_GOBJECT2_INCLUDE_DIRS} ${PKG_GOBJECT2_INCLUDEDIR}
   PATHS /usr/include/glib-2.0/
   PATH_SUFFIXES glib-2.0
   )

FIND_LIBRARY(_GObjectLibs NAMES gobject-2.0
   HINTS
   ${PKG_GOBJECT2_LIBRARY_DIRS}
   ${PKG_GOBJECT2_LIBDIR}
   )
FIND_LIBRARY(_GModuleLibs NAMES gmodule-2.0
   HINTS
   ${PKG_GOBJECT2_LIBRARY_DIRS}
   ${PKG_GOBJECT2_LIBDIR}
   )
FIND_LIBRARY(_GThreadLibs NAMES gthread-2.0
   HINTS
   ${PKG_GOBJECT2_LIBRARY_DIRS}
   ${PKG_GOBJECT2_LIBDIR}
   )
FIND_LIBRARY(_GLibs NAMES glib-2.0
   HINTS
   ${PKG_GOBJECT2_LIBRARY_DIRS}
   ${PKG_GOBJECT2_LIBDIR}
   )

SET (GOBJECT_LIBRARIES ${_GObjectLibs} ${_GModuleLibs} ${_GThreadLibs} ${_GLibs})

MARK_AS_ADVANCED(GOBJECT_INCLUDE_DIR GOBJECT_LIBRARIES)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(GObject DEFAULT_MSG GOBJECT_INCLUDE_DIR GOBJECT_LIBRARIES)
