# SPDX-FileCopyrightText: 2017 Niels Ole Salscheider <niels_ole@salscheider-online.de>
#
# SPDX-License-Identifier: CC0-1.0

include(GenerateExportHeader)

set(INSTALL_HEADER_FILES
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
    base/QXmppDataFormBase.h
    base/QXmppDiscoveryIq.h
    base/QXmppE2eeMetadata.h
    base/QXmppEncryptedFileSource.h
    base/QXmppElement.h
    base/QXmppEntityTimeIq.h
    base/QXmppError.h
    base/QXmppExtension.h
    base/QXmppFileMetadata.h
    base/QXmppFileShare.h
    base/QXmppFutureUtils_p.h
    base/QXmppGeolocItem.h
    base/QXmppGlobal.h
    base/QXmppHash.h
    base/QXmppHttpFileSource.h
    base/QXmppHttpUploadIq.h
    base/QXmppIbbIq.h
    base/QXmppIq.h
    base/QXmppJingleIq.h
    base/QXmppLogger.h
    base/QXmppMamIq.h
    base/QXmppMessage.h
    base/QXmppMessageReaction.h
    base/QXmppMixInfoItem.h
    base/QXmppMixInvitation.h
    base/QXmppMixIq.h
    base/QXmppMixParticipantItem.h
    base/QXmppMucIq.h
    base/QXmppNonza.h
    base/QXmppNonSASLAuth.h
    base/QXmppOutOfBandUrl.h
    base/QXmppPingIq.h
    base/QXmppPresence.h
    base/QXmppPromise.h
    base/QXmppPubSubAffiliation.h
    base/QXmppPubSubEvent.h
    base/QXmppPubSubIq_p.h
    base/QXmppPubSubBaseItem.h
    base/QXmppPubSubMetadata.h
    base/QXmppPubSubNodeConfig.h
    base/QXmppPubSubPublishOptions.h
    base/QXmppPubSubSubscribeOptions.cpp
    base/QXmppPubSubSubscription.h
    base/QXmppPubSubSubAuthorization.h
    base/QXmppPushEnableIq.h
    base/QXmppRegisterIq.h
    base/QXmppResultSet.h
    base/QXmppRosterIq.h
    base/QXmppRpcIq.h
    base/QXmppSendResult.h
    base/QXmppSessionIq.h
    base/QXmppSocks.h
    base/QXmppStanza.h
    base/QXmppStartTlsPacket.h
    base/QXmppStream.h
    base/QXmppStreamFeatures.h
    base/QXmppStun.h
    base/QXmppTask.h
    base/QXmppThumbnail.h
    base/QXmppTrustMessageElement.h
    base/QXmppTrustMessageKeyOwner.h
    base/QXmppTrustMessages.h
    base/QXmppUserTuneItem.h
    base/QXmppUtils.h
    base/QXmppVCardIq.h
    base/QXmppVersionIq.h
    base/compat/QXmppPubSubIq.h
    base/compat/QXmppPubSubItem.h

    # Client
    client/QXmppArchiveManager.h
    client/QXmppAtmManager.h
    client/QXmppAtmTrustMemoryStorage.h
    client/QXmppAtmTrustStorage.h
    client/QXmppAttentionManager.h
    client/QXmppBookmarkManager.h
    client/QXmppCarbonManager.h
    client/QXmppCarbonManagerV2.h
    client/QXmppClient.h
    client/QXmppClientExtension.h
    client/QXmppConfiguration.h
    client/QXmppDiscoveryManager.h
    client/QXmppE2eeExtension.h
    client/QXmppEntityTimeManager.h
    client/QXmppFileSharingManager.h
    client/QXmppFileSharingProvider.h
    client/QXmppHttpFileSharingProvider.h
    client/QXmppHttpUploadManager.h
    client/QXmppInvokable.h
    client/QXmppIqHandling.h
    client/QXmppMamManager.h
    client/QXmppMessageHandler.h
    client/QXmppMessageReceiptManager.h
    client/QXmppMucManager.h
    client/QXmppOutgoingClient.h
    client/QXmppRegistrationManager.h
    client/QXmppPubSubEventHandler.h
    client/QXmppPubSubManager.h
    client/QXmppRemoteMethod.h
    client/QXmppRosterManager.h
    client/QXmppRpcManager.h
    client/QXmppSendStanzaParams.h
    client/QXmppTransferManager.h
    client/QXmppTransferManager_p.h
    client/QXmppTrustLevel.h
    client/QXmppTrustManager.h
    client/QXmppTrustMemoryStorage.h
    client/QXmppTrustSecurityPolicy.h
    client/QXmppTrustStorage.h
    client/QXmppUploadRequestManager.h
    client/QXmppUserTuneManager.h
    client/QXmppUserLocationManager.h
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
    base/QXmppBitsOfBinaryIq.cpp
    base/QXmppBookmarkSet.cpp
    base/QXmppByteStreamIq.cpp
    base/QXmppConstants.cpp
    base/QXmppDataForm.cpp
    base/QXmppDataFormBase.cpp
    base/QXmppDiscoveryIq.cpp
    base/QXmppElement.cpp
    base/QXmppEncryptedFileSource.cpp
    base/QXmppEntityTimeIq.cpp
    base/QXmppError.cpp
    base/QXmppFileMetadata.cpp
    base/QXmppFileShare.cpp
    base/QXmppGeolocItem.cpp
    base/QXmppGlobal.cpp
    base/QXmppHash.cpp
    base/QXmppHashing.cpp
    base/QXmppHttpFileSource.cpp
    base/QXmppHttpUploadIq.cpp
    base/QXmppIbbIq.cpp
    base/QXmppIq.cpp
    base/QXmppJingleIq.cpp
    base/QXmppLogger.cpp
    base/QXmppMamIq.cpp
    base/QXmppMessage.cpp
    base/QXmppMessageReaction.cpp
    base/QXmppMixInvitation.cpp
    base/QXmppMixIq.cpp
    base/QXmppMixItems.cpp
    base/QXmppMucIq.cpp
    base/QXmppNonza.cpp
    base/QXmppNonSASLAuth.cpp
    base/QXmppOutOfBandUrl.cpp
    base/QXmppPacket.cpp
    base/QXmppPingIq.cpp
    base/QXmppPresence.cpp
    base/QXmppPubSubAffiliation.cpp
    base/QXmppPubSubEvent.cpp
    base/QXmppPubSubIq.cpp
    base/QXmppPubSubBaseItem.cpp
    base/QXmppPubSubMetadata.cpp
    base/QXmppPubSubNodeConfig.cpp
    base/QXmppPubSubSubscribeOptions.cpp
    base/QXmppPubSubSubscription.cpp
    base/QXmppPubSubSubAuthorization.cpp
    base/QXmppPushEnableIq.cpp
    base/QXmppRegisterIq.cpp
    base/QXmppResultSet.cpp
    base/QXmppRosterIq.cpp
    base/QXmppRpcIq.cpp
    base/QXmppSasl.cpp
    base/QXmppSessionIq.cpp
    base/QXmppSocks.cpp
    base/QXmppStanza.cpp
    base/QXmppStartTlsPacket.cpp
    base/QXmppStream.cpp
    base/QXmppStreamFeatures.cpp
    base/QXmppStreamInitiationIq.cpp
    base/QXmppStreamManagement.cpp
    base/QXmppStun.cpp
    base/QXmppTask.cpp
    base/QXmppThumbnail.cpp
    base/QXmppTrustMessages.cpp
    base/QXmppUserTuneItem.cpp
    base/QXmppUtils.cpp
    base/QXmppVCardIq.cpp
    base/QXmppVersionIq.cpp
    base/compat/QXmppPubSubIq.cpp
    base/compat/QXmppPubSubItem.cpp

    # Client
    client/QXmppArchiveManager.cpp
    client/QXmppAtmManager.cpp
    client/QXmppAtmTrustMemoryStorage.cpp
    client/QXmppAtmTrustStorage.cpp
    client/QXmppAttentionManager.cpp
    client/QXmppBookmarkManager.cpp
    client/QXmppCarbonManager.cpp
    client/QXmppCarbonManagerV2.cpp
    client/QXmppClient.cpp
    client/QXmppClientExtension.cpp
    client/QXmppConfiguration.cpp
    client/QXmppDiscoveryManager.cpp
    client/QXmppE2eeExtension.cpp
    client/QXmppEntityTimeManager.cpp
    client/QXmppFileSharingManager.cpp
    client/QXmppHttpFileSharingProvider.cpp
    client/QXmppHttpUploadManager.cpp
    client/QXmppInternalClientExtension.cpp
    client/QXmppInvokable.cpp
    client/QXmppIqHandling.cpp
    client/QXmppMamManager.cpp
    client/QXmppMessageReceiptManager.cpp
    client/QXmppMucManager.cpp
    client/QXmppOutgoingClient.cpp
    client/QXmppRosterManager.cpp
    client/QXmppRegistrationManager.cpp
    client/QXmppPubSubManager.cpp
    client/QXmppRemoteMethod.cpp
    client/QXmppRpcManager.cpp
    client/QXmppSendStanzaParams.cpp
    client/QXmppTlsManager.cpp
    client/QXmppTransferManager.cpp
    client/QXmppTrustManager.cpp
    client/QXmppTrustMemoryStorage.cpp
    client/QXmppTrustStorage.cpp
    client/QXmppUploadRequestManager.cpp
    client/QXmppUserLocationManager.cpp
    client/QXmppUserTuneManager.cpp
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

if(BUILD_SHARED)
    add_library(${QXMPP_TARGET} SHARED ${SOURCE_FILES})
else()
    add_library(${QXMPP_TARGET} STATIC ${SOURCE_FILES})
endif()

set_target_properties(${QXMPP_TARGET} PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${SO_VERSION}
    EXPORT_NAME QXmpp
)

target_include_directories(${QXMPP_TARGET}
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/base>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/client>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/server>
    $<INSTALL_INTERFACE:include/${QXMPP_TARGET}>
    PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(${QXMPP_TARGET}
    PUBLIC
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::Xml
)

if(WITH_GSTREAMER)
    find_package(GStreamer REQUIRED)
    find_package(GLIB2 REQUIRED)
    find_package(GObject REQUIRED)

    set(INSTALL_HEADER_FILES
        ${INSTALL_HEADER_FILES}
        client/QXmppCall.h
        client/QXmppCallManager.h
        client/QXmppCallStream.h
    )

    target_sources(${QXMPP_TARGET}
        PRIVATE
        client/QXmppCall.cpp
        client/QXmppCallManager.cpp
        client/QXmppCallStream.cpp
    )

    target_link_libraries(${QXMPP_TARGET}
        PRIVATE
        ${GLIB2_LIBRARIES}
        ${GOBJECT_LIBRARIES}
        ${GSTREAMER_LIBRARY}
    )

    target_include_directories(${QXMPP_TARGET}
        PRIVATE
        ${GLIB2_INCLUDE_DIR}
        ${GOBJECT_INCLUDE_DIR}
        ${GSTREAMER_INCLUDE_DIRS}
    )
endif()

if(BUILD_OMEMO)
    # required to be used in QXmppMessage
    target_sources(${QXMPP_TARGET} PRIVATE base/QXmppOmemoDataBase.cpp)
endif()

if(WITH_QCA)
    target_sources(${QXMPP_TARGET} PRIVATE client/QXmppEncryptedFileSharingProvider.cpp client/QXmppFileEncryption.cpp client/QcaInitializer.cpp)
    set(INSTALL_HEADER_FILES ${INSTALL_HEADER_FILES} client/QXmppEncryptedFileSharingProvider.h)
    target_link_libraries(${QXMPP_TARGET} PRIVATE qca-qt${QT_VERSION_MAJOR})
endif()

# qxmpp_export.h generation
if(BUILD_SHARED)
    set(QXMPP_BUILD_SHARED true)
else()
    set(QXMPP_BUILD_SHARED false)
endif()

set(QXMPP_CUSTOM_EXPORT_CONTENT "
#define QXMPP_BUILD_SHARED ${QXMPP_BUILD_SHARED}
#define QXMPP_VERSION_MAJOR ${PROJECT_VERSION_MAJOR}
#define QXMPP_VERSION_MINOR ${PROJECT_VERSION_MINOR}
#define QXMPP_VERSION_PATCH ${PROJECT_VERSION_PATCH}
")

generate_export_header(${QXMPP_TARGET}
    BASE_NAME qxmpp
    CUSTOM_CONTENT_FROM_VARIABLE QXMPP_CUSTOM_EXPORT_CONTENT
)

install(
    TARGETS ${QXMPP_TARGET}
    EXPORT QXmppTarget
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(
    EXPORT QXmppTarget
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${QXMPP_TARGET}"
    FILE QXmpp.cmake
    NAMESPACE QXmpp::
    COMPONENT Devel
)

export(
    TARGETS ${QXMPP_TARGET}
    FILE QXmpp.cmake
)

install(
    FILES ${INSTALL_HEADER_FILES} ${CMAKE_CURRENT_BINARY_DIR}/qxmpp_export.h
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${QXMPP_TARGET}"
)

# submodules
if(BUILD_OMEMO)
    add_subdirectory(omemo)
endif()
