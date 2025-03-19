# SPDX-FileCopyrightText: 2025 Linus Jahn <lnj@kaidan.im>
#
# SPDX-License-Identifier: CC0-1.0

find_package(GStreamer REQUIRED)
find_package(GLIB2 REQUIRED)
find_package(GObject REQUIRED)

add_executable(audio_call AudioCall.cpp)

target_link_libraries(audio_call
    PUBLIC
    ${QXMPP_TARGET}
    ${GLIB2_LIBRARIES}
    ${GOBJECT_LIBRARIES}
    ${GSTREAMER_LIBRARY}
)

target_include_directories(audio_call
    PUBLIC
    ${QXMPP_TARGET}
    ${GLIB2_INCLUDE_DIR}
    ${GOBJECT_INCLUDE_DIR}
    ${GSTREAMER_INCLUDE_DIRS}
)
