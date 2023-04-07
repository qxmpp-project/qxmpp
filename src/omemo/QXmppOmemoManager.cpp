// SPDX-FileCopyrightText: 2022 Melvin Keskin <melvo@olomono.de>
// SPDX-FileCopyrightText: 2022 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppClient.h"
#include "QXmppOmemoElement_p.h"
#include "QXmppOmemoIq_p.h"
#include "QXmppOmemoItems_p.h"
#include "QXmppOmemoManager_p.h"
#include "QXmppPubSubEvent.h"
#include "QXmppTrustManager.h"
#include "QXmppUtils.h"

#include <QStringBuilder>

#undef max
#undef interface

using namespace QXmpp;
using namespace QXmpp::Private;
using namespace QXmpp::Omemo::Private;

using Manager = QXmppOmemoManager;
using ManagerPrivate = QXmppOmemoManagerPrivate;

// default label used for the own device
const auto DEVICE_LABEL = QStringLiteral("QXmpp");

class QXmppOmemoOwnDevicePrivate : public QSharedData
{
public:
    QString label;
    QByteArray keyId;
};

///
/// \class QXmppOmemoOwnDevice
///
/// \brief The QXmppOmemoOwnDevice class represents the \xep{0384, OMEMO Encryption} device of this
/// client instance.
///

///
/// Constructs an OMEMO device for this client instance.
///
QXmppOmemoOwnDevice::QXmppOmemoOwnDevice()
    : d(new QXmppOmemoOwnDevicePrivate)
{
}

/// Copy-constructor.
QXmppOmemoOwnDevice::QXmppOmemoOwnDevice(const QXmppOmemoOwnDevice &other) = default;
/// Move-constructor.
QXmppOmemoOwnDevice::QXmppOmemoOwnDevice(QXmppOmemoOwnDevice &&) noexcept = default;
QXmppOmemoOwnDevice::~QXmppOmemoOwnDevice() = default;
/// Assignment operator.
QXmppOmemoOwnDevice &QXmppOmemoOwnDevice::operator=(const QXmppOmemoOwnDevice &) = default;
/// Move-assignment operator.
QXmppOmemoOwnDevice &QXmppOmemoOwnDevice::operator=(QXmppOmemoOwnDevice &&) = default;

///
/// Returns the human-readable string used to identify the device by users.
///
/// If no label is set, a default-constructed QString is returned.
///
/// \return the label to identify the device
///
QString QXmppOmemoOwnDevice::label() const
{
    return d->label;
}

///
/// Sets an optional human-readable string used to identify the device by users.
///
/// The label should not contain more than 53 characters.
///
/// \param label label to identify the device
///
void QXmppOmemoOwnDevice::setLabel(const QString &label)
{
    d->label = label;
}

///
/// Returns the ID of the public long-term key which never changes.
///
/// \return public long-term key ID
///
QByteArray QXmppOmemoOwnDevice::keyId() const
{
    return d->keyId;
}

///
/// Sets the ID of the public long-term key which never changes.
///
/// \param keyId public long-term key ID
///
void QXmppOmemoOwnDevice::setKeyId(const QByteArray &keyId)
{
    d->keyId = keyId;
}

class QXmppOmemoDevicePrivate : public QSharedData
{
public:
    QString jid;
    TrustLevel trustLevel = TrustLevel::Undecided;
    QString label;
    QByteArray keyId;
};

///
/// \class QXmppOmemoDevice
///
/// \brief The QXmppOmemoDevice class represents a \xep{0384, OMEMO Encryption} device.
///

///
/// Constructs an OMEMO device.
///
QXmppOmemoDevice::QXmppOmemoDevice()
    : d(new QXmppOmemoDevicePrivate)
{
}

/// Copy-constructor.
QXmppOmemoDevice::QXmppOmemoDevice(const QXmppOmemoDevice &other) = default;
/// Move-constructor.
QXmppOmemoDevice::QXmppOmemoDevice(QXmppOmemoDevice &&) noexcept = default;
QXmppOmemoDevice::~QXmppOmemoDevice() = default;
/// Assignment operator.
QXmppOmemoDevice &QXmppOmemoDevice::operator=(const QXmppOmemoDevice &) = default;
/// Move-assignment operator.
QXmppOmemoDevice &QXmppOmemoDevice::operator=(QXmppOmemoDevice &&) = default;

///
/// Returns the device owner's bare JID.
///
/// \return the bare JID
///
QString QXmppOmemoDevice::jid() const
{
    return d->jid;
}

///
/// Sets the device owner's bare JID.
///
/// \param jid bare JID of the device owner
///
void QXmppOmemoDevice::setJid(const QString &jid)
{
    d->jid = jid;
}

///
/// Returns the human-readable string used to identify the device by users.
///
/// If no label is set, a default-constructed QString is returned.
///
/// \return the label to identify the device
///
QString QXmppOmemoDevice::label() const
{
    return d->label;
}

///
/// Sets an optional human-readable string used to identify the device by users.
///
/// The label should not contain more than 53 characters.
///
/// \param label label to identify the device
///
void QXmppOmemoDevice::setLabel(const QString &label)
{
    d->label = label;
}

