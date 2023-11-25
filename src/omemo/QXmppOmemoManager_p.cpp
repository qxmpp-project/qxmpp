// SPDX-FileCopyrightText: 2022 Melvin Keskin <melvo@olomono.de>
// SPDX-FileCopyrightText: 2022 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

/// \cond

#include "QXmppOmemoManager_p.h"

#include "QXmppOmemoDeviceElement_p.h"
#include "QXmppOmemoElement_p.h"
#include "QXmppOmemoEnvelope_p.h"
#include "QXmppOmemoIq_p.h"
#include "QXmppOmemoItems_p.h"
#include "QXmppPubSubBaseItem.h"
#include "QXmppSceEnvelope_p.h"
#include "QXmppTrustManager.h"
#include "QXmppUtils.h"
#include "QXmppUtils_p.h"

#include <protocol.h>

#include "OmemoCryptoProvider.h"
#include <QRandomGenerator>
#include <QStringBuilder>

#undef max
#undef interface

using namespace QXmpp;
using namespace QXmpp::Private;
using namespace QXmpp::Omemo::Private;

using Error = QXmppStanza::Error;
using Manager = QXmppOmemoManager;
using ManagerPrivate = QXmppOmemoManagerPrivate;

constexpr auto ns_client = "jabber:client";
constexpr auto ns_pubsub_auto_create = "http://jabber.org/protocol/pubsub#auto-create";
constexpr auto ns_pubsub_config_node = "http://jabber.org/protocol/pubsub#config-node";
constexpr auto ns_pubsub_config_node_max = "http://jabber.org/protocol/pubsub#config-node-max";
constexpr auto ns_pubsub_create_and_configure = "http://jabber.org/protocol/pubsub#create-and-configure";
constexpr auto ns_pubsub_create_nodes = "http://jabber.org/protocol/pubsub#create-nodes";
constexpr auto ns_pubsub_publish = "http://jabber.org/protocol/pubsub#publish";
constexpr auto ns_pubsub_publish_options = "http://jabber.org/protocol/pubsub#publish-options";

namespace QXmpp::Omemo::Private {

const QString PAYLOAD_MESSAGE_AUTHENTICATION_CODE_TYPE = QStringLiteral("hmac(sha256)");

}  // namespace QXmpp::Omemo::Private

//
// Contains address data for an OMEMO device and a method to get the corresponding OMEMO library
// data structure.
//
class Address
{
public:
    //
    // Creates an OMEMO device address.
    //
    // \param jid bare JID of the device owner
    // \param deviceId ID of the device
    //
    Address(const QString &jid, uint32_t deviceId)
        : m_jid(jid.toUtf8()), m_deviceId(int32_t(deviceId))
    {
    }
    //
    // Returns the representation of the OMEMO device address used by the OMEMO library.
    //
    // \return the OMEMO library device address
    //
    signal_protocol_address data() const
    {
        return { m_jid.data(), size_t(m_jid.size()), m_deviceId };
    }

private:
    QByteArray m_jid;
    int32_t m_deviceId;
};

//
// Creates a PEP node configuration for the device list.
//
// \return the device list node configuration
//
static QXmppPubSubNodeConfig deviceListNodeConfig()
{
    QXmppPubSubNodeConfig config;
    config.setAccessModel(QXmppPubSubNodeConfig::Open);

    return config;
}

//
// Creates publish options for publishing the device list to a corresponding PEP node.
//
// \return the device list node publish options
//
static QXmppPubSubPublishOptions deviceListNodePublishOptions()
{
    QXmppPubSubPublishOptions publishOptions;
    publishOptions.setAccessModel(QXmppPubSubPublishOptions::Open);

    return publishOptions;
}

//
// Creates a PEP node configuration for device bundles.
//
// \return the device bundles node configuration
//
static QXmppPubSubNodeConfig deviceBundlesNodeConfig(QXmppPubSubNodeConfig::ItemLimit itemLimit = QXmppPubSubNodeConfig::Max())
{
    QXmppPubSubNodeConfig config;
    config.setAccessModel(QXmppPubSubNodeConfig::Open);
    config.setMaxItems(itemLimit);

    return config;
}

//
// Creates publish options for publishing device bundles to a corresponding PEP node.
//
// \return the device bundles node publish options
//
static QXmppPubSubPublishOptions deviceBundlesNodePublishOptions(QXmppPubSubNodeConfig::ItemLimit itemLimit = QXmppPubSubNodeConfig::Max())
{
    QXmppPubSubPublishOptions publishOptions;
    publishOptions.setAccessModel(QXmppPubSubPublishOptions::Open);
    publishOptions.setMaxItems(itemLimit);

    return publishOptions;
}

//
// Deserializes the signature of a signed public pre key.
//
// \param signedPublicPreKeySignature signed public pre key signature location
// \param serializedSignedPublicPreKeySignature serialized signature of the
//        signed public pre key
//
// \return whether it succeeded
//
static int deserializeSignedPublicPreKeySignature(const uint8_t **signedPublicPreKeySignature, const QByteArray &serializedSignedPublicPreKeySignature)
{
    *signedPublicPreKeySignature = reinterpret_cast<const uint8_t *>(serializedSignedPublicPreKeySignature.constData());
    return serializedSignedPublicPreKeySignature.size();
}

//
// Extracts the JID from an address used by the OMEMO library.
//
// \param address address containing the JID data
//
// \return the extracted JID
//
static QString extractJid(signal_protocol_address address)
{
    return QString::fromUtf8(address.name, address.name_len);
}

static QString errorToString(const QXmppStanza::Error &err)
{
    return u"Error('" % err.text() % u"', type=" % QString::number(err.type()) % u", condition=" %
        QString::number(err.condition()) % u")";
}

static QString errorToString(const QXmppError &err)
{
    if (auto sErr = err.value<QXmppStanza::Error>()) {
        return errorToString(*sErr);
    }
    return err.description;
}

static void replaceChildElements(QDomElement &oldElement, const QDomElement &newElement)
{
    // remove old child elements
    while (true) {
        if (auto childElement = oldElement.firstChildElement(); !childElement.isNull()) {
            oldElement.removeChild(childElement);
        } else {
            break;
        }
    }
    // append new child elements
    for (auto childElement = newElement.firstChildElement();
         !childElement.isNull();
         childElement = childElement.nextSiblingElement()) {
        oldElement.appendChild(childElement);
    }
}

template<typename T, typename Err>
auto mapToSuccess(std::variant<T, Err> var)
{
    return mapSuccess(std::move(var), [](T) { return Success(); });
}

QXmppOmemoManagerPrivate::QXmppOmemoManagerPrivate(Manager *parent, QXmppOmemoStorage *omemoStorage)
    : q(parent),
      omemoStorage(omemoStorage),
      signedPreKeyPairsRenewalTimer(parent),
      deviceRemovalTimer(parent)
{
}

//
// Initializes the OMEMO library.
//
void ManagerPrivate::init()
{
    if (initGlobalContext() &&
        initLocking() &&
        initCryptoProvider()) {
        initStores();
    } else {
        warning(QStringLiteral("OMEMO library could not be initialized"));
    }
}

//
// Initializes the OMEMO library's global context.
//
// \return whether the initialization succeeded
//
bool ManagerPrivate::initGlobalContext()
{
    // "q" is passed as the parameter "user_data" to functions called by
    // the OMEMO library when no explicit "user_data" is set for those
    // functions (e.g., to the lock and unlock functions).
    if (signal_context_create(globalContext.ptrRef(), q) < 0) {
        warning("Signal context could not be be created");
        return false;
    }

    return true;
}

//
// Initializes the OMEMO library's locking functions.
//
// \return whether the initialization succeeded
//
bool ManagerPrivate::initLocking()
{
    const auto lock = [](void *user_data) {
        const auto *manager = reinterpret_cast<Manager *>(user_data);
        auto *d = manager->d.get();
        d->mutex.lock();
    };

    const auto unlock = [](void *user_data) {
        const auto *manager = reinterpret_cast<Manager *>(user_data);
        auto *d = manager->d.get();
        d->mutex.unlock();
    };

    if (signal_context_set_locking_functions(globalContext.get(), lock, unlock) < 0) {
        warning("Locking functions could not be set");
        return false;
    }

    return true;
}

//
// Initializes the OMEMO library's crypto provider.
//
// \return whether the initialization succeeded
//
bool ManagerPrivate::initCryptoProvider()
{
    cryptoProvider = createOmemoCryptoProvider(this);

    if (signal_context_set_crypto_provider(globalContext.get(), &cryptoProvider) < 0) {
        warning("Crypto provider could not be set");
        return false;
    }

    return true;
}

//
// Initializes the OMEMO library's stores.
//
// \return whether the initialization succeeded
//
void ManagerPrivate::initStores()
{
    identityKeyStore = createIdentityKeyStore();
    preKeyStore = createPreKeyStore();
    signedPreKeyStore = createSignedPreKeyStore();
    sessionStore = createSessionStore();

    signal_protocol_store_context_create(storeContext.ptrRef(), globalContext.get());
    signal_protocol_store_context_set_identity_key_store(storeContext.get(), &identityKeyStore);
    signal_protocol_store_context_set_pre_key_store(storeContext.get(), &preKeyStore);
    signal_protocol_store_context_set_signed_pre_key_store(storeContext.get(), &signedPreKeyStore);
    signal_protocol_store_context_set_session_store(storeContext.get(), &sessionStore);
}

//
// Creates the OMEMO library's identity key store.
//
// The identity key is the long-term key.
//
// \return the identity key store
//
signal_protocol_identity_key_store ManagerPrivate::createIdentityKeyStore() const
{
    signal_protocol_identity_key_store store;

    store.get_identity_key_pair = [](signal_buffer **public_data, signal_buffer **private_data, void *user_data) {
        auto *manager = reinterpret_cast<Manager *>(user_data);
        const auto *d = manager->d.get();

        const auto &privateIdentityKey = d->ownDevice.privateIdentityKey;
        if (!(*private_data = signal_buffer_create(reinterpret_cast<const uint8_t *>(privateIdentityKey.constData()), privateIdentityKey.size()))) {
            manager->warning("Private identity key could not be loaded");
            return -1;
        }

        const auto &publicIdentityKey = d->ownDevice.publicIdentityKey;
        if (!(*public_data = signal_buffer_create(reinterpret_cast<const uint8_t *>(publicIdentityKey.constData()), publicIdentityKey.size()))) {
            manager->warning("Public identity key could not be loaded");
            return -1;
        }

        return 0;
    };

    store.get_local_registration_id = [](void *user_data, uint32_t *registration_id) {
        const auto *manager = reinterpret_cast<Manager *>(user_data);
        const auto *d = manager->d.get();
        *registration_id = d->ownDevice.id;
        return 0;
    };

    store.save_identity = [](const signal_protocol_address *, uint8_t *, size_t, void *) {
        // Do not use the OMEMO library's trust management.
        return 0;
    };

    store.is_trusted_identity = [](const signal_protocol_address *, uint8_t *, size_t, void *) {
        // Do not use the OMEMO library's trust management.
        // All keys are trusted at this level / by the OMEMO library.
        return 1;
    };

    store.destroy_func = [](void *) {
    };

    store.user_data = q;

    return store;
}

//
// Creates the OMEMO library's signed pre key store.
//
// A signed pre key is used for building a session.
//
// \return the signed pre key store
//
signal_protocol_signed_pre_key_store ManagerPrivate::createSignedPreKeyStore() const
{
    signal_protocol_signed_pre_key_store store;

    store.load_signed_pre_key = [](signal_buffer **record, uint32_t signed_pre_key_id, void *user_data) {
        auto *manager = reinterpret_cast<Manager *>(user_data);
        const auto *d = manager->d.get();
        const auto &signedPreKeyPair = d->signedPreKeyPairs.value(signed_pre_key_id).data;

        if (signedPreKeyPair.isEmpty()) {
            return SG_ERR_INVALID_KEY_ID;
        }

        if (!(*record = signal_buffer_create(reinterpret_cast<const uint8_t *>(signedPreKeyPair.constData()), signedPreKeyPair.size()))) {
            manager->warning("Signed pre key pair could not be loaded");
            return SG_ERR_INVALID_KEY_ID;
        }

        return SG_SUCCESS;
    };

    store.store_signed_pre_key = [](uint32_t signed_pre_key_id, uint8_t *record, size_t record_len, void *user_data) {
        auto *manager = reinterpret_cast<Manager *>(user_data);
        auto *d = manager->d.get();

        QXmppOmemoStorage::SignedPreKeyPair signedPreKeyPair;
        signedPreKeyPair.creationDate = QDateTime::currentDateTimeUtc();
        signedPreKeyPair.data = QByteArray(reinterpret_cast<const char *>(record), record_len);

        d->signedPreKeyPairs.insert(signed_pre_key_id, signedPreKeyPair);
        d->omemoStorage->addSignedPreKeyPair(signed_pre_key_id, signedPreKeyPair);

        return 0;
    };

    store.contains_signed_pre_key = [](uint32_t signed_pre_key_id, void *user_data) {
        const auto *manager = reinterpret_cast<Manager *>(user_data);
        const auto *d = manager->d.get();
        return d->signedPreKeyPairs.contains(signed_pre_key_id) ? 1 : 0;
    };

    store.remove_signed_pre_key = [](uint32_t signed_pre_key_id, void *user_data) {
        const auto *manager = reinterpret_cast<Manager *>(user_data);
        auto *d = manager->d.get();
        d->signedPreKeyPairs.remove(signed_pre_key_id);
        d->omemoStorage->removeSignedPreKeyPair(signed_pre_key_id);
        return 0;
    };

    store.destroy_func = [](void *) {
    };

    store.user_data = q;

    return store;
}

//
// Creates the OMEMO library's pre key store.
//
// A pre key is used for building a session.
//
// \return the pre key store
//
signal_protocol_pre_key_store ManagerPrivate::createPreKeyStore() const
{
    signal_protocol_pre_key_store store;

    store.load_pre_key = [](signal_buffer **record, uint32_t pre_key_id, void *user_data) {
        auto *manager = reinterpret_cast<Manager *>(user_data);
        const auto *d = manager->d.get();
        const auto &preKey = d->preKeyPairs.value(pre_key_id);

        if (preKey.isEmpty()) {
            return SG_ERR_INVALID_KEY_ID;
        }

        if (!(*record = signal_buffer_create(reinterpret_cast<const uint8_t *>(preKey.constData()), preKey.size()))) {
            manager->warning("Pre key could not be loaded");
            return SG_ERR_INVALID_KEY_ID;
        }

        return SG_SUCCESS;
    };

    store.store_pre_key = [](uint32_t pre_key_id, uint8_t *record, size_t record_len, void *user_data) {
        const auto *manager = reinterpret_cast<Manager *>(user_data);
        auto *d = manager->d.get();
        const auto preKey = QByteArray(reinterpret_cast<const char *>(record), record_len);
        d->preKeyPairs.insert(pre_key_id, preKey);
        d->omemoStorage->addPreKeyPairs({ { pre_key_id, preKey } });
        return 0;
    };

    store.contains_pre_key = [](uint32_t pre_key_id, void *user_data) {
        const auto *manager = reinterpret_cast<Manager *>(user_data);
        const auto *d = manager->d.get();
        return d->preKeyPairs.contains(pre_key_id) ? 1 : 0;
    };

    store.remove_pre_key = [](uint32_t pre_key_id, void *user_data) {
        auto *manager = reinterpret_cast<Manager *>(user_data);
        auto *d = manager->d.get();

        if (!d->renewPreKeyPairs(pre_key_id)) {
            return -1;
        }

        return 0;
    };

    store.destroy_func = [](void *) {
    };

    store.user_data = q;

    return store;
}

//
// Creates the OMEMO library's session store.
//
// A session contains all data needed for encryption and decryption.
//
// \return the session store
//
signal_protocol_session_store ManagerPrivate::createSessionStore() const
{
    signal_protocol_session_store store;

    store.load_session_func = [](signal_buffer **record, signal_buffer **, const signal_protocol_address *address, void *user_data) {
        auto *manager = reinterpret_cast<Manager *>(user_data);
        const auto *d = manager->d.get();
        const auto jid = extractJid(*address);

        const auto &session = d->devices.value(jid).value(uint32_t(address->device_id)).session;

        if (session.isEmpty()) {
            return 0;
        }

        if (!(*record = signal_buffer_create(reinterpret_cast<const uint8_t *>(session.constData()), size_t(session.size())))) {
            manager->warning("Session could not be loaded");
            return -1;
        }

        return 1;
    };

    store.get_sub_device_sessions_func = [](signal_int_list **sessions, const char *name, size_t name_len, void *user_data) {
        auto *manager = reinterpret_cast<Manager *>(user_data);
        const auto *d = manager->d.get();
        const auto jid = QString::fromUtf8(name, name_len);
        auto userDevices = d->devices.value(jid);

        // Remove all devices not having an active session.
        for (auto itr = userDevices.begin(); itr != userDevices.end();) {
            const auto &device = itr.value();
            if (device.session.isEmpty() || device.unrespondedSentStanzasCount == UNRESPONDED_STANZAS_UNTIL_ENCRYPTION_IS_STOPPED) {
                itr = userDevices.erase(itr);
            } else {
                ++itr;
            }
        }

        signal_int_list *deviceIds = signal_int_list_alloc();
        for (auto itr = userDevices.cbegin(); itr != userDevices.cend(); ++itr) {
            const auto deviceId = itr.key();
            if (signal_int_list_push_back(deviceIds, int(deviceId)) < 0) {
                manager->warning("Device ID could not be added to list");
                return -1;
            }
        }

        *sessions = deviceIds;
        return int(signal_int_list_size(*sessions));
    };

    store.store_session_func = [](const signal_protocol_address *address, uint8_t *record, size_t record_len, uint8_t *, size_t, void *user_data) {
        const auto *manager = reinterpret_cast<Manager *>(user_data);
        auto *d = manager->d.get();
        const auto session = QByteArray(reinterpret_cast<const char *>(record), record_len);
        const auto jid = extractJid(*address);
        const auto deviceId = int(address->device_id);

        auto &device = d->devices[jid][deviceId];
        device.session = session;
        d->omemoStorage->addDevice(jid, deviceId, device);
        return 0;
    };

    store.contains_session_func = [](const signal_protocol_address *address, void *user_data) {
        const auto *manager = reinterpret_cast<Manager *>(user_data);
        const auto *d = manager->d.get();
        const auto jid = extractJid(*address);
        return d->devices.value(jid).value(int(address->device_id)).session.isEmpty() ? 0 : 1;
    };

    store.delete_session_func = [](const signal_protocol_address *address, void *user_data) {
        const auto *manager = reinterpret_cast<Manager *>(user_data);
        auto *d = manager->d.get();
        const auto jid = extractJid(*address);
        const auto deviceId = int(address->device_id);
        auto &device = d->devices[jid][deviceId];
        if (!device.session.isEmpty()) {
            device.session.clear();
            d->omemoStorage->addDevice(jid, deviceId, device);
        }
        return 1;
    };

    store.delete_all_sessions_func = [](const char *name, size_t name_len, void *user_data) {
        const auto *manager = reinterpret_cast<Manager *>(user_data);
        auto *d = manager->d.get();
        const auto jid = QString::fromUtf8(name, name_len);
        auto deletedSessionsCount = 0;
        auto &userDevices = d->devices[jid];
        for (auto itr = userDevices.begin(); itr != userDevices.end(); ++itr) {
            const auto &deviceId = itr.key();
            auto &device = itr.value();
            if (!device.session.isEmpty()) {
                device.session.clear();
                d->omemoStorage->addDevice(jid, deviceId, device);
                ++deletedSessionsCount;
            }
        }
        return deletedSessionsCount;
    };

    store.destroy_func = [](void *) {
    };

    store.user_data = q;

    return store;
}

