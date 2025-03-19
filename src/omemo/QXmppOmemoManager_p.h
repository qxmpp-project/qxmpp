// SPDX-FileCopyrightText: 2022 Melvin Keskin <melvo@olomono.de>
// SPDX-FileCopyrightText: 2022 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPOMEMOMANAGER_P_H
#define QXMPPOMEMOMANAGER_P_H

#include "QXmppConstants_p.h"
#include "QXmppE2eeMetadata.h"
#include "QXmppOmemoDeviceBundle_p.h"
#include "QXmppOmemoManager.h"
#include "QXmppOmemoStorage.h"
#include "QXmppPubSubManager.h"

#include "OmemoLibWrappers.h"
#include "QcaInitializer_p.h"

#include <QDomElement>
#include <QTimer>
#include <QtCrypto>

#undef max

class QXmppTrustManager;
class QXmppOmemoManager;
class QXmppPubSubManager;
class QXmppPubSubNodeConfig;
class QXmppPubSubPublishOptions;
class QXmppOmemoIq;
class QXmppOmemoEnvelope;
class QXmppOmemoElement;
class QXmppOmemoDeviceListItem;
class QXmppOmemoDeviceBundleItem;

using namespace QXmpp;
using namespace std::chrono_literals;

namespace QXmpp::Omemo::Private {

// default possible trust levels a key must have to be used for encryption
// The class documentation must be adapted if the trust levels are modified.
constexpr auto ACCEPTED_TRUST_LEVELS = TrustLevel::AutomaticallyTrusted | TrustLevel::ManuallyTrusted | TrustLevel::Authenticated;

// count of unresponded stanzas sent to a device until QXmpp stops encrypting for it
constexpr int UNRESPONDED_STANZAS_UNTIL_ENCRYPTION_IS_STOPPED = 106;

// count of unresponded stanzas received from a device until a heartbeat message is sent to it
constexpr int UNRESPONDED_STANZAS_UNTIL_HEARTBEAT_MESSAGE_IS_SENT = 53;

// size of empty OMEMO message's decryption data
constexpr int EMPTY_MESSAGE_DECRYPTION_DATA_SIZE = 32;

// workaround for PubSub nodes that are not configurable to store 'max' as the value for
// 'pubsub#max_items'
constexpr uint64_t PUBSUB_NODE_MAX_ITEMS_1 = 1000;
constexpr uint64_t PUBSUB_NODE_MAX_ITEMS_2 = 100;
constexpr uint64_t PUBSUB_NODE_MAX_ITEMS_3 = 10;

constexpr uint32_t PRE_KEY_ID_MIN = 1;
constexpr uint32_t SIGNED_PRE_KEY_ID_MIN = 1;
constexpr uint32_t PRE_KEY_ID_MAX = std::numeric_limits<int32_t>::max();
constexpr uint32_t SIGNED_PRE_KEY_ID_MAX = std::numeric_limits<int32_t>::max();
constexpr uint32_t PRE_KEY_INITIAL_CREATION_COUNT = 100;

// maximum count of devices stored per JID
constexpr int DEVICES_PER_JID_MAX = 200;

// maximum count of devices for whom a stanza is encrypted
constexpr int DEVICES_PER_STANZA_MAX = 1000;

// interval to remove old signed pre keys and create new ones
constexpr auto SIGNED_PRE_KEY_RENEWAL_INTERVAL = 24h * 7 * 4;

// interval to check for old signed pre keys
constexpr auto SIGNED_PRE_KEY_RENEWAL_CHECK_INTERVAL = 24h;

// interval to remove devices locally after removal from their servers
constexpr auto DEVICE_REMOVAL_INTERVAL = 24h * 7 * 12;

// interval to check for devices removed from their servers
constexpr auto DEVICE_REMOVAL_CHECK_INTERVAL = 24h;

constexpr QStringView PAYLOAD_CIPHER_TYPE = u"aes256";
constexpr QCA::Cipher::Mode PAYLOAD_CIPHER_MODE = QCA::Cipher::CBC;
constexpr QCA::Cipher::Padding PAYLOAD_CIPHER_PADDING = QCA::Cipher::PKCS7;

inline constexpr auto HKDF_INFO = "OMEMO Payload";
constexpr int HKDF_KEY_SIZE = 32;
constexpr int HKDF_SALT_SIZE = 32;
constexpr int HKDF_OUTPUT_SIZE = 80;

inline constexpr QStringView PAYLOAD_MESSAGE_AUTHENTICATION_CODE_TYPE = u"hmac(sha256)";
constexpr uint32_t PAYLOAD_MESSAGE_AUTHENTICATION_CODE_SIZE = 16;

constexpr int PAYLOAD_KEY_SIZE = 32;
constexpr uint32_t PAYLOAD_INITIALIZATION_VECTOR_SIZE = 16;
constexpr uint32_t PAYLOAD_AUTHENTICATION_KEY_SIZE = 32;

// boundaries for the count of characters in SCE's <rpad/> element
constexpr uint32_t SCE_RPAD_SIZE_MIN = 0;
constexpr uint32_t SCE_RPAD_SIZE_MAX = 200;

struct PayloadEncryptionResult {
    QCA::SecureArray decryptionData;
    QByteArray encryptedPayload;
};

struct DecryptionResult {
    QDomElement sceContent;
    QXmppE2eeMetadata e2eeMetadata;
};

struct IqDecryptionResult {
    QDomElement iq;
    QXmppE2eeMetadata e2eeMetadata;
};

}  // namespace QXmpp::Omemo::Private

