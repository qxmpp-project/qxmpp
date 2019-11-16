add_definitions(-DQXMPP_BUILD)

# disable Q_FOREACH()
add_definitions(-DQT_NO_FOREACH)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/base)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/client)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/server)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/base/QXmppGlobal.h.in ${CMAKE_CURRENT_BINARY_DIR}/base/QXmppGlobal.h @ONLY)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/base)

set(INSTALL_HEADER_FILES
    ${CMAKE_CURRENT_BINARY_DIR}/base/QXmppGlobal.h

    # Base
    base/QXmppArchiveIq.h
    base/QXmppBindIq.h
    base/QXmppBitsOfBinaryContentId.h
    base/QXmppBitsOfBinaryData.h
    base/QXmppBitsOfBinaryDataList.h
    base/QXmppBitsOfBinaryIq.h
    base/QXmppBookmarkSet.h
    base/QXmppByteStreamIq.h
    base/QXmppDataForm.h
    base/QXmppDiscoveryIq.h
    base/QXmppElement.h
    base/QXmppEntityTimeIq.h
    base/QXmppHttpUploadIq.h
    base/QXmppIbbIq.h
    base/QXmppIq.h
    base/QXmppJingleIq.h
    base/QXmppLogger.h
    base/QXmppMamIq.h
    base/QXmppMessage.h
    base/QXmppMixIq.h
    base/QXmppMixItem.h
    base/QXmppMucIq.h
    base/QXmppNonSASLAuth.h
    base/QXmppPingIq.h
    base/QXmppPresence.h
    base/QXmppPubSubIq.h
    base/QXmppRegisterIq.h
    base/QXmppResultSet.h
    base/QXmppRosterIq.h
    base/QXmppRpcIq.h
    base/QXmppRtcpPacket.h
    base/QXmppRtpChannel.h
    base/QXmppRtpPacket.h
    base/QXmppSessionIq.h
    base/QXmppSocks.h
    base/QXmppStanza.h
    base/QXmppStream.h
    base/QXmppStreamFeatures.h
    base/QXmppStun.h
    base/QXmppUtils.h
    base/QXmppVCardIq.h
    base/QXmppVersionIq.h

    # Client
    client/QXmppArchiveManager.h
    client/QXmppBookmarkManager.h
    client/QXmppCallManager.h
    client/QXmppCarbonManager.h
    client/QXmppClient.h
    client/QXmppClientExtension.h
    client/QXmppConfiguration.h
    client/QXmppDiscoveryManager.h
    client/QXmppEntityTimeManager.h
    client/QXmppInvokable.h
    client/QXmppMamManager.h
    client/QXmppMessageReceiptManager.h
    client/QXmppMucManager.h
    client/QXmppOutgoingClient.h
    client/QXmppRemoteMethod.h
    client/QXmppRosterManager.h
    client/QXmppRpcManager.h
    client/QXmppTransferManager.h
    client/QXmppTransferManager_p.h
    client/QXmppUploadRequestManager.h
    client/QXmppVCardManager.h
    client/QXmppVersionManager.h

    # Server
    server/QXmppDialback.h
    server/QXmppIncomingClient.h
    server/QXmppIncomingServer.h
    server/QXmppOutgoingServer.h
    server/QXmppPasswordChecker.h
    server/QXmppServer.h
    server/QXmppServerExtension.h
    server/QXmppServerPlugin.h
)