///
/// Returns the ID of the public long-term key which never changes.
///
/// \return public long-term key ID
///
QByteArray QXmppOmemoDevice::keyId() const
{
    return d->keyId;
}

///
/// Sets the ID of the public long-term key which never changes.
///
/// \param keyId public long-term key ID
///
void QXmppOmemoDevice::setKeyId(const QByteArray &keyId)
{
    d->keyId = keyId;
}

///
/// Returns the trust level of the key.
///
/// \return the key's trust level
///
TrustLevel QXmppOmemoDevice::trustLevel() const
{
    return d->trustLevel;
}

///
/// Sets the trust level of the key.
///
/// \param trustLevel key's trust level
///
void QXmppOmemoDevice::setTrustLevel(TrustLevel trustLevel)
{
    d->trustLevel = trustLevel;
}

///
/// \class QXmppOmemoManager
///
/// The QXmppOmemoManager class manages OMEMO encryption as defined in \xep{0384,
/// OMEMO Encryption}.
///
/// OMEMO uses \xep{0060, Publish-Subscribe} (PubSub) and \xep{0163, Personal Eventing Protocol}
/// (PEP).
/// Thus, they must be supported by the server and the corresponding PubSub manager must be added to
/// the client:
/// \code
/// QXmppPubSubManager *pubSubManager = new QXmppPubSubManager;
/// client->addExtension(pubSubManager);
/// \endcode
///
/// For interacting with the storage, corresponding implementations of the storage interfaces must
/// be instantiated.
/// Those implementations have to be adapted to your storage such as a database.
/// In case you only need memory and no persistent storage, you can use the existing
/// implementations:
/// \code
/// QXmppOmemoStorage *omemoStorage = new QXmppOmemoMemoryStorage;
/// QXmppTrustStorage *trustStorage = new QXmppTrustMemoryStorage;
/// \endcode
///
/// A trust manager using its storage must be added to the client:
/// \code
/// client->addNewExtension<QXmppAtmManager>(trustStorage);
/// \endcode
///
/// Afterwards, the OMEMO manager using its storage must be added to the client:
/// \code
/// auto *manager = client->addNewExtension<QXmppOmemoManager>(omemoStorage);
/// \endcode
///
/// You can set a security policy used by OMEMO.
/// Is is recommended to apply TOAKAFA for good security and usability when using
/// \xep{0450, Automatic Trust Management (ATM)}:
/// \code
/// manager->setSecurityPolicy(QXmpp::Toakafa);
/// \endcode
///
/// \xep{0280, Message Carbons} should be used for delivering messages to all endpoints of a user:
/// \code
/// client->addNewExtension<QXmppCarbonManagerV2>();
/// \endcode
///
/// The old QXmppCarbonManager cannot be used with OMEMO.
///
/// The OMEMO data must be loaded before connecting to the server:
/// \code
///     manager->load();
/// });
/// \endcode
///
/// If no OMEMO data could be loaded (i.e., the result of \c load() is "false"), it must be set up
/// first.
/// That can be done as soon as the user is logged in to the server:
/// \code
/// connect(client, &QXmppClient::connected, this, [=]() {
///     auto future = manager->start();
/// });
/// \endcode
///
/// Once the future is finished and the result is "true", the manager is ready for use.
/// Otherwise, check the logging output for details.
///
/// By default, stanzas are only sent to devices having keys with the following trust levels:
/// \code
/// QXmpp::TrustLevel::AutomaticallyTrusted | QXmpp::TrustLevel::ManuallyTrusted
/// | QXmpp::TrustLevel::Authenticated
/// \endcode
/// That behavior can be changed for each message being sent by specifying the
/// accepted trust levels:
/// \code
/// QXmppSendStanzaParams params;
/// params.setAcceptedTrustLevels(QXmpp::TrustLevel::Authenticated)
/// client->send(stanza, params);
/// \endcode
///
/// Stanzas can be encrypted for multiple JIDs which is needed in group chats:
/// \code
/// QXmppSendStanzaParams params;
/// params.setEncryptionJids({ "alice@example.org", "bob@example.com" })
/// client->send(stanza, params);
/// \endcode
///
/// \warning THIS API IS NOT FINALIZED YET!
///
/// \ingroup Managers
///
/// \since QXmpp 1.5
///

///
/// \typedef QXmppOmemoManager::Result
///
/// Contains QXmpp::Success for success or an QXmppStanza::Error for an error.
///

///
/// Constructs an OMEMO manager.
///
/// \param omemoStorage storage used to store all OMEMO data
///
QXmppOmemoManager::QXmppOmemoManager(QXmppOmemoStorage *omemoStorage)
    : d(new ManagerPrivate(this, omemoStorage))
{
    d->ownDevice.label = DEVICE_LABEL;
    d->init();
    d->schedulePeriodicTasks();
}

QXmppOmemoManager::~QXmppOmemoManager() = default;

