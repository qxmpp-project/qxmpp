/*
 * Copyright (C) 2008-2021 The QXmpp developers
 *
 * Author:
 *  Melvin Keskin
 *  Linus Jahn
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

#include "QXmppRegistrationManager.h"

#include "QXmppClient.h"
#include "QXmppConstants_p.h"
#include "QXmppDiscoveryManager.h"
#include "QXmppRegisterIq.h"
#include "QXmppStreamFeatures.h"
#include "QXmppUtils.h"

#include <QDomElement>

class QXmppRegistrationManagerPrivate
{
public:
    QXmppRegistrationManagerPrivate();

    // whether to block login and request the registration form on connect
    bool registerOnConnectEnabled;
    // whether the server supports registration (after login)
    bool supportedByServer;

    // caching
    QString changePasswordIqId;
    QString newPassword;

    QString deleteAccountIqId;

    QString registrationIqId;
    QXmppRegisterIq registrationFormToSend;
};

QXmppRegistrationManagerPrivate::QXmppRegistrationManagerPrivate()
    : registerOnConnectEnabled(false),
      supportedByServer(false)
{
}

///
/// Default constructor.
///
QXmppRegistrationManager::QXmppRegistrationManager()
    : d(new QXmppRegistrationManagerPrivate)
{
}

QXmppRegistrationManager::~QXmppRegistrationManager() = default;

///
/// This adds the \c jabber:iq:register namespace to the features.
///
QStringList QXmppRegistrationManager::discoveryFeatures() const
{
    return QStringList {
        ns_register
    };
}

///
/// Changes the password of the user's account.
///
/// \note Be sure to only call this when any previous requests have finished.
///
/// \param newPassword The new password to be set. This must not be empty.
///
void QXmppRegistrationManager::changePassword(const QString &newPassword)
{
    auto iq = QXmppRegisterIq::createChangePasswordRequest(client()->configuration().user(), newPassword);

    d->changePasswordIqId = iq.id();
    d->newPassword = newPassword;

    client()->sendPacket(iq);
}

///
/// Cancels an existing registration on the server.
///
/// \sa accountDeleted()
/// \sa accountDeletionFailed()
///
void QXmppRegistrationManager::deleteAccount()
{
    auto iq = QXmppRegisterIq::createUnregistrationRequest();
    d->deleteAccountIqId = iq.id();

    client()->sendPacket(iq);
}

bool QXmppRegistrationManager::supportedByServer() const
{
    return d->supportedByServer;
}

///
/// Requests the registration form for registering.
///
/// \param service The service which the registration form should be requested
/// from. If left empty, this will default to the local server.
///
void QXmppRegistrationManager::requestRegistrationForm(const QString &service)
{
    QXmppRegisterIq iq;
    iq.setType(QXmppIq::Get);
    iq.setTo(service);
    client()->sendPacket(iq);
}

///
/// Sets a registration form to be sent on the next connect with the server.
/// \param iq The completed registration form.
///
void QXmppRegistrationManager::setRegistrationFormToSend(const QXmppRegisterIq &iq)
{
    d->registrationFormToSend = iq;
}

///
/// Sets a registration form to be sent on the next connect with the server.
/// \param dataForm The completed data form for registration.
///
void QXmppRegistrationManager::setRegistrationFormToSend(const QXmppDataForm &dataForm)
{
    d->registrationFormToSend = QXmppRegisterIq();
    d->registrationFormToSend.setForm(dataForm);
}

///
/// Sends a completed registration form that was previously set using
/// setRegistrationFormToSend().
///
/// You usually only need to set the form and the manager will automatically
/// send it when connected. More details can be found in the documentation of
/// the QXmppRegistrationManager.
///
void QXmppRegistrationManager::sendCachedRegistrationForm()
{
    if (auto form = d->registrationFormToSend.form(); !form.isNull()) {
        form.setType(QXmppDataForm::Submit);
        d->registrationFormToSend.setForm(form);
    }

    d->registrationFormToSend.setType(QXmppIq::Set);

    client()->sendPacket(d->registrationFormToSend);
    d->registrationIqId = d->registrationFormToSend.id();

    // clear cache
    d->registrationFormToSend = QXmppRegisterIq();
}

///
/// Returns whether to only request the registration form and not to connect
/// with username/password.
///
bool QXmppRegistrationManager::registerOnConnectEnabled() const
{
    return d->registerOnConnectEnabled;
}

///
/// Sets whether to only request the registration form and not to connect with
/// username/password.
///
/// \param enabled true to register, false to connect normally.
///
void QXmppRegistrationManager::setRegisterOnConnectEnabled(bool enabled)
{
    d->registerOnConnectEnabled = enabled;
}

/// \cond
bool QXmppRegistrationManager::handleStanza(const QDomElement &stanza)
{
    if (d->registerOnConnectEnabled && QXmppStreamFeatures::isStreamFeatures(stanza)) {
        QXmppStreamFeatures features;
        features.parse(stanza);

        if (features.registerMode() == QXmppStreamFeatures::Disabled) {
            warning(QStringLiteral("Could not request the registration form, because the server does not advertise the register stream feature."));
            client()->disconnectFromServer();
            emit registrationFailed({ QXmppStanza::Error::Cancel,
                                      QXmppStanza::Error::FeatureNotImplemented,
                                      QStringLiteral("The server does not advertise the register stream feature.") });
            return true;
        }

        if (!d->registrationFormToSend.form().isNull() || !d->registrationFormToSend.username().isNull()) {
            info(QStringLiteral("Sending completed form."));
            sendCachedRegistrationForm();
            return true;
        }

        info(QStringLiteral("Requesting registration form from server."));
        requestRegistrationForm();
        return true;
    }

    if (stanza.tagName() == "iq") {
        const QString &id = stanza.attribute(QStringLiteral("id"));

        if (!id.isEmpty() && id == d->registrationIqId) {
            QXmppIq iq;
            iq.parse(stanza);

            switch (iq.type()) {
            case QXmppIq::Result:
                info(QStringLiteral("Successfully registered with the service."));
                emit registrationSucceeded();
                break;
            case QXmppIq::Error:
                warning(QStringLiteral("Registering with the service failed: ").append(iq.error().text()));
                emit registrationFailed(iq.error());
                break;
            default:
                break;  // should never occur
            }

            d->registrationIqId.clear();
            return true;
        } else if (!id.isEmpty() && id == d->changePasswordIqId) {
            QXmppIq iq;
            iq.parse(stanza);

            switch (iq.type()) {
            case QXmppIq::Result:
                info(QStringLiteral("Changed password successfully."));
                client()->configuration().setPassword(d->newPassword);
                emit passwordChanged(d->newPassword);
                break;
            case QXmppIq::Error:
                warning(QStringLiteral("Failed to change password: ").append(iq.error().text()));
                emit passwordChangeFailed(iq.error());
                break;
            default:
                break;  // should never occur
            }

            d->changePasswordIqId.clear();
            d->newPassword.clear();
            return true;
        } else if (!id.isEmpty() && id == d->deleteAccountIqId) {
            QXmppIq iq;
            iq.parse(stanza);

            switch (iq.type()) {
            case QXmppIq::Result:
                info(QStringLiteral("Account deleted successfully."));
                emit accountDeleted();
                client()->disconnectFromServer();
                break;
            case QXmppIq::Error:
                warning(QStringLiteral("Failed to delete account: ").append(iq.error().text()));
                emit accountDeletionFailed(iq.error());
                break;
            default:
                break;  // should never occur
            }

            d->deleteAccountIqId.clear();
            return true;
        } else if (QXmppRegisterIq::isRegisterIq(stanza)) {
            QXmppRegisterIq iq;
            iq.parse(stanza);

            emit registrationFormReceived(iq);
        }
    }
    return false;
}
/// \endcond

void QXmppRegistrationManager::setClient(QXmppClient *client)
{
    QXmppClientExtension::setClient(client);
    // get service discovery manager
    auto *disco = client->findExtension<QXmppDiscoveryManager>();
    if (disco) {
        connect(disco, &QXmppDiscoveryManager::infoReceived, this, &QXmppRegistrationManager::handleDiscoInfo);
    }

    connect(client, &QXmppClient::disconnected, [=]() {
        setSupportedByServer(false);
    });
}

void QXmppRegistrationManager::handleDiscoInfo(const QXmppDiscoveryIq &iq)
{
    // check features of own server
    if (iq.from().isEmpty() || iq.from() == client()->configuration().domain()) {
        if (iq.features().contains(ns_register))
            setSupportedByServer(true);
    }
}

void QXmppRegistrationManager::setSupportedByServer(bool registrationSupported)
{
    if (d->supportedByServer != registrationSupported) {
        d->supportedByServer = registrationSupported;
        emit supportedByServerChanged();
    }
}
