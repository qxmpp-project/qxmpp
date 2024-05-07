// SPDX-FileCopyrightText: 2024 Filipe Azevedo <pasnox@gmail.com>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppMovedManager.h"

#include "QXmppConstants_p.h"
#include "QXmppDiscoveryIq.h"
#include "QXmppDiscoveryManager.h"
#include "QXmppMovedItem.h"
#include "QXmppPubSubManager.h"
#include "QXmppTask.h"
#include "QXmppUtils.h"

#include <QUrl>

using namespace QXmpp::Private;

class QXmppMovedManagerPrivate
{
public:
    QXmppPubSubManager *pubSubManager = nullptr;
    QXmppDiscoveryManager *discoveryManager = nullptr;
    bool supportedByServer = false;
};

///
/// \class QXmppMovedManager
///
/// This class manages user account moving as specified in the following XEPs:
///     * \xep{0283, Moved}
///
/// In order to use this manager, make sure to add all managers needed by this manager:
/// \code
/// client->addNewExtension<QXmppDiscoveryManager>();
/// client->addNewExtension<QXmppPubSubManager>();
/// \endcode
///
/// Afterwards, you need to add this manager to the client:
/// \code
/// auto *manager = client->addNewExtension<QXmppMovedManager>();
/// \endcode
///
/// If you want to publish a moved statement use the publishStatement call with the old account:
/// \code
/// manager->publishStatement("new@example.org");
/// \endcode
///
/// Once you published your statement, you then need to subscribe to your old contacts with the new account:
/// \code
/// manager->notifyContact("contact@xmpp.example", "old@example.org", "Hey, I moved my account, please accept me.");
/// \endcode
///
/// When a contact receive a subscription request from a moved user he needs to verify the authenticity of the request.
/// The QXmppRosterManager handle it on its own if the client has the QXmppMovedManager extension available.
/// The request will be ignored entirely if the old jid incoming subscription is not part of the roster with a 'from' or 'both' type.
/// In case of the authenticity can't be established the request is ignored entirely. Alternatively, if the client
/// does not has QXmppMovedManager support the request message will be changed to introduce a warning message before emitting
/// the subscription{Request}Received signal.
///
/// \ingroup Managers
///
/// \since QXmpp 1.7
///

///
/// Constructs a \xep{0283, Moved} manager.
///
QXmppMovedManager::QXmppMovedManager()
    : d(new QXmppMovedManagerPrivate())
{
}

QXmppMovedManager::~QXmppMovedManager() = default;

QStringList QXmppMovedManager::discoveryFeatures() const
{
    return { ns_moved.toString() };
}

///
/// \property QXmppMovedManager::supportedByServer
///
/// \see QXmppMovedManager::supportedByServer()
///

///
/// Returns whether the own server supports \xep{0283, Moved} feature.
///
/// \return whether \xep{0283, Moved} feature is supported
///
bool QXmppMovedManager::supportedByServer() const
{
    return d->supportedByServer;
}

///
/// \fn QXmppMovedManager::supportedByServerChanged()
///
/// Emitted when the server enabled or disabled supporting \xep{0283, Moved} feature.
///

///
/// Publish a moved statement.
///
/// \param newBareJid JID of the new account
///
/// \return the result of the action
///
QXmppTask<QXmppClient::EmptyResult> QXmppMovedManager::publishStatement(const QString &newBareJid)
{
    return chainSuccess(d->pubSubManager->publishOwnPepItem(ns_moved.toString(), QXmppMovedItem { newBareJid }), this);
}