///
/// Loads all locally stored OMEMO data.
///
/// This should be called after starting the client and before the login.
/// It must only be called after \c setUp() has been called once for the user
/// during one of the past login sessions.
/// It does not need to be called if setUp() has been called during the current
/// login session.
///
/// \see QXmppOmemoManager::setUp()
///
/// \return whether everything is loaded successfully
///
QXmppTask<bool> Manager::load()
{
    QXmppPromise<bool> interface;

    auto future = d->omemoStorage->allData();
    future.then(this, [=](QXmppOmemoStorage::OmemoData omemoData) mutable {
        const auto &optionalOwnDevice = omemoData.ownDevice;
        if (optionalOwnDevice) {
            d->ownDevice = *optionalOwnDevice;
        } else {
            debug("Device could not be loaded because it is not stored");
            interface.finish(false);
            return;
        }

        const auto &signedPreKeyPairs = omemoData.signedPreKeyPairs;
        if (signedPreKeyPairs.isEmpty()) {
            warning("Signed Pre keys could not be loaded because none is stored");
            interface.finish(false);
            return;
        } else {
            d->signedPreKeyPairs = signedPreKeyPairs;
            d->renewSignedPreKeyPairs();
        }

        const auto &preKeyPairs = omemoData.preKeyPairs;
        if (preKeyPairs.isEmpty()) {
            warning("Pre keys could not be loaded because none is stored");
            interface.finish(false);
            return;
        } else {
            d->preKeyPairs = preKeyPairs;
        }

        d->devices = omemoData.devices;
        d->removeDevicesRemovedFromServer();

        d->isStarted = true;
        interface.finish(true);
    });

    return interface.task();
}

///
/// Sets up all OMEMO data locally and on the server.
///
/// The user must be logged in while calling this.
///
/// \return whether everything is set up successfully
///
QXmppTask<bool> Manager::setUp()
{
    QXmppPromise<bool> interface;

    auto future = d->setUpDeviceId();
    future.then(this, [=](bool isDeviceIdSetUp) mutable {
        if (isDeviceIdSetUp) {
            // The identity key pair in its deserialized form is not stored as a
            // member variable because it is only needed by
            // updateSignedPreKeyPair().
            RefCountedPtr<ratchet_identity_key_pair> identityKeyPair;

            if (d->setUpIdentityKeyPair(identityKeyPair.ptrRef()) &&
                d->updateSignedPreKeyPair(identityKeyPair.get()) &&
                d->updatePreKeyPairs(PRE_KEY_INITIAL_CREATION_COUNT)) {
                auto future = d->omemoStorage->setOwnDevice(d->ownDevice);
                future.then(this, [=]() mutable {
                    auto future = d->publishOmemoData();
                    future.then(this, [=](bool isPublished) mutable {
                        d->isStarted = isPublished;
                        interface.finish(std::move(isPublished));
                    });
                });
            } else {
                interface.finish(false);
            }
        } else {
            interface.finish(false);
        }
    });

    return interface.task();
}

///
/// Returns the key of this client instance.
///
/// \return the own key
///
QXmppTask<QByteArray> Manager::ownKey()
{
    return d->trustManager->ownKey(ns_omemo_2);
}

///
/// Returns the JIDs of all key owners mapped to the IDs of their keys with
/// specific trust levels.
///
/// If no trust levels are passed, all keys are returned.
///
/// This should be called in order to get all stored keys which can be more than
/// the stored devices because of trust decisions made without a published or
/// received device.
///
/// \param trustLevels trust levels of the keys
///
/// \return the key owner JIDs mapped to their keys with specific trust levels
///
QXmppTask<QHash<QXmpp::TrustLevel, QMultiHash<QString, QByteArray>>> Manager::keys(QXmpp::TrustLevels trustLevels)
{
    return d->trustManager->keys(ns_omemo_2, trustLevels);
}

///
/// Returns the IDs of keys mapped to their trust levels for specific key
/// owners.
///
/// If no trust levels are passed, all keys for jids are returned.
///
/// This should be called in order to get the stored keys which can be more than
/// the stored devices because of trust decisions made without a published or
/// received device.
///
/// \param jids key owners' bare JIDs
/// \param trustLevels trust levels of the keys
///
/// \return the key IDs mapped to their trust levels for specific key owners
///
QXmppTask<QHash<QString, QHash<QByteArray, QXmpp::TrustLevel>>> Manager::keys(const QList<QString> &jids, QXmpp::TrustLevels trustLevels)
{
    return d->trustManager->keys(ns_omemo_2, jids, trustLevels);
}

///
/// Changes the label of the own (this client instance's current user's) device.
///
/// The label is a human-readable string used to identify the device by users.
///
/// If the OMEMO manager is not started yet, the device label is only changed
/// locally in memory.
/// It is stored persistently in the OMEMO storage and updated on the
/// server if the OMEMO manager is already started or once it is.
///
/// \param deviceLabel own device's label
///
/// \return whether the action was successful
///
QXmppTask<bool> Manager::changeDeviceLabel(const QString &deviceLabel)
{
    return d->changeDeviceLabel(deviceLabel);
}

///
/// Returns the maximum count of devices stored per JID.
///
/// If more devices than that maximum are received for one JID from a server,
/// they will not be stored locally and thus not used for encryption.
///
/// \return the maximum count of devices stored per JID
///
int Manager::maximumDevicesPerJid() const
{
    return d->maximumDevicesPerJid;
}

