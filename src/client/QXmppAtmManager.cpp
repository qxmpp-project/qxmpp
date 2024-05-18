// SPDX-FileCopyrightText: 2021 Melvin Keskin <melvo@olomono.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppAtmManager.h"

#include "QXmppCarbonManager.h"
#include "QXmppClient.h"
#include "QXmppConstants_p.h"
#include "QXmppE2eeMetadata.h"
#include "QXmppFutureUtils_p.h"
#include "QXmppMessage.h"
#include "QXmppTrustMessageElement.h"
#include "QXmppTrustMessageKeyOwner.h"
#include "QXmppUtils.h"
#include "QXmppUtils_p.h"

using namespace QXmpp;
using namespace QXmpp::Private;

///
/// \class QXmppAtmManager
///
/// \brief The QXmppAtmManager class represents a manager for
/// \xep{0450, Automatic Trust Management (ATM)}.
///
/// For interacting with the storage, a corresponding implementation of the
/// storage interface must be added.
/// That implementation has to be adapted to your storage such as a database.
/// In case you only need memory and no peristent storage, you can use the
/// existing implementation and add the storage with it:
///
///\code
/// QXmppAtmTrustStorage *trustStorage = new QXmppAtmTrustMemoryStorage;
/// QXmppAtmManager *manager = new QXmppAtmManager(trustStorage);
/// client->addExtension(manager);
/// \endcode
///
/// It is strongly recommended to enable \xep{0280, Message Carbons} with
/// \code
/// QXmppCarbonManager *carbonManager = new QXmppCarbonManager;
/// carbonManager->setCarbonsEnabled(true);
/// client->addExtension(carbonManager);
/// \endcode
/// and \xep{0313, Message Archive Management} with
/// \code
/// QXmppMamManager *mamManager = new QXmppMamManager;
/// client->addExtension(mamManager);
/// \endcode
/// for delivering trust messages to all online and offline endpoints.
///
/// In addition, archiving via MAM must be enabled on the server.
///
/// \warning THIS API IS NOT FINALIZED YET!
///
/// \ingroup Managers
///
/// \since QXmpp 1.5
///

///
/// Constructs an ATM manager.
///
/// \param trustStorage trust storage implementation
///
QXmppAtmManager::QXmppAtmManager(QXmppAtmTrustStorage *trustStorage)
    : QXmppTrustManager(trustStorage)
{
}