//
// Sets up the device ID.
//
// The more devices a user has, the higher the possibility of duplicate device IDs is.
// Especially for IoT scenarios with millions of devices, that can be an issue.
// Therefore, a new device ID is generated in case of a duplicate.
//
// \return whether it succeeded
//
QXmppTask<bool> ManagerPrivate::setUpDeviceId()
{
    auto future = pubSubManager->requestOwnPepItemIds(ns_omemo_2_bundles);
    return chain<bool>(std::move(future), q, [this](QXmppPubSubManager::ItemIdsResult result) mutable {
        // There can be the following cases:
        // 1. There is no PubSub node for device bundles: XEP-0030 states that a server must
        // respond with an error (at least ejabberd 22.05 responds with an empty node instead).
        // 2. There is an empty PubSub node for device bundles: XEP-0030 states that a server must
        // respond with a node without included items.
        auto error = std::get_if<QXmppError>(&result);
        if (error) {
            if (auto stanzaErr = error->value<QXmppStanza::Error>()) {
                // allow Cancel|ItemNotFound here
                if (!(stanzaErr->type() == Error::Cancel && stanzaErr->condition() == Error::ItemNotFound)) {
                    warning("Existing / Published device IDs could not be retrieved: " % errorToString(*error));
                    return false;
                }
                // do not return here
            } else {
                return false;
            }
        }

        // The first generated device ID can be used if no device bundle node exists.
        // Otherwise, duplicates must be avoided.
        auto deviceId = error ? generateDeviceId() : generateDeviceId(std::get<QVector<QString>>(result));
        if (deviceId) {
            ownDevice.id = *deviceId;
        }
        return deviceId.has_value();
    });
}

//
// Generates a new device ID.
//
// Returns the device ID or an empty optional on errors.
//
std::optional<uint32_t> QXmppOmemoManagerPrivate::generateDeviceId()
{
    uint32_t deviceId = 0;
    if (signal_protocol_key_helper_generate_registration_id(&deviceId, 0, globalContext.get()) < 0) {
        warning("Device ID could not be generated");
        return {};
    }
    return deviceId;
}

//
// Generates a new device ID that does not exist yet.
//
// Returns the device ID or an empty optional on errors.
//
std::optional<uint32_t> QXmppOmemoManagerPrivate::generateDeviceId(const QVector<QString> &existingIds)
{
    uint32_t deviceId = 0;

    do {
        if (signal_protocol_key_helper_generate_registration_id(&deviceId, 0, globalContext.get()) < 0) {
            warning("Device ID could not be generated");
            return {};
        }
    } while (existingIds.contains(QString::number(deviceId)));

    return deviceId;
}

//
// Sets up an identity key pair.
//
// The identity key pair consists of a private and a public long-term key.
//
// \return whether it succeeded
//
bool ManagerPrivate::setUpIdentityKeyPair(ratchet_identity_key_pair **identityKeyPair)
{
    if (signal_protocol_key_helper_generate_identity_key_pair(identityKeyPair, globalContext.get()) < 0) {
        warning("Identity key pair could not be generated");
        return false;
    }

    BufferSecurePtr privateIdentityKeyBuffer;

    if (ec_private_key_serialize(privateIdentityKeyBuffer.ptrRef(), ratchet_identity_key_pair_get_private(*identityKeyPair)) < 0) {
        warning("Private identity key could not be serialized");
        return false;
    }

    const auto privateIdentityKey = privateIdentityKeyBuffer.toByteArray();
    ownDevice.privateIdentityKey = privateIdentityKey;

    BufferPtr publicIdentityKeyBuffer(ec_public_key_get_ed(ratchet_identity_key_pair_get_public(*identityKeyPair)));
    const auto publicIdentityKey = publicIdentityKeyBuffer.toByteArray();
    deviceBundle.setPublicIdentityKey(publicIdentityKey);
    ownDevice.publicIdentityKey = publicIdentityKey;
    storeOwnKey();

    return true;
}

//
// Schedules periodic (time-based) tasks that cannot be done on a specific event.
//
void ManagerPrivate::schedulePeriodicTasks()
{
    QObject::connect(&signedPreKeyPairsRenewalTimer, &QTimer::timeout, q, [=]() mutable {
        renewSignedPreKeyPairs();
    });

    QObject::connect(&deviceRemovalTimer, &QTimer::timeout, q, [=]() mutable {
        removeDevicesRemovedFromServer();
    });

    signedPreKeyPairsRenewalTimer.start(SIGNED_PRE_KEY_RENEWAL_CHECK_INTERVAL);
    deviceRemovalTimer.start(DEVICE_REMOVAL_CHECK_INTERVAL);
}

//
// Removes old signed pre key pairs and creates a new one.
//
void ManagerPrivate::renewSignedPreKeyPairs()
{
    const auto currentDate = QDateTime::currentDateTimeUtc().toSecsSinceEpoch() * 1s;
    auto isSignedPreKeyPairRemoved = false;

    for (auto itr = signedPreKeyPairs.begin(); itr != signedPreKeyPairs.end();) {
        const auto creationDate = itr.value().creationDate.toSecsSinceEpoch() * 1s;

        // Remove signed pre key pairs older than
        // SIGNED_PRE_KEY_RENEWAL_INTERVAL.
        if (currentDate - creationDate > SIGNED_PRE_KEY_RENEWAL_INTERVAL) {
            itr = signedPreKeyPairs.erase(itr);
            omemoStorage->removeSignedPreKeyPair(itr.key());
            isSignedPreKeyPairRemoved = true;
        } else {
            ++itr;
        }
    }

    if (isSignedPreKeyPairRemoved) {
        RefCountedPtr<ratchet_identity_key_pair> identityKeyPair;
        deserializeIdentityKeyPair(identityKeyPair.ptrRef());
        updateSignedPreKeyPair(identityKeyPair.get());

        // Store the own device containing the new signed pre key ID.
        omemoStorage->setOwnDevice(ownDevice);

        publishDeviceBundleItem([=](bool isPublished) {
            if (!isPublished) {
                warning("Own device bundle item could not be published during renewal of signed pre key pairs");
            }
        });
    }
}

//
// Updates the signed pre key pairs.
//
// Make sure that
// \code
// d->omemoStorage->setOwnDevice(d->ownDevice);
// \endcode
// is called afterwards to store the change of
// \code
//  d->ownDevice.latestSignedPreKeyId()
// \endcode
// .
//
// \return whether it succeeded
//
bool ManagerPrivate::updateSignedPreKeyPair(ratchet_identity_key_pair *identityKeyPair)
{
    RefCountedPtr<session_signed_pre_key> signedPreKeyPair;
    auto latestSignedPreKeyId = ownDevice.latestSignedPreKeyId;

    // Ensure that no signed pre key ID exceeds SIGNED_PRE_KEY_ID_MAX
    // Do not increment during setup.
    if (latestSignedPreKeyId + 1 > SIGNED_PRE_KEY_ID_MAX) {
        latestSignedPreKeyId = SIGNED_PRE_KEY_ID_MIN;
    } else if (latestSignedPreKeyId != SIGNED_PRE_KEY_ID_MIN) {
        ++latestSignedPreKeyId;
    }

    if (signal_protocol_key_helper_generate_signed_pre_key(
            signedPreKeyPair.ptrRef(),
            identityKeyPair,
            latestSignedPreKeyId,
            uint64_t(QDateTime::currentMSecsSinceEpoch()),
            globalContext.get()) < 0) {
        warning("Signed pre key pair could not be generated");
        return false;
    }

    BufferSecurePtr signedPreKeyPairBuffer;

    if (session_signed_pre_key_serialize(signedPreKeyPairBuffer.ptrRef(), signedPreKeyPair.get()) < 0) {
        warning("Signed pre key pair could not be serialized");
        return false;
    }

    QXmppOmemoStorage::SignedPreKeyPair signedPreKeyPairForStorage;
    signedPreKeyPairForStorage.creationDate = QDateTime::currentDateTimeUtc();
    signedPreKeyPairForStorage.data = signedPreKeyPairBuffer.toByteArray();

    signedPreKeyPairs.insert(latestSignedPreKeyId, signedPreKeyPairForStorage);
    omemoStorage->addSignedPreKeyPair(latestSignedPreKeyId, signedPreKeyPairForStorage);

    BufferPtr signedPublicPreKeyBuffer(ec_public_key_get_mont(ec_key_pair_get_public(session_signed_pre_key_get_key_pair(signedPreKeyPair.get()))));
    const auto signedPublicPreKeyByteArray = signedPublicPreKeyBuffer.toByteArray();

    deviceBundle.setSignedPublicPreKeyId(latestSignedPreKeyId);
    deviceBundle.setSignedPublicPreKey(signedPublicPreKeyByteArray);
    deviceBundle.setSignedPublicPreKeySignature(QByteArray(reinterpret_cast<const char *>(session_signed_pre_key_get_signature_omemo(signedPreKeyPair.get())), session_signed_pre_key_get_signature_omemo_len(signedPreKeyPair.get())));

    ownDevice.latestSignedPreKeyId = latestSignedPreKeyId;

    return true;
}

//
// Deletes a pre key pair and creates a new one.
//
// \param keyPairBeingRenewed key pair being replaced by a new one
//
// \return whether it succeeded
//
bool ManagerPrivate::renewPreKeyPairs(uint32_t keyPairBeingRenewed)
{
    preKeyPairs.remove(keyPairBeingRenewed);
    omemoStorage->removePreKeyPair(keyPairBeingRenewed);
    deviceBundle.removePublicPreKey(keyPairBeingRenewed);

    if (!updatePreKeyPairs()) {
        return false;
    }

    // Store the own device containing the new pre key ID.
    omemoStorage->setOwnDevice(ownDevice);

    publishDeviceBundleItem([=](bool isPublished) {
        if (!isPublished) {
            warning("Own device bundle item could not be published during renewal of pre key pairs");
        }
    });

    return true;
}

//
// Updates the pre key pairs locally.
//
// Make sure that
// \code
// d->omemoStorage->setOwnDevice(d->ownDevice)
// \endcode
// is called
// afterwards to store the change of
// \code
// d->ownDevice.latestPreKeyId()
// \endcode
// .
//
// \param count number of pre key pairs to update
//
// \return whether it succeeded
//
bool ManagerPrivate::updatePreKeyPairs(uint32_t count)
{
    KeyListNodePtr newPreKeyPairs;
    auto latestPreKeyId = ownDevice.latestPreKeyId;

    // Ensure that no pre key ID exceeds PRE_KEY_ID_MAX.
    // Do not increment during setup.
    if (latestPreKeyId + count > PRE_KEY_ID_MAX) {
        latestPreKeyId = PRE_KEY_ID_MIN;
    } else if (latestPreKeyId != PRE_KEY_ID_MIN) {
        ++latestPreKeyId;
    }

    if (signal_protocol_key_helper_generate_pre_keys(newPreKeyPairs.ptrRef(), latestPreKeyId, count, globalContext.get()) < 0) {
        warning("Pre key pairs could not be generated");
        return false;
    }

    QHash<uint32_t, QByteArray> serializedPreKeyPairs;

    for (auto *node = newPreKeyPairs.get();
         node != nullptr;
         node = signal_protocol_key_helper_key_list_next(node)) {
        BufferSecurePtr preKeyPairBuffer;

        auto preKeyPair = signal_protocol_key_helper_key_list_element(node);

        if (session_pre_key_serialize(preKeyPairBuffer.ptrRef(), preKeyPair) < 0) {
            warning("Pre key pair could not be serialized");
            return false;
        }

        const auto preKeyId = session_pre_key_get_id(preKeyPair);

        serializedPreKeyPairs.insert(preKeyId, preKeyPairBuffer.toByteArray());

        BufferPtr publicPreKeyBuffer(ec_public_key_get_mont(ec_key_pair_get_public(session_pre_key_get_key_pair(preKeyPair))));
        const auto serializedPublicPreKey = publicPreKeyBuffer.toByteArray();
        deviceBundle.addPublicPreKey(preKeyId, serializedPublicPreKey);
    }

    preKeyPairs.insert(serializedPreKeyPairs);
    omemoStorage->addPreKeyPairs(serializedPreKeyPairs);
    ownDevice.latestPreKeyId = latestPreKeyId - 1 + count;

    return true;
}

//
// Removes locally stored devices after a specific time if they are removed from their owners'
// device lists on their servers.
//
void ManagerPrivate::removeDevicesRemovedFromServer()
{
    const auto currentDate = QDateTime::currentDateTimeUtc().toSecsSinceEpoch() * 1s;

    for (auto itr = devices.begin(); itr != devices.end(); ++itr) {
        const auto &jid = itr.key();
        auto &userDevices = itr.value();

        for (auto devicesItr = userDevices.begin(); devicesItr != userDevices.end();) {
            const auto &deviceId = devicesItr.key();
            const auto &device = devicesItr.value();

            // Remove data for devices removed from their servers after
            // DEVICE_REMOVAL_INTERVAL.
            const auto &removalDate = device.removalFromDeviceListDate;
            if (!removalDate.isNull() &&
                currentDate - removalDate.toSecsSinceEpoch() * 1s > DEVICE_REMOVAL_INTERVAL) {
                devicesItr = userDevices.erase(devicesItr);
                omemoStorage->removeDevice(jid, deviceId);
                trustManager->removeKeys(ns_omemo_2, QList { device.keyId });
                Q_EMIT q->deviceRemoved(jid, deviceId);
            } else {
                ++devicesItr;
            }
        }
    }
}

//
// Encrypts a message for specific recipients.
//
// \param message message to be encrypted
// \param recipientJids JIDs for whom the message is encrypted
// \param acceptedTrustLevels trust levels the keys of the recipients' devices must have to
//        encrypt for them
//
// \return the result of the encryption
//
QXmppTask<QXmppE2eeExtension::MessageEncryptResult> ManagerPrivate::encryptMessageForRecipients(QXmppMessage &&message, QVector<QString> recipientJids, TrustLevels acceptedTrustLevels)
{
    QXmppPromise<QXmppE2eeExtension::MessageEncryptResult> interface;

    if (!isStarted) {
        QXmppError error {
            QStringLiteral("OMEMO manager must be started before encrypting"),
            SendError::EncryptionError
        };
        interface.finish(error);
    } else {
        recipientJids.append(ownBareJid());

        auto future = encryptStanza(message, recipientJids, acceptedTrustLevels);
        future.then(q, [=, message = std::move(message)](std::optional<QXmppOmemoElement> omemoElement) mutable {
            if (!omemoElement) {
                QXmppError error {
                    QStringLiteral("OMEMO element could not be created"),
                    QXmpp::SendError::EncryptionError,
                };
                interface.finish(error);
            } else {
                const auto deliveryReceiptsUsed = message.isReceiptRequested() || !message.receiptId().isEmpty();
                const auto chatMarkerUsed = message.marker() != QXmppMessage::NoMarker;

                // The following cases are covered:
                // 1. Message with body (optionally including a chat state, chat marker or used for
                //    delivery) => usage of EME and fallback body
                // 2. Message without body
                //  2.1. Message with chat state, chat marker or used for delivery receipts
                //       => neither usage of EME nor fallback body, but hint for server-side storage
                //       in case of delivery receipts or chat marker usage
                //  2.2. Other message (e.g., trust message) => usage of EME and fallback body to
                //       look like a normal message
                if (!message.body().isEmpty() || (message.state() == QXmppMessage::None && !deliveryReceiptsUsed && !chatMarkerUsed)) {
                    message.setEncryptionMethod(QXmpp::Omemo2);

                    // A message processing hint for instructing the server to store the message is
                    // not needed because of the public fallback body.
                    message.setE2eeFallbackBody(QStringLiteral("This message is encrypted with %1 but could not be decrypted").arg(message.encryptionName()));
                    message.setIsFallback(true);
                } else if (deliveryReceiptsUsed || chatMarkerUsed) {
                    // A message processing hint for instructing the server to store the message is
                    // needed because of the missing public fallback body.
                    message.addHint(QXmppMessage::Store);
                }

                message.setOmemoElement(omemoElement);

                interface.finish(std::make_unique<QXmppMessage>(std::move(message)));
            }
        });
    }

    return interface.task();
}