///
/// Sets the maximum count of devices stored per JID.
///
/// If more devices than that maximum are received for one JID from a server,
/// they will not be stored locally and thus not used for encryption.
///
/// \param maximum maximum count of devices stored per JID
///
void Manager::setMaximumDevicesPerJid(int maximum)
{
    d->maximumDevicesPerJid = maximum;
}

///
/// Returns the maximum count of devices for whom a stanza is encrypted.
///
/// If more devices than that maximum are stored for all addressed recipients of
/// a stanza, the stanza will only be encrypted for first devices until the
/// maximum is reached.
///
/// \return the maximum count of devices for whom a stanza is encrypted
///
int Manager::maximumDevicesPerStanza() const
{
    return d->maximumDevicesPerStanza;
}

/// Sets the maximum count of devices for whom a stanza is encrypted.
///
/// If more devices than that maximum are stored for all addressed recipients of
/// a stanza, the stanza will only be encrypted for first devices until the
/// maximum is reached.
///
/// \param maximum maximum count of devices for whom a stanza is encrypted
///
void Manager::setMaximumDevicesPerStanza(int maximum)
{
    d->maximumDevicesPerStanza = maximum;
}

///
/// Requests device lists from contacts and stores them locally.
///
/// The user must be logged in while calling this.
/// The JID of the current user must not be passed.
///
/// \param jids JIDs of the contacts whose device lists are being requested
///
/// \return the results of the requests for each JID
///
QXmppTask<QVector<Manager::DevicesResult>> Manager::requestDeviceLists(const QList<QString> &jids)
{
    if (const auto jidsCount = jids.size()) {
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
            Q_ASSERT_X(jid != d->ownBareJid(), "Requesting contact's device list", "Own JID passed");

            auto future = d->requestDeviceList(jid);
            future.then(this, [jid, state](auto result) mutable {
                state->devicesResults << DevicesResult {
                    jid,
                    mapSuccess(std::move(result), [](QXmppOmemoDeviceListItem) { return Success(); })
                };

                if (++(state->processed) == state->jidsCount) {
                    state->interface.finish(std::move(state->devicesResults));
                }
            });
        }
        return state->interface.task();
    }
    return makeReadyTask(QVector<Manager::DevicesResult>());
}

///
/// Subscribes the current user's resource to device lists manually.
///
/// This should be called after each login and only for contacts without
/// presence subscription because their device lists are not automatically
/// subscribed.
/// The user must be logged in while calling this.
///
/// Call \c QXmppOmemoManager::unsubscribeFromDeviceLists() before logout.
///
/// \param jids JIDs of the contacts whose device lists are being subscribed
///
/// \return the results of the subscription for each JID
///
QXmppTask<QVector<Manager::DevicesResult>> Manager::subscribeToDeviceLists(const QList<QString> &jids)
{
    if (const auto jidsCount = jids.size()) {
        struct State
        {
            int processed = 0;
            int jidsCount = 0;
            QXmppPromise<QVector<Manager::DevicesResult>> interface;
            QVector<Manager::DevicesResult> devicesResults;
        };

        auto state = std::make_shared<State>();
        state->jidsCount = jids.size();

        for (const auto &jid : jids) {
            d->subscribeToDeviceList(jid).then(this, [state, jid](QXmppPubSubManager::Result result) mutable {
                Manager::DevicesResult devicesResult;
                devicesResult.jid = jid;
                devicesResult.result = result;
                state->devicesResults << devicesResult;

                if (++(state->processed) == state->jidsCount) {
                    state->interface.finish(std::move(state->devicesResults));
                }
            });
        }
        return state->interface.task();
    }
    return makeReadyTask(QVector<Manager::DevicesResult>());
}

///
/// Unsubscribes the current user's resource from all device lists that were
/// manually subscribed by \c QXmppOmemoManager::subscribeToDeviceList().
///
/// This should be called before each logout.
/// The user must be logged in while calling this.
///
/// \return the results of the unsubscription for each JID
///
QXmppTask<QVector<Manager::DevicesResult>> Manager::unsubscribeFromDeviceLists()
{
    return d->unsubscribeFromDeviceLists(d->jidsOfManuallySubscribedDevices);
}

///
/// Returns the device of this client instance's current user.
///
/// \return the own device
///
QXmppOmemoOwnDevice Manager::ownDevice()
{
    const auto &ownDevice = d->ownDevice;

    QXmppOmemoOwnDevice device;
    device.setLabel(ownDevice.label);
    device.setKeyId(ownDevice.publicIdentityKey);

    return device;
}

/// Returns all locally stored devices except the own device.
///
/// Only devices that have been received after subscribing the corresponding device lists on the
/// server are stored locally.
/// Thus, only those are returned.
/// Call \c QXmppOmemoManager::subscribeToDeviceLists() for contacts without presence subscription
/// before.
///
/// You must build sessions before you can get devices with corresponding keys.
///
/// /\return all devices except the own device
///
QXmppTask<QVector<QXmppOmemoDevice>> Manager::devices()
{
    return devices(d->devices.keys());
}

