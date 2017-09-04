add_executable(tst_qxmpputils tst_qxmpputils.cpp tst_qxmpputils.qrc)
add_test(tst_qxmpputils tst_qxmpputils)
target_link_libraries(tst_qxmpputils Qt5::Test qxmpp)
