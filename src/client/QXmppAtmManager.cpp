/*
 * Copyright (C) 2008-2021 The QXmpp developers
 *
 * Author:
 *  Melvin Keskin <melvo@olomono.de>
 *
 * Source:
 *  https://github.com/qxmpp-project/qxmpp
 *
 * This file is a part of QXmpp library.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 */

#include "QXmppAtmManager.h"

#include "QXmppCarbonManager.h"
#include "QXmppClient.h"
#include "QXmppConstants_p.h"
#include "QXmppFutureUtils_p.h"
#include "QXmppTrustMessageElement.h"
#include "QXmppUtils.h"

#include <QDomElement>
#include <QList>

///
/// \class QXmppAtmManager
///
/// \brief The QXmppAtmManager class represents a manager for
/// \xep{0450, Automatic Trust Management (ATM)}.
///
/// It is strongly recommended to enable \xep{0280, Message Carbons} with
/// \code
/// QXmppCarbonManager *manager = new QXmppCarbonManager;
/// manager->setCarbonsEnabled(true)
/// client->addExtension(manager);
/// \endcode
/// and \xep{0313, Message Archive Management} with
/// \code
/// QXmppMamManager *manager = new QXmppMamManager;
/// client->addExtension(manager);
/// \endcode
/// for delivering trust messages to all online and offline endpoints.
///
/// \ingroup Managers
///
/// \since QXmpp 1.5
///

