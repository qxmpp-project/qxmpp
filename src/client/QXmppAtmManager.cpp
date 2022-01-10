// SPDX-FileCopyrightText: 2021 Melvin Keskin <melvo@olomono.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppAtmManager.h"

#include "QXmppCarbonManager.h"
#include "QXmppClient.h"
#include "QXmppConstants_p.h"
#include "QXmppFutureUtils_p.h"
#include "QXmppMessage.h"
#include "QXmppTrustMessageElement.h"
#include "QXmppTrustMessageKeyOwner.h"
#include "QXmppUtils.h"

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
{
    m_trustStorage = trustStorage;
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
QFuture<void> QXmppAtmManager::makeTrustDecisions(const QString &encryption, const QString &keyOwnerJid, const QList<QByteArray> &keyIdsForAuthentication, const QList<QByteArray> &keyIdsForDistrusting)
{
    auto interface = std::make_shared<QFutureInterface<void>>(QFutureInterfaceBase::Started);

    auto future = m_trustStorage->keys(encryption, QXmppTrustStorage::Authenticated | QXmppTrustStorage::ManuallyDistrusted);
    await(future, this, [=](const QHash<QXmppTrustStorage::TrustLevel, QMultiHash<QString, QByteArray>> &&keys) {
        const auto authenticatedKeys = keys.value(QXmppTrustStorage::Authenticated);
        const auto manuallyDistrustedKeys = keys.value(QXmppTrustStorage::ManuallyDistrusted);
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
            interface->reportFinished();
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
                await(future, this, [=]() {
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

                    interface->reportFinished();
                });
            } else {
                // Send a trust message for the keys of the contact's endpoints
                // being authenticated or distrusted to own endpoints
                // with authenticated keys.
                if (!ownAuthenticatedKeys.isEmpty()) {
                    sendTrustMessage(encryption, { keyOwner }, ownJid);
                }

                auto future = makeTrustDecisions(encryption, keysBeingAuthenticated, keysBeingDistrusted);
                await(future, this, [=]() {
                    // Send a trust message for own authenticated or distrusted
                    // keys to the contact's endpoints whose keys have been
                    // authenticated.
                    // It is skipped if no keys of contacts have been
                    // authenticated or there are no keys for the trust message.
                    if (!keyOwner.trustedKeys().isEmpty() && !(ownKeyOwner.trustedKeys().isEmpty() && ownKeyOwner.distrustedKeys().isEmpty())) {
                        sendTrustMessage(encryption, { ownKeyOwner }, keyOwnerJid);
                    }

                    interface->reportFinished();
                });
            }
        }
    });

    return interface->future();
}

/// \cond
bool QXmppAtmManager::handleStanza(const QDomElement &)
{
    return false;
}