//
// Encrypts a message or IQ stanza.
//
// \param stanza stanza to be encrypted
// \param recipientJids JIDs of the devices for whom the stanza is encrypted
// \param acceptedTrustLevels trust levels the keys of the recipients' devices must have to
//        encrypt for them
//
// \return the OMEMO element containing the stanza's encrypted content if the encryption is
//         successful, otherwise none
//
template<typename T>
QXmppTask<std::optional<QXmppOmemoElement>> ManagerPrivate::encryptStanza(const T &stanza, const QVector<QString> &recipientJids, TrustLevels acceptedTrustLevels)
{
    Q_ASSERT_X(!recipientJids.isEmpty(), "Creating OMEMO envelope", "OMEMO element could not be created because no recipient JIDs are passed");

    QXmppPromise<std::optional<QXmppOmemoElement>> interface;

    if (const auto optionalPayloadEncryptionResult = encryptPayload(createSceEnvelope(stanza))) {
        const auto &payloadEncryptionResult = *optionalPayloadEncryptionResult;

        auto devicesCount = std::accumulate(recipientJids.cbegin(), recipientJids.cend(), 0, [=](const auto sum, const auto &jid) {
            return sum + devices.value(jid).size();
        });

        // Do not exceed the maximum of manageable devices.
        if (devicesCount > maximumDevicesPerStanza) {
            warning(u"OMEMO payload could not be encrypted for all recipients because their "
                    "devices are altogether more than the maximum of manageable devices " %
                    QString::number(maximumDevicesPerStanza) %
                    u" - Use QXmppOmemoManager::setMaximumDevicesPerStanza() to increase the maximum");
            devicesCount = maximumDevicesPerStanza;
        }

        if (devicesCount) {
            auto omemoElement = std::make_shared<QXmppOmemoElement>();
            auto processedDevicesCount = std::make_shared<int>(0);
            auto successfullyProcessedDevicesCount = std::make_shared<int>(0);
            auto skippedDevicesCount = std::make_shared<int>(0);

            // Add envelopes for all devices of the recipients.
            for (const auto &jid : recipientJids) {
                auto recipientDevices = devices.value(jid);

                for (auto itr = recipientDevices.begin(); itr != recipientDevices.end(); ++itr) {
                    const auto &deviceId = itr.key();
                    const auto &device = itr.value();

                    // Skip encrypting for a device if it does not respond for a while.
                    if (const auto unrespondedSentStanzasCount = device.unrespondedSentStanzasCount; unrespondedSentStanzasCount == UNRESPONDED_STANZAS_UNTIL_ENCRYPTION_IS_STOPPED) {
                        if (++(*skippedDevicesCount) == devicesCount) {
                            warning("OMEMO element could not be created because no recipient device responded to " %
                                    QString::number(unrespondedSentStanzasCount) % " sent stanzas");
                            interface.finish(std::nullopt);
                        }

                        continue;
                    }

                    auto controlDeviceProcessing = [=](bool isSuccessful = true) mutable {
                        if (isSuccessful) {
                            ++(*successfullyProcessedDevicesCount);
                        }

                        if (++(*processedDevicesCount) == devicesCount) {
                            if (*successfullyProcessedDevicesCount == 0) {
                                warning("OMEMO element could not be created because no recipient "
                                        "devices with keys having accepted trust levels could be found");
                                interface.finish(std::nullopt);
                            } else {
                                omemoElement->setSenderDeviceId(ownDevice.id);
                                omemoElement->setPayload(payloadEncryptionResult.encryptedPayload);
                                interface.finish(*omemoElement);
                            }
                        }
                    };

                    const auto address = Address(jid, deviceId);

                    auto addOmemoEnvelope = [=](bool isKeyExchange = false) mutable {
                        // Create and add an OMEMO envelope only if its data could be created
                        // and the corresponding device has not been removed by another method
                        // in the meantime.
                        if (const auto data = createOmemoEnvelopeData(address.data(), payloadEncryptionResult.decryptionData); data.isEmpty()) {
                            warning("OMEMO envelope for recipient JID '" % jid %
                                    "' and device ID '" % QString::number(deviceId) %
                                    "' could not be created because its data could not be encrypted");
                            controlDeviceProcessing(false);
                        } else if (devices.value(jid).contains(deviceId)) {
                            auto &deviceBeingModified = devices[jid][deviceId];
                            deviceBeingModified.unrespondedReceivedStanzasCount = 0;
                            ++deviceBeingModified.unrespondedSentStanzasCount;
                            omemoStorage->addDevice(jid, deviceId, deviceBeingModified);

                            QXmppOmemoEnvelope omemoEnvelope;
                            omemoEnvelope.setRecipientDeviceId(deviceId);
                            if (isKeyExchange) {
                                omemoEnvelope.setIsUsedForKeyExchange(true);
                            }
                            omemoEnvelope.setData(data);
                            omemoElement->addEnvelope(jid, omemoEnvelope);
                            controlDeviceProcessing();
                        }
                    };

                    auto buildSessionDependingOnTrustLevel = [=](const QXmppOmemoDeviceBundle &deviceBundle, TrustLevel trustLevel) mutable {
                        // Build a session if the device's key has a specific trust level.
                        if (!acceptedTrustLevels.testFlag(trustLevel)) {
                            q->debug("Session could not be created for JID '" % jid %
                                     "' with device ID '" % QString::number(deviceId) %
                                     "' because its key's trust level '" %
                                     QString::number(int(trustLevel)) % "' is not accepted");
                            controlDeviceProcessing(false);
                        } else if (!buildSession(address.data(), deviceBundle)) {
                            warning("Session could not be created for JID '" % jid % "' and device ID '" % QString::number(deviceId) % "'");
                            controlDeviceProcessing(false);
                        } else {
                            addOmemoEnvelope(true);
                        }
                    };

                    // If the key ID is not stored (empty), the device bundle must be retrieved
                    // first.
                    // Afterwards, the bundle can be used to determine the key's trust level and
                    // to build the session.
                    // If the key ID is stored (not empty), the trust level can be directly
                    // determined and the session built.
                    if (device.keyId.isEmpty()) {
                        auto future = requestDeviceBundle(jid, deviceId);
                        future.then(q, [=](std::optional<QXmppOmemoDeviceBundle> optionalDeviceBundle) mutable {
                            // Process the device bundle only if one could be fetched and the
                            // corresponding device has not been removed by another method in
                            // the meantime.
                            if (optionalDeviceBundle && devices.value(jid).contains(deviceId)) {
                                auto &deviceBeingModified = devices[jid][deviceId];
                                const auto &deviceBundle = *optionalDeviceBundle;
                                deviceBeingModified.keyId = deviceBundle.publicIdentityKey();

                                auto future = q->trustLevel(jid, deviceBeingModified.keyId);
                                future.then(q, [=](TrustLevel trustLevel) mutable {
                                    // Store the retrieved key's trust level if it is not stored
                                    // yet.
                                    if (trustLevel == TrustLevel::Undecided) {
                                        auto future = storeKeyDependingOnSecurityPolicy(jid, deviceBeingModified.keyId);
                                        future.then(q, [=](TrustLevel trustLevel) mutable {
                                            omemoStorage->addDevice(jid, deviceId, deviceBeingModified);
                                            Q_EMIT q->deviceChanged(jid, deviceId);
                                            buildSessionDependingOnTrustLevel(deviceBundle, trustLevel);
                                        });
                                    } else {
                                        omemoStorage->addDevice(jid, deviceId, deviceBeingModified);
                                        Q_EMIT q->deviceChanged(jid, deviceId);
                                        buildSessionDependingOnTrustLevel(deviceBundle, trustLevel);
                                    }
                                });
                            } else {
                                warning("OMEMO envelope could not be created because no device bundle could be fetched");
                                controlDeviceProcessing(false);
                            }
                        });
                    } else {
                        auto future = q->trustLevel(jid, device.keyId);
                        future.then(q, [=](TrustLevel trustLevel) mutable {
                            // Create only OMEMO envelopes for devices that have keys with
                            // specific trust levels.
                            if (acceptedTrustLevels.testFlag(trustLevel)) {
                                // Build a new session if none is stored.
                                // Otherwise, use the existing session.
                                if (device.session.isEmpty()) {
                                    auto future = requestDeviceBundle(jid, deviceId);
                                    future.then(q, [=](std::optional<QXmppOmemoDeviceBundle> optionalDeviceBundle) mutable {
                                        if (optionalDeviceBundle) {
                                            const auto &deviceBundle = *optionalDeviceBundle;
                                            buildSessionDependingOnTrustLevel(deviceBundle, trustLevel);
                                        } else {
                                            warning("OMEMO envelope could not be created because no device bundle could be fetched");
                                            controlDeviceProcessing(false);
                                        }
                                    });
                                } else {
                                    addOmemoEnvelope();
                                }
                            } else {
                                q->debug("OMEMO envelope could not be created for JID '" % jid %
                                         "' and device ID '" % QString::number(deviceId) %
                                         "' because the device's key has an unaccepted trust level '" %
                                         QString::number(int(trustLevel)) % "'");
                                controlDeviceProcessing(false);
                            }
                        });
                    }
                }
            }
        } else {
            warning("OMEMO element could not be created because no recipient devices could be found");
            interface.finish(std::nullopt);
        }
    } else {
        warning("OMEMO payload could not be encrypted");
        interface.finish(std::nullopt);
    }

    return interface.task();
}

template QXmppTask<std::optional<QXmppOmemoElement>> ManagerPrivate::encryptStanza<QXmppIq>(const QXmppIq &, const QVector<QString> &, TrustLevels);
template QXmppTask<std::optional<QXmppOmemoElement>> ManagerPrivate::encryptStanza<QXmppMessage>(const QXmppMessage &, const QVector<QString> &, TrustLevels);

//
// Encrypts a payload symmetrically.
//
// \param payload payload being symmetrically encrypted
//
// \return the data used for encryption and the result
//
std::optional<PayloadEncryptionResult> ManagerPrivate::encryptPayload(const QByteArray &payload) const
{
    auto hkdfKey = QCA::SecureArray(QCA::Random::randomArray(HKDF_KEY_SIZE));
    const auto hkdfSalt = QCA::InitializationVector(QCA::SecureArray(HKDF_SALT_SIZE));
    const auto hkdfInfo = QCA::InitializationVector(QCA::SecureArray(HKDF_INFO));
    auto hkdfOutput = QCA::HKDF().makeKey(hkdfKey, hkdfSalt, hkdfInfo, HKDF_OUTPUT_SIZE);

    // first part of hkdfKey
    auto encryptionKey = QCA::SymmetricKey(hkdfOutput);
    encryptionKey.resize(PAYLOAD_KEY_SIZE);

    // middle part of hkdfKey
    auto authenticationKey = QCA::SymmetricKey(PAYLOAD_AUTHENTICATION_KEY_SIZE);
    const auto authenticationKeyOffset = hkdfOutput.data() + PAYLOAD_KEY_SIZE;
    std::copy(authenticationKeyOffset, authenticationKeyOffset + PAYLOAD_AUTHENTICATION_KEY_SIZE, authenticationKey.data());

    // last part of hkdfKey
    auto initializationVector = QCA::InitializationVector(PAYLOAD_INITIALIZATION_VECTOR_SIZE);
    const auto initializationVectorOffset = hkdfOutput.data() + PAYLOAD_KEY_SIZE + PAYLOAD_AUTHENTICATION_KEY_SIZE;
    std::copy(initializationVectorOffset, initializationVectorOffset + PAYLOAD_INITIALIZATION_VECTOR_SIZE, initializationVector.data());

    QCA::Cipher cipher(PAYLOAD_CIPHER_TYPE, PAYLOAD_CIPHER_MODE, PAYLOAD_CIPHER_PADDING, QCA::Encode, encryptionKey, initializationVector);
    auto encryptedPayload = cipher.process(QCA::MemoryRegion(payload));

    if (encryptedPayload.isEmpty()) {
        warning("Following payload could not be encrypted: " % QString::fromUtf8(payload));
        return {};
    }

    if (!QCA::MessageAuthenticationCode::supportedTypes().contains(PAYLOAD_MESSAGE_AUTHENTICATION_CODE_TYPE)) {
        warning("Message authentication code type '" % QString(PAYLOAD_MESSAGE_AUTHENTICATION_CODE_TYPE) % "' is not supported by this system");
        return {};
    }

    auto messageAuthenticationCodeGenerator = QCA::MessageAuthenticationCode(PAYLOAD_MESSAGE_AUTHENTICATION_CODE_TYPE, authenticationKey);
    auto messageAuthenticationCode = QCA::SecureArray(messageAuthenticationCodeGenerator.process(encryptedPayload));
    messageAuthenticationCode.resize(PAYLOAD_MESSAGE_AUTHENTICATION_CODE_SIZE);

    PayloadEncryptionResult payloadEncryptionData;
    payloadEncryptionData.decryptionData = hkdfKey.append(messageAuthenticationCode);
    payloadEncryptionData.encryptedPayload = encryptedPayload.toByteArray();

    return payloadEncryptionData;
}

//
// Creates the SCE envelope as defined in \xep{0420, Stanza Content Encryption} for a message
// or IQ stanza.
//
// The stanza's content that should be encrypted is put into the SCE content and that is added
// to the SCE envelope.
// Additionally, the standard SCE affix elements are added to the SCE envelope.
//
// \param stanza stanza for whom the SCE envelope is created
//
// \return the serialized SCE envelope
//
template<typename T>
QByteArray ManagerPrivate::createSceEnvelope(const T &stanza)
{
    QByteArray serializedSceEnvelope;
    QXmlStreamWriter writer(&serializedSceEnvelope);
    QXmppSceEnvelopeWriter sceEnvelopeWriter(writer);
    sceEnvelopeWriter.start();
    sceEnvelopeWriter.writeTimestamp(QDateTime::currentDateTimeUtc());
    sceEnvelopeWriter.writeTo(QXmppUtils::jidToBareJid(stanza.to()));
    sceEnvelopeWriter.writeFrom(q->client()->configuration().jidBare());
    sceEnvelopeWriter.writeRpad(generateRandomBytes(SCE_RPAD_SIZE_MIN, SCE_RPAD_SIZE_MAX).toBase64());
    sceEnvelopeWriter.writeContent([&writer, &stanza] {
        if constexpr (std::is_same_v<T, QXmppMessage>) {
            stanza.serializeExtensions(&writer, SceSensitive, ns_client);
        } else {
            // If the IQ stanza contains an error (i.e., it is an error response), that error is
            // serialized instead of actual content.
            if (auto err = stanza.errorOptional()) {
                err->toXml(&writer);
            } else {
                stanza.toXmlElementFromChild(&writer);
            }
        }
    });
    sceEnvelopeWriter.end();

    return serializedSceEnvelope;
}

//
// Creates the data of an OMEMO envelope.
//
// Encrypts the data used for a symmetric encryption of a payload asymmetrically with the
// recipient device's key.
//
// \param address address of a recipient device
// \param payloadDecryptionData data used for symmetric encryption being asymmetrically
//        encrypted
//
// \return the encrypted and serialized OMEMO envelope data or a default-constructed byte array
//         on failure
//
QByteArray ManagerPrivate::createOmemoEnvelopeData(const signal_protocol_address &address, const QCA::SecureArray &payloadDecryptionData) const
{
    SessionCipherPtr sessionCipher;

    if (session_cipher_create(sessionCipher.ptrRef(), storeContext.get(), &address, globalContext.get()) < 0) {
        warning("Session cipher could not be created");
        return {};
    }

    session_cipher_set_version(sessionCipher.get(), CIPHERTEXT_OMEMO_VERSION);

    RefCountedPtr<ciphertext_message> encryptedOmemoEnvelopeData;
    if (session_cipher_encrypt(sessionCipher.get(), reinterpret_cast<const uint8_t *>(payloadDecryptionData.constData()), payloadDecryptionData.size(), encryptedOmemoEnvelopeData.ptrRef()) != SG_SUCCESS) {
        warning("Payload decryption data could not be encrypted");
        return {};
    }

    signal_buffer *serializedEncryptedOmemoEnvelopeData = ciphertext_message_get_serialized(encryptedOmemoEnvelopeData.get());

    return {
        reinterpret_cast<const char *>(signal_buffer_data(serializedEncryptedOmemoEnvelopeData)),
        int(signal_buffer_len(serializedEncryptedOmemoEnvelopeData))
    };
}

//
// Decrypts a message stanza.
//
// In case of an empty (i.e., without payload) OMEMO message for session initiation, only the
// dummy payload's decryption data is decrypted but no payload.
// In case of a normal OMEMO message (i.e., with payload), the payload is decrypted and set as
// the content (i.e., first child element) of the returned stanza.
//
// \param stanza message stanza to be decrypted
//
// \return the decrypted stanza if it could be decrypted
//
QXmppTask<std::optional<QXmppMessage>> ManagerPrivate::decryptMessage(QXmppMessage stanza)
{
    // At this point, the stanza has always an OMEMO element.
    const auto omemoElement = *stanza.omemoElement();

    if (const auto omemoEnvelope = omemoElement.searchEnvelope(ownBareJid(), ownDevice.id)) {
        QXmppPromise<std::optional<QXmppMessage>> interface;

        const auto senderJid = QXmppUtils::jidToBareJid(stanza.from());
        const auto senderDeviceId = omemoElement.senderDeviceId();
        const auto omemoPayload = omemoElement.payload();

        subscribeToNewDeviceLists(senderJid, senderDeviceId);

        // Process empty OMEMO messages sent by a receiver of this device's first OMEMO message
        // for it after building the initial session or sent by devices to build a new session
        // with this device.
        if (omemoPayload.isEmpty()) {
            auto future = extractPayloadDecryptionData(senderJid, senderDeviceId, *omemoEnvelope);
            future.then(q, [=](std::optional<QCA::SecureArray> payloadDecryptionData) mutable {
                if (!payloadDecryptionData) {
                    warning("Empty OMEMO message could not be successfully processed");
                } else {
                    q->debug("Successfully processed empty OMEMO message");
                }

                interface.finish(std::nullopt);
            });
        } else {
            auto future = decryptStanza(stanza, senderJid, senderDeviceId, *omemoEnvelope, omemoPayload);
            future.then(q, [=](std::optional<DecryptionResult> optionalDecryptionResult) mutable {
                if (optionalDecryptionResult) {
                    const auto decryptionResult = std::move(*optionalDecryptionResult);
                    stanza.parseExtensions(decryptionResult.sceContent, SceSensitive);

                    // Remove the OMEMO element from the message because it is not needed
                    // anymore after decryption.
                    stanza.setOmemoElement({});

                    stanza.setE2eeMetadata(decryptionResult.e2eeMetadata);

                    interface.finish(stanza);
                } else {
                    interface.finish(std::nullopt);
                }
            });
        }

        return interface.task();
    } else {
        return makeReadyTask<std::optional<QXmppMessage>>(std::nullopt);
    }
}