///
/// Returns locally stored devices except the own device.
///
/// Only devices that have been received after subscribing the corresponding device lists on the
/// server are stored locally.
/// Thus, only those are returned.
/// Call \c QXmppOmemoManager::subscribeToDeviceLists() for contacts without presence subscription
/// before.
///
/// You must build sessions before you can get devices with corresponding keys.
///
/// \param jids JIDs whose devices are being retrieved
///
/// \return all devices of the passed JIDs
///
QXmppTask<QVector<QXmppOmemoDevice>> Manager::devices(const QList<QString> &jids)
{
    QXmppPromise<QVector<QXmppOmemoDevice>> interface;

    auto future = keys(jids);
    future.then(this, [=](QHash<QString, QHash<QByteArray, TrustLevel>> keys) mutable {
        QVector<QXmppOmemoDevice> devices;

        for (const auto &jid : jids) {
            const auto &storedDevices = d->devices.value(jid);
            const auto &storedKeys = keys.value(jid);

            for (const auto &storedDevice : storedDevices) {
                const auto &keyId = storedDevice.keyId;

                QXmppOmemoDevice device;
                device.setJid(jid);
                device.setLabel(storedDevice.label);

                if (!keyId.isEmpty()) {
                    device.setKeyId(keyId);
                    device.setTrustLevel(storedKeys.value(keyId));
                }

                devices.append(device);
            }
        }

        interface.finish(std::move(devices));
    });

    return interface.task();
}

///
/// Removes all devices of a contact and the subscription to the contact's
/// device list.
///
/// This should be called after removing a contact.
/// The JID of the current user must not be passed.
/// Use \c QXmppOmemoManager::resetAll() in order to remove all devices of the
/// user.
///
/// \param jid JID of the contact whose devices are being removed
///
/// \return the result of the contact device removals
///
QXmppTask<QXmppPubSubManager::Result> Manager::removeContactDevices(const QString &jid)
{
    QXmppPromise<QXmppPubSubManager::Result> interface;

    Q_ASSERT_X(jid != d->ownBareJid(), "Removing contact device", "Own JID passed");

    auto future = d->unsubscribeFromDeviceList(jid);
    future.then(this, [=](QXmppPubSubManager::Result result) mutable {
        if (std::holds_alternative<QXmppError>(result)) {
            warning("Contact '" % jid % "' could not be removed because the device list subscription could not be removed");
            interface.finish(std::move(result));
        } else {
            d->devices.remove(jid);

            auto future = d->omemoStorage->removeDevices(jid);
            future.then(this, [=]() mutable {
                auto future = d->trustManager->removeKeys(ns_omemo_2, jid);
                future.then(this, [=]() mutable {
                    interface.finish(std::move(result));
                    Q_EMIT devicesRemoved(jid);
                });
            });
        }
    });

    return interface.task();
}

///
/// Sets the trust levels keys must have in order to build sessions for their
/// devices.
///
/// \param trustLevels trust levels of the keys used for building sessions
///
void Manager::setAcceptedSessionBuildingTrustLevels(QXmpp::TrustLevels trustLevels)
{
    d->acceptedSessionBuildingTrustLevels = trustLevels;
}

///
/// Returns the trust levels keys must have in order to build sessions for their
/// devices.
///
/// \return the trust levels of the keys used for building sessions
///
TrustLevels Manager::acceptedSessionBuildingTrustLevels()
{
    return d->acceptedSessionBuildingTrustLevels;
}

///
/// Sets whether sessions are built when new devices are received from the
/// server.
///
/// This can be used to not call \c QXmppOmemoManager::buildMissingSessions
/// manually.
/// But it should not be used before the initial setup and storing lots of
/// devices locally.
/// Otherwise, it could lead to a massive computation and network load when
/// there are many devices for whom sessions are built.
///
/// \see QXmppOmemoManager::buildMissingSessions
///
/// \param isNewDeviceAutoSessionBuildingEnabled whether sessions are built for
///        incoming devices
///
void Manager::setNewDeviceAutoSessionBuildingEnabled(bool isNewDeviceAutoSessionBuildingEnabled)
{
    d->isNewDeviceAutoSessionBuildingEnabled = isNewDeviceAutoSessionBuildingEnabled;
}

///
/// Returns whether sessions are built when new devices are received from the
/// server.
///
/// \see QXmppOmemoManager::setNewDeviceAutoSessionBuildingEnabled
///
/// \return whether sessions are built for incoming devices
///
bool Manager::isNewDeviceAutoSessionBuildingEnabled()
{
    return d->isNewDeviceAutoSessionBuildingEnabled;
}

