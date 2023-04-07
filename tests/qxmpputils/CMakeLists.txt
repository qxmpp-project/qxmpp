# SPDX-FileCopyrightText: 2017 Niels Ole Salscheider <niels_ole@salscheider-online.de>
#
# SPDX-License-Identifier: CC0-1.0

include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_executable(tst_qxmpputils tst_qxmpputils.cpp tst_qxmpputils.qrc)
add_test(tst_qxmpputils tst_qxmpputils)
target_link_libraries(tst_qxmpputils Qt${QT_VERSION_MAJOR}::Test ${QXMPP_TARGET})
