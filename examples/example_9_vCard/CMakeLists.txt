find_package(Qt6 COMPONENTS Gui)
if(NOT Qt6_FOUND)
    find_package(Qt5 REQUIRED Gui)
endif()

add_executable(9_vCard example_9_vCard.cpp)
target_link_libraries(9_vCard ${QT_TARGET}::Gui qxmpp)