///
/// Builds sessions manually with devices for whom no sessions are available.
///
/// Usually, sessions are built during sending a first message to a device or after a first message
/// is received from a device.
/// This can be called in order to speed up the sending of a message.
/// If this method is called before sending the first message, all sessions can be built and when
/// the first message is being sent, the message only needs to be encrypted.
/// Especially for chats with multiple devices, that can decrease the noticeable time a user has to
/// wait for sending a message.
/// Additionally, the keys are automatically retrieved from the server which is helpful in order to
/// get them when calling \c QXmppOmemoManager::devices().
///
/// The user must be logged in while calling this.
///
/// \param jids JIDs of the device owners for whom the sessions are built
///
QXmppTask<void> Manager::buildMissingSessions(const QList<QString> &jids)
{
    QXmppPromise<void> interface;

    auto &devices = d->devices;
    auto devicesCount = 0;

    for (const auto &jid : jids) {
        // Do not exceed the maximum of manageable devices.
        if (devicesCount > d->maximumDevicesPerStanza - devicesCount) {
            warning("Sessions could not be built for all JIDs because their devices are "
                    "altogether more than the maximum of manageable devices " %
                    QString::number(d->maximumDevicesPerStanza) %
                    u" - Use QXmppOmemoManager::setMaximumDevicesPerStanza() to increase the maximum");
            break;
        } else {
            devicesCount += devices.value(jid).size();
        }
    }

    if (devicesCount) {
        auto processedDevicesCount = std::make_shared<int>(0);

        for (const auto &jid : jids) {
            auto &processedDevices = devices[jid];

            for (auto itr = processedDevices.begin(); itr != processedDevices.end(); ++itr) {
                const auto &deviceId = itr.key();
                auto &device = itr.value();

                if (device.session.isEmpty()) {
                    auto future = d->buildSessionWithDeviceBundle(jid, deviceId, device);
                    future.then(this, [=](auto) mutable {
                        if (++(*processedDevicesCount) == devicesCount) {
                            interface.finish();
                        }
                    });
                } else if (++(*processedDevicesCount) == devicesCount) {
                    interface.finish();
                }
            }
        }
    } else {
        interface.finish();
    }

    return interface.task();
}

///
/// Resets all OMEMO data for this device and the trust data used by OMEMO.
///
/// ATTENTION: This should only be called when an account is removed locally or
/// if there are unrecoverable problems with the OMEMO setup of this device.
///
/// The data on the server for other own devices is not removed.
/// Call \c resetAll() for that purpose.
///
/// The user must be logged in while calling this.
///
/// Call \c setUp() once this method is finished if you want to set up
/// everything again for this device.
/// Existing sessions are reset, which might lead to undecryptable incoming
/// stanzas until everything is set up again.
///
QXmppTask<bool> Manager::resetOwnDevice()
{
    return d->resetOwnDevice();
}

///
/// Resets all OMEMO data for all own devices and the trust data used by OMEMO.
///
/// ATTENTION: This should only be called if there is a certain reason for it
/// since it deletes the data for this device and for other own devices from the
/// server.
///
/// Call \c resetOwnDevice() if you only want to delete the OMEMO data for this
/// device.
///
/// The user must be logged in while calling this.
///
/// Call \c setUp() once this method is finished if you want to set up
/// everything again.
/// Existing sessions are reset, which might lead to undecryptable incoming
/// stanzas until everything is set up again.
///
QXmppTask<bool> Manager::resetAll()
{
    return d->resetAll();
}

///
/// Sets the security policy used by this E2EE extension.
///
/// \param securityPolicy security policy being set
///
QXmppTask<void> Manager::setSecurityPolicy(QXmpp::TrustSecurityPolicy securityPolicy)
{
    return d->trustManager->setSecurityPolicy(ns_omemo_2, securityPolicy);
}

///
/// Returns the security policy used by this E2EE extension.
///
/// \return the used security policy
///
QXmppTask<QXmpp::TrustSecurityPolicy> Manager::securityPolicy()
{
    return d->trustManager->securityPolicy(ns_omemo_2);
}

///
/// Sets the trust level of keys.
///
/// If a key is not stored, it is added to the storage.
///
/// \param keyIds key owners' bare JIDs mapped to the IDs of their keys
/// \param trustLevel trust level being set
///
QXmppTask<void> Manager::setTrustLevel(const QMultiHash<QString, QByteArray> &keyIds, QXmpp::TrustLevel trustLevel)
{
    return d->trustManager->setTrustLevel(ns_omemo_2, keyIds, trustLevel);
}

///
/// Returns the trust level of a key.
///
/// If the key is not stored, the trust in that key is undecided.
///
/// \param keyOwnerJid key owner's bare JID
/// \param keyId ID of the key
///
/// \return the key's trust level
///
QXmppTask<QXmpp::TrustLevel> Manager::trustLevel(const QString &keyOwnerJid, const QByteArray &keyId)
{
    return d->trustManager->trustLevel(ns_omemo_2, keyOwnerJid, keyId);
}

/// \cond
QXmppTask<QXmppE2eeExtension::MessageEncryptResult> Manager::encryptMessage(QXmppMessage &&message, const std::optional<QXmppSendStanzaParams> &params)
{
    QVector<QString> recipientJids;
    std::optional<TrustLevels> acceptedTrustLevels;

    if (params) {
        recipientJids = params->encryptionJids();
        acceptedTrustLevels = params->acceptedTrustLevels();
    }

    if (recipientJids.isEmpty()) {
        recipientJids.append(QXmppUtils::jidToBareJid(message.to()));
    }

    if (!acceptedTrustLevels) {
        acceptedTrustLevels = ACCEPTED_TRUST_LEVELS;
    }

    return d->encryptMessageForRecipients(std::move(message), recipientJids, *acceptedTrustLevels);
}