///
/// Verify a user moved statement.
///
/// \param oldBareJid JID of the old account to check statement
/// \param newBareJid JID of the new account that send the subscription request
///
/// \return the result of the action
///
QXmppTask<QXmppClient::EmptyResult> QXmppMovedManager::verifyStatement(const QString &oldBareJid, const QString &newBareJid)
{
    return chain<QXmppClient::EmptyResult>(d->pubSubManager->requestItem<QXmppMovedItem>(oldBareJid, ns_moved.toString(), QStringLiteral("current")),
                                           this,
                                           [=, this](QXmppPubSubManager::ItemResult<QXmppMovedItem> &&result) {
                                               return std::visit(overloaded {
                                                                     [=, this](QXmppMovedItem item) -> QXmppClient::EmptyResult {
                                                                         return movedJidsMatch(newBareJid, item.newJid());
                                                                     },
                                                                     [=, this](QXmppError err) -> QXmppClient::EmptyResult {
                                                                         // As a special case, if the attempt to retrieve the moved statement results in an error with the <gone/> condition
                                                                         // as defined in RFC 6120, and that <gone/> element contains a valid XMPP URI (e.g. xmpp:user@example.com), then the
                                                                         // error response MUST be handled equivalent to a <moved/> statement containing a <new-jid/> element with the JID
                                                                         // provided in the URI (e.g. user@example.com).
                                                                         if (err.isStanzaError()) {
                                                                             const auto e = std::any_cast<QXmppStanza::Error>(err.error);
                                                                             const auto newJid = [&e]() -> QString {
                                                                                 if (e.condition() != QXmppStanza::Error::Condition::Gone) {
                                                                                     return {};
                                                                                 }

                                                                                 // TODO: Move Kaidan QXmppUri to QXmpp library
                                                                                 QUrl url(e.redirectionUri());

                                                                                 if (!url.isValid() || url.scheme() != u"xmpp") {
                                                                                     return {};
                                                                                 }

                                                                                 return url.path();
                                                                             }();

                                                                             if (!newJid.isEmpty()) {
                                                                                 return movedJidsMatch(newBareJid, newJid);
                                                                             }
                                                                         }

                                                                         return err;
                                                                     } },
                                                                 std::move(result));
                                           });
}

///
/// Notify a contact we have moved
///
/// \param contactBareJid JID of the contact to send the subscription request
/// \param oldBareJid JID of the old account we moved from
/// \param reason The reason of the move
///
/// \return the result of the action
///
QXmppTask<QXmpp::SendResult> QXmppMovedManager::notifyContact(const QString &contactBareJid, const QString &oldBareJid, const QString &reason)
{
    QXmppPresence packet;
    packet.setTo(QXmppUtils::jidToBareJid(contactBareJid));
    packet.setType(QXmppPresence::Subscribe);
    packet.setStatusText(reason);
    packet.setOldJid(oldBareJid);
    return client()->sendSensitive(std::move(packet));
}

/// \cond
void QXmppMovedManager::onRegistered(QXmppClient *client)
{
    connect(client, &QXmppClient::connected, this, [this, client]() {
        if (client->streamManagementState() == QXmppClient::NewStream) {
            resetCachedData();
        }
    });

    d->discoveryManager = client->findExtension<QXmppDiscoveryManager>();
    Q_ASSERT_X(d->discoveryManager, "QXmppMovedManager", "QXmppDiscoveryManager is missing");

    connect(d->discoveryManager, &QXmppDiscoveryManager::infoReceived, this, &QXmppMovedManager::handleDiscoInfo);

    d->pubSubManager = client->findExtension<QXmppPubSubManager>();
    Q_ASSERT_X(d->pubSubManager, "QXmppMovedManager", "QXmppPubSubManager is missing");
}

void QXmppMovedManager::onUnregistered(QXmppClient *client)
{
    disconnect(d->discoveryManager, &QXmppDiscoveryManager::infoReceived, this, &QXmppMovedManager::handleDiscoInfo);
    resetCachedData();
    disconnect(client, &QXmppClient::connected, this, nullptr);
}
/// \endcond

///
/// Handles incoming service infos specified by \xep{0030, Service Discovery}
///
/// \param iq received Service Discovery IQ stanza
///
void QXmppMovedManager::handleDiscoInfo(const QXmppDiscoveryIq &iq)
{
    // Check the server's functionality to support MOVED feature.
    if (iq.from().isEmpty() || iq.from() == client()->configuration().domain()) {
        // Check whether MOVED is supported.
        setSupportedByServer(iq.features().contains(ns_moved));
    }
}

///
/// Ensure both JIDs match
///
/// \param newBareJid JID of the contact that sent the subscription request
/// \param pepBareJid JID of the new account as fetched from the old account statement
///
/// \return the result of the action
///
QXmppClient::EmptyResult QXmppMovedManager::movedJidsMatch(const QString &newBareJid, const QString &pepBareJid) const
{
    if (newBareJid == pepBareJid) {
        return QXmpp::Success {};
    }

    return QXmppError { QStringLiteral("The JID does not match user statement."), {} };
}

///
/// Sets whether the own server supports \xep{0283, Moved}.
///
/// \param supportedByServer whether \xep{0283, Moved} is supported by the own server
///
void QXmppMovedManager::setSupportedByServer(bool supportedByServer)
{
    if (d->supportedByServer != supportedByServer) {
        d->supportedByServer = supportedByServer;
        Q_EMIT supportedByServerChanged();
    }
}

///
/// Resets the cached data.
///
void QXmppMovedManager::resetCachedData()
{
    setSupportedByServer(false);
}