//
// Decrypts an IQ stanza.
//
// The payload is decrypted and set as the content (i.e., first child element) of the returned
// stanza.
//
// \param iqElement DOM element of the IQ stanza to be decrypted. It MUST be an QXmppOmemoIq.
//
// \return the serialized decrypted stanza if it could be decrypted
//
QXmppTask<std::optional<IqDecryptionResult>> ManagerPrivate::decryptIq(const QDomElement &iqElement)
{
    using Result = std::optional<IqDecryptionResult>;

    QXmppOmemoIq iq;
    iq.parse(iqElement);
    auto omemoElement = iq.omemoElement();

    if (const auto omemoEnvelope = omemoElement.searchEnvelope(ownBareJid(), ownDevice.id)) {
        const auto senderJid = QXmppUtils::jidToBareJid(iq.from());
        const auto senderDeviceId = omemoElement.senderDeviceId();

        subscribeToNewDeviceLists(senderJid, senderDeviceId);

        auto future = decryptStanza(iq, senderJid, senderDeviceId, *omemoEnvelope, omemoElement.payload(), false);
        return chain<Result>(std::move(future), q, [iqElement](auto result) -> Result {
            if (result) {
                auto decryptedElement = iqElement.cloneNode(true).toElement();
                replaceChildElements(decryptedElement, result->sceContent);

                return IqDecryptionResult { decryptedElement, result->e2eeMetadata };
            }
            return {};
        });
    }
    return makeReadyTask<Result>(std::nullopt);
}

//
// Decrypts a message or IQ stanza.
//
// In case of an empty (i.e., without payload) OMEMO message for session initiation, only the
// dummy payload decryption data is decrypted but no payload.
// In case of a normal OMEMO stanza (i.e., with payload), the payload is decrypted and set as
// the content (i.e., first child element) of the returned stanza.
//
// \param stanza message or IQ stanza being decrypted
// \param senderJid JID of the stanza's sender
// \param senderDeviceId device ID of the stanza's sender
// \param omemoEnvelope OMEMO envelope within the OMEMO element
// \param omemoPayload OMEMO payload within the OMEMO element
// \param isMessageStanza whether the received stanza is a message stanza
//
// \return the result of the decryption if it succeeded
//
template<typename T>
QXmppTask<std::optional<DecryptionResult>> ManagerPrivate::decryptStanza(T stanza, const QString &senderJid, uint32_t senderDeviceId, const QXmppOmemoEnvelope &omemoEnvelope, const QByteArray &omemoPayload, bool isMessageStanza)
{
    QXmppPromise<std::optional<DecryptionResult>> interface;

    auto future = extractSceEnvelope(senderJid, senderDeviceId, omemoEnvelope, omemoPayload, isMessageStanza);
    future.then(q, [=](QByteArray serializedSceEnvelope) mutable {
        if (serializedSceEnvelope.isEmpty()) {
            warning("SCE envelope could not be extracted");
            interface.finish(std::nullopt);
        } else {
            QDomDocument document;
            document.setContent(serializedSceEnvelope, true);
            QXmppSceEnvelopeReader sceEnvelopeReader(document.documentElement());

            if (sceEnvelopeReader.from() != senderJid) {
                q->info("Sender '" % senderJid % "' of stanza does not match SCE 'from' affix element '" % sceEnvelopeReader.from() % "'");
            }

            if (const auto recipientJid = QXmppUtils::jidToBareJid(stanza.to()); isMessageStanza) {
                if (const auto &message = dynamic_cast<const QXmppMessage &>(stanza); message.type() == QXmppMessage::GroupChat && (sceEnvelopeReader.to() != recipientJid)) {
                    warning("Recipient of group chat message does not match SCE affix element '<to/>'");
                    interface.finish(std::nullopt);
                    return;
                }
            } else if (sceEnvelopeReader.to() != recipientJid) {
                q->info("Recipient of IQ does not match SCE affix element '<to/>'");
            }

            auto &device = devices[senderJid][senderDeviceId];
            device.unrespondedSentStanzasCount = 0;

            // Send a heartbeat message to the sender if too many stanzas were
            // received responding to none.
            if (device.unrespondedReceivedStanzasCount == UNRESPONDED_STANZAS_UNTIL_HEARTBEAT_MESSAGE_IS_SENT) {
                sendEmptyMessage(senderJid, senderDeviceId);
                device.unrespondedReceivedStanzasCount = 0;
            } else {
                ++device.unrespondedReceivedStanzasCount;
            }

            QXmppE2eeMetadata e2eeMetadata;
            e2eeMetadata.setSceTimestamp(sceEnvelopeReader.timestamp());
            e2eeMetadata.setEncryption(QXmpp::Omemo2);
            const auto &senderDevice = devices.value(senderJid).value(senderDeviceId);
            e2eeMetadata.setSenderKey(senderDevice.keyId);

            interface.finish(DecryptionResult { sceEnvelopeReader.contentElement(), e2eeMetadata });
        }
    });

    return interface.task();
}

//
// Extracts the SCE envelope from an OMEMO payload.
//
// The data used to encrypt the payload is decrypted and then used to decrypt the payload which
// contains the SCE envelope.
//
// \param senderJid bare JID of the stanza's sender
// \param senderDeviceId device ID of the stanza's sender
// \param omemoEnvelope OMEMO envelope containing the payload decryption data
// \param omemoPayload OMEMO payload containing the SCE envelope
// \param isMessageStanza whether the received stanza is a message stanza
//
// \return the serialized SCE envelope if it could be extracted, otherwise a
//         default-constructed byte array
//
QXmppTask<QByteArray> ManagerPrivate::extractSceEnvelope(const QString &senderJid, uint32_t senderDeviceId, const QXmppOmemoEnvelope &omemoEnvelope, const QByteArray &omemoPayload, bool isMessageStanza)
{
    QXmppPromise<QByteArray> interface;

    auto future = extractPayloadDecryptionData(senderJid, senderDeviceId, omemoEnvelope, isMessageStanza);
    future.then(q, [=](std::optional<QCA::SecureArray> payloadDecryptionData) mutable {
        if (!payloadDecryptionData) {
            warning("Data for decrypting OMEMO payload could not be extracted");
            interface.finish(QByteArray());
        } else {
            interface.finish(decryptPayload(*payloadDecryptionData, omemoPayload));
        }
    });

    return interface.task();
}

//
// Extracts the data used to decrypt the OMEMO payload.
//
// Decrypts the the payload decryption data and handles the OMEMO sessions.
//
// \param senderJid bare JID of the stanza's sender
// \param senderDeviceId device ID of the stanza's sender
// \param omemoEnvelope OMEMO envelope containing the payload decryption data
// \param isMessageStanza whether the received stanza is a message stanza
//
// \return the serialized payload decryption data if it could be extracted, otherwise std::nullopt
//
QXmppTask<std::optional<QCA::SecureArray>> ManagerPrivate::extractPayloadDecryptionData(const QString &senderJid, uint32_t senderDeviceId, const QXmppOmemoEnvelope &omemoEnvelope, bool isMessageStanza)
{
    QXmppPromise<std::optional<QCA::SecureArray>> interface;

    SessionCipherPtr sessionCipher;
    const auto address = Address(senderJid, senderDeviceId);
    const auto addressData = address.data();

    if (session_cipher_create(sessionCipher.ptrRef(), storeContext.get(), &addressData, globalContext.get()) < 0) {
        warning("Session cipher could not be created");
        return makeReadyTask<std::optional<QCA::SecureArray>>(std::nullopt);
    }

    session_cipher_set_version(sessionCipher.get(), CIPHERTEXT_OMEMO_VERSION);

    BufferSecurePtr payloadDecryptionDataBuffer;

    auto reportResult = [=](const BufferSecurePtr &buffer) mutable {
        // The buffer is copied into the SecureArray to avoid a QByteArray which is not secure.
        // However, it would be simpler if SecureArray had an appropriate constructor for that.
        const auto payloadDecryptionDataPointer = signal_buffer_data(buffer.get());
        const auto payloadDecryptionDataBufferSize = signal_buffer_len(buffer.get());
        auto payloadDecryptionData = QCA::SecureArray(payloadDecryptionDataBufferSize);
        std::copy_n(payloadDecryptionDataPointer, payloadDecryptionDataBufferSize, payloadDecryptionData.data());

        interface.finish(std::move(payloadDecryptionData));
    };

    // There are three cases:
    // 1. If the stanza contains key exchange data, a new session is automatically built by the
    // OMEMO library during decryption.
    // 2. If the stanza does not contain key exchange data and there is no existing session, the
    // stanza cannot be decrypted but a new session is built for future communication.
    // 3. If the stanza does not contain key exchange data and there is an existing session,
    // that session is used to decrypt the stanza.
    if (omemoEnvelope.isUsedForKeyExchange()) {
        RefCountedPtr<pre_key_signal_message> omemoEnvelopeData;
        const auto serializedOmemoEnvelopeData = omemoEnvelope.data();

        if (pre_key_signal_message_deserialize_omemo(omemoEnvelopeData.ptrRef(),
                                                     reinterpret_cast<const uint8_t *>(serializedOmemoEnvelopeData.data()),
                                                     serializedOmemoEnvelopeData.size(),
                                                     senderDeviceId,
                                                     globalContext.get()) < 0) {
            warning("OMEMO envelope data could not be deserialized");
            interface.finish(std::nullopt);
        } else {
            BufferPtr publicIdentityKeyBuffer;

            if (ec_public_key_serialize(publicIdentityKeyBuffer.ptrRef(), pre_key_signal_message_get_identity_key(omemoEnvelopeData.get())) < 0) {
                warning("Public Identity key could not be retrieved");
                interface.finish(std::nullopt);
            } else {
                const auto key = publicIdentityKeyBuffer.toByteArray();
                auto &device = devices[senderJid][senderDeviceId];
                auto &storedKeyId = device.keyId;

                // Store the key if its ID has changed.
                if (storedKeyId != key) {
                    storedKeyId = key;
                    omemoStorage->addDevice(senderJid, senderDeviceId, device);
                    Q_EMIT q->deviceChanged(senderJid, senderDeviceId);
                }

                // Decrypt the OMEMO envelope data and build a session.
                switch (session_cipher_decrypt_pre_key_signal_message(sessionCipher.get(), omemoEnvelopeData.get(), nullptr, payloadDecryptionDataBuffer.ptrRef())) {
                case SG_ERR_INVALID_MESSAGE:
                    warning("OMEMO envelope data for key exchange is not valid");
                    interface.finish(std::nullopt);
                    break;
                case SG_ERR_DUPLICATE_MESSAGE:
                    warning("OMEMO envelope data for key exchange is already received");
                    interface.finish(std::nullopt);
                    break;
                case SG_ERR_LEGACY_MESSAGE:
                    warning("OMEMO envelope data for key exchange format is deprecated");
                    interface.finish(std::nullopt);
                    break;
                case SG_ERR_INVALID_KEY_ID: {
                    const auto preKeyId = QString::number(pre_key_signal_message_get_pre_key_id(omemoEnvelopeData.get()));
                    warning("Pre key with ID '" % preKeyId %
                            "' of OMEMO envelope data for key exchange could not be found locally");
                    interface.finish(std::nullopt);
                    break;
                }
                case SG_ERR_INVALID_KEY:
                    warning("OMEMO envelope data for key exchange is incorrectly formatted");
                    interface.finish(std::nullopt);
                    break;
                case SG_ERR_UNTRUSTED_IDENTITY:
                    warning("Identity key of OMEMO envelope data for key exchange is not trusted by OMEMO library");
                    interface.finish(std::nullopt);
                    break;
                case SG_SUCCESS:
                    reportResult(payloadDecryptionDataBuffer);

                    // Send an empty message back to the sender in order to notify the sender's
                    // device that the session initiation is completed.
                    // Do not send an empty message if the received stanza is an IQ stanza
                    // because a response is already directly sent.
                    if (isMessageStanza) {
                        sendEmptyMessage(senderJid, senderDeviceId);
                    }

                    // Store the key's trust level if it is not stored yet.
                    auto future = q->trustLevel(senderJid, storedKeyId);
                    future.then(q, [=](TrustLevel trustLevel) mutable {
                        if (trustLevel == TrustLevel::Undecided) {
                            storeKeyDependingOnSecurityPolicy(senderJid, key);
                        }
                    });
                }
            }
        }
    } else if (auto &device = devices[senderJid][senderDeviceId]; device.session.isEmpty()) {
        warning("Received OMEMO stanza cannot be decrypted because there is no session with "
                "sending device, new session is being built");

        auto future = buildSessionWithDeviceBundle(senderJid, senderDeviceId, device);
        future.then(q, [=](auto) mutable {
            interface.finish(std::nullopt);
        });
    } else {
        RefCountedPtr<signal_message> omemoEnvelopeData;
        const auto serializedOmemoEnvelopeData = omemoEnvelope.data();

        if (signal_message_deserialize_omemo(omemoEnvelopeData.ptrRef(), reinterpret_cast<const uint8_t *>(serializedOmemoEnvelopeData.data()), serializedOmemoEnvelopeData.size(), globalContext.get()) < 0) {
            warning("OMEMO envelope data could not be deserialized");
            interface.finish(std::nullopt);
        } else {
            // Decrypt the OMEMO envelope data.
            switch (session_cipher_decrypt_signal_message(sessionCipher.get(), omemoEnvelopeData.get(), nullptr, payloadDecryptionDataBuffer.ptrRef())) {
            case SG_ERR_INVALID_MESSAGE:
                warning("OMEMO envelope data is not valid");
                interface.finish(std::nullopt);
                break;
            case SG_ERR_DUPLICATE_MESSAGE:
                warning("OMEMO envelope data is already received");
                interface.finish(std::nullopt);
                break;
            case SG_ERR_LEGACY_MESSAGE:
                warning("OMEMO envelope data format is deprecated");
                interface.finish(std::nullopt);
                break;
            case SG_ERR_NO_SESSION:
                warning("Session for OMEMO envelope data could not be found");
                interface.finish(std::nullopt);
            case SG_SUCCESS:
                reportResult(payloadDecryptionDataBuffer);
            }
        }
    }

    return interface.task();
}

//
// Decrypts the OMEMO payload.
//
// \param payloadDecryptionData data needed to decrypt the payload
// \param payload payload to be decrypted
//
// \return the decrypted payload or a default-constructed byte array on failure
//
QByteArray ManagerPrivate::decryptPayload(const QCA::SecureArray &payloadDecryptionData, const QByteArray &payload) const
{
    auto hkdfKey = QCA::SecureArray(payloadDecryptionData);
    hkdfKey.resize(HKDF_KEY_SIZE);
    const auto hkdfSalt = QCA::InitializationVector(QCA::SecureArray(HKDF_SALT_SIZE));
    const auto hkdfInfo = QCA::InitializationVector(QCA::SecureArray(HKDF_INFO));
    auto hkdfOutput = QCA::HKDF().makeKey(hkdfKey, hkdfSalt, hkdfInfo, HKDF_OUTPUT_SIZE);

    // first part of hkdfKey
    auto encryptionKey = QCA::SymmetricKey(hkdfOutput);
    encryptionKey.resize(PAYLOAD_KEY_SIZE);

    // middle part of hkdfKey
    auto authenticationKey = QCA::SymmetricKey(PAYLOAD_AUTHENTICATION_KEY_SIZE);
    const auto authenticationKeyOffset = hkdfOutput.data() + PAYLOAD_KEY_SIZE;
    std::copy(authenticationKeyOffset, authenticationKeyOffset + PAYLOAD_AUTHENTICATION_KEY_SIZE, authenticationKey.data());

    // last part of hkdfKey
    auto initializationVector = QCA::InitializationVector(PAYLOAD_INITIALIZATION_VECTOR_SIZE);
    const auto initializationVectorOffset = hkdfOutput.data() + PAYLOAD_KEY_SIZE + PAYLOAD_AUTHENTICATION_KEY_SIZE;
    std::copy(initializationVectorOffset, initializationVectorOffset + PAYLOAD_INITIALIZATION_VECTOR_SIZE, initializationVector.data());

    if (!QCA::MessageAuthenticationCode::supportedTypes().contains(PAYLOAD_MESSAGE_AUTHENTICATION_CODE_TYPE)) {
        warning("Message authentication code type '" % QString(PAYLOAD_MESSAGE_AUTHENTICATION_CODE_TYPE) % "' is not supported by this system");
        return {};
    }

    auto messageAuthenticationCodeGenerator = QCA::MessageAuthenticationCode(PAYLOAD_MESSAGE_AUTHENTICATION_CODE_TYPE, authenticationKey);
    auto messageAuthenticationCode = QCA::SecureArray(messageAuthenticationCodeGenerator.process(payload));
    messageAuthenticationCode.resize(PAYLOAD_MESSAGE_AUTHENTICATION_CODE_SIZE);

    auto expectedMessageAuthenticationCode = QCA::SecureArray(payloadDecryptionData.toByteArray().right(PAYLOAD_MESSAGE_AUTHENTICATION_CODE_SIZE));

    if (messageAuthenticationCode != expectedMessageAuthenticationCode) {
        warning("Message authentication code does not match expected one");
        return {};
    }

    QCA::Cipher cipher(PAYLOAD_CIPHER_TYPE, PAYLOAD_CIPHER_MODE, PAYLOAD_CIPHER_PADDING, QCA::Decode, encryptionKey, initializationVector);
    auto decryptedPayload = cipher.process(QCA::MemoryRegion(payload));

    if (decryptedPayload.isEmpty()) {
        warning("Following payload could not be decrypted: " % QString(payload));
        return {};
    }

    return decryptedPayload.toByteArray();
}