set(SOURCE_FILES
    # Base
    base/QXmppArchiveIq.cpp
    base/QXmppBindIq.cpp
    base/QXmppBitsOfBinaryContentId.cpp
    base/QXmppBitsOfBinaryData.cpp
    base/QXmppBitsOfBinaryDataList.cpp
    base/QXmppBitsOfBinaryIq.cpp
    base/QXmppBookmarkSet.cpp
    base/QXmppByteStreamIq.cpp
    base/QXmppCodec.cpp
    base/QXmppConstants.cpp
    base/QXmppDataForm.cpp
    base/QXmppDiscoveryIq.cpp
    base/QXmppElement.cpp
    base/QXmppEntityTimeIq.cpp
    base/QXmppHttpUploadIq.cpp
    base/QXmppIbbIq.cpp
    base/QXmppIq.cpp
    base/QXmppJingleIq.cpp
    base/QXmppLogger.cpp
    base/QXmppMamIq.cpp
    base/QXmppMessage.cpp
    base/QXmppMixIq.cpp
    base/QXmppMixItem.cpp
    base/QXmppMucIq.cpp
    base/QXmppNonSASLAuth.cpp
    base/QXmppPingIq.cpp
    base/QXmppPresence.cpp
    base/QXmppPubSubIq.cpp
    base/QXmppRegisterIq.cpp
    base/QXmppResultSet.cpp
    base/QXmppRosterIq.cpp
    base/QXmppRpcIq.cpp
    base/QXmppRtcpPacket.cpp
    base/QXmppRtpChannel.cpp
    base/QXmppRtpPacket.cpp
    base/QXmppSasl.cpp
    base/QXmppSessionIq.cpp
    base/QXmppSocks.cpp
    base/QXmppStanza.cpp
    base/QXmppStream.cpp
    base/QXmppStreamFeatures.cpp
    base/QXmppStreamInitiationIq.cpp
    base/QXmppStreamManagement.cpp
    base/QXmppStun.cpp
    base/QXmppUtils.cpp
    base/QXmppVCardIq.cpp
    base/QXmppVersionIq.cpp

    # Client
    client/QXmppDiscoveryManager.cpp
    client/QXmppArchiveManager.cpp
    client/QXmppBookmarkManager.cpp
    client/QXmppCallManager.cpp
    client/QXmppCarbonManager.cpp
    client/QXmppClient.cpp
    client/QXmppClientExtension.cpp
    client/QXmppConfiguration.cpp
    client/QXmppEntityTimeManager.cpp
    client/QXmppInvokable.cpp
    client/QXmppMamManager.cpp
    client/QXmppMessageReceiptManager.cpp
    client/QXmppMucManager.cpp
    client/QXmppOutgoingClient.cpp
    client/QXmppRemoteMethod.cpp
    client/QXmppRosterManager.cpp
    client/QXmppRpcManager.cpp
    client/QXmppTransferManager.cpp
    client/QXmppUploadRequestManager.cpp
    client/QXmppVCardManager.cpp
    client/QXmppVersionManager.cpp

    # Server
    server/QXmppDialback.cpp
    server/QXmppIncomingClient.cpp
    server/QXmppIncomingServer.cpp
    server/QXmppOutgoingServer.cpp
    server/QXmppPasswordChecker.cpp
    server/QXmppServer.cpp
    server/QXmppServerExtension.cpp
    server/QXmppServerPlugin.cpp
)

option(WITH_SPEEX "Support the Speex codec" OFF)
option(WITH_OPUS "Support the Opus codec" OFF)
option(WITH_THEORA "Support the Theora codec" OFF)
option(WITH_VPX "Support the VPX codec" OFF)
option(BUILD_SHARED "Build SHARED library" ON)

if(WITH_SPEEX)
    find_package(Speex REQUIRED)
    include_directories(${Speex_INCLUDE_DIRS})
    set(MULTIMEDIA_LIBS ${MULTIMEDIA_LIBS} ${Speex_LIBRARIES})
    add_definitions(-DQXMPP_USE_SPEEX)
endif()
if(WITH_OPUS)
    find_package(Opus REQUIRED)
    include_directories(${Opus_INCLUDE_DIRS})
    set(MULTIMEDIA_LIBS ${MULTIMEDIA_LIBS} ${Opus_LIBRARIES})
    add_definitions(-DQXMPP_USE_OPUS)
endif()
if(WITH_THEORA)
    find_package(Theora REQUIRED)
    include_directories(${Theora_INCLUDE_DIRS})
    set(MULTIMEDIA_LIBS ${MULTIMEDIA_LIBS} ${Theora_LIBRARIES})
    add_definitions(-DQXMPP_USE_THEORA)
endif()
if(WITH_VPX)
    find_package(VPX REQUIRED)
    include_directories(${VPX_INCLUDE_DIRS})
    set(MULTIMEDIA_LIBS ${MULTIMEDIA_LIBS} ${VPX_LIBRARIES})
    add_definitions(-DQXMPP_USE_VPX)
endif()

if(BUILD_SHARED)
    add_library(qxmpp SHARED ${SOURCE_FILES})
else()
    add_library(qxmpp STATIC ${SOURCE_FILES})
endif()

set_target_properties(qxmpp PROPERTIES
    VERSION ${VERSION_STRING}
    SOVERSION ${SO_VERSION}
    EXPORT_NAME QXmpp
)

target_include_directories(qxmpp
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/base>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/client>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/server>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/base>
    $<INSTALL_INTERFACE:include/qxmpp>
)

target_link_libraries(qxmpp
    PUBLIC
    Qt5::Core
    Qt5::Network
    Qt5::Xml
    PRIVATE
    ${MULTIMEDIA_LIBS}
)

install(
    TARGETS qxmpp
    DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    EXPORT QXmppTarget
)

install(
    EXPORT QXmppTarget
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/qxmpp"
    FILE QXmpp.cmake
    NAMESPACE QXmpp::
    COMPONENT Devel
)

export(
    TARGETS qxmpp
    FILE QXmpp.cmake
)

install(
    FILES ${INSTALL_HEADER_FILES}
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/qxmpp"
)

