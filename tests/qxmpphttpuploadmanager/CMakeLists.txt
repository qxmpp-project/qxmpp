# SPDX-FileCopyrightText: 2019 Yury Gubich <blue@macaw.me>
#
# SPDX-License-Identifier: CC0-1.0

include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_executable(tst_qxmpphttpuploadmanager tst_qxmpphttpuploadmanager.cpp tst_qxmpphttpuploadmanager.qrc ../TestClient.h)
add_test(tst_qxmpphttpuploadmanager tst_qxmpphttpuploadmanager)
target_link_libraries(tst_qxmpphttpuploadmanager Qt${QT_VERSION_MAJOR}::Test ${QXMPP_TARGET})