///
/// Constructs an ATM manager.
///
/// @param trustStorage trust storage implementation
///
QXmppAtmManager::QXmppAtmManager(QXmppTrustStorage *trustStorage)
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
void QXmppAtmManager::makeTrustDecisions(const QString &encryption, const QString &keyOwnerJid, const QList<QString> &keyIdsForAuthentication, const QList<QString> &keyIdsForDistrusting)
{
    QXmppTrustMessageKeyOwner keyOwner;
    keyOwner.setJid(keyOwnerJid);
    keyOwner.setTrustedKeys(keyIdsForAuthentication);
    keyOwner.setDistrustedKeys(keyIdsForDistrusting);

    const auto future = m_trustStorage->authenticatedKeys(encryption);
    QXmpp::Private::await(future, [=](const QMultiHash<QString, QString> &&authenticatedKeys) {
        const auto ownJid = client()->configuration().jidBare();
        const auto ownAuthenticatedKeys = authenticatedKeys.values(ownJid);

        const auto future = m_trustStorage->unauthenticatedKeys(encryption);
        QXmpp::Private::await(future, [=](const QMultiHash<QString, QString> &&unauthenticatedKeys) {
            // Create a key owner for authenticated and distrusted keys of own
            // endpoints.
            QXmppTrustMessageKeyOwner ownKeyOwner;
            ownKeyOwner.setJid(ownJid);
            if (!ownAuthenticatedKeys.isEmpty()) {
                ownKeyOwner.setTrustedKeys(ownAuthenticatedKeys);
            }
            const auto ownUnauthenticatedKeys = unauthenticatedKeys.values(ownJid);
            if (!ownUnauthenticatedKeys.isEmpty()) {
                ownKeyOwner.setDistrustedKeys(ownUnauthenticatedKeys);
            }

            const auto areOwnKeysProcessed = keyOwnerJid == ownJid;
            if (areOwnKeysProcessed) {
                auto authenticatedKeysTemp = authenticatedKeys;
                authenticatedKeysTemp.remove(ownJid);
                const auto contactsAuthenticatedKeys = authenticatedKeysTemp;

                // Send trust messages for the keys of the own endpoints being
                // authenticated or distrusted to endpoints of contacts with
                // authenticated keys.
                // Own endpoints with authenticated keys can receive the trust
                // messages via Message Carbons.
                for (auto contactJid : contactsAuthenticatedKeys.uniqueKeys()) {
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

                authenticate(encryption, keyOwnerJid, keyOwner.trustedKeys());
                distrust(encryption, keyOwnerJid, keyOwner.distrustedKeys());

                // Send a trust message for all authenticated or distrusted keys
                // to the own endpoints whose keys have been authenticated.
                // It is skipped if no keys of own endpoints have been
                // authenticated.
                if (!keyOwner.trustedKeys().isEmpty()) {
                    auto unauthenticatedKeysTemp = unauthenticatedKeys;
                    unauthenticatedKeysTemp.remove(ownJid);
                    const auto contactsUnauthenticatedKeys = unauthenticatedKeysTemp;

                    QList<QXmppTrustMessageKeyOwner> contactsKeyOwners;

                    for (const auto &contactJid : contactsAuthenticatedKeys.uniqueKeys()) {
                        QXmppTrustMessageKeyOwner contactKeyOwner;
                        contactKeyOwner.setJid(contactJid);
                        contactKeyOwner.setTrustedKeys(contactsAuthenticatedKeys.values(contactJid));
                        if (const auto contactUnauthenticatedKeys = contactsUnauthenticatedKeys.values(contactJid); !contactUnauthenticatedKeys.isEmpty()) {
                            contactKeyOwner.setDistrustedKeys(contactUnauthenticatedKeys);
                        }

                        contactsKeyOwners.append(contactKeyOwner);
                    }

                    auto allKeyOwners = contactsKeyOwners;
                    if (!ownAuthenticatedKeys.isEmpty() || !ownUnauthenticatedKeys.isEmpty()) {
                        allKeyOwners.append(ownKeyOwner);
                    }
                    if (!allKeyOwners.isEmpty()) {
                        sendTrustMessage(encryption, allKeyOwners, ownJid);
                    }
                }
            } else {
                // Send a trust message for the keys of the contact's endpoints
                // being authenticated or distrusted to own endpoints
                // with authenticated keys.
                if (!ownAuthenticatedKeys.isEmpty()) {
                    sendTrustMessage(encryption, { keyOwner }, ownJid);
                }

                authenticate(encryption, keyOwnerJid, keyOwner.trustedKeys());
                distrust(encryption, keyOwnerJid, keyOwner.distrustedKeys());

                // Send a trust message for own authenticated or distrusted keys
                // to the contact's endpoints whose keys have been
                // authenticated.
                // It is skipped if no keys of contacts have been
                // authenticated.
                if (!keyOwner.trustedKeys().isEmpty()) {
                    sendTrustMessage(encryption, { ownKeyOwner }, keyOwnerJid);
                }
            }
        });
    });
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
    // Skip own trust messages reflected via Message Carbons.
    if (message.from() == client()->configuration().jid()) {
        return;
    }

    if (const auto optionalSenderKey = message.senderKey()) {
        const auto senderKey = optionalSenderKey.value();

        if (const auto trustMessageElement = message.trustMessageElement(); trustMessageElement->usage() == ns_atm) {
            const auto senderJid = QXmppUtils::jidToBareJid(message.from());
            const auto ownJid = client()->configuration().jidBare();
            const auto isOwnTrustMessage = senderJid == ownJid;

            for (const auto &keyOwner : trustMessageElement->keyOwners()) {
                const auto keyOwnerJid = keyOwner.jid();

                // A trust message from an own endpoint may authenticate or
                // distrust the keys of own endpoints and endpoints of contacts.
                // Whereas a trust message from an endpoint of a contact may
                // only authenticate or distrust the keys of that contact's own
                // endpoints.
                const auto isSenderQualifiedForTrustDecisions = isOwnTrustMessage || senderJid == keyOwnerJid;

                if (isSenderQualifiedForTrustDecisions) {
                    const auto encryption = trustMessageElement->encryption();
                    const auto keysBeingAuthenticated = keyOwner.trustedKeys();
                    const auto keysBeingDistrusted = keyOwner.distrustedKeys();

                    const auto future = m_trustStorage->trustLevel(encryption, keyOwnerJid, senderKey);
                    QXmpp::Private::await(future, [&](const auto &&senderKeyTrustLevel) {
                        if (senderKeyTrustLevel == QXmppTrustStorage::Authenticated) {
                            authenticate(encryption, keyOwnerJid, keysBeingAuthenticated);
                            distrust(encryption, keyOwnerJid, keysBeingDistrusted);
                        } else {
                            // Store the key IDs received from endpoints with
                            // not yet authenticated keys for authenticating or
                            // distrusting the keys later.
                            m_trustStorage->addKeysForLaterTrustDecisions(encryption, senderKey, keyOwnerJid, keysBeingAuthenticated, keysBeingDistrusted);
                        }
                    });
                }
            }
        }
    }
}
/// \endcond

///
/// Authenticates keys automatically by the content of a trust message.
///
/// \param encryption encryption protocol namespace
/// \param keyOwnerJid JID of the key owner
/// \param keyIds IDs of the keys being authenticated
///
void QXmppAtmManager::authenticate(const QString &encryption, const QString &keyOwnerJid, const QList<QString> &keyIds)
{
    m_trustStorage->setTrustLevel(encryption, keyOwnerJid, keyIds, QXmppTrustStorage::Authenticated);
    distrustAutomaticallyTrustedKeys(encryption, keyOwnerJid);
    makeUpForTrustDecision(encryption, keyIds);
}

///
/// Distrusts keys automatically by the content of a trust message.
///
/// \param encryption encryption protocol namespace
/// \param keyOwnerJid JID of the key owner
/// \param keyIds IDs of the keys being distrusted
///
void QXmppAtmManager::distrust(const QString &encryption, const QString &keyOwnerJid, const QList<QString> &keyIds)
{
    m_trustStorage->setTrustLevel(encryption, keyOwnerJid, keyIds, QXmppTrustStorage::ManuallyDistrusted);
    m_trustStorage->removeKeysForLaterTrustDecisions(encryption, keyIds);
}

///
/// Distrusts all formerly automatically trusted keys (as specifed by the
/// security policy TOAKAFA).
///
/// \param encryption encryption protocol namespace
/// \param keyOwnerJid JID of the key owner
///
void QXmppAtmManager::distrustAutomaticallyTrustedKeys(const QString &encryption, const QString &keyOwnerJid)
{
    const auto future = m_trustStorage->keys(encryption, keyOwnerJid, QXmppTrustStorage::AutomaticallyTrusted);
    QXmpp::Private::await(future, [=](const auto &&keyOwnerAutomaticallyTrustedKeys) {
        m_trustStorage->setTrustLevel(encryption, keyOwnerJid, keyOwnerAutomaticallyTrustedKeys, QXmppTrustStorage::AutomaticallyDistrusted);
    });
}

///
/// Authenticates or distrusts keys for whom earlier trust messages were
/// received but not used for authenticating or distrusting at that time.
///
/// As soon as the senders' keys have been authenticated, all trust decisions
/// specified by the data of their trust messages stored by
/// QXmppTrustStorage::addKeyIdsForLaterTrustDecisions can be performed by this
/// method.
///
/// \param encryption encryption protocol namespace
/// \param senderKeyIds IDs of the keys that were used by the senders
///
void QXmppAtmManager::makeUpForTrustDecision(const QString &encryption, const QList<QString> &senderKeyIds)
{
    QList<QString> keysForTrustDecision;

    for (auto trust : { true, false }) {
        const auto future = m_trustStorage->keysForLaterTrustDecisions(encryption, senderKeyIds, trust);
        QXmpp::Private::await(future, [&](const auto &&keysForLaterTrustDecisions) {
            for (auto keyOwnerJid : keysForLaterTrustDecisions.uniqueKeys()) {
                for (auto keyId : keysForLaterTrustDecisions.values(keyOwnerJid)) {
                    keysForTrustDecision.append(keyId);
                }

                if (trust) {
                    authenticate(encryption, keyOwnerJid, keysForTrustDecision);
                } else {
                    distrust(encryption, keyOwnerJid, keysForTrustDecision);
                }
            }

            m_trustStorage->removeKeysForLaterTrustDecisions(encryption, keysForLaterTrustDecisions.values(), trust);
        });
    }
}

///
/// Sends a trust message.
///
/// \param encryption namespace of the encryption
/// \param keyOwners key owners containing the data for authentication or distrusting
/// \param recipientJid JID of the recipient
///
void QXmppAtmManager::sendTrustMessage(const QString &encryption, const QList<QXmppTrustMessageKeyOwner> &keyOwners, const QString &recipientJid)
{
    QXmppTrustMessageElement trustMessageElement;
    trustMessageElement.setUsage(ns_atm);
    trustMessageElement.setEncryption(encryption);
    trustMessageElement.setKeyOwners(keyOwners);

    QXmppMessage message;
    message.setTo(recipientJid);
    message.setTrustMessageElement(trustMessageElement);
    client()->sendPacket(message);
}