///
/// Authenticates or distrusts keys manually (e.g., by the Trust Message URI of
/// a scanned QR code or after entering key IDs by hand) and sends corresponding
/// trust messages.
///
/// \param encryption encryption protocol namespace
/// \param keyOwnerJid JID of the key owner
/// \param keyIdsForAuthentication IDs of the keys being authenticated
/// \param keyIdsForDistrusting IDs of the keys being distrusted
///
QXmppTask<void> QXmppAtmManager::makeTrustDecisions(const QString &encryption, const QString &keyOwnerJid, const QList<QByteArray> &keyIdsForAuthentication, const QList<QByteArray> &keyIdsForDistrusting)
{
    QXmppPromise<void> promise;

    auto future = keys(encryption, TrustLevel::Authenticated | TrustLevel::ManuallyDistrusted);
    future.then(this, [=, this](QHash<TrustLevel, QMultiHash<QString, QByteArray>> keys) mutable {
        const auto authenticatedKeys = keys.value(TrustLevel::Authenticated);
        const auto manuallyDistrustedKeys = keys.value(TrustLevel::ManuallyDistrusted);
        const auto ownJid = client()->configuration().jidBare();
        const auto ownAuthenticatedKeys = authenticatedKeys.values(ownJid);

        // Create a key owner for the keys being authenticated or
        // distrusted.
        QXmppTrustMessageKeyOwner keyOwner;
        keyOwner.setJid(keyOwnerJid);

        QList<QByteArray> modifiedAuthenticatedKeys;
        QList<QByteArray> modifiedManuallyDistrustedKeys;

        for (const auto &keyId : keyIdsForAuthentication) {
            if (!authenticatedKeys.contains(keyOwnerJid, keyId)) {
                modifiedAuthenticatedKeys.append(keyId);
            }
        }

        for (const auto &keyId : keyIdsForDistrusting) {
            if (!manuallyDistrustedKeys.contains(keyOwnerJid, keyId)) {
                modifiedManuallyDistrustedKeys.append(keyId);
            }
        }

        if (modifiedAuthenticatedKeys.isEmpty() && modifiedManuallyDistrustedKeys.isEmpty()) {
            // Skip further processing if there are no changes.
            promise.finish();
        } else {
            keyOwner.setTrustedKeys(modifiedAuthenticatedKeys);
            keyOwner.setDistrustedKeys(modifiedManuallyDistrustedKeys);

            QMultiHash<QString, QByteArray> keysBeingAuthenticated;
            QMultiHash<QString, QByteArray> keysBeingDistrusted;

            for (const auto &key : std::as_const(modifiedAuthenticatedKeys)) {
                keysBeingAuthenticated.insert(keyOwnerJid, key);
            }

            for (const auto &key : std::as_const(modifiedManuallyDistrustedKeys)) {
                keysBeingDistrusted.insert(keyOwnerJid, key);
            }

            // Create a key owner for authenticated and distrusted keys of own
            // endpoints.
            QXmppTrustMessageKeyOwner ownKeyOwner;
            ownKeyOwner.setJid(ownJid);

            if (!ownAuthenticatedKeys.isEmpty()) {
                ownKeyOwner.setTrustedKeys(ownAuthenticatedKeys);
            }

            const auto ownManuallyDistrustedKeys = manuallyDistrustedKeys.values(ownJid);
            if (!ownManuallyDistrustedKeys.isEmpty()) {
                ownKeyOwner.setDistrustedKeys(ownManuallyDistrustedKeys);
            }

            const auto areOwnKeysProcessed = keyOwnerJid == ownJid;
            if (areOwnKeysProcessed) {
                auto authenticatedKeysTemp = authenticatedKeys;
                authenticatedKeysTemp.remove(ownJid);
                const auto contactsAuthenticatedKeys = authenticatedKeysTemp;

                const auto contactsWithAuthenticatedKeys = contactsAuthenticatedKeys.uniqueKeys();

                // Send trust messages for the keys of the own endpoints being
                // authenticated or distrusted to endpoints of contacts with
                // authenticated keys.
                // Own endpoints with authenticated keys can receive the trust
                // messages via Message Carbons.
                for (const auto &contactJid : contactsWithAuthenticatedKeys) {
                    sendTrustMessage(encryption, { keyOwner }, contactJid);
                }

                // Send a trust message for the keys of the own endpoints being
                // authenticated or distrusted to other own endpoints with
                // authenticated keys.
                // It is skipped if a trust message is already delivered via
                // Message Carbons or there are no other own endpoints with
                // authenticated keys.
                const auto *carbonManager = client()->findExtension<QXmppCarbonManager>();
                const auto isMessageCarbonsDisabled = !carbonManager || !carbonManager->carbonsEnabled();
                if (isMessageCarbonsDisabled || (contactsAuthenticatedKeys.isEmpty() && !ownAuthenticatedKeys.isEmpty())) {
                    sendTrustMessage(encryption, { keyOwner }, ownJid);
                }

                auto future = makeTrustDecisions(encryption, keysBeingAuthenticated, keysBeingDistrusted);
                future.then(this, [=, this]() mutable {
                    // Send a trust message for all authenticated or distrusted
                    // keys to the own endpoints whose keys have been
                    // authenticated.
                    // It is skipped if no keys of own endpoints have been
                    // authenticated.
                    if (!keyOwner.trustedKeys().isEmpty()) {
                        auto manuallyDistrustedKeysTemp = manuallyDistrustedKeys;
                        manuallyDistrustedKeysTemp.remove(ownJid);
                        const auto contactsManuallyDistrustedKeys = manuallyDistrustedKeysTemp;

                        auto contactJids = contactsManuallyDistrustedKeys.uniqueKeys() << contactsWithAuthenticatedKeys;

                        // Remove duplicates from contactJids.
                        std::sort(contactJids.begin(), contactJids.end());
                        contactJids.erase(std::unique(contactJids.begin(), contactJids.end()), contactJids.end());

                        QList<QXmppTrustMessageKeyOwner> contactsKeyOwners;

                        for (const auto &contactJid : std::as_const(contactJids)) {
                            QXmppTrustMessageKeyOwner contactKeyOwner;
                            contactKeyOwner.setJid(contactJid);
                            contactKeyOwner.setTrustedKeys(contactsAuthenticatedKeys.values(contactJid));

                            if (const auto contactManuallyDistrustedKeys = contactsManuallyDistrustedKeys.values(contactJid); !contactsManuallyDistrustedKeys.isEmpty()) {
                                contactKeyOwner.setDistrustedKeys(contactManuallyDistrustedKeys);
                            }

                            contactsKeyOwners.append(contactKeyOwner);
                        }

                        auto allKeyOwners = contactsKeyOwners;

                        if (!(ownKeyOwner.trustedKeys().isEmpty() && ownKeyOwner.distrustedKeys().isEmpty())) {
                            allKeyOwners.append(ownKeyOwner);
                        }

                        if (!allKeyOwners.isEmpty()) {
                            sendTrustMessage(encryption, allKeyOwners, ownJid);
                        }
                    }

                    promise.finish();
                });
            } else {
                // Send a trust message for the keys of the contact's endpoints
                // being authenticated or distrusted to own endpoints
                // with authenticated keys.
                if (!ownAuthenticatedKeys.isEmpty()) {
                    sendTrustMessage(encryption, { keyOwner }, ownJid);
                }

                auto future = makeTrustDecisions(encryption, keysBeingAuthenticated, keysBeingDistrusted);
                future.then(this, [=, this]() mutable {
                    // Send a trust message for own authenticated or distrusted
                    // keys to the contact's endpoints whose keys have been
                    // authenticated.
                    // It is skipped if no keys of contacts have been
                    // authenticated or there are no keys for the trust message.
                    if (!keyOwner.trustedKeys().isEmpty() && !(ownKeyOwner.trustedKeys().isEmpty() && ownKeyOwner.distrustedKeys().isEmpty())) {
                        sendTrustMessage(encryption, { ownKeyOwner }, keyOwnerJid);
                    }

                    promise.finish();
                });
            }
        }
    });

    return promise.task();
}

