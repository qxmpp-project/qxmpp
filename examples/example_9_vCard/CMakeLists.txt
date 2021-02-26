find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Gui)

add_executable(9_vCard example_9_vCard.cpp)
target_link_libraries(9_vCard Qt${QT_VERSION_MAJOR}::Gui qxmpp)
