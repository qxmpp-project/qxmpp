find_package(Qt5 REQUIRED COMPONENTS Gui)

add_executable(9_vCard example_9_vCard.cpp)
target_link_libraries(9_vCard Qt5::Gui qxmpp)
