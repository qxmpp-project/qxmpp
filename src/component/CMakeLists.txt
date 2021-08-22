add_library(QXmppComponent SHARED
    QXmppComponent.cpp
    QXmppComponentExtension.cpp
    QXmppComponentConfig.cpp
    QXmppOutgoingComponent.cpp
)

set(COMPONENT_INSTALL_HEADER_FILES
    QXmppComponent.h
    QXmppComponentExtension.h
    QXmppComponentConfig.h
    QXmppOutgoingComponent.h
)

target_link_libraries(QXmppComponent PUBLIC qxmpp Qt5::Core)
set(COMPONENT_HEADER_DIR "${CMAKE_INSTALL_FULL_INCLUDEDIR}/qxmpp/component")
target_include_directories(QXmppComponent PUBLIC ${COMPONENT_HEADER_DIR})

set_target_properties(QXmppComponent PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${SO_VERSION}
    EXPORT_NAME Component
)

install(
    TARGETS QXmppComponent
    DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    EXPORT QXmppComponentTargets
)

install(
    EXPORT QXmppComponentTargets
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/qxmpp"
    FILE QXmppComponent.cmake
    NAMESPACE QXmpp::
    COMPONENT Devel
)

export(
    TARGETS QXmppComponent
    FILE QXmppComponent.cmake
)

install(
    FILES ${COMPONENT_INSTALL_HEADER_FILES}
    DESTINATION ${COMPONENT_HEADER_DIR}
)
