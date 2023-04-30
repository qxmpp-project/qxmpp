# SPDX-FileCopyrightText: 2017 Niels Ole Salscheider <niels_ole@salscheider-online.de>
#
# SPDX-License-Identifier: CC0-1.0

cmake_minimum_required(VERSION 3.7)
project(qxmpp VERSION 1.6.0)

set(SO_VERSION 4)

# C++ standard settings:
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/modules")

# Qt
if(NOT DEFINED QT_VERSION_MAJOR)
    find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Network Xml)
endif()
message("-- Choosing Qt ${QT_VERSION_MAJOR}")
find_package(Qt${QT_VERSION_MAJOR} 5.15.0 REQUIRED COMPONENTS Core Network Xml)

# QCA (optional)
find_package(Qca-qt${QT_VERSION_MAJOR} QUIET)
if(${QT_VERSION_MAJOR} EQUAL 6)
    find_package(Qt6Core5Compat)
endif()

include(GNUInstallDirs)

option(BUILD_SHARED "Build shared library" ON)
option(BUILD_TESTS "Build tests." ON)
option(BUILD_INTERNAL_TESTS "Build internal tests." OFF)
option(BUILD_DOCUMENTATION "Build API documentation." OFF)
option(BUILD_EXAMPLES "Build examples." ON)
option(BUILD_OMEMO "Build the OMEMO module" OFF)
option(WITH_GSTREAMER "Build with GStreamer support for Jingle" OFF)
option(WITH_QCA "Build with QCA for OMEMO or encrypted file sharing" ${Qca-qt${QT_VERSION_MAJOR}_FOUND})

set(QXMPP_TARGET QXmppQt${QT_VERSION_MAJOR})
set(QXMPPOMEMO_TARGET QXmppOmemoQt${QT_VERSION_MAJOR})

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

# Normal QXmppQt5/6 package
configure_package_config_file(
    QXmppConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/${QXMPP_TARGET}Config.cmake
    INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${QXMPP_TARGET}"
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/${QXMPP_TARGET}ConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/${QXMPP_TARGET}Config.cmake
          ${CMAKE_CURRENT_BINARY_DIR}/${QXMPP_TARGET}ConfigVersion.cmake
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${QXMPP_TARGET}"
    COMPONENT Devel
)

# QXmpp package with Qt version autodetect
configure_package_config_file(
    cmake/QXmppQtAutoConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/QXmppConfig.cmake
    INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/QXmpp"
)

install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/QXmppConfig.cmake
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/QXmpp"
    COMPONENT Devel
)
install(
    FILES ${CMAKE_CURRENT_SOURCE_DIR}/cmake/QXmppQtAutoConfigVersion.cmake
    RENAME QXmppConfigVersion.cmake
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/QXmpp"
    COMPONENT Devel
)

# Generate QXmppQt5/6.pc
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/qxmpp.pc.in ${CMAKE_CURRENT_BINARY_DIR}/${QXMPP_TARGET}.pc @ONLY)
install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/${QXMPP_TARGET}.pc
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig"
    COMPONENT Devel
)

# "qxmpp.pc" for backwards-compatibility
if(QT_VERSION_MAJOR EQUAL 5)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/qxmpp_legacy.pc.in ${CMAKE_CURRENT_BINARY_DIR}/qxmpp.pc @ONLY)
    install(
        FILES ${CMAKE_CURRENT_BINARY_DIR}/qxmpp.pc
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig"
        COMPONENT Devel
    )
endif()