//
// Publishes the OMEMO data for this device.
//
// \return whether it succeeded
//
QXmppTask<bool> ManagerPrivate::publishOmemoData()
{
    QXmppPromise<bool> interface;

    auto future = pubSubManager->requestOwnPepFeatures();
    future.then(q, [=](QXmppPubSubManager::FeaturesResult result) mutable {
        if (const auto error = std::get_if<QXmppError>(&result)) {
            warning("Features of PEP service '" % ownBareJid() % "' could not be retrieved: " % errorToString(*error));
            warning("Device bundle and device list could not be published");
            interface.finish(false);
        } else {
            const auto &pepServiceFeatures = std::get<QVector<QString>>(result);

            // Check if the PEP service supports publishing items at all and also publishing
            // multiple items.
            // The support for publishing multiple items is needed to publish multiple device
            // bundles to the corresponding node.
            // It is checked here because if that is not possible, the publication of the device
            // element must not be published.
            // TODO: Uncomment the following line and remove the other one once ejabberd released version > 21.12
            // if (pepServiceFeatures.contains(ns_pubsub_publish) && pepServiceFeatures.contains(ns_pubsub_multi_items)) {
            if (pepServiceFeatures.contains(ns_pubsub_publish)) {
                auto future = pubSubManager->requestOwnPepNodes();
                future.then(q, [=](QXmppPubSubManager::NodesResult result) mutable {
                    if (const auto error = std::get_if<QXmppError>(&result)) {
                        warning("Nodes of JID '" % ownBareJid() % "' could not be fetched to check if nodes '" %
                                QString(ns_omemo_2_bundles) % "' and '" % QString(ns_omemo_2_devices) %
                                "' exist: " % errorToString(*error));
                        warning("Device bundle and device list could not be published");
                        interface.finish(false);
                    } else {
                        const auto &nodes = std::get<QVector<QString>>(result);

                        const auto deviceListNodeExists = nodes.contains(ns_omemo_2_devices);
                        const auto arePublishOptionsSupported = pepServiceFeatures.contains(ns_pubsub_publish_options);
                        const auto isAutomaticCreationSupported = pepServiceFeatures.contains(ns_pubsub_auto_create);
                        const auto isCreationAndConfigurationSupported = pepServiceFeatures.contains(ns_pubsub_create_and_configure);
                        const auto isCreationSupported = pepServiceFeatures.contains(ns_pubsub_create_nodes);
                        const auto isConfigurationSupported = pepServiceFeatures.contains(ns_pubsub_config_node);

                        // The device bundle is published before the device data is published.
                        // That way, it ensures that other devices are notified about this new
                        // device only after the corresponding device bundle is published.
                        auto handleResult = [this,
                                             interface,
                                             deviceListNodeExists,
                                             arePublishOptionsSupported,
                                             isAutomaticCreationSupported,
                                             isCreationAndConfigurationSupported,
                                             isCreationSupported,
                                             isConfigurationSupported](bool isPublished) mutable {
                            if (isPublished) {
                                publishDeviceElement(deviceListNodeExists,
                                                     arePublishOptionsSupported,
                                                     isAutomaticCreationSupported,
                                                     isCreationAndConfigurationSupported,
                                                     isCreationSupported,
                                                     isConfigurationSupported,
                                                     [=](bool isPublished) mutable {
                                                         if (!isPublished) {
                                                             warning("Device element could not be published");
                                                         }
                                                         interface.finish(std::move(isPublished));
                                                     });
                            } else {
                                warning("Device bundle could not be published");
                                interface.finish(false);
                            }
                        };
                        publishDeviceBundle(nodes.contains(ns_omemo_2_bundles),
                                            arePublishOptionsSupported,
                                            isAutomaticCreationSupported,
                                            isCreationAndConfigurationSupported,
                                            isCreationSupported,
                                            isConfigurationSupported,
                                            pepServiceFeatures.contains(ns_pubsub_config_node_max),
                                            handleResult);
                    }
                });
            } else {
                warning("Publishing (multiple) items to PEP node '" % ownBareJid() % "' is not supported");
                warning("Device bundle and device list could not be published");
                interface.finish(false);
            }
        }
    });

    return interface.task();
}

//
// Publishes this device's bundle.
//
// If no node for device bundles exists, a new one is created.
//
// \param isDeviceBundlesNodeExistent whether the PEP node for device bundles exists
// \param arePublishOptionsSupported whether publish options are supported by the PEP service
// \param isAutomaticCreationSupported whether the PEP service supports the automatic creation
//        of nodes when new items are published
// \param isCreationAndConfigurationSupported whether the PEP service supports the
//        configuration of nodes during their creation
// \param isCreationSupported whether the PEP service supports creating nodes
// \param isConfigurationSupported whether the PEP service supports configuring existing
//        nodes
// \param isConfigNodeMaxSupported whether the PEP service supports to set the maximum number
//        of allowed items per node to the maximum it supports
// \param continuation function to be called with the bool value whether it succeeded
//
template<typename Function>
void ManagerPrivate::publishDeviceBundle(bool isDeviceBundlesNodeExistent,
                                         bool arePublishOptionsSupported,
                                         bool isAutomaticCreationSupported,
                                         bool isCreationAndConfigurationSupported,
                                         bool isCreationSupported,
                                         bool isConfigurationSupported,
                                         bool isConfigNodeMaxSupported,
                                         Function continuation)
{
    // Check if the PEP service supports configuration of nodes during publication of items.
    if (arePublishOptionsSupported) {
        if (isAutomaticCreationSupported || isDeviceBundlesNodeExistent) {
            // The supported publish options cannot be determined because they are not announced
            // via Service Discovery.
            // Especially, there is no feature like ns_pubsub_multi_items and no error case
            // specified for the usage of
            // QXmppPubSubNodeConfig::ItemLimit as a publish option.
            // Thus, it simply tries to publish the item with that publish option.
            // If that fails, it tries to manually create and configure the node and publish the
            // item.
            publishDeviceBundleItemWithOptions([=](bool isPublished) mutable {
                if (isPublished) {
                    continuation(true);
                } else {
                    auto handleResult = [this, continuation = std::move(continuation)](bool isPublished) mutable {
                        if (!isPublished) {
                            q->debug("PEP service '" % ownBareJid() %
                                     "' does not support feature '" %
                                     QString(ns_pubsub_publish_options) %
                                     "' for all publish options, also not '" %
                                     QString(ns_pubsub_create_and_configure) %
                                     "', '" % QString(ns_pubsub_create_nodes) % "', '" %
                                     QString(ns_pubsub_config_node) % "' and the node does not exist");
                        }
                        continuation(isPublished);
                    };
                    publishDeviceBundleWithoutOptions(isDeviceBundlesNodeExistent,
                                                      isCreationAndConfigurationSupported,
                                                      isCreationSupported,
                                                      // TODO: Uncomment the following line and remove the other one once ejabberd released version > 21.12
                                                      // isConfigurationSupported,
                                                      true,
                                                      isConfigNodeMaxSupported,
                                                      handleResult);
                }
            });
        } else if (isCreationSupported) {
            // Create a node manually if the PEP service does not support creation of nodes
            // during publication of items and no node already
            // exists.
            createDeviceBundlesNode([=](bool isCreated) mutable {
                if (isCreated) {
                    // The supported publish options cannot be determined because they are not
                    // announced via Service Discovery.
                    // Especially, there is no feature like ns_pubsub_multi_items and no error
                    // case specified for the usage of QXmppPubSubNodeConfig::ItemLimit as a
                    // publish option.
                    // Thus, it simply tries to publish the item with that publish option.
                    // If that fails, it tries to manually configure the node and publish the
                    // item.
                    publishDeviceBundleItemWithOptions([=](bool isPublished) mutable {
                        if (isPublished) {
                            continuation(true);
                        } else if (isConfigurationSupported) {
                            configureNodeAndPublishDeviceBundle(isConfigNodeMaxSupported, continuation);
                        } else {
                            q->debug("PEP service '" % ownBareJid() %
                                     "' does not support feature '" %
                                     QString(ns_pubsub_publish_options) %
                                     "' for all publish options and also not '" %
                                     QString(ns_pubsub_config_node) % "'");
                            continuation(false);
                        }
                    });
                } else {
                    continuation(false);
                }
            });
        } else {
            q->debug("PEP service '" % ownBareJid() % "' does not support features '" %
                     QString(ns_pubsub_auto_create) % "', '" % QString(ns_pubsub_create_nodes) %
                     "' and the node does not exist");
            continuation(false);
        }
    } else {
        auto handleResult = [this, continuation = std::move(continuation)](bool isPublished) mutable {
            if (!isPublished) {
                q->debug("PEP service '" % ownBareJid() % "' does not support features '" %
                         QString(ns_pubsub_publish_options) % "', '" %
                         QString(ns_pubsub_create_and_configure) % "', '" %
                         QString(ns_pubsub_create_nodes) % "', '" %
                         QString(ns_pubsub_config_node) % "' and the node does not exist");
            }
            continuation(isPublished);
        };
        publishDeviceBundleWithoutOptions(isDeviceBundlesNodeExistent,
                                          isCreationAndConfigurationSupported,
                                          isCreationSupported,
                                          // TODO: Uncomment the following line and remove the other one once ejabberd released version > 21.12
                                          // isConfigurationSupported,
                                          true,
                                          isConfigNodeMaxSupported,
                                          handleResult);
    }
}

//
// Publish this device's bundle without publish options.
//
// If no node for device bundles exists, a new one is created.
//
// \param isDeviceBundlesNodeExistent whether the PEP node for device bundles exists
// \param isCreationAndConfigurationSupported whether the PEP service supports the
//        configuration of nodes during their creation
// \param isCreationSupported whether the PEP service supports creating nodes
// \param isConfigurationSupported whether the PEP service supports configuring existing
//        nodes
// \param isConfigNodeMaxSupported whether the PEP service supports to set the maximum number
//        of allowed items per node to the maximum it supports
// \param continuation function to be called with the bool value whether it succeeded
//
template<typename Function>
void ManagerPrivate::publishDeviceBundleWithoutOptions(bool isDeviceBundlesNodeExistent,
                                                       bool isCreationAndConfigurationSupported,
                                                       bool isCreationSupported,
                                                       bool isConfigurationSupported,
                                                       bool isConfigNodeMaxSupported,
                                                       Function continuation)
{
    if (isDeviceBundlesNodeExistent && isConfigurationSupported) {
        configureNodeAndPublishDeviceBundle(isConfigNodeMaxSupported, continuation);
    } else if (isCreationAndConfigurationSupported) {
        createAndConfigureDeviceBundlesNode(isConfigNodeMaxSupported, [=](bool isCreatedAndConfigured) mutable {
            if (isCreatedAndConfigured) {
                publishDeviceBundleItem(continuation);
            } else {
                continuation(false);
            }
        });
    } else if (isCreationSupported && isConfigurationSupported) {
        createDeviceBundlesNode([=](bool isCreated) mutable {
            if (isCreated) {
                configureNodeAndPublishDeviceBundle(isConfigNodeMaxSupported, continuation);
            } else {
                continuation(false);
            }
        });
    } else {
        continuation(false);
    }
}

//
// Configures the existing PEP node for device bundles and publishes this device's bundle on it.
//
// \param isConfigNodeMaxSupported whether the PEP service supports to set the maximum number
//        of allowed items per node to the maximum it supports
// \param continuation function to be called with the bool value whether it succeeded
//
template<typename Function>
void ManagerPrivate::configureNodeAndPublishDeviceBundle(bool isConfigNodeMaxSupported, Function continuation)
{
    configureDeviceBundlesNode(isConfigNodeMaxSupported, [=](bool isConfigured) mutable {
        if (isConfigured) {
            publishDeviceBundleItem(continuation);
        } else {
            continuation(false);
        }
    });
}

//
// Creates a PEP node for device bundles and configures it accordingly.
//
// \param isConfigNodeMaxSupported whether the PEP service supports to set the maximum number
//        of allowed items per node to the maximum it supports
// \param continuation function to be called with the bool value whether it succeeded
//
template<typename Function>
void ManagerPrivate::createAndConfigureDeviceBundlesNode(bool isConfigNodeMaxSupported, Function continuation)
{
    if (isConfigNodeMaxSupported) {
        createNode(ns_omemo_2_bundles, deviceBundlesNodeConfig(), continuation);
    } else {
        createNode(ns_omemo_2_bundles, deviceBundlesNodeConfig(PUBSUB_NODE_MAX_ITEMS_1), [=](bool isCreated) mutable {
            if (isCreated) {
                continuation(true);
            } else {
                createNode(ns_omemo_2_bundles, deviceBundlesNodeConfig(PUBSUB_NODE_MAX_ITEMS_2), [=](bool isCreated) mutable {
                    if (isCreated) {
                        continuation(true);
                    } else {
                        createNode(ns_omemo_2_bundles, deviceBundlesNodeConfig(PUBSUB_NODE_MAX_ITEMS_3), continuation);
                    }
                });
            }
        });
    }
}

//
// Creates a PEP node for device bundles.
//
// \param continuation function to be called with the bool value whether it succeeded
//
template<typename Function>
void ManagerPrivate::createDeviceBundlesNode(Function continuation)
{
    createNode(ns_omemo_2_bundles, continuation);
}

//
// Configures an existing PEP node for device bundles.
//
// There is no feature (like ns_pubsub_config_node_max as a config option) and no error case
// specified for the usage of \c QXmppPubSubNodeConfig::Max() as the value for the config
// option \c QXmppPubSubNodeConfig::ItemLimit.
// Thus, it tries to configure the node with that config option's value and if it fails, it
// tries again with pre-defined values.
// Each pre-defined value can exceed the maximum supported by the PEP service.
// Therefore, multiple values are tried.
//
// \param isConfigNodeMaxSupported whether the PEP service supports to set the
//        maximum number of allowed items per node to the maximum it supports
// \param continuation function to be called with the bool value whether it succeeded
//
template<typename Function>
void ManagerPrivate::configureDeviceBundlesNode(bool isConfigNodeMaxSupported, Function continuation)
{
    if (isConfigNodeMaxSupported) {
        configureNode(ns_omemo_2_bundles, deviceBundlesNodeConfig(), continuation);
    } else {
        configureNode(ns_omemo_2_bundles, deviceBundlesNodeConfig(PUBSUB_NODE_MAX_ITEMS_1), [=](bool isConfigured) mutable {
            if (isConfigured) {
                continuation(true);
            } else {
                configureNode(ns_omemo_2_bundles, deviceBundlesNodeConfig(PUBSUB_NODE_MAX_ITEMS_2), [=](bool isConfigured) mutable {
                    if (isConfigured) {
                        continuation(true);
                    } else {
                        configureNode(ns_omemo_2_bundles, deviceBundlesNodeConfig(PUBSUB_NODE_MAX_ITEMS_3), continuation);
                    }
                });
            }
        });
    }
}

//
// Publishes this device bundle's item on the corresponding existing PEP node.
//
// \param continuation function to be called with the bool value whether it succeeded
//
template<typename Function>
void ManagerPrivate::publishDeviceBundleItem(Function continuation)
{
    publishItem(ns_omemo_2_bundles, deviceBundleItem(), continuation);
}

//
// Publishes this device bundle's item with publish options.
//
// If no node for device bundles exists, a new one is created.
//
// There is no feature (like ns_pubsub_config_node_max as a config option) and no error case
// specified for the usage of \c QXmppPubSubNodeConfig::Max() as the value for the publish
// option \c QXmppPubSubNodeConfig::ItemLimit.
// Thus, it tries to publish the item with that publish option's value and if it fails, it
// tries again with pre-defined values.
// Each pre-defined value can exceed the maximum supported by the PEP service.
// Therefore, multiple values are tried.
//
// \param continuation function to be called with the bool value whether it succeeded
//
template<typename Function>
void ManagerPrivate::publishDeviceBundleItemWithOptions(Function continuation)
{
    publishItem(ns_omemo_2_bundles, deviceBundleItem(), deviceBundlesNodePublishOptions(), [=](bool isPublished) mutable {
        if (isPublished) {
            continuation(true);
        } else {
            publishItem(ns_omemo_2_bundles, deviceBundleItem(), deviceBundlesNodePublishOptions(PUBSUB_NODE_MAX_ITEMS_1), [=](bool isPublished) mutable {
                if (isPublished) {
                    continuation(true);
                } else {
                    publishItem(ns_omemo_2_bundles, deviceBundleItem(), deviceBundlesNodePublishOptions(PUBSUB_NODE_MAX_ITEMS_2), [=](bool isPublished) mutable {
                        if (isPublished) {
                            continuation(true);
                        } else {
                            publishItem(ns_omemo_2_bundles, deviceBundleItem(), deviceBundlesNodePublishOptions(PUBSUB_NODE_MAX_ITEMS_3), continuation);
                        }
                    });
                }
            });
        }
    });
}

//
// Creates a PEP item for this device's bundle.
//
// \return this device bundle's item
//
QXmppOmemoDeviceBundleItem ManagerPrivate::deviceBundleItem() const
{
    QXmppOmemoDeviceBundleItem item;
    item.setId(QString::number(ownDevice.id));
    item.setDeviceBundle(deviceBundle);

    return item;
}