using namespace QXmpp::Private;
using namespace QXmpp::Omemo::Private;

class QXmppOmemoManagerPrivate
{
public:
    using Result = std::variant<QXmpp::Success, QXmppError>;

    QXmppOmemoManager *q;

    bool isStarted = false;
    bool isNewDeviceAutoSessionBuildingEnabled = false;

    QXmppOmemoStorage *omemoStorage;
    QXmppTrustManager *trustManager = nullptr;
    QXmppPubSubManager *pubSubManager = nullptr;

    QcaInitializer cryptoLibInitializer;
    QTimer signedPreKeyPairsRenewalTimer;
    QTimer deviceRemovalTimer;

    TrustLevels acceptedSessionBuildingTrustLevels = ACCEPTED_TRUST_LEVELS;

    QXmppOmemoStorage::OwnDevice ownDevice;
    QHash<uint32_t, QByteArray> preKeyPairs;
    QHash<uint32_t, QXmppOmemoStorage::SignedPreKeyPair> signedPreKeyPairs;
    QXmppOmemoDeviceBundle deviceBundle;

    int maximumDevicesPerJid = DEVICES_PER_JID_MAX;
    int maximumDevicesPerStanza = DEVICES_PER_STANZA_MAX;

    // recipient JID mapped to device ID mapped to device
    QHash<QString, QHash<uint32_t, QXmppOmemoStorage::Device>> devices;

    QList<QString> jidsOfManuallySubscribedDevices;

    OmemoContextPtr globalContext;
    StoreContextPtr storeContext;
    QRecursiveMutex mutex;
    signal_crypto_provider cryptoProvider;

    signal_protocol_identity_key_store identityKeyStore;
    signal_protocol_pre_key_store preKeyStore;
    signal_protocol_signed_pre_key_store signedPreKeyStore;
    signal_protocol_session_store sessionStore;

    QXmppOmemoManagerPrivate(QXmppOmemoManager *parent, QXmppOmemoStorage *omemoStorage);

    void init();
    // exports for unit tests
    QXMPP_EXPORT bool initGlobalContext();
    QXMPP_EXPORT bool initLocking();
    QXMPP_EXPORT bool initCryptoProvider();
    void initStores();

    signal_protocol_identity_key_store createIdentityKeyStore() const;
    signal_protocol_signed_pre_key_store createSignedPreKeyStore() const;
    signal_protocol_pre_key_store createPreKeyStore() const;
    signal_protocol_session_store createSessionStore() const;

    QXmppTask<bool> setUpDeviceId();
    std::optional<uint32_t> generateDeviceId();
    std::optional<uint32_t> generateDeviceId(const QVector<QString> &existingIds);
    bool setUpIdentityKeyPair(ratchet_identity_key_pair **identityKeyPair);
    void schedulePeriodicTasks();
    void renewSignedPreKeyPairs();
    bool updateSignedPreKeyPair(ratchet_identity_key_pair *identityKeyPair);
    bool renewPreKeyPairs(uint32_t keyPairBeingRenewed);
    bool updatePreKeyPairs(uint32_t count = 1);
    void removeDevicesRemovedFromServer();

