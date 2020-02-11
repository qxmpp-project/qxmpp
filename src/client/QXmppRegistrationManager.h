/*
 * Copyright (C) 2008-2020 The QXmpp developers
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

#ifndef QXMPPREGISTRATIONMANAGER_H
#define QXMPPREGISTRATIONMANAGER_H

#include "QXmppClientExtension.h"
#include "QXmppRegisterIq.h"

#include <QScopedPointer>

class QXmppRegistrationManagerPrivate;

///
/// \brief The QXmppRegistrationManager class manages in-band registration and
/// account management tasks like changing the password as defined in
/// \xep{0077}: In-Band Registration.
///
/// <h3 id="activation">Activating the manager</h3>
///
/// To make use of this manager, you need to instantiate it and load it into
/// the QXmppClient instance as follows:
///
/// \code
/// auto *registrationManager = new QXmppRegistrationManager;
/// client->addExtension(registrationManager);
/// \endcode
///
/// <h3>Setting up service discovery correctly for this manager</h3>
///
/// This manager automatically recognizes whether the local server supports
/// \xep{0077} (see supportedByServer()). You just need to request the service
/// discovery information from the server on connect as below:
///
/// \code
/// connect(client, &QXmppClient::connected, [=]() {
///     // The service discovery manager is added to the client by default.
///     auto *discoManager = client->findExtension<QXmppDiscoveryManager>();
///     discoManager->requestInfo(client->configuration().server());
/// });
/// \endcode
///
/// As soon as the result is retrieved, the supportedByServer() property should
/// be correct and could be used to display the user whether account management
/// tasks can be performed on this server.
///
/// However, this is not relevant if you only want to
/// <a href="#register-account">register a new account on a server</a>.
///
/// <h3>Changing the account's password</h3>
///
/// To change the password of the current account changePassword() can be used.
/// Upon that either passwordChanged() or passwordChangeFailed() is emitted.
///
/// If changing the password was successful, the new password is automatically
/// set in the QXmppClient::configuration(), so reconnecting works properly.
///
/// Example:
/// \code
/// auto *registrationManager = client->findExtension<QXmppRegistrationManager>();
/// connect(registrationManager, &QXmppRegistrationManager::passwordChanged, [=](const QString &newPassword) {
///     qDebug() << "Password changed to:" << newPassword;
/// });
/// connect(registrationManager, &QXmppRegistrationManager::passwordChangeFailed, [=](QXmppStanza::Error error) {
///     qDebug() << "Couldn't change the password:" << error.text();
/// });
///
/// registrationManager->changePassword(client->configuration().user(), "m1cr0$0ft");
/// \endcode
///
/// <h3>Unregistration with the server</h3>
///
/// If you want to delete your account on the server, you can do that using
/// deleteAccount(). When the result is received either accountDeleted() or
/// accountDeletionFailed() is emitted. In case it was successful the manager
/// automatically disconnects from the client.
///
/// \code
/// auto *registrationManager = client->findExtension<QXmppRegistrationManager>();
/// connect(registrationManager, &QXmppRegistrationManager::accountDeleted, [=]() {
///     qDebug() << "Account deleted successfull, the client is disconnecting now";
/// });
/// connect(registrationManager, &QXmppRegistrationManager::accountDeletionFailed, [=](QXmppStanza::Error error) {
///     qDebug() << "Couldn't delete account:" << error.text();
/// });
/// registrationManager->deleteAccount();
/// \endcode
///
/// <h3 id="register-account">Registering with a server</h3>
///
/// Registering with a server consists of multiple steps:
///   -# Requesting the registration form from the server
///   -# Filling out the registration form
///   -# Sending the completed form to the server
///     - On failure (e.g. because of a username conflict), the process
///       continues at step 1 again.
///   -# Connecting with the newly created account
///
/// <h4>Requesting the registration form from the server</h4>
///
/// First of all, you need to enable the registration process in the
/// registration manager, which of course needs to be <a href="#activation">
/// activated</a> in the client.
///
/// \code
/// auto *registrationManager = client->findExtension<QXmppRegistrationManager>();
/// registrationManager->setRegisterOnConnectEnabled(true);
/// \endcode
///
/// After that you can start to connect to the server you want to register
/// with. No JID is set in the QXmppConfiguration for the client and instead
/// only the server is set.
///
/// \code
/// QXmppConfiguration config;
/// config.setDomain("example.org");
///
/// client->connectToServer(config);
/// \endcode
///
/// Alternatively, you can also provide a domain-only JID and no password to
/// connectToServer():
///
/// \code
/// client->connectToServer("example.org", QString());
/// \endcode
///
/// Now as soon as (START)TLS was handled, the registration manager interrupts
/// the normal connection process. The manager checks whether the server
/// supports in-band registration and whether the server advertises this as a
/// stream feature.
///
/// If the server does not support in-band registration, the manager will abort
/// the connection at this point and emit the registrationFailed() signal with
/// a fixed QXmppStanza::Error of type QXmppStanza::Error::Cancel and with a
/// condition of QXmppStanza::Error::FeatureNotImplemented.
///
/// The manager will now request the registration form. This will either result
/// in an error (reported by registrationFailed()) or, if everything went well,
/// the registration form is reported by registrationFormReceived().
///
/// To handle everything correctly, you need to connect to both Q_SIGNALS:
///
/// \code
/// connect(registrationManager, &QXmppRegistrationManager::registrationFormReceived, [=](const QXmppRegisterIq &iq) {
///     qDebug() << "Form received:" << iq.instructions().
///     // you now need to complete the form
/// });
/// connect(registrationManager, &QXmppRegistrationManager::registrationFailed, [=](const QXmppStanza::Error &error) {
///     qDebug() << "Requesting the registration form failed:" << error.text();
/// });
/// \endcode
///
/// <h4>Filling out the registration form</h4>
///
/// Now you need to fill out the registration form. If this requires user
/// interaction, it is recommended that you disconnect from the server at this
/// point now. This is required, because some servers will kick inactive, not
/// authorized clients after a few seconds.
///
/// If the returned IQ contains a data form, that can be displayed to a user or
/// can be filled out in another way.
///
/// If the server does not support data forms, you can check the standard
/// fields of the QXmppRegisterIq. You need to search the fields for empty
/// (non-null) strings. All fields that contain an empty string are required
/// and can be filled out. You can just set values for those fields and send
/// the form as described in the next step.
///
/// \note QXmpp currently has only implemented the most important default
/// fields in the QXmppRegisterIq. The other fields are not very widespread,
/// because data forms are usually used for such purposes.
///
/// <h4>Sending the completed form to the server</h4>
///
/// <b>Option A</b>: If filling out the form goes very quick, you can set the
/// filled out form directly using setRegistrationFormToSend() and then
/// directly trigger the form to be sent using sendCachedRegistrationForm().
///
/// \code
/// registrationManager->setRegistrationFormToSend(completedForm);
/// registrationManager->sendCachedRegistrationForm();
/// \endcode
///
/// <b>Option B</b>: If filling out the form takes longer, i.e. because user
/// interaction is required, you should disconnect now. As soon as you have
/// completed the form, you can set it using setRegistrationFormToSend(). After
/// that you can reconnect to the server and the registration manager will
/// automatically send the set form.
///
/// \code
/// client->disconnectFromServer();
/// // user fills out form ...
/// registrationManager->setRegistrationFormToSend(completedForm);
///
/// // As before, you only need to provide a domain to connectToServer()
/// client->connectToServer(...);
/// // the registration manager sends the form automatically
/// \endcode
///
/// The form is now sent to the server. As soon as the result is received,
/// either registrationSucceeded() or registrationFailed() is emitted.
///
/// In case there was a conflict or another error, you should request a new
/// form and restart the process. This is especially important, if the form can
/// only be used once as with most CAPTCHA implementations.
///
/// <h4>Connecting with the newly created account</h4>
///
/// You need to disconnect now. The user can then enter their credentials and
/// connect as usually.
///
/// It is also possible to extract username and password from the sent form,
/// but that does not work always. There might also be forms that have no clear
/// username or password fields.
///
/// \ingroup Managers
///
/// \since QXmpp 1.2
///
class QXMPP_EXPORT QXmppRegistrationManager : public QXmppClientExtension
{
    Q_OBJECT

    /// Whether support of \xep{0077}: In-band Registration has been discovered on the server.
    Q_PROPERTY(bool supportedByServer READ supportedByServer NOTIFY supportedByServerChanged)

public:
    QXmppRegistrationManager();
    ~QXmppRegistrationManager();

    QStringList discoveryFeatures() const override;

    void changePassword(const QString &newPassword);
    void deleteAccount();

    // documentation needs to be here, see https://stackoverflow.com/questions/49192523/
    ///
    /// Returns whether the server supports registration.
    ///
    /// By default this is set to false and only changes, if you request the
    /// service discovery info of the connected server using
    /// QXmppDiscoveryManager::requestInfo().
    ///
    /// This is only relevant to actions that happen after authentication.
    ///
    /// \sa QXmppRegistrationManager::supportedByServerChanged()
    ///
    bool supportedByServer() const;

    void requestRegistrationForm(const QString &service = {});

    void setRegistrationFormToSend(const QXmppRegisterIq &iq);
    void setRegistrationFormToSend(const QXmppDataForm &dataForm);
    void sendCachedRegistrationForm();

    bool registerOnConnectEnabled() const;
    void setRegisterOnConnectEnabled(bool enabled);

    /// \cond
    bool handleStanza(const QDomElement &stanza) override;
    /// \endcond

Q_SIGNALS:
    ///
    /// Emitted, when registrationSupported() changed.
    ///
    /// This can happen after the service discovery info of the server was
    /// retrieved using QXmppDiscoveryManager::requestInfo() or on disconnect.
    ///
    void supportedByServerChanged();

    ///
    /// Emitted, when the password of the account was changed successfully.
    ///
    /// The new password is automatically set in QXmppClient::configuration().
    ///
    /// \param newPassword The new password that was set on the server.
    ///
    void passwordChanged(const QString &newPassword);

    ///
    /// Emitted, when changing the password did not succeed.
    ///
    /// \param error Error returned from the service.
    ///
    void passwordChangeFailed(QXmppStanza::Error error);

    ///
    /// Emitted, when a registration form has been received.
    ///
    /// When registering an account on the server and user interaction is
    /// required now to complete the form, it is recommended to disconnect and
    /// sending the completed registration form on reconnect using
    /// QXmppRegistrationManager::setRegistrationFormToSend(). Some servers
    /// (i.e. ejabberd) kick their clients after a timeout when they are not
    /// active. This can be avoided this way.
    ///
    /// \param iq The received form. If it does not contain a valid data form
    /// (see QXmppRegisterIq::form()), the required fields should be marked by
    /// empty (but not null) strings in the QXmppRegisterIq (i.e.
    /// QXmppRegisterIq::password().isNull() => false).
    ///
    void registrationFormReceived(const QXmppRegisterIq &iq);

    ///
    /// Emitted, when the account was deleted successfully.
    ///
    void accountDeleted();

    ///
    /// Emitted, when the account could not be deleted.
    ///
    void accountDeletionFailed(QXmppStanza::Error error);

    ///
    /// Emitted, when the registration with a service completed successfully.
    ///
    /// To connect with the account you still need to set the correct
    /// credentials in QXmppClient::configuration() and reconnect.
    ///
    void registrationSucceeded();

    ///
    /// Emitted, when the registration failed.
    ///
    /// \param error The returned error from the service. The reported errors
    /// might be different from server to server, but the common ones are the
    /// following:
    /// \li type=Cancel and condition=Conflict: The username already exists.
    /// \li type=Cancel and condition=NotAllowed: The CAPTCHA verification
    /// failed.
    /// \li type=Modify and condition=NotAcceptable: Some required information
    /// was missing or the selected password was too weak.
    /// \li type=Modify and condition=JidMalformed: No username was provided or
    /// the username has an illegal format.
    ///
    void registrationFailed(const QXmppStanza::Error &error);

protected:
    void setClient(QXmppClient *client) override;

private Q_SLOTS:
    void handleDiscoInfo(const QXmppDiscoveryIq &iq);

private:
    void setSupportedByServer(bool supportedByServer);

    QScopedPointer<QXmppRegistrationManagerPrivate> d;
};

#endif  // QXMPPREGISTRATIONMANAGER_H