/// \cond
void QXmppAtmManager::onRegistered(QXmppClient *client)
{
    connect(client, &QXmppClient::messageReceived, this, &QXmppAtmManager::handleMessageReceived);
}

void QXmppAtmManager::onUnregistered(QXmppClient *client)
{
    disconnect(client, &QXmppClient::messageReceived, this, &QXmppAtmManager::handleMessageReceived);
}

void QXmppAtmManager::handleMessageReceived(const QXmppMessage &message)
{
    handleMessage(message);
}
/// \endcond

///
/// Authenticates or distrusts keys.
///
/// \param encryption encryption protocol namespace
/// \param keyIdsForAuthentication key owners' bare JIDs mapped to the IDs of
///        their keys being authenticated
/// \param keyIdsForDistrusting key owners' bare JIDs mapped to the IDs of their
///        keys being distrusted
///
QXmppTask<void> QXmppAtmManager::makeTrustDecisions(const QString &encryption, const QMultiHash<QString, QByteArray> &keyIdsForAuthentication, const QMultiHash<QString, QByteArray> &keyIdsForDistrusting)
{
    QXmppPromise<void> promise;

    auto future = authenticate(encryption, keyIdsForAuthentication);

    future.then(this, [=, this]() mutable {
        auto future = distrust(encryption, keyIdsForDistrusting);
        future.then(this, [=]() mutable {
            promise.finish();
        });
    });

    return promise.task();
}