    QXmppTask<QXmppE2eeExtension::MessageEncryptResult> encryptMessageForRecipients(QXmppMessage &&message,
                                                                                    QVector<QString> recipientJids,
                                                                                    TrustLevels acceptedTrustLevels);
    template<typename T>
    QXmppTask<std::optional<QXmppOmemoElement>> encryptStanza(const T &stanza, const QVector<QString> &recipientJids, TrustLevels acceptedTrustLevels);
    std::optional<PayloadEncryptionResult> encryptPayload(const QByteArray &payload) const;
    template<typename T>
    QByteArray createSceEnvelope(const T &stanza);
    QByteArray createOmemoEnvelopeData(const signal_protocol_address &address, const QCA::SecureArray &payloadDecryptionData) const;

    QXmppTask<std::optional<QXmppMessage>> decryptMessage(QXmppMessage stanza);
    QXmppTask<std::optional<IqDecryptionResult>> decryptIq(const QDomElement &iqElement);
    template<typename T>
    QXmppTask<std::optional<DecryptionResult>> decryptStanza(T stanza,
                                                             const QString &senderJid,
                                                             uint32_t senderDeviceId,
                                                             const QXmppOmemoEnvelope &omemoEnvelope,
                                                             const QByteArray &omemoPayload,
                                                             bool isMessageStanza = true);
    QXmppTask<QByteArray> extractSceEnvelope(const QString &senderJid,
                                             uint32_t senderDeviceId,
                                             const QXmppOmemoEnvelope &omemoEnvelope,
                                             const QByteArray &omemoPayload,
                                             bool isMessageStanza);
    QXmppTask<std::optional<QCA::SecureArray>> extractPayloadDecryptionData(const QString &senderJid,
                                                                            uint32_t senderDeviceId,
                                                                            const QXmppOmemoEnvelope &omemoEnvelope,
                                                                            bool isMessageStanza = true);
    QByteArray decryptPayload(const QCA::SecureArray &payloadDecryptionData, const QByteArray &payload) const;

    QXmppTask<bool> publishOmemoData();

    template<typename Function>
    void publishDeviceBundle(bool isDeviceBundlesNodeExistent,
                             bool arePublishOptionsSupported,
                             bool isAutomaticCreationSupported,
                             bool isCreationAndConfigurationSupported,
                             bool isCreationSupported,
                             bool isConfigurationSupported,
                             bool isConfigNodeMaxSupported,
                             Function continuation);
    template<typename Function>
    void publishDeviceBundleWithoutOptions(bool isDeviceBundlesNodeExistent,
                                           bool isCreationAndConfigurationSupported,
                                           bool isCreationSupported,
                                           bool isConfigurationSupported,
                                           bool isConfigNodeMaxSupported,
                                           Function continuation);
    template<typename Function>
    void configureNodeAndPublishDeviceBundle(bool isConfigNodeMaxSupported, Function continuation);
    template<typename Function>
    void createAndConfigureDeviceBundlesNode(bool isConfigNodeMaxSupported, Function continuation);
    template<typename Function>
    void createDeviceBundlesNode(Function continuation);
    template<typename Function>
    void configureDeviceBundlesNode(bool isConfigNodeMaxSupported, Function continuation);
    template<typename Function>
    void publishDeviceBundleItem(Function continuation);
    template<typename Function>
    void publishDeviceBundleItemWithOptions(Function continuation);
    QXmppOmemoDeviceBundleItem deviceBundleItem() const;
    QXmppTask<std::optional<QXmppOmemoDeviceBundle>> requestDeviceBundle(const QString &deviceOwnerJid, uint32_t deviceId) const;
    template<typename Function>
    void deleteDeviceBundle(Function continuation);

    template<typename Function>
    void publishDeviceElement(bool isDeviceListNodeExistent,
                              bool arePublishOptionsSupported,
                              bool isAutomaticCreationSupported,
                              bool isCreationAndConfigurationSupported,
                              bool isCreationSupported,
                              bool isConfigurationSupported,
                              Function continuation);
    template<typename Function>
    void publishDeviceElementWithoutOptions(bool isDeviceListNodeExistent,
                                            bool isCreationAndConfigurationSupported,
                                            bool isCreationSupported,
                                            bool isConfigurationSupported,
                                            Function continuation);
    template<typename Function>
    void configureNodeAndPublishDeviceElement(Function continuation);
    template<typename Function>
    void createAndConfigureDeviceListNode(Function continuation);
    template<typename Function>
    void createDeviceListNode(Function continuation);
    template<typename Function>
    void configureDeviceListNode(Function continuation);
    template<typename Function>
    void publishDeviceListItem(bool addOwnDevice, Function continuation);
    template<typename Function>
    void publishDeviceListItemWithOptions(Function continuation);
    QXmppOmemoDeviceListItem deviceListItem(bool addOwnDevice = true);
    template<typename Function>
    void updateOwnDevicesLocally(bool isDeviceListNodeExistent, Function continuation);
    std::optional<QXmppOmemoDeviceListItem> updateContactDevices(const QString &deviceOwnerJid, const QVector<QXmppOmemoDeviceListItem> &deviceListItems);
    void updateDevices(const QString &deviceOwnerJid, const QXmppOmemoDeviceListItem &deviceListItem);
    void handleIrregularDeviceListChanges(const QString &deviceOwnerJid);
    template<typename Function>
    void deleteDeviceElement(Function continuation);

