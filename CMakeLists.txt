project(qxmpp)

cmake_minimum_required(VERSION 3.2)

set(VERSION_MAJOR 0)
set(VERSION_MINOR 9)
set(VERSION_PATCH 4)
set(SO_VERSION 0)
set(VERSION_STRING ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})
mark_as_advanced(VERSION_MAJOR VERSION_MINOR VERSION_PATCH VERSION_STRING)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/modules")

find_package(Qt5 REQUIRED COMPONENTS Core Network Xml)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

include(GNUInstallDirs)

option(BUILD_TESTS "Build tests." ON)
option(BUILD_DOCUMENTATION "Build API documentation." OFF)
option(BUILD_EXAMPLES "Build examples." ON)

add_subdirectory(src)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

if(BUILD_DOCUMENTATION)
    add_subdirectory(doc)
endif()

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

install(
    FILES QXmppConfig.cmake
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/qxmpp"
    COMPONENT Devel
)

# Generate qxmpp.pc
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/qxmpp.pc.in ${CMAKE_CURRENT_BINARY_DIR}/qxmpp.pc @ONLY)
install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/qxmpp.pc
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig"
    COMPONENT Devel
)

