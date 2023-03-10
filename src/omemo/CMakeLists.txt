# SPDX-FileCopyrightText: 2022 Linus Jahn <lnj@kaidan.im>
#
# SPDX-License-Identifier: CC0-1.0

include(CMakePackageConfigHelpers)

set(OMEMO_CMAKE_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/QXmppOmemo")
set(OMEMO_HEADER_DIR "${CMAKE_INSTALL_FULL_INCLUDEDIR}/qxmpp/omemo")
set(OMEMO_INSTALL_HEADER_FILES
    QXmppOmemoManager.h
    QXmppOmemoMemoryStorage.h
    QXmppOmemoStorage.h
)
set(OMEMO_SOURCE_FILES
    OmemoCryptoProvider.cpp
    QXmppOmemoData.cpp
    QXmppOmemoManager.cpp
    QXmppOmemoManager_p.cpp
    QXmppOmemoMemoryStorage.cpp
    QXmppOmemoStorage.cpp
)

add_library(QXmppOmemo SHARED ${OMEMO_SOURCE_FILES})

target_link_libraries(QXmppOmemo
    PUBLIC
    qxmpp
    Qt${QT_VERSION_MAJOR}::Core
    PRIVATE
    PkgConfig::OmemoC
    qca-qt${QT_VERSION_MAJOR}
)
target_include_directories(QXmppOmemo
    PUBLIC
    ${OMEMO_HEADER_DIR}
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${PROJECT_BINARY_DIR}/src
)

generate_export_header(QXmppOmemo)

install(
    FILES ${OMEMO_INSTALL_HEADER_FILES} ${CMAKE_CURRENT_BINARY_DIR}/qxmppomemo_export.h
    DESTINATION ${OMEMO_HEADER_DIR}
)

set_target_properties(QXmppOmemo PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${SO_VERSION}
    EXPORT_NAME Omemo
)

install(
    TARGETS QXmppOmemo
    EXPORT QXmppOmemoTargets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(
    EXPORT QXmppOmemoTargets
    DESTINATION ${OMEMO_CMAKE_DIR}
    FILE QXmppOmemo.cmake
    NAMESPACE QXmpp::
    COMPONENT Devel
)

export(
    TARGETS QXmppOmemo
    FILE QXmppOmemo.cmake
)

configure_package_config_file(
    QXmppOmemoConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/QXmppOmemoConfig.cmake
    INSTALL_DESTINATION ${OMEMO_CMAKE_DIR}
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/QXmppOmemoConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/QXmppOmemoConfig.cmake
          ${CMAKE_CURRENT_BINARY_DIR}/QXmppOmemoConfigVersion.cmake
    DESTINATION ${OMEMO_CMAKE_DIR}
    COMPONENT Devel
)