void QXmppAtmManager::setClient(QXmppClient *client)
{
    QXmppClientExtension::setClient(client);
    connect(client, &QXmppClient::messageReceived, this, &QXmppAtmManager::handleMessageReceived);
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
QFuture<void> QXmppAtmManager::makeTrustDecisions(const QString &encryption, const QMultiHash<QString, QByteArray> &keyIdsForAuthentication, const QMultiHash<QString, QByteArray> &keyIdsForDistrusting)
{
    auto interface = std::make_shared<QFutureInterface<void>>(QFutureInterfaceBase::Started);

    auto future = authenticate(encryption, keyIdsForAuthentication);
    await(future, this, [=]() {
        auto future = distrust(encryption, keyIdsForDistrusting);
        await(future, this, [=]() {
            interface->reportFinished();
        });
    });

    return interface->future();
}

///
/// Handles incoming messages and uses included trust message elements for
/// making automatic trust decisions.
///
/// \param message message that can contain a trust message element
///
QFuture<void> QXmppAtmManager::handleMessage(const QXmppMessage &message)
{
    auto interface = std::make_shared<QFutureInterface<void>>(QFutureInterfaceBase::Started);

    if (const auto trustMessageElement = message.trustMessageElement(); trustMessageElement && trustMessageElement->usage() == ns_atm && message.from() != client()->configuration().jid()) {
        const auto senderJid = QXmppUtils::jidToBareJid(message.from());
        const auto senderKey = message.senderKey();
        const auto encryption = trustMessageElement->encryption();

        auto future = m_trustStorage->trustLevel(encryption, senderKey);
        await(future, this, [=](const auto &&senderKeyTrustLevel) {
            const auto isSenderKeyAuthenticated = senderKeyTrustLevel == QXmppTrustStorage::Authenticated;

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

            auto future = m_trustStorage->addKeysForPostponedTrustDecisions(encryption, senderKey, keyOwnersForPostponedTrustDecisions);
            await(future, this, [=]() {
                auto future = makeTrustDecisions(encryption, keysBeingAuthenticated, keysBeingDistrusted);
                await(future, this, [=]() {
                    interface->reportFinished();
                });
            });
        });
    } else {
        // Skip further processing in the following cases:
        // 1. The message does not contain a trust message element.
        // 2. The trust message is sent by this endpoint and reflected via
        //    Message Carbons.
        interface->reportFinished();
    }

    return interface->future();
}

///
/// Authenticates keys automatically by the content of a trust message.
///
/// \param encryption encryption protocol namespace
/// \param keyIds key owners' bare JIDs mapped to the IDs of their keys
///
QFuture<void> QXmppAtmManager::authenticate(const QString &encryption, const QMultiHash<QString, QByteArray> &keyIds)
{
    auto interface = std::make_shared<QFutureInterface<void>>(QFutureInterfaceBase::Started);

    if (keyIds.isEmpty()) {
        interface->reportFinished();
    } else {
        auto future = m_trustStorage->setTrustLevel(encryption, keyIds, QXmppTrustStorage::Authenticated);
        await(future, this, [=]() {
            await(m_trustStorage->securityPolicy(encryption), this, [=](const auto securityPolicy) {
                if (securityPolicy == QXmppTrustStorage::Toakafa) {
                    auto future = distrustAutomaticallyTrustedKeys(encryption, keyIds.uniqueKeys());
                    await(future, this, [=]() {
                        auto future = makePostponedTrustDecisions(encryption, keyIds.values());
                        await(future, this, [=]() {
                            interface->reportFinished();
                        });
                    });
                } else {
                    auto future = makePostponedTrustDecisions(encryption, keyIds.values());
                    await(future, this, [=]() {
                        interface->reportFinished();
                    });
                }
            });
        });
    }

    return interface->future();
}

///
/// Distrusts keys automatically by the content of a trust message.
///
/// \param encryption encryption protocol namespace
/// \param keyIds key owners' bare JIDs mapped to the IDs of their keys
///
QFuture<void> QXmppAtmManager::distrust(const QString &encryption, const QMultiHash<QString, QByteArray> &keyIds)
{
    auto interface = std::make_shared<QFutureInterface<void>>(QFutureInterfaceBase::Started);

    if (keyIds.isEmpty()) {
        interface->reportFinished();
    } else {
        auto future = m_trustStorage->setTrustLevel(encryption, keyIds, QXmppTrustStorage::ManuallyDistrusted);
        await(future, this, [=]() {
            auto future = m_trustStorage->removeKeysForPostponedTrustDecisions(encryption, keyIds.values());
            await(future, this, [=]() {
                interface->reportFinished();
            });
        });
    }

    return interface->future();
}

///
/// Distrusts all formerly automatically trusted keys (as specifed by the
/// security policy TOAKAFA).
///
/// \param encryption encryption protocol namespace
/// \param keyOwnerJids bare JIDs of the key owners
///
QFuture<void> QXmppAtmManager::distrustAutomaticallyTrustedKeys(const QString &encryption, const QList<QString> &keyOwnerJids)
{
    return m_trustStorage->setTrustLevel(encryption, keyOwnerJids, QXmppTrustStorage::AutomaticallyTrusted, QXmppTrustStorage::AutomaticallyDistrusted);
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
QFuture<void> QXmppAtmManager::makePostponedTrustDecisions(const QString &encryption, const QList<QByteArray> &senderKeyIds)
{
    auto interface = std::make_shared<QFutureInterface<void>>(QFutureInterfaceBase::Started);

    auto future = m_trustStorage->keysForPostponedTrustDecisions(encryption, senderKeyIds);
    await(future, this, [=](const QHash<bool, QMultiHash<QString, QByteArray>> &&keysForPostponedTrustDecisions) {
        // JIDs of key owners mapped to the IDs of their keys
        const auto keysBeingAuthenticated = keysForPostponedTrustDecisions.value(true);
        const auto keysBeingDistrusted = keysForPostponedTrustDecisions.value(false);

        auto future = m_trustStorage->removeKeysForPostponedTrustDecisions(encryption, keysBeingAuthenticated.values(), keysBeingDistrusted.values());
        await(future, this, [=]() {
            auto future = makeTrustDecisions(encryption, keysBeingAuthenticated, keysBeingDistrusted);
            await(future, this, [=]() {
                interface->reportFinished();
            });
        });
    });

    return interface->future();
}

///
/// Sends a trust message.
///
/// \param encryption namespace of the encryption
/// \param keyOwners key owners containing the data for authentication or distrusting
/// \param recipientJid JID of the recipient
///
QFuture<QXmpp::SendResult> QXmppAtmManager::sendTrustMessage(const QString &encryption, const QList<QXmppTrustMessageKeyOwner> &keyOwners, const QString &recipientJid)
{
    QXmppTrustMessageElement trustMessageElement;
    trustMessageElement.setUsage(ns_atm);
    trustMessageElement.setEncryption(encryption);
    trustMessageElement.setKeyOwners(keyOwners);

    QXmppMessage message;
    message.setTo(recipientJid);
    message.setTrustMessageElement(trustMessageElement);
    return client()->send(std::move(message));
}