QXmppTask<QXmppE2eeExtension::MessageDecryptResult> QXmppOmemoManager::decryptMessage(QXmppMessage &&message)
{
    if (!d->isStarted) {
        return makeReadyTask<MessageDecryptResult>(QXmppError {
            QStringLiteral("OMEMO manager must be started before decrypting"),
            SendError::EncryptionError });
    }

    auto omemoElement = message.omemoElement();
    if (!omemoElement) {
        return makeReadyTask<MessageDecryptResult>(NotEncrypted());
    }

    return chain<MessageDecryptResult>(d->decryptMessage(message), this, [](std::optional<QXmppMessage> message) -> MessageDecryptResult {
        if (message) {
            return std::move(*message);
        }
        return QXmppError {
            QStringLiteral("Couldn't decrypt message"),
            {}
        };
    });
}

QXmppTask<QXmppE2eeExtension::IqEncryptResult> Manager::encryptIq(QXmppIq &&iq, const std::optional<QXmppSendStanzaParams> &params)
{
    QXmppPromise<QXmppE2eeExtension::IqEncryptResult> interface;

    if (!d->isStarted) {
        interface.finish(QXmppError {
            QStringLiteral("OMEMO manager must be started before encrypting"),
            SendError::EncryptionError });
    } else {
        std::optional<TrustLevels> acceptedTrustLevels;

        if (params) {
            acceptedTrustLevels = params->acceptedTrustLevels();
        }

        if (!acceptedTrustLevels) {
            acceptedTrustLevels = ACCEPTED_TRUST_LEVELS;
        }

        auto future = d->encryptStanza(iq, { QXmppUtils::jidToBareJid(iq.to()) }, *acceptedTrustLevels);
        future.then(this, [=, iq = std::move(iq)](std::optional<QXmppOmemoElement> omemoElement) mutable {
            if (!omemoElement) {
                interface.finish(QXmppError {
                    QStringLiteral("OMEMO element could not be created"),
                    SendError::EncryptionError });

            } else {
                auto omemoIq = std::make_unique<QXmppOmemoIq>();
                omemoIq->setId(iq.id());
                omemoIq->setType(iq.type());
                omemoIq->setLang(iq.lang());
                omemoIq->setFrom(iq.from());
                omemoIq->setTo(iq.to());
                omemoIq->setOmemoElement(*omemoElement);

                interface.finish(std::move(omemoIq));
            }
        });
    }

    return interface.task();
}

QXmppTask<QXmppE2eeExtension::IqDecryptResult> Manager::decryptIq(const QDomElement &element)
{
    if (!d->isStarted) {
        // TODO: Add decryption queue to avoid this error
        return makeReadyTask<IqDecryptResult>(QXmppError {
            QStringLiteral("OMEMO manager must be started before decrypting"),
            SendError::EncryptionError });
    }

    if (QXmppOmemoIq::isOmemoIq(element)) {
        // Tag name and iq type are already checked in QXmppClient.
        return chain<IqDecryptResult>(d->decryptIq(element), this, [](auto result) -> IqDecryptResult {
            if (result) {
                return result->iq;
            }
            return QXmppError {
                QStringLiteral("OMEMO message could not be decrypted"),
                SendError::EncryptionError
            };
        });
    }

    return makeReadyTask<IqDecryptResult>(NotEncrypted());
}

bool QXmppOmemoManager::isEncrypted(const QDomElement &el)
{
    for (auto subEl = el.firstChildElement();
         !subEl.isNull();
         subEl = subEl.nextSiblingElement()) {
        if (subEl.tagName() == "encrypted" && subEl.namespaceURI() == ns_omemo_2) {
            return true;
        }
    }
    return false;
}

bool QXmppOmemoManager::isEncrypted(const QXmppMessage &message)
{
    return message.omemoElement().has_value();
}

QStringList Manager::discoveryFeatures() const
{
    return {
        QString(ns_omemo_2_devices) % "+notify"
    };
}

bool Manager::handleStanza(const QDomElement &stanza)
{
    if (stanza.tagName() != "iq" || !QXmppOmemoIq::isOmemoIq(stanza)) {
        return false;
    }

    // TODO: Queue incoming IQs until OMEMO is initialized
    if (!d->isStarted) {
        warning("Couldn't decrypt incoming IQ because the manager isn't initialized yet.");
        return false;
    }

    auto type = stanza.attribute("type");
    if (type != "get" && type != "set") {
        // ignore incoming result and error IQs (they are handled via Client::sendIq())
        return false;
    }

    d->decryptIq(stanza).then(this, [=](auto result) {
        if (result) {
            injectIq(result->iq, result->e2eeMetadata);
        } else {
            warning("Could not decrypt incoming OMEMO IQ.");
        }
    });
    return true;
}

bool Manager::handleMessage(const QXmppMessage &message)
{
    if (d->isStarted && message.omemoElement()) {
        auto future = d->decryptMessage(message);
        future.then(this, [=](std::optional<QXmppMessage> optionalDecryptedMessage) mutable {
            if (optionalDecryptedMessage) {
                injectMessage(std::move(*optionalDecryptedMessage));
            }
        });

        return true;
    }

    return false;
}
/// \endcond

///
/// \fn QXmppOmemoManager::trustLevelsChanged(const QMultiHash<QString, QByteArray> &modifiedKeys)
///
/// Emitted when the trust levels of keys changed.
///
/// \param modifiedKeys key owners' bare JIDs mapped to their modified keys
///

