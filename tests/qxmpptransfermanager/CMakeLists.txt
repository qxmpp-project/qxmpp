# SPDX-FileCopyrightText: 2017 Niels Ole Salscheider <niels_ole@salscheider-online.de>
#
# SPDX-License-Identifier: CC0-1.0

include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_executable(tst_qxmpptransfermanager tst_qxmpptransfermanager.cpp tst_qxmpptransfermanager.qrc)
add_test(tst_qxmpptransfermanager tst_qxmpptransfermanager)
target_link_libraries(tst_qxmpptransfermanager Qt${QT_VERSION_MAJOR}::Test ${QXMPP_TARGET})