//
// Requests a device bundle from a PEP service.
//
// \param deviceOwnerJid bare JID of the device's owner
// \param deviceId ID of the device whose bundle is requested
//
// \return the device bundle on success, otherwise a nullptr
//
QXmppTask<std::optional<QXmppOmemoDeviceBundle>> ManagerPrivate::requestDeviceBundle(const QString &deviceOwnerJid, uint32_t deviceId) const
{
    QXmppPromise<std::optional<QXmppOmemoDeviceBundle>> interface;

    auto future = pubSubManager->requestItem<QXmppOmemoDeviceBundleItem>(deviceOwnerJid, ns_omemo_2_bundles, QString::number(deviceId));
    future.then(q, [=](QXmppPubSubManager::ItemResult<QXmppOmemoDeviceBundleItem> result) mutable {
        if (const auto error = std::get_if<QXmppError>(&result)) {
            warning("Device bundle for JID '" % deviceOwnerJid % "' and device ID '" %
                    QString::number(deviceId) % "' could not be retrieved: " % errorToString(*error));
            interface.finish(std::nullopt);
        } else {
            const auto &item = std::get<QXmppOmemoDeviceBundleItem>(result);
            interface.finish(item.deviceBundle());
        }
    });

    return interface.task();
}

//
// Removes the device bundle for this device or deletes the whole node if it would be empty
// after the retraction.
//
// \param continuation function to be called with the bool value whether it succeeded
//
template<typename Function>
void ManagerPrivate::deleteDeviceBundle(Function continuation)
{
    if (otherOwnDevices().isEmpty()) {
        deleteNode(ns_omemo_2_bundles, continuation);
    } else {
        retractItem(ns_omemo_2_bundles, ownDevice.id, continuation);
    }
}

//
// Publishes this device's element within the device list.
//
// If no node for the device list exists, a new one is created.
//
// \param isDeviceListNodeExistent whether the PEP node for the device list exists
// \param arePublishOptionsSupported whether publish options are supported by the PEP service
// \param isAutomaticCreationSupported whether the PEP service supports the automatic creation
//        of nodes when new items are published
// \param isCreationAndConfigurationSupported whether the PEP service supports the
//        configuration of nodes during their creation
// \param isCreationSupported whether the PEP service supports creating nodes
// \param isConfigurationSupported whether the PEP service supports configuring existing
//        nodes
// \param continuation function to be called with the bool value whether it succeeded
//
template<typename Function>
void ManagerPrivate::publishDeviceElement(bool isDeviceListNodeExistent,
                                          bool arePublishOptionsSupported,
                                          bool isAutomaticCreationSupported,
                                          bool isCreationAndConfigurationSupported,
                                          bool isCreationSupported,
                                          bool isConfigurationSupported,
                                          Function continuation)
{
    updateOwnDevicesLocally(isDeviceListNodeExistent, [=](bool isUpdated) mutable {
        if (isUpdated) {
            // Check if the PEP service supports configuration of nodes during
            // publication of items.
            if (arePublishOptionsSupported) {
                if (isAutomaticCreationSupported || isDeviceListNodeExistent) {
                    // The supported publish options cannot be determined because they
                    // are not announced via Service Discovery.
                    // Thus, it simply tries to publish the item with the specified
                    // publish options.
                    // If that fails, it tries to manually create and configure the node
                    // and publish the item.
                    publishDeviceListItemWithOptions([=](bool isPublished) mutable {
                        if (isPublished) {
                            continuation(true);
                        } else {
                            auto handleResult = [this, continuation = std::move(continuation)](bool isPublished) mutable {
                                if (!isPublished) {
                                    q->debug("PEP service '" % ownBareJid() % "' does not support feature '" % QString(ns_pubsub_publish_options) % "' for all publish options, also not '" % QString(ns_pubsub_create_and_configure) % "', '" % QString(ns_pubsub_create_nodes) % "', '" % QString(ns_pubsub_config_node) % "' and the node does not exist");
                                }
                                continuation(isPublished);
                            };
                            publishDeviceElementWithoutOptions(isDeviceListNodeExistent,
                                                               isCreationAndConfigurationSupported,
                                                               isCreationSupported,
                                                               // TODO: Uncomment the following line and remove the other one once ejabberd released version > 21.12
                                                               // isConfigurationSupported);
                                                               true,
                                                               handleResult);
                        }
                    });
                } else if (isCreationSupported) {
                    // Create a node manually if the PEP service does not support creation of
                    // nodes during publication of items and no node already exists.
                    createDeviceListNode([=](bool isCreated) mutable {
                        if (isCreated) {
                            // The supported publish options cannot be determined because they
                            // are not announced via Service Discovery.
                            // Thus, it simply tries to publish the item with the specified
                            // publish options.
                            // If that fails, it tries to manually configure the node and
                            // publish the item.
                            publishDeviceListItemWithOptions([=, continuation = std::move(continuation)](bool isPublished) mutable {
                                if (isPublished) {
                                    continuation(true);
                                } else if (isConfigurationSupported) {
                                    configureNodeAndPublishDeviceElement(continuation);
                                } else {
                                    q->debug("PEP service '" % ownBareJid() %
                                             "' does not support feature '" %
                                             QString(ns_pubsub_publish_options) %
                                             "' for all publish options and also not '" %
                                             QString(ns_pubsub_config_node) % "'");
                                    continuation(false);
                                }
                            });
                        } else {
                            continuation(false);
                        }
                    });
                } else {
                    q->debug("PEP service '" % ownBareJid() % "' does not support features '" %
                             QString(ns_pubsub_auto_create) % "', '" %
                             QString(ns_pubsub_create_nodes) % "' and the node does not exist");
                    continuation(false);
                }
            } else {
                auto handleResult = [=](bool isPublished) mutable {
                    if (!isPublished) {
                        q->debug("PEP service '" % ownBareJid() % "' does not support features '" % QString(ns_pubsub_publish_options) % "', '" % QString(ns_pubsub_create_and_configure) % "', '" % QString(ns_pubsub_create_nodes) % "', '" % QString(ns_pubsub_config_node) % "' and the node does not exist");
                    }
                    continuation(isPublished);
                };
                publishDeviceElementWithoutOptions(isDeviceListNodeExistent,
                                                   isCreationAndConfigurationSupported,
                                                   isCreationSupported,
                                                   // TODO: Uncomment the following line and remove the other one once ejabberd released version > 21.12
                                                   // isConfigurationSupported);
                                                   true,
                                                   handleResult);
            }
        } else {
            continuation(false);
        }
    });
}

//
// Publish this device's element without publish options.
//
// If no node for the device list exists, a new one is created.
//
// \param isDeviceListNodeExistent whether the PEP node for the device list exists
// \param isCreationAndConfigurationSupported whether the PEP service supports the
//        configuration of nodes during their creation
// \param isCreationSupported whether the PEP service supports creating nodes
// \param isConfigurationSupported whether the PEP service supports configuring existing
//        nodes
// \param continuation function to be called with the bool value whether it succeeded
//
template<typename Function>
void ManagerPrivate::publishDeviceElementWithoutOptions(bool isDeviceListNodeExistent, bool isCreationAndConfigurationSupported, bool isCreationSupported, bool isConfigurationSupported, Function continuation)
{
    if (isDeviceListNodeExistent && isConfigurationSupported) {
        configureNodeAndPublishDeviceElement(continuation);
    } else if (isCreationAndConfigurationSupported) {
        createAndConfigureDeviceListNode([=](bool isCreatedAndConfigured) mutable {
            if (isCreatedAndConfigured) {
                publishDeviceListItem(true, continuation);
            } else {
                continuation(false);
            }
        });
    } else if (isCreationSupported && isConfigurationSupported) {
        createDeviceListNode([=](bool isCreated) mutable {
            if (isCreated) {
                configureNodeAndPublishDeviceElement(continuation);
            } else {
                continuation(false);
            }
        });
    } else {
        continuation(false);
    }
}

//
// Configures the existing PEP node for the device list and publishes this device's element on
// it.
//
// \param continuation function to be called with the bool value whether it succeeded
//
template<typename Function>
void ManagerPrivate::configureNodeAndPublishDeviceElement(Function continuation)
{
    configureDeviceListNode([=](bool isConfigured) mutable {
        if (isConfigured) {
            publishDeviceListItem(true, continuation);
        } else {
            continuation(false);
        }
    });
}

//
// Creates a PEP node for the device list and configures it accordingly.
//
// \param continuation function to be called with the bool value whether it succeeded
//
template<typename Function>
void ManagerPrivate::createAndConfigureDeviceListNode(Function continuation)
{
    createNode(ns_omemo_2_devices, deviceListNodeConfig(), continuation);
}

//
// Creates a PEP node for the device list.
//
// \param continuation function to be called with the bool value whether it succeeded
//
template<typename Function>
void ManagerPrivate::createDeviceListNode(Function continuation)
{
    createNode(ns_omemo_2_devices, continuation);
}

//
// Configures an existing PEP node for the device list.
//
// \param continuation function to be called with the bool value whether it succeeded
//
template<typename Function>
void ManagerPrivate::configureDeviceListNode(Function continuation)
{
    configureNode(ns_omemo_2_devices, deviceListNodeConfig(), std::move(continuation));
}

//
// Publishes the device list item containing this device's element on the corresponding existing
// PEP node.
//
// \param continuation function to be called with the bool value whether it succeeded
//
template<typename Function>
void ManagerPrivate::publishDeviceListItem(bool addOwnDevice, Function continuation)
{
    publishItem(ns_omemo_2_devices, deviceListItem(addOwnDevice), continuation);
}

//
// Publishes the device list item containing this device's element with publish options.
//
// If no node for the device list exists, a new one is created.
//
// \param continuation function to be called with the bool value whether it succeeded
//
template<typename Function>
void ManagerPrivate::publishDeviceListItemWithOptions(Function continuation)
{
    publishItem(ns_omemo_2_devices, deviceListItem(), deviceListNodePublishOptions(), continuation);
}

//
// Creates a PEP item for the device list containing this device's element.
//
// \return the device list item
//
QXmppOmemoDeviceListItem ManagerPrivate::deviceListItem(bool addOwnDevice)
{
    QXmppOmemoDeviceList deviceList;

    // Add this device to the device list.
    if (addOwnDevice) {
        QXmppOmemoDeviceElement deviceElement;
        deviceElement.setId(ownDevice.id);
        deviceElement.setLabel(ownDevice.label);
        deviceList.append(deviceElement);
    }

    // Add all remaining own devices to the device list.
    const auto ownDevices = otherOwnDevices();
    for (auto itr = ownDevices.cbegin(); itr != ownDevices.cend(); ++itr) {
        const auto &deviceId = itr.key();
        const auto &device = itr.value();

        QXmppOmemoDeviceElement deviceElement;
        deviceElement.setId(deviceId);
        deviceElement.setLabel(device.label);
        deviceList.append(deviceElement);
    }

    QXmppOmemoDeviceListItem item;
    item.setId(QXmppPubSubManager::standardItemIdToString(QXmppPubSubManager::Current));
    item.setDeviceList(deviceList);

    return item;
}

//
// Updates the own locally stored devices by requesting the current device list from the own
// PEP service.
//
// \param isDeviceListNodeExistent whether the node for the device list exists
// \param continuation function to be called with the bool value whether it succeeded
//
template<typename Function>
void ManagerPrivate::updateOwnDevicesLocally(bool isDeviceListNodeExistent, Function continuation)
{
    if (isDeviceListNodeExistent && otherOwnDevices().isEmpty()) {
        auto future = pubSubManager->requestOwnPepItem<QXmppOmemoDeviceListItem>(ns_omemo_2_devices, QXmppPubSubManager::Current);
        future.then(q, [=](QXmppPubSubManager::ItemResult<QXmppOmemoDeviceListItem> result) mutable {
            if (const auto error = std::get_if<QXmppError>(&result)) {
                warning("Device list for JID '" % ownBareJid() %
                        "' could not be retrieved and thus not updated: " %
                        errorToString(*error));
                continuation(false);
            } else {
                const auto &deviceListItem = std::get<QXmppOmemoDeviceListItem>(result);
                QList<QXmppOmemoDeviceElement> deviceList = deviceListItem.deviceList();

                if (auto devicesCount = deviceList.size()) {
                    // Do not exceed the maximum of manageable devices.
                    if (devicesCount > maximumDevicesPerJid) {
                        warning(u"Received own OMEMO device list could not be stored locally "
                                "completely because the devices are more than the maximum of "
                                "manageable devices " %
                                QString::number(maximumDevicesPerJid) %
                                u" - Use 'QXmppOmemoManager::setMaximumDevicesPerJid()' to "
                                "increase the maximum");
                        deviceList = deviceList.mid(0, maximumDevicesPerJid);
                        devicesCount = maximumDevicesPerJid;
                    }

                    auto processedDevicesCount = std::make_shared<int>(0);

                    // Store all device elements retrieved from the device list locally as
                    // devices.
                    // The own device (i.e., a device element in the device list with the same
                    // ID as of this device) is skipped.
                    for (const auto &deviceElement : std::as_const(deviceList)) {
                        if (const auto deviceId = deviceElement.id(); deviceId != ownDevice.id) {
                            const auto jid = ownBareJid();
                            auto &device = devices[jid][deviceId];
                            device.label = deviceElement.label();

                            auto future = omemoStorage->addDevice(jid, deviceId, device);
                            future.then(q, [=, &device]() mutable {
                                auto future = buildSessionForNewDevice(jid, deviceId, device);
                                future.then(q, [=](auto) mutable {
                                    Q_EMIT q->deviceAdded(jid, deviceId);

                                    if (++(*processedDevicesCount) == devicesCount) {
                                        continuation(true);
                                    }
                                });
                            });
                        }
                    }
                } else {
                    continuation(true);
                }
            }
        });
    } else {
        continuation(true);
    }
}

//
// Updates all locally stored devices of a contact.
//
// \param deviceOwnerJid bare JID of the devices' owner
// \param deviceListItems PEP items that may contain a device list
//
// \returns a found device list item
//
std::optional<QXmppOmemoDeviceListItem> QXmppOmemoManagerPrivate::updateContactDevices(const QString &deviceOwnerJid, const QVector<QXmppOmemoDeviceListItem> &deviceListItems)
{
    if (deviceListItems.size() > 1) {
        const auto itr = std::find_if(deviceListItems.cbegin(), deviceListItems.cend(), [=](const QXmppOmemoDeviceListItem &item) {
            return item.id() == QXmppPubSubManager::standardItemIdToString(QXmppPubSubManager::Current);
        });

        if (itr != deviceListItems.cend()) {
            updateDevices(deviceOwnerJid, *itr);
            return *itr;
        } else {
            warning("Device list for JID '" % deviceOwnerJid % "' could not be updated because the node contains more than one item but none with the singleton node's specific ID '" % QXmppPubSubManager::standardItemIdToString(QXmppPubSubManager::Current) % "'");
            handleIrregularDeviceListChanges(deviceOwnerJid);
            return {};
        }
    }

    const auto &item = deviceListItems.constFirst();
    updateDevices(deviceOwnerJid, item);
    return item;
}

//
// Updates all locally stored devices by a passed device list item.
//
// \param deviceOwnerJid bare JID of the devices' owner
// \param deviceListItem PEP item containing the device list
//
void ManagerPrivate::updateDevices(const QString &deviceOwnerJid, const QXmppOmemoDeviceListItem &deviceListItem)
{
    const auto isOwnDeviceListNode = ownBareJid() == deviceOwnerJid;
    QList<QXmppOmemoDeviceElement> deviceList = deviceListItem.deviceList();
    auto isOwnDeviceListIncorrect = false;

    // Do not exceed the maximum of manageable devices.
    if (deviceList.size() > maximumDevicesPerJid) {
        warning(u"Received OMEMO device list of JID '" % deviceOwnerJid %
                "' could not be stored locally completely because the devices are more than the "
                "maximum of manageable devices " %
                QString::number(maximumDevicesPerJid) %
                u" - Use 'QXmppOmemoManager::setMaximumDevicesPerJid()' to increase the maximum");
        deviceList = deviceList.mid(0, maximumDevicesPerJid);
    }

    if (isOwnDeviceListNode) {
        QList<uint32_t> deviceIds;

        // Search for inconsistencies in the device list to keep it
        // correct.
        // The following problems are corrected:
        //   * Multiple device elements have the same IDs.
        //   * There is no device element for this device.
        //   * There are device elements with the same ID as this device
        //     but different labels.
        for (auto itr = deviceList.begin(); itr != deviceList.end();) {
            const auto deviceElementId = itr->id();

            if (deviceIds.contains(deviceElementId)) {
                isOwnDeviceListIncorrect = true;
                itr = deviceList.erase(itr);
            } else {
                deviceIds.append(deviceElementId);

                if (itr->id() == ownDevice.id) {
                    if (itr->label() != ownDevice.label) {
                        isOwnDeviceListIncorrect = true;
                    }

                    itr = deviceList.erase(itr);
                } else {
                    ++itr;
                }
            }
        }
    }

    // Set a timestamp for locally stored devices that are removed later if
    // they are not included in the device list (i.e., they were removed
    // by their owner).
    auto &ownerDevices = devices[deviceOwnerJid];
    for (auto itr = ownerDevices.begin(); itr != ownerDevices.end(); ++itr) {
        const auto &deviceId = itr.key();
        auto &device = itr.value();
        auto isDeviceFound = false;

        for (const auto &deviceElement : std::as_const(deviceList)) {
            if (deviceId == deviceElement.id()) {
                isDeviceFound = true;
                break;
            }
        }

        if (!isDeviceFound) {
            device.removalFromDeviceListDate = QDateTime::currentDateTimeUtc();
            omemoStorage->addDevice(deviceOwnerJid, deviceId, device);
        }
    }

    // Update locally stored devices if they are modified in the device
    // list or store devices locally if they are new in the device list.
    for (const auto &deviceElement : std::as_const(deviceList)) {
        auto isDeviceFound = false;

        for (auto itr = ownerDevices.begin(); itr != ownerDevices.end(); ++itr) {
            const auto &deviceId = itr.key();
            auto &device = itr.value();

            if (deviceId == deviceElement.id()) {
                auto isDeviceModified = false;
                auto isDeviceLabelModified = false;

                // Reset the date of removal from server, if it has been
                // removed before.
                if (!device.removalFromDeviceListDate.isNull()) {
                    device.removalFromDeviceListDate = {};
                    isDeviceModified = true;
                }

                // Update the stored label if it differs from the new
                // one.
                if (device.label != deviceElement.label()) {
                    device.label = deviceElement.label();
                    isDeviceModified = true;
                    isDeviceLabelModified = true;
                }

                // Store the modifications.
                if (isDeviceModified) {
                    omemoStorage->addDevice(deviceOwnerJid, deviceId, device);

                    if (isDeviceLabelModified) {
                        Q_EMIT q->deviceChanged(deviceOwnerJid, deviceId);
                    }
                }

                isDeviceFound = true;
                break;
            }
        }

        // Create a new entry and store it if there is no such entry
        // yet.
        if (!isDeviceFound) {
            const auto deviceId = deviceElement.id();
            auto &device = ownerDevices[deviceId];
            device.label = deviceElement.label();
            omemoStorage->addDevice(deviceOwnerJid, deviceId, device);

            auto future = buildSessionForNewDevice(deviceOwnerJid, deviceId, device);
            future.then(q, [=](auto) {
                Q_EMIT q->deviceAdded(deviceOwnerJid, deviceId);
            });
        }
    }

    // Publish an own correct device list if the PEP service's one is incorrect
    // and the devices are already set up locally.
    if (isOwnDeviceListIncorrect) {
        if (!devices.isEmpty()) {
            publishDeviceListItem(true, [=](bool isPublished) {
                if (!isPublished) {
                    warning("Own device list item could not be published in order to correct the PEP service's one");
                }
            });
        }
    }
}