    template<typename Function>
    void createNode(const QString &node, Function continuation);
    template<typename Function>
    void createNode(const QString &node, const QXmppPubSubNodeConfig &config, Function continuation);
    template<typename Function>
    void configureNode(const QString &node, const QXmppPubSubNodeConfig &config, Function continuation);
    template<typename Function>
    void retractItem(const QString &node, uint32_t itemId, Function continuation);
    template<typename Function>
    void deleteNode(const QString &node, Function continuation);

    template<typename T, typename Function>
    void publishItem(const QString &node, const T &item, Function continuation);
    template<typename T, typename Function>
    void publishItem(const QString &node, const T &item, const QXmppPubSubPublishOptions &publishOptions, Function continuation);

    template<typename T, typename Function>
    void runPubSubQueryWithContinuation(QXmppTask<T> future, const QString &errorMessage, Function continuation);

    QXmppTask<bool> changeDeviceLabel(const QString &deviceLabel);

    QXmppTask<QXmppPubSubManager::ItemResult<QXmppOmemoDeviceListItem>> requestDeviceList(const QString &jid);
    void subscribeToNewDeviceLists(const QString &jid, uint32_t deviceId);
    QXmppTask<Result> subscribeToDeviceList(const QString &jid);
    QXmppTask<QVector<QXmppOmemoManager::DevicesResult>> unsubscribeFromDeviceLists(const QList<QString> &jids);
    QXmppTask<Result> unsubscribeFromDeviceList(const QString &jid);

    QXmppTask<bool> resetOwnDevice();
    QXmppTask<void> resetOwnDeviceLocally();
    QXmppTask<bool> resetAll();
    void resetCachedData();

    QXmppTask<bool> buildSessionForNewDevice(const QString &jid, uint32_t deviceId, QXmppOmemoStorage::Device &device);
    QXmppTask<bool> buildSessionWithDeviceBundle(const QString &jid, uint32_t deviceId, QXmppOmemoStorage::Device &device);
    bool buildSession(signal_protocol_address address, const QXmppOmemoDeviceBundle &deviceBundle);
    bool createSessionBundle(session_pre_key_bundle **sessionBundle,
                             const QByteArray &serializedPublicIdentityKey,
                             const QByteArray &serializedSignedPublicPreKey,
                             uint32_t signedPublicPreKeyId,
                             const QByteArray &serializedSignedPublicPreKeySignature,
                             const QByteArray &serializedPublicPreKey,
                             uint32_t publicPreKeyId);

    bool deserializeIdentityKeyPair(ratchet_identity_key_pair **identityKeyPair) const;
    bool deserializePrivateIdentityKey(ec_private_key **privateIdentityKey, const QByteArray &serializedPrivateIdentityKey) const;
    bool deserializePublicIdentityKey(ec_public_key **publicIdentityKey, const QByteArray &serializedPublicIdentityKey) const;
    bool deserializeSignedPublicPreKey(ec_public_key **signedPublicPreKey, const QByteArray &serializedSignedPublicPreKey) const;
    bool deserializePublicPreKey(ec_public_key **publicPreKey, const QByteArray &serializedPublicPreKey) const;

    QXmppTask<QXmpp::SendResult> sendEmptyMessage(const QString &recipientJid, uint32_t recipientDeviceId, bool isKeyExchange = false) const;
    QXmppTask<void> storeOwnKey() const;
    QXmppTask<TrustLevel> storeKeyDependingOnSecurityPolicy(const QString &keyOwnerJid, const QByteArray &key);
    QXmppTask<TrustLevel> storeKey(const QString &keyOwnerJid, const QByteArray &key, TrustLevel trustLevel = TrustLevel::AutomaticallyDistrusted) const;
    QString ownBareJid() const;
    QString ownFullJid() const;
    QHash<uint32_t, QXmppOmemoStorage::Device> otherOwnDevices();

    void warning(const QString &msg) const;
};

#endif  // QXMPPOMEMOMANAGER_P_H