///
/// Handles incoming messages and uses included trust message elements for
/// making automatic trust decisions.
///
/// \param message message that can contain a trust message element
///
QXmppTask<void> QXmppAtmManager::handleMessage(const QXmppMessage &message)
{
    QXmppPromise<void> promise;

    if (const auto trustMessageElement = message.trustMessageElement();
        trustMessageElement &&
        trustMessageElement->usage() == ns_atm &&
        message.from() != client()->configuration().jid()) {
        const auto senderJid = QXmppUtils::jidToBareJid(message.from());
        const auto e2eeMetadata = message.e2eeMetadata();
        const auto senderKey = e2eeMetadata ? e2eeMetadata->senderKey() : QByteArray();
        const auto encryption = trustMessageElement->encryption();

        auto future = trustLevel(encryption, senderJid, senderKey);
        future.then(this, [=, this](const auto &&senderKeyTrustLevel) mutable {
            const auto isSenderKeyAuthenticated = senderKeyTrustLevel == TrustLevel::Authenticated;

            // key owner JIDs mapped to key IDs
            QMultiHash<QString, QByteArray> keysBeingAuthenticated;
            QMultiHash<QString, QByteArray> keysBeingDistrusted;

            QList<QXmppTrustMessageKeyOwner> keyOwnersForPostponedTrustDecisions;

            const auto ownJid = client()->configuration().jidBare();
            const auto isOwnTrustMessage = senderJid == ownJid;
            const auto keyOwners = trustMessageElement->keyOwners();

            for (const auto &keyOwner : keyOwners) {
                const auto keyOwnerJid = keyOwner.jid();

                // A trust message from an own endpoint is allowed to
                // authenticate or distrust the keys of own endpoints and
                // endpoints of contacts.
                // Whereas a trust message from an endpoint of a contact is
                // only allowed to authenticate or distrust the keys of that
                // contact's own endpoints.
                const auto isSenderQualifiedForTrustDecisions = isOwnTrustMessage || senderJid == keyOwnerJid;
                if (isSenderQualifiedForTrustDecisions) {
                    // Make trust decisions if the key of the sender is
                    // authenticated.
                    // Othwerwise, store the keys of the trust message for
                    // making the trust decisions as soon as the key of the
                    // sender is authenticated.
                    if (isSenderKeyAuthenticated) {
                        const auto trustedKeys = keyOwner.trustedKeys();
                        for (const auto &key : trustedKeys) {
                            keysBeingAuthenticated.insert(keyOwnerJid, key);
                        }

                        const auto distrustedKeys = keyOwner.distrustedKeys();
                        for (const auto &key : distrustedKeys) {
                            keysBeingDistrusted.insert(keyOwnerJid, key);
                        }
                    } else {
                        keyOwnersForPostponedTrustDecisions.append(keyOwner);
                    }
                }
            }

            auto future = trustStorage()->addKeysForPostponedTrustDecisions(encryption, senderKey, keyOwnersForPostponedTrustDecisions);
            future.then(this, [=, this]() mutable {
                auto future = makeTrustDecisions(encryption, keysBeingAuthenticated, keysBeingDistrusted);
                future.then(this, [=]() mutable {
                    promise.finish();
                });
            });
        });
    } else {
        // Skip further processing in the following cases:
        // 1. The message does not contain a trust message element.
        // 2. The trust message is sent by this endpoint and reflected via
        //    Message Carbons.
        promise.finish();
    }

    return promise.task();
}

///
/// Authenticates keys automatically by the content of a trust message.
///
/// \param encryption encryption protocol namespace
/// \param keyIds key owners' bare JIDs mapped to the IDs of their keys
///
QXmppTask<void> QXmppAtmManager::authenticate(const QString &encryption, const QMultiHash<QString, QByteArray> &keyIds)
{
    if (keyIds.isEmpty()) {
        return makeReadyTask();
    }

    QXmppPromise<void> promise;

    auto future = setTrustLevel(encryption, keyIds, TrustLevel::Authenticated);
    future.then(this, [=, this]() mutable {
        auto future = securityPolicy(encryption);
        future.then(this, [=, this](auto securityPolicy) mutable {
            if (securityPolicy == Toakafa) {
                auto future = distrustAutomaticallyTrustedKeys(encryption, keyIds.uniqueKeys());
                future.then(this, [=, this]() mutable {
                    auto future = makePostponedTrustDecisions(encryption, keyIds.values());
                    future.then(this, [=, this]() mutable {
                        promise.finish();
                    });
                });
            } else {
                auto future = makePostponedTrustDecisions(encryption, keyIds.values());
                future.then(this, [=, this]() mutable {
                    promise.finish();
                });
            }
        });
    });

    return promise.task();
}