//
// Corrects the own device list on the PEP service by the locally stored
// devices or sets a contact device to be removed locally in the future.
//
// \param deviceOwnerJid bare JID of the devices' owner
//
void ManagerPrivate::handleIrregularDeviceListChanges(const QString &deviceOwnerJid)
{
    const auto isOwnDeviceListNode = ownBareJid() == deviceOwnerJid;

    if (isOwnDeviceListNode) {
        // Publish a new device list for the own devices if their device list
        // item is removed, if their device list node is removed or if all
        // the node's items are removed.
        auto future = pubSubManager->deleteOwnPepNode(ns_omemo_2_devices);
        future.then(q, [=](QXmppPubSubManager::Result result) {
            if (const auto error = std::get_if<QXmppError>(&result)) {
                warning("Node '" % QString(ns_omemo_2_devices) % "' of JID '" % deviceOwnerJid %
                        "' could not be deleted in order to recover from an inconsistent node: " %
                        errorToString(*error));
            } else {
                auto future = pubSubManager->requestOwnPepFeatures();
                future.then(q, [=](QXmppPubSubManager::FeaturesResult result) {
                    if (const auto error = std::get_if<QXmppError>(&result)) {
                        warning("Features of PEP service '" % deviceOwnerJid %
                                "' could not be retrieved: " % errorToString(*error));
                        warning("Device list could not be published");
                    } else {
                        const auto &pepServiceFeatures = std::get<QVector<QString>>(result);

                        const auto arePublishOptionsSupported = pepServiceFeatures.contains(ns_pubsub_publish_options);
                        const auto isAutomaticCreationSupported = pepServiceFeatures.contains(ns_pubsub_auto_create);
                        const auto isCreationAndConfigurationSupported = pepServiceFeatures.contains(ns_pubsub_create_and_configure);
                        const auto isCreationSupported = pepServiceFeatures.contains(ns_pubsub_create_nodes);
                        const auto isConfigurationSupported = pepServiceFeatures.contains(ns_pubsub_config_node);

                        publishDeviceElement(false,
                                             arePublishOptionsSupported,
                                             isAutomaticCreationSupported,
                                             isCreationAndConfigurationSupported,
                                             isCreationSupported,
                                             isConfigurationSupported,
                                             [=](bool isPublished) {
                                                 if (!isPublished) {
                                                     warning("Device element could not be published");
                                                 }
                                             });
                    }
                });
            }
        });
    } else {
        auto &ownerDevices = devices[deviceOwnerJid];

        // Set a timestamp for locally stored contact devices being removed
        // later if their device list item is removed, if their device list node
        // is removed or if all the node's items are removed.
        for (auto itr = ownerDevices.begin(); itr != ownerDevices.end(); ++itr) {
            const auto &deviceId = itr.key();
            auto &device = itr.value();

            device.removalFromDeviceListDate = QDateTime::currentDateTimeUtc();

            // Store the modification.
            omemoStorage->addDevice(deviceOwnerJid, deviceId, device);
        }
    }
}

//
// Removes the device element for this device or deletes the whole PEP node if
// it would be empty after the retraction.
//
// \param continuation function to be called with the bool value whether it succeeded
//
template<typename Function>
void ManagerPrivate::deleteDeviceElement(Function continuation)
{
    if (otherOwnDevices().isEmpty()) {
        deleteNode(ns_omemo_2_devices, std::move(continuation));
    } else {
        publishDeviceListItem(false, std::move(continuation));
    }
}

//
// Creates a PEP node.
//
// \param node node to be created
// \param continuation function to be called with the bool value whether it succeeded
//
template<typename Function>
void ManagerPrivate::createNode(const QString &node, Function continuation)
{
    runPubSubQueryWithContinuation(pubSubManager->createOwnPepNode(node),
                                   "Node '" % node % "' of JID '" % ownBareJid() % "' could not be created",
                                   std::move(continuation));
}

//
// Creates a PEP node with a configuration.
//
// \param node node to be created
// \param config configuration to be applied
// \param continuation function to be called with the bool value whether it succeeded
//
template<typename Function>
void ManagerPrivate::createNode(const QString &node, const QXmppPubSubNodeConfig &config, Function continuation)
{
    runPubSubQueryWithContinuation(pubSubManager->createOwnPepNode(node, config),
                                   "Node '" % node % "' of JID '" % ownBareJid() % "' could not be created",
                                   std::move(continuation));
}

//
// Configures an existing PEP node.
//
// \param node node to be configured
// \param config configuration to be applied
// \param continuation function to be called with the bool value whether it succeeded
//
template<typename Function>
void ManagerPrivate::configureNode(const QString &node, const QXmppPubSubNodeConfig &config, Function continuation)
{
    runPubSubQueryWithContinuation(pubSubManager->configureOwnPepNode(node, config),
                                   "Node '" % node % "' of JID '" % ownBareJid() % "' could not be configured",
                                   std::move(continuation));
}

//
// Retracts an item from a PEP node.
//
// \param node node containing the item
// \param itemId ID of the item
// \param continuation function to be called with the bool value whether it succeeded
//
template<typename Function>
void ManagerPrivate::retractItem(const QString &node, uint32_t itemId, Function continuation)
{
    const auto itemIdString = QString::number(itemId);
    runPubSubQueryWithContinuation(pubSubManager->retractOwnPepItem(node, itemIdString),
                                   "Item '" % itemIdString % "' of node '" % node % "' and JID '" % ownBareJid() % "' could not be retracted",
                                   std::move(continuation));
}

//
// Deletes a PEP node.
//
// \param node node to be deleted
// \param continuation function to be called with the bool value whether it succeeded
//
template<typename Function>
void ManagerPrivate::deleteNode(const QString &node, Function continuation)
{
    auto future = pubSubManager->deleteOwnPepNode(node);
    future.then(q, [=, continuation = std::move(continuation)](QXmppPubSubManager::Result result) mutable {
        if (auto error = std::get_if<QXmppError>(&result)) {
            if (auto err = error->value<QXmppStanza::Error>()) {
                // Skip the error handling if the node is already deleted.
                if (!(err->type() == Error::Cancel && err->condition() == Error::ItemNotFound)) {
                    warning("Node '" % node % "' of JID '" % ownBareJid() % "' could not be deleted: " %
                            errorToString(*error));
                    continuation(false);
                } else {
                    continuation(true);
                }
            } else {
                continuation(false);
            }
        } else {
            continuation(true);
        }
    });
}

//
// Publishes a PEP item.
//
// \param node node containing the item
// \param item item to be published
// \param continuation function to be called with the bool value whether it succeeded
//
template<typename T, typename Function>
void ManagerPrivate::publishItem(const QString &node, const T &item, Function continuation)
{
    runPubSubQueryWithContinuation(pubSubManager->publishOwnPepItem(node, item),
                                   "Item with ID '" % item.id() %
                                       "' could not be published to node '" % node % "' of JID '" %
                                       ownBareJid() % "'",
                                   std::move(continuation));
}

//
// Publishes a PEP item with publish options.
//
// \param node node containing the item
// \param item item to be published
// \param publishOptions publish options to be applied
// \param continuation function to be called with the bool value whether it succeeded
//
template<typename T, typename Function>
void ManagerPrivate::publishItem(const QString &node, const T &item, const QXmppPubSubPublishOptions &publishOptions, Function continuation)
{
    runPubSubQueryWithContinuation(pubSubManager->publishOwnPepItem(node, item, publishOptions),
                                   "Item with ID '" % item.id() % "' could not be published to node '" % node % "' of JID '" % ownBareJid() % "'",
                                   std::move(continuation));
}

//
// Runs a PubSub query and processes a continuation function.
//
// \param future PubSub query to be run
// \param errorMessage message to be logged in case of an error
// \param continuation function to be called after the PubSub query
//
template<typename T, typename Function>
void QXmppOmemoManagerPrivate::runPubSubQueryWithContinuation(QXmppTask<T> future, const QString &errorMessage, Function continuation)
{
    future.then(q, [this, errorMessage, continuation = std::move(continuation)](auto result) mutable {
        if (auto error = std::get_if<QXmppError>(&result)) {
            warning(errorMessage % u": " % errorToString(*error));
            continuation(false);
        } else {
            continuation(true);
        }
    });
}

// See QXmppOmemoManager for documentation
QXmppTask<bool> ManagerPrivate::changeDeviceLabel(const QString &deviceLabel)
{
    QXmppPromise<bool> interface;

    ownDevice.label = deviceLabel;

    if (isStarted) {
        auto future = omemoStorage->setOwnDevice(ownDevice);
        future.then(q, [=]() mutable {
            publishDeviceListItem(true, [=](bool isPublished) mutable {
                interface.finish(std::move(isPublished));
            });
        });
    } else {
        interface.finish(true);
    }

    return interface.task();
}

//
// Requests the device list of a contact manually and stores it locally.
//
// This should be called for offline contacts whose servers do not distribute
// the last published PubSub item if that contact is offline (e.g., with at
// least ejabberd version <= 21.12)
//
// \param jid JID of the contact whose device list is being requested
//
// \return the result of the request
//
QXmppTask<QXmppPubSubManager::ItemResult<QXmppOmemoDeviceListItem>> ManagerPrivate::requestDeviceList(const QString &jid)
{
    QXmppPromise<QXmppPubSubManager::ItemResult<QXmppOmemoDeviceListItem>> interface;

    // Since the usage of the item ID \c QXmppPubSubManager::Current is only RECOMMENDED by
    // \xep{0060, Publish-Subscribe} (PubSub) but not obligatory, all items are requested even if
    // the node should contain only one item.
    auto future = pubSubManager->requestItems<QXmppOmemoDeviceListItem>(jid, ns_omemo_2_devices);
    future.then(q, [this, interface, jid](QXmppPubSubManager::ItemsResult<QXmppOmemoDeviceListItem> result) mutable {
        if (const auto error = std::get_if<QXmppError>(&result)) {
            warning("Device list for JID '" % jid % "' could not be retrieved: " % errorToString(*error));
            interface.finish(*error);
        } else if (const auto &items = std::get<QXmppPubSubManager::Items<QXmppOmemoDeviceListItem>>(result).items; items.isEmpty()) {
            const auto errorMessage = "Device list for JID '" % jid % "' could not be retrieved because the node does not contain any item";
            warning(errorMessage);
            interface.finish(QXmppError { errorMessage });
        } else if (const auto item = updateContactDevices(jid, items); item) {
            interface.finish(*item);
        } else {
            interface.finish(QXmppError { "Device list for JID '" % jid % "' could not be retrieved because the node does not contain an appropriate item" });
        }
    });
    return interface.task();
}

//
// Subscribes to the device list of a contact if the contact's device is not stored yet.
//
// \param jid JID of the contact whose device list is being subscribed
// \param deviceId ID of the device that is checked
//
void ManagerPrivate::subscribeToNewDeviceLists(const QString &jid, uint32_t deviceId)
{
    if (!devices.value(jid).contains(deviceId)) {
        subscribeToDeviceList(jid);
    }
}

//
// Subscribes the current user's resource to a device list manually.
//
// A server may not send the last published item automatically.
// To ensure that the subscribed device list can be stored locally in any case,
// the current PubSub item containing the device list is requested manually.
//
// \param jid JID of the contact whose device list is being subscribed
//
// \return the result of the subscription and manual request
//
QXmppTask<QXmppPubSubManager::Result> ManagerPrivate::subscribeToDeviceList(const QString &jid)
{
    QXmppPromise<QXmppPubSubManager::Result> interface;

    auto future = pubSubManager->subscribeToNode(jid, ns_omemo_2_devices, ownFullJid());
    future.then(q, [=](QXmppPubSubManager::Result result) mutable {
        if (const auto error = std::get_if<QXmppError>(&result)) {
            warning("Device list for JID '" % jid % "' could not be subscribed: " % errorToString(*error));
            interface.finish(std::move(*error));
        } else {
            jidsOfManuallySubscribedDevices.append(jid);

            auto future = requestDeviceList(jid);
            future.then(q, [=](auto result) mutable {
                interface.finish(mapToSuccess(std::move(result)));
            });
        }
    });

    return interface.task();
}

//
// Unsubscribes the current user's resource from device lists that were
// manually subscribed by
// \c QXmppOmemoManagerPrivate::subscribeToDeviceList().
//
// \param jids JIDs of the contacts whose device lists are being
//             unsubscribed
//
// \return the results of each unsubscribe request
//
QXmppTask<QVector<Manager::DevicesResult>> ManagerPrivate::unsubscribeFromDeviceLists(const QList<QString> &jids)
{
    if (jids.isEmpty()) {
        return makeReadyTask(QVector<Manager::DevicesResult>());
    }

    struct State
    {
        int processed = 0;
        int jidsCount = 0;
        QXmppPromise<QVector<Manager::DevicesResult>> interface;
        QVector<Manager::DevicesResult> devicesResults;
    };

    auto state = std::make_shared<State>();
    state->jidsCount = jids.count();

    for (const auto &jid : jids) {
        unsubscribeFromDeviceList(jid).then(q, [jid, state](QXmppPubSubManager::Result result) mutable {
            state->devicesResults << Manager::DevicesResult {
                jid, result
            };

            if (++(state->processed) == state->jidsCount) {
                state->interface.finish(std::move(state->devicesResults));
            }
        });
    }

    return state->interface.task();
}

//
// Unsubscribes the current user's resource from a device list that were
// manually subscribed by
// \c QXmppOmemoManagerPrivate::subscribeToDeviceList().
//
// \param jid JID of the contact whose device list is being unsubscribed
//
// \return the result of the unsubscription
//
QXmppTask<QXmppPubSubManager::Result> ManagerPrivate::unsubscribeFromDeviceList(const QString &jid)
{
    QXmppPromise<QXmppPubSubManager::Result> interface;

    auto future = pubSubManager->unsubscribeFromNode(jid, ns_omemo_2_devices, ownFullJid());
    future.then(q, [=](QXmppPubSubManager::Result result) mutable {
        if (const auto error = std::get_if<QXmppError>(&result)) {
            warning("Device list for JID '" % jid % "' could not be unsubscribed: " % errorToString(*error));
        } else {
            jidsOfManuallySubscribedDevices.removeAll(jid);
        }

        interface.finish(std::move(result));
    });

    return interface.task();
}

// See QXmppOmemoManager for documentation
QXmppTask<bool> ManagerPrivate::resetOwnDevice()
{
    QXmppPromise<bool> interface;

    isStarted = false;

    auto future = trustManager->resetAll(ns_omemo_2);
    future.then(q, [=]() mutable {
        auto future = omemoStorage->resetAll();
        future.then(q, [=]() mutable {
            deleteDeviceElement([=](bool isDeviceElementDeleted) mutable {
                if (isDeviceElementDeleted) {
                    deleteDeviceBundle([=](bool isDeviceBundleDeleted) mutable {
                        if (isDeviceBundleDeleted) {
                            ownDevice = {};
                            preKeyPairs.clear();
                            signedPreKeyPairs.clear();
                            deviceBundle = {};
                            devices.clear();

                            Q_EMIT q->allDevicesRemoved();
                        }

                        interface.finish(std::move(isDeviceBundleDeleted));
                    });
                } else {
                    interface.finish(false);
                }
            });
        });
    });

    return interface.task();
}

// See QXmppOmemoManager for documentation
QXmppTask<bool> ManagerPrivate::resetAll()
{
    QXmppPromise<bool> interface;

    isStarted = false;

    auto future = trustManager->resetAll(ns_omemo_2);
    future.then(q, [this, interface]() mutable {
        auto future = omemoStorage->resetAll();
        future.then(q, [this, interface]() mutable {
            deleteNode(ns_omemo_2_devices, [this, interface](bool isDevicesNodeDeleted) mutable {
                if (isDevicesNodeDeleted) {
                    deleteNode(ns_omemo_2_bundles, [this, interface](bool isBundlesNodeDeleted) mutable {
                        if (isBundlesNodeDeleted) {
                            ownDevice = {};
                            preKeyPairs.clear();
                            signedPreKeyPairs.clear();
                            deviceBundle = {};
                            devices.clear();

                            Q_EMIT q->allDevicesRemoved();
                        }

                        interface.finish(std::move(isBundlesNodeDeleted));
                    });
                } else {
                    interface.finish(false);
                }
            });
        });
    });

    return interface.task();
}

