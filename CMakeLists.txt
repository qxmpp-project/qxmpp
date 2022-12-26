# SPDX-FileCopyrightText: 2017 Niels Ole Salscheider <niels_ole@salscheider-online.de>
#
# SPDX-License-Identifier: CC0-1.0

cmake_minimum_required(VERSION 3.3)
project(qxmpp VERSION 1.5.0)

set(SO_VERSION 4)

# C++ standard settings:
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/modules")

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Network Xml)
find_package(Qt${QT_VERSION_MAJOR} 5.9.2 REQUIRED COMPONENTS Core Network Xml)
# QCA (optional)
find_package(Qca-qt${QT_VERSION_MAJOR} QUIET)
if(${QT_VERSION_MAJOR} EQUAL 6)
    find_package(Qt6Core5Compat)
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

include(GNUInstallDirs)

option(BUILD_TESTS "Build tests." ON)
option(BUILD_INTERNAL_TESTS "Build internal tests." OFF)
option(BUILD_DOCUMENTATION "Build API documentation." OFF)
option(BUILD_EXAMPLES "Build examples." ON)
option(BUILD_OMEMO "Build the OMEMO module" OFF)
option(WITH_GSTREAMER "Build with GStreamer support for Jingle" OFF)
option(WITH_QCA "Build with QCA for OMEMO or encrypted file sharing" ${Qca-qt${QT_VERSION_MAJOR}_FOUND})

add_definitions(
    -DQT_DISABLE_DEPRECATED_BEFORE=0x050F00
    -DQURL_NO_CAST_FROM_STRING
    -DQT_NO_CAST_TO_ASCII
    -DQT_NO_FOREACH
    -DQT_NO_KEYWORDS
)

if(BUILD_OMEMO)
    add_definitions(-DBUILD_OMEMO)

    # libomemo-c
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(OmemoC REQUIRED IMPORTED_TARGET libomemo-c)

    # QCA
    if(NOT WITH_QCA)
        message(FATAL_ERROR "OMEMO requires QCA (Qt Cryptographic Architecture)")
    endif()
endif()

if(WITH_QCA)
    add_definitions(-DWITH_QCA)
endif()

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

include(CMakePackageConfigHelpers)

configure_package_config_file(
    QXmppConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/QXmppConfig.cmake
    INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/qxmpp"
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/QXmppConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/QXmppConfig.cmake
          ${CMAKE_CURRENT_BINARY_DIR}/QXmppConfigVersion.cmake
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