///
/// Distrusts keys automatically by the content of a trust message.
///
/// \param encryption encryption protocol namespace
/// \param keyIds key owners' bare JIDs mapped to the IDs of their keys
///
QXmppTask<void> QXmppAtmManager::distrust(const QString &encryption, const QMultiHash<QString, QByteArray> &keyIds)
{
    if (keyIds.isEmpty()) {
        return makeReadyTask();
    }

    QXmppPromise<void> promise;

    auto future = setTrustLevel(encryption, keyIds, TrustLevel::ManuallyDistrusted);
    future.then(this, [=, this]() mutable {
        auto future = trustStorage()->removeKeysForPostponedTrustDecisions(encryption, keyIds.values());
        future.then(this, [=]() mutable {
            promise.finish();
        });
    });

    return promise.task();
}

///
/// Distrusts all formerly automatically trusted keys (as specifed by the
/// security policy TOAKAFA).
///
/// \param encryption encryption protocol namespace
/// \param keyOwnerJids bare JIDs of the key owners
///
QXmppTask<void> QXmppAtmManager::distrustAutomaticallyTrustedKeys(const QString &encryption, const QList<QString> &keyOwnerJids)
{
    return setTrustLevel(encryption, keyOwnerJids, TrustLevel::AutomaticallyTrusted, TrustLevel::AutomaticallyDistrusted);
}

///
/// Authenticates or distrusts keys for whom earlier trust messages were
/// received but not used for authenticating or distrusting at that time.
///
/// As soon as the senders' keys have been authenticated, all postponed trust
/// decisions can be performed by this method.
///
/// \param encryption encryption protocol namespace
/// \param senderKeyIds IDs of the keys that were used by the senders
///
QXmppTask<void> QXmppAtmManager::makePostponedTrustDecisions(const QString &encryption, const QList<QByteArray> &senderKeyIds)
{
    QXmppPromise<void> promise;

    auto future = trustStorage()->keysForPostponedTrustDecisions(encryption, senderKeyIds);
    future.then(this, [=, this](const QHash<bool, QMultiHash<QString, QByteArray>> &&keysForPostponedTrustDecisions) mutable {
        // JIDs of key owners mapped to the IDs of their keys
        const auto keysBeingAuthenticated = keysForPostponedTrustDecisions.value(true);
        const auto keysBeingDistrusted = keysForPostponedTrustDecisions.value(false);

        auto future = trustStorage()->removeKeysForPostponedTrustDecisions(encryption, keysBeingAuthenticated.values(), keysBeingDistrusted.values());
        future.then(this, [=, this]() mutable {
            auto future = makeTrustDecisions(encryption, keysBeingAuthenticated, keysBeingDistrusted);
            future.then(this, [=]() mutable {
                promise.finish();
            });
        });
    });

    return promise.task();
}

///
/// Sends a trust message.
///
/// \param encryption namespace of the encryption
/// \param keyOwners key owners containing the data for authentication or distrusting
/// \param recipientJid JID of the recipient
///
QXmppTask<QXmpp::SendResult> QXmppAtmManager::sendTrustMessage(const QString &encryption, const QList<QXmppTrustMessageKeyOwner> &keyOwners, const QString &recipientJid)
{
    QXmppTrustMessageElement trustMessageElement;
    trustMessageElement.setUsage(ns_atm.toString());
    trustMessageElement.setEncryption(encryption);
    trustMessageElement.setKeyOwners(keyOwners);

    QXmppMessage message;
    message.setTo(recipientJid);
    message.setTrustMessageElement(trustMessageElement);

    QXmppSendStanzaParams params;
    params.setAcceptedTrustLevels(TrustLevel::Authenticated);

    return client()->sendSensitive(std::move(message), params);
}