//
// Builds a new session for a new received device if that is enabled.
//
// \see QXmppOmemoManager::setNewDeviceAutoSessionBuildingEnabled()
//
// \param jid JID of the device's owner
// \param deviceId ID of the device
// \param device locally stored device which will be modified
//
// \return true if a session could be built or it is not enabled, otherwise
//         false
//
QXmppTask<bool> ManagerPrivate::buildSessionForNewDevice(const QString &jid, uint32_t deviceId, QXmppOmemoStorage::Device &device)
{
    if (isNewDeviceAutoSessionBuildingEnabled) {
        return buildSessionWithDeviceBundle(jid, deviceId, device);
    } else {
        return makeReadyTask(true);
    }
}

//
// Requests a device bundle and builds a new session with it.
//
// \param jid JID of the device's owner
// \param deviceId ID of the device
// \param device locally stored device which will be modified
//
// \return whether a session could be built
//
QXmppTask<bool> ManagerPrivate::buildSessionWithDeviceBundle(const QString &jid, uint32_t deviceId, QXmppOmemoStorage::Device &device)
{
    QXmppPromise<bool> interface;

    auto future = requestDeviceBundle(jid, deviceId);
    future.then(q, [=, &device](std::optional<QXmppOmemoDeviceBundle> optionalDeviceBundle) mutable {
        if (optionalDeviceBundle) {
            const auto &deviceBundle = *optionalDeviceBundle;
            device.keyId = deviceBundle.publicIdentityKey();

            auto future = q->trustLevel(jid, device.keyId);
            future.then(q, [=](TrustLevel trustLevel) mutable {
                auto buildSessionDependingOnTrustLevel = [=](TrustLevel trustLevel) mutable {
                    // Build a session if the device's key has a specific trust
                    // level and send an empty OMEMO (key exchange) message to
                    // make the receiving device build a new session too.
                    if (!acceptedSessionBuildingTrustLevels.testFlag(trustLevel)) {
                        warning("Session could not be created for JID '" % jid % "' with device ID '" %
                                QString::number(deviceId) % "' because its key's trust level '" %
                                QString::number(int(trustLevel)) % "' is not accepted");
                        interface.finish(false);
                    } else if (const auto address = Address(jid, deviceId); !buildSession(address.data(), deviceBundle)) {
                        warning("Session could not be created for JID '" % jid % "' and device ID '" %
                                QString::number(deviceId) % "'");
                        interface.finish(false);
                    } else {
                        auto future = sendEmptyMessage(jid, deviceId, true);
                        future.then(q, [=](QXmpp::SendResult result) mutable {
                            if (std::holds_alternative<QXmppError>(result)) {
                                warning("Session could be created but empty message could not be sent to JID '" %
                                        jid % "' and device ID '" % QString::number(deviceId) % "'");
                                interface.finish(false);
                            } else {
                                interface.finish(true);
                            }
                        });
                    }
                };

                if (trustLevel == TrustLevel::Undecided) {
                    // Store the key's trust level if it is not stored yet.
                    auto future = storeKeyDependingOnSecurityPolicy(jid, device.keyId);
                    future.then(q, [=](TrustLevel trustLevel) mutable {
                        buildSessionDependingOnTrustLevel(trustLevel);
                    });
                } else {
                    buildSessionDependingOnTrustLevel(trustLevel);
                }
            });
        } else {
            warning("Session could not be created because no device bundle could be fetched for "
                    "JID '" %
                    jid % "' and device ID '" % QString::number(deviceId) % "'");
            interface.finish(false);
        }
    });

    return interface.task();
}

//
// Builds an OMEMO session.
//
// A session is used for encryption and decryption.
//
// \param address address of the device for whom the session is built
// \param deviceBundle device bundle containing all data to build the session
//
// \return whether it succeeded
//
bool ManagerPrivate::buildSession(signal_protocol_address address, const QXmppOmemoDeviceBundle &deviceBundle)
{
    QXmppPromise<bool> interface;

    // Choose a pre key randomly.
    const auto publicPreKeys = deviceBundle.publicPreKeys();
    if (publicPreKeys.isEmpty()) {
        warning("No public pre key could be found in device bundle");
    }
    const auto publicPreKeyIds = publicPreKeys.keys();
    const auto publicPreKeyIndex = QRandomGenerator::system()->bounded(publicPreKeyIds.size());
    const auto publicPreKeyId = publicPreKeyIds.at(publicPreKeyIndex);
    const auto publicPreKey = publicPreKeys.value(publicPreKeyId);

    SessionBuilderPtr sessionBuilder;
    if (session_builder_create(sessionBuilder.ptrRef(), storeContext.get(), &address, globalContext.get()) < 0) {
        warning("Session builder could not be created");
        return false;
    }
    session_builder_set_version(sessionBuilder.get(), CIPHERTEXT_OMEMO_VERSION);

    RefCountedPtr<session_pre_key_bundle> sessionBundle;

    if (!createSessionBundle(sessionBundle.ptrRef(),
                             deviceBundle.publicIdentityKey(),
                             deviceBundle.signedPublicPreKey(),
                             deviceBundle.signedPublicPreKeyId(),
                             deviceBundle.signedPublicPreKeySignature(),
                             publicPreKey,
                             publicPreKeyId)) {
        warning("Session bundle could not be created");
        return false;
    }

    if (session_builder_process_pre_key_bundle(sessionBuilder.get(), sessionBundle.get()) != SG_SUCCESS) {
        warning("Session bundle could not be processed");
        return false;
    }

    return true;
}

//
// Creates a session bundle.
//
// \param sessionBundle session bundle location
// \param serializedPublicIdentityKey serialized public identity key
// \param serializedSignedPublicPreKey serialized signed public pre key
// \param signedPublicPreKeyId ID of the signed public pre key
// \param serializedSignedPublicPreKeySignature serialized signature of the
//        signed public pre key
// \param serializedPublicPreKey serialized public pre key
// \param publicPreKeyId ID of the public pre key
//
// \return whether it succeeded
//
bool ManagerPrivate::createSessionBundle(session_pre_key_bundle **sessionBundle,
                                         const QByteArray &serializedPublicIdentityKey,
                                         const QByteArray &serializedSignedPublicPreKey,
                                         uint32_t signedPublicPreKeyId,
                                         const QByteArray &serializedSignedPublicPreKeySignature,
                                         const QByteArray &serializedPublicPreKey,
                                         uint32_t publicPreKeyId)
{
    RefCountedPtr<ec_public_key> publicIdentityKey;
    RefCountedPtr<ec_public_key> signedPublicPreKey;
    RefCountedPtr<const uint8_t> signedPublicPreKeySignature;
    int signedPublicPreKeySignatureSize;
    RefCountedPtr<ec_public_key> publicPreKey;

    if (deserializePublicIdentityKey(publicIdentityKey.ptrRef(), serializedPublicIdentityKey) &&
        deserializeSignedPublicPreKey(signedPublicPreKey.ptrRef(), serializedSignedPublicPreKey) &&
        (signedPublicPreKeySignatureSize = deserializeSignedPublicPreKeySignature(signedPublicPreKeySignature.ptrRef(), serializedSignedPublicPreKeySignature)) &&
        deserializePublicPreKey(publicPreKey.ptrRef(), serializedPublicPreKey)) {

        // "0" is passed as "device_id" to the OMEMO library because it is not
        // used by OMEMO.
        // Only the device ID is of interest which is used as "registration_id"
        // within the OMEMO library.
        if (session_pre_key_bundle_create(sessionBundle,
                                          ownDevice.id,
                                          0,
                                          publicPreKeyId,
                                          publicPreKey.get(),
                                          signedPublicPreKeyId,
                                          signedPublicPreKey.get(),
                                          signedPublicPreKeySignature.get(),
                                          signedPublicPreKeySignatureSize,
                                          publicIdentityKey.get()) < 0) {
            return false;
        }

        return true;
    } else {
        warning("Session bundle data could not be deserialized");
        return false;
    }
}

//
// Deserializes the locally stored identity key pair.
//
// The identity key pair is the pair of private and a public long-term keys.
//
// \param identityKeyPair identity key pair location
//
// \return whether it succeeded
//
bool ManagerPrivate::deserializeIdentityKeyPair(ratchet_identity_key_pair **identityKeyPair) const
{
    RefCountedPtr<ec_private_key> privateIdentityKey;
    deserializePrivateIdentityKey(privateIdentityKey.ptrRef(), ownDevice.privateIdentityKey);

    RefCountedPtr<ec_public_key> publicIdentityKey;
    deserializePublicIdentityKey(publicIdentityKey.ptrRef(), ownDevice.publicIdentityKey);

    if (ratchet_identity_key_pair_create(identityKeyPair, publicIdentityKey.get(), privateIdentityKey.get()) < 0) {
        warning("Identity key pair could not be deserialized");
        return false;
    }

    return true;
}

//
// Deserializes a private identity key.
//
// \param privateIdentityKey private identity key location
// \param serializedPrivateIdentityKey serialized private identity key
//
// \return whether it succeeded
//
bool ManagerPrivate::deserializePrivateIdentityKey(ec_private_key **privateIdentityKey, const QByteArray &serializedPrivateIdentityKey) const
{
    BufferSecurePtr privateIdentityKeyBuffer = BufferSecurePtr::fromByteArray(serializedPrivateIdentityKey);

    if (!privateIdentityKeyBuffer) {
        warning("Buffer for serialized private identity key could not be created");
        return false;
    }

    if (curve_decode_private_point(privateIdentityKey, signal_buffer_data(privateIdentityKeyBuffer.get()), signal_buffer_len(privateIdentityKeyBuffer.get()), globalContext.get()) < 0) {
        warning("Private identity key could not be deserialized");
        return false;
    }

    return true;
}

//
// Deserializes a public identity key.
//
// \param publicIdentityKey public identity key location
// \param serializedPublicIdentityKey serialized public identity key
//
// \return whether it succeeded
//
bool ManagerPrivate::deserializePublicIdentityKey(ec_public_key **publicIdentityKey, const QByteArray &serializedPublicIdentityKey) const
{
    BufferPtr publicIdentityKeyBuffer = BufferPtr::fromByteArray(serializedPublicIdentityKey);

    if (!publicIdentityKeyBuffer) {
        warning("Buffer for serialized public identity key could not be created");
        return false;
    }

    if (curve_decode_point_ed(publicIdentityKey, signal_buffer_data(publicIdentityKeyBuffer.get()), signal_buffer_len(publicIdentityKeyBuffer.get()), globalContext.get()) < 0) {
        warning("Public identity key could not be deserialized");
        return false;
    }

    return true;
}

//
// Deserializes a signed public pre key.
//
// \param signedPublicPreKey signed public pre key location
// \param serializedSignedPublicPreKey serialized signed public pre key
//
// \return whether it succeeded
//
bool ManagerPrivate::deserializeSignedPublicPreKey(ec_public_key **signedPublicPreKey, const QByteArray &serializedSignedPublicPreKey) const
{
    BufferPtr signedPublicPreKeyBuffer = BufferPtr::fromByteArray(serializedSignedPublicPreKey);

    if (!signedPublicPreKeyBuffer) {
        warning("Buffer for serialized signed public pre key could not be created");
        return false;
    }

    if (curve_decode_point_mont(signedPublicPreKey, signal_buffer_data(signedPublicPreKeyBuffer.get()), signal_buffer_len(signedPublicPreKeyBuffer.get()), globalContext.get()) < 0) {
        warning("Signed public pre key could not be deserialized");
        return false;
    }

    return true;
}

//
// Deserializes a public pre key.
//
// \param publicPreKey public pre key location
// \param serializedPublicPreKey serialized public pre key
//
// \return whether it succeeded
//
bool ManagerPrivate::deserializePublicPreKey(ec_public_key **publicPreKey, const QByteArray &serializedPublicPreKey) const
{
    auto publicPreKeyBuffer = BufferPtr::fromByteArray(serializedPublicPreKey);

    if (!publicPreKeyBuffer) {
        warning("Buffer for serialized public pre key could not be created");
        return false;
    }

    if (curve_decode_point_mont(publicPreKey, signal_buffer_data(publicPreKeyBuffer.get()), signal_buffer_len(publicPreKeyBuffer.get()), globalContext.get()) < 0) {
        warning("Public pre key could not be deserialized");
        return false;
    }

    return true;
}

//
// Sends an empty OMEMO message.
//
// An empty OMEMO message is a message without an OMEMO payload.
// It is used to trigger the completion, rebuilding or refreshing of OMEMO
// sessions.
//
// \param recipientJid JID of the message's recipient
// \param recipientDeviceId ID of the recipient's device
// \param isKeyExchange whether the message is used to build a new session
//
// \return the result of the sending
//
QXmppTask<QXmpp::SendResult> ManagerPrivate::sendEmptyMessage(const QString &recipientJid, uint32_t recipientDeviceId, bool isKeyExchange) const
{
    QXmppPromise<QXmpp::SendResult> interface;

    const auto address = Address(recipientJid, recipientDeviceId);
    const auto decryptionData = QCA::SecureArray(EMPTY_MESSAGE_DECRYPTION_DATA_SIZE);

    if (const auto data = createOmemoEnvelopeData(address.data(), decryptionData); data.isEmpty()) {
        warning("OMEMO envelope for recipient JID '" % recipientJid % "' and device ID '" %
                QString::number(recipientDeviceId) %
                "' could not be created because its data could not be encrypted");
        QXmppError error {
            QStringLiteral("OMEMO envelope could not be created"),
            SendError::EncryptionError
        };
        interface.finish(error);
    } else {
        QXmppOmemoEnvelope omemoEnvelope;
        omemoEnvelope.setRecipientDeviceId(recipientDeviceId);
        if (isKeyExchange) {
            omemoEnvelope.setIsUsedForKeyExchange(true);
        }
        omemoEnvelope.setData(data);

        QXmppOmemoElement omemoElement;
        omemoElement.addEnvelope(recipientJid, omemoEnvelope);
        omemoElement.setSenderDeviceId(ownDevice.id);

        QXmppMessage message;
        message.setTo(recipientJid);
        message.addHint(QXmppMessage::Store);
        message.setOmemoElement(omemoElement);

        auto future = q->client()->send(std::move(message));
        future.then(q, [=](QXmpp::SendResult result) mutable {
            interface.finish(std::move(result));
        });
    }

    return interface.task();
}

//
// Sets the key of this client instance's device.
//
QXmppTask<void> ManagerPrivate::storeOwnKey() const
{
    QXmppPromise<void> interface;

    auto future = trustManager->setOwnKey(ns_omemo_2, ownDevice.publicIdentityKey);
    future.then(q, [=]() mutable {
        interface.finish();
    });

    return interface.task();
}

//
// Stores a key while its trust level is determined by the used security
// policy.
//
// \param keyOwnerJid bare JID of the key owner
// \param key key to store
//
// \return the trust level of the stored key
//
QXmppTask<TrustLevel> ManagerPrivate::storeKeyDependingOnSecurityPolicy(const QString &keyOwnerJid, const QByteArray &key)
{
    QXmppPromise<TrustLevel> interface;

    auto awaitStoreKey = [=](QXmppTask<TrustLevel> &future) mutable {
        future.then(q, [=](TrustLevel trustLevel) mutable {
            interface.finish(std::move(trustLevel));
        });
    };

    auto future = q->securityPolicy();
    future.then(q, [=](TrustSecurityPolicy securityPolicy) mutable {
        switch (securityPolicy) {
        case NoSecurityPolicy: {
            auto future = storeKey(keyOwnerJid, key);
            awaitStoreKey(future);
            break;
        }
        case Toakafa: {
            auto future = trustManager->hasKey(ns_omemo_2, keyOwnerJid, TrustLevel::Authenticated);
            future.then(q, [=](bool hasAuthenticatedKey) mutable {
                if (hasAuthenticatedKey) {
                    // If there is at least one authenticated key, add the
                    // new key as an automatically distrusted one.
                    auto future = storeKey(keyOwnerJid, key);
                    awaitStoreKey(future);
                } else {
                    // If no key is authenticated yet, add the new key as an
                    // automatically trusted one.
                    auto future = storeKey(keyOwnerJid, key, TrustLevel::AutomaticallyTrusted);
                    awaitStoreKey(future);
                }
            });
            break;
        }
        default:
            Q_UNREACHABLE();
        }
    });

    return interface.task();
}

//
// Stores a key.
//
// \param keyOwnerJid bare JID of the key owner
// \param key key to store
// \param trustLevel trust level of the key
//
// \return the trust level of the stored key
//
QXmppTask<TrustLevel> ManagerPrivate::storeKey(const QString &keyOwnerJid, const QByteArray &key, TrustLevel trustLevel) const
{
    QXmppPromise<TrustLevel> interface;

    auto future = trustManager->addKeys(ns_omemo_2, keyOwnerJid, { key }, trustLevel);
    future.then(q, [=]() mutable {
        Q_EMIT q->trustLevelsChanged({ { keyOwnerJid, key } });
        interface.finish(std::move(trustLevel));
    });

    return interface.task();
}

//
// Returns the own bare JID set in the client's configuration.
//
// \return the own bare JID
//
QString ManagerPrivate::ownBareJid() const
{
    return q->client()->configuration().jidBare();
}

//
// Returns the own full JID set in the client's configuration.
//
// \return the own full JID
//
QString ManagerPrivate::ownFullJid() const
{
    return q->client()->configuration().jid();
}

//
// Returns the devices with the own JID except the device of this client
// instance.
//
// \return the other own devices
//
QHash<uint32_t, QXmppOmemoStorage::Device> ManagerPrivate::otherOwnDevices()
{
    return devices.value(ownBareJid());
}

//
// Calls the logger warning method.
//
// \param msg warning message
//
void ManagerPrivate::warning(const QString &msg) const
{
    q->warning(msg);
}

/// \endcond