///
/// \fn QXmppOmemoManager::deviceAdded(const QString &jid, uint32_t deviceId)
///
/// Emitted when a device is added.
///
/// \param jid device owner's bare JID
/// \param deviceId ID of the device
///

///
/// \fn QXmppOmemoManager::deviceChanged(const QString &jid, uint32_t deviceId)
///
/// Emitted when a device changed.
///
/// \param jid device owner's bare JID
/// \param deviceId ID of the device
///

///
/// \fn QXmppOmemoManager::deviceRemoved(const QString &jid, uint32_t deviceId)
///
/// Emitted when a device is removed.
///
/// \param jid device owner's bare JID
/// \param deviceId ID of the device
///

///
/// \fn QXmppOmemoManager::devicesRemoved(const QString &jid)
///
/// Emitted when all devices of an owner are removed.
///
/// \param jid device owner's bare JID
///

///
/// \fn QXmppOmemoManager::allDevicesRemoved()
///
/// Emitted when all devices are removed.
///

/// \cond
void Manager::setClient(QXmppClient *client)
{
    QXmppClientExtension::setClient(client);
    client->setEncryptionExtension(this);

    d->trustManager = client->findExtension<QXmppTrustManager>();
    if (!d->trustManager) {
        qFatal("QXmppTrustManager is not available, it must be added to the client before adding QXmppOmemoManager");
    }

    d->pubSubManager = client->findExtension<QXmppPubSubManager>();
    if (!d->pubSubManager) {
        qFatal("QXmppPubSubManager is not available, it must be added to the client before adding QXmppOmemoManager");
    }

    connect(d->trustManager, &QXmppTrustManager::trustLevelsChanged, this, [=](const QHash<QString, QMultiHash<QString, QByteArray>> &modifiedKeys) {
        const auto &modifiedOmemoKeys = modifiedKeys.value(ns_omemo_2);

        if (!modifiedOmemoKeys.isEmpty()) {
            Q_EMIT trustLevelsChanged(modifiedOmemoKeys);
        }

        QMultiHash<QString, uint32_t> modifiedDevices;

        for (auto itr = modifiedOmemoKeys.cbegin(); itr != modifiedOmemoKeys.cend(); ++itr) {
            const auto &keyOwnerJid = itr.key();
            const auto &keyId = itr.value();

            // Ensure to emit 'deviceChanged()' later only if there is a device with the key.
            const auto &devices = d->devices.value(keyOwnerJid);
            for (auto devicesItr = devices.cbegin(); devicesItr != devices.cend(); ++devicesItr) {
                if (devicesItr->keyId == keyId) {
                    modifiedDevices.insert(keyOwnerJid, devicesItr.key());
                    break;
                }
            }
        }

        for (auto modifiedDevicesItr = modifiedDevices.cbegin(); modifiedDevicesItr != modifiedDevices.cend(); ++modifiedDevicesItr) {
            Q_EMIT deviceChanged(modifiedDevicesItr.key(), modifiedDevicesItr.value());
        }
    });
}

bool Manager::handlePubSubEvent(const QDomElement &element, const QString &pubSubService, const QString &nodeName)
{
    if (nodeName == ns_omemo_2_devices && QXmppPubSubEvent<QXmppOmemoDeviceListItem>::isPubSubEvent(element)) {
        QXmppPubSubEvent<QXmppOmemoDeviceListItem> event;
        event.parse(element);

        switch (event.eventType()) {
        // Items have been published.
        case QXmppPubSubEventBase::Items: {
            // Only process items if the event notification contains one.
            // That is necessary because PubSub allows publishing without items leading to
            // notification-only events.
            if (const auto &items = event.items(); !items.isEmpty()) {
                // Since the usage of the item ID \c QXmppPubSubManager::Current is only RECOMMENDED
                // by \xep{0060, Publish-Subscribe} (PubSub) but not obligatory, an appropriate
                // contact device list is determined.
                // In case of the own device list node, it is sctrictly processed as a recommended
                // singleton item and changed to fit that if needed.
                const auto isOwnDeviceListNode = d->ownBareJid() == pubSubService;
                if (isOwnDeviceListNode) {
                    const auto &deviceListItem = items.constFirst();
                    if (deviceListItem.id() == QXmppPubSubManager::standardItemIdToString(QXmppPubSubManager::Current)) {
                        d->updateDevices(pubSubService, event.items().constFirst());
                    } else {
                        d->handleIrregularDeviceListChanges(pubSubService);
                    }
                } else {
                    d->updateContactDevices(pubSubService, items);
                }
            }

            break;
        }
        // Specific items are deleted.
        case QXmppPubSubEventBase::Retract: {
            d->handleIrregularDeviceListChanges(pubSubService);
        }
        // All items are deleted.
        case QXmppPubSubEventBase::Purge:
        // The whole node is deleted.
        case QXmppPubSubEventBase::Delete:
            d->handleIrregularDeviceListChanges(pubSubService);
            break;
        case QXmppPubSubEventBase::Configuration:
        case QXmppPubSubEventBase::Subscription:
            break;
        }

        return true;
    }

    return false;
}
/// \endcond
