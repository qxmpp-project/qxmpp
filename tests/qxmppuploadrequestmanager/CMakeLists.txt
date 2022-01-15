# SPDX-FileCopyrightText: 2019 Yury Gubich <blue@macaw.me>
#
# SPDX-License-Identifier: CC0-1.0

include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_executable(tst_qxmppuploadrequestmanager tst_qxmppuploadrequestmanager.cpp tst_qxmppuploadrequestmanager.qrc)
add_test(tst_qxmppuploadrequestmanager tst_qxmppuploadrequestmanager)
target_link_libraries(tst_qxmppuploadrequestmanager Qt${QT_VERSION_MAJOR}::Test qxmpp)
