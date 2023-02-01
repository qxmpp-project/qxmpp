// SPDX-FileCopyrightText: 2009 Manjeet Dahiya <manjeetdahiya@gmail.com>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppClient.h"

#include "QXmppClientExtension.h"
#include "QXmppClient_p.h"
#include "QXmppConstants_p.h"
#include "QXmppDiscoveryIq.h"
#include "QXmppDiscoveryManager.h"
#include "QXmppE2eeExtension.h"
#include "QXmppE2eeMetadata.h"
#include "QXmppEntityTimeManager.h"
#include "QXmppFutureUtils_p.h"
#include "QXmppLogger.h"
#include "QXmppMessage.h"
#include "QXmppMessageHandler.h"
#include "QXmppOutgoingClient.h"
#include "QXmppPacket_p.h"
#include "QXmppPromise.h"
#include "QXmppRosterManager.h"
#include "QXmppTask.h"
#include "QXmppTlsManager_p.h"
#include "QXmppUtils.h"
#include "QXmppVCardManager.h"
#include "QXmppVersionManager.h"

#include <QDomElement>
#include <QSslSocket>
#include <QTimer>

using namespace QXmpp::Private;
using MessageEncryptResult = QXmppE2eeExtension::MessageEncryptResult;
using IqEncryptResult = QXmppE2eeExtension::IqEncryptResult;
using IqDecryptResult = QXmppE2eeExtension::IqDecryptResult;

static bool isIqResponse(const QDomElement &el)
{
    auto type = el.attribute("type");
    return el.tagName() == "iq" && (type == "result" || type == "error");
}

/// \cond
QXmppClientPrivate::QXmppClientPrivate(QXmppClient *qq)
    : clientPresence(QXmppPresence::Available),
      logger(nullptr),
      stream(nullptr),
      encryptionExtension(nullptr),
      receivedConflict(false),
      reconnectionTries(0),
      reconnectionTimer(nullptr),
      isActive(true),
      q(qq)
{
}

void QXmppClientPrivate::addProperCapability(QXmppPresence &presence)
{
    auto *ext = q->findExtension<QXmppDiscoveryManager>();
    if (ext) {
        presence.setCapabilityHash("sha-1");
        presence.setCapabilityNode(ext->clientCapabilitiesNode());
        presence.setCapabilityVer(ext->capabilities().verificationString());
    }
}

int QXmppClientPrivate::getNextReconnectTime() const
{
    if (reconnectionTries < 5) {
        return 10 * 1000;
    } else if (reconnectionTries < 10) {
        return 20 * 1000;
    } else if (reconnectionTries < 15) {
        return 40 * 1000;
    } else {
        return 60 * 1000;
    }
}

QStringList QXmppClientPrivate::discoveryFeatures()
{
    return {
        // XEP-0004: Data Forms
        ns_data,
        // XEP-0059: Result Set Management
        ns_rsm,
        // XEP-0066: Out of Band Data
        ns_oob,
        // XEP-0071: XHTML-IM
        ns_xhtml_im,
        // XEP-0085: Chat State Notifications
        ns_chat_states,
        // XEP-0115: Entity Capabilities
        ns_capabilities,
        // XEP-0199: XMPP Ping
        ns_ping,
        // XEP-0249: Direct MUC Invitations
        ns_conference,
        // XEP-0308: Last Message Correction
        ns_message_correct,
        // XEP-0333: Chat Markers
        ns_chat_markers,
        // XEP-0334: Message Processing Hints
        ns_message_processing_hints,
        // XEP-0359: Unique and Stable Stanza IDs
        ns_sid,
        // XEP-0367: Message Attaching
        ns_message_attaching,
        // XEP-0380: Explicit Message Encryption
        ns_eme,
        // XEP-0382: Spoiler messages
        ns_spoiler,
        // XEP-0428: Fallback Indication
        ns_fallback_indication,
        // XEP-0444: Message Reactions
        ns_reactions,
    };
}
/// \endcond

namespace QXmpp::Private::StanzaPipeline {

bool process(const QList<QXmppClientExtension *> &extensions, const QDomElement &element, const std::optional<QXmppE2eeMetadata> &e2eeMetadata)
{
    const bool unencrypted = !e2eeMetadata.has_value();
    for (auto *extension : extensions) {
        // e2e encrypted stanzas are not passed to the old handleStanza() overload, because such
        // managers are likely not handling the encrypted contents correctly (e.g. sending
        // unencrypted replies and thereby leaking information).
        if (extension->handleStanza(element, e2eeMetadata) ||
            (unencrypted && extension->handleStanza(element))) {
            return true;
        }
    }
    return false;
}

}  // namespace QXmpp::Private::StanzaPipeline

namespace QXmpp::Private::MessagePipeline {

bool process(QXmppClient *client, const QList<QXmppClientExtension *> &extensions, QXmppMessage &&message)
{
    for (auto *extension : extensions) {
        if (auto *messageHandler = dynamic_cast<QXmppMessageHandler *>(extension)) {
            if (messageHandler->handleMessage(message)) {
                return true;
            }
        }
    }
    return false;
}

bool process(QXmppClient *client, const QList<QXmppClientExtension *> &extensions, const QDomElement &element)
{
    if (element.tagName() != "message") {
        return false;
    }
    QXmppMessage message;
    message.parse(element);
    return process(client, extensions, std::move(message));
}

}  // namespace QXmpp::Private::MessagePipeline

///
/// \typedef QXmppClient::IqResult
///
/// Result of an IQ request, either contains the QDomElement of the IQ answer
/// (with type 'error' or 'result') or it contains the packet error, if the
/// request couldn't be sent.
///
/// \since QXmpp 1.5
///

///
/// \typedef QXmppClient::EmptyResult
///
/// Result of a generic request without a return value. Contains Success in case
/// everything went well. If the returned IQ contained an error a
/// QXmppStanza::Error is reported.
///
/// \since QXmpp 1.5
///

/// Creates a QXmppClient object.
/// \param parent is passed to the QObject's constructor.
/// The default value is 0.

QXmppClient::QXmppClient(QObject *parent)
    : QXmppLoggable(parent),
      d(new QXmppClientPrivate(this))
{

    d->stream = new QXmppOutgoingClient(this);
    d->addProperCapability(d->clientPresence);

    connect(d->stream, &QXmppOutgoingClient::elementReceived,
            this, &QXmppClient::_q_elementReceived);

    connect(d->stream, &QXmppOutgoingClient::messageReceived,
            this, &QXmppClient::messageReceived);

    connect(d->stream, &QXmppOutgoingClient::presenceReceived,
            this, &QXmppClient::presenceReceived);

    connect(d->stream, &QXmppOutgoingClient::iqReceived,
            this, &QXmppClient::iqReceived);

    connect(d->stream, &QXmppOutgoingClient::sslErrors,
            this, &QXmppClient::sslErrors);

    connect(d->stream->socket(), &QAbstractSocket::stateChanged,
            this, &QXmppClient::_q_socketStateChanged);

    connect(d->stream, &QXmppStream::connected,
            this, &QXmppClient::_q_streamConnected);

    connect(d->stream, &QXmppStream::disconnected,
            this, &QXmppClient::_q_streamDisconnected);

    connect(d->stream, &QXmppOutgoingClient::error,
            this, &QXmppClient::_q_streamError);

    // reconnection
    d->reconnectionTimer = new QTimer(this);
    d->reconnectionTimer->setSingleShot(true);
    connect(d->reconnectionTimer, &QTimer::timeout,
            this, &QXmppClient::_q_reconnect);

    // logging
    setLogger(QXmppLogger::getLogger());

    addExtension(new QXmppTlsManager);
    addExtension(new QXmppRosterManager(this));
    addExtension(new QXmppVCardManager);
    addExtension(new QXmppVersionManager);
    addExtension(new QXmppEntityTimeManager());
    addExtension(new QXmppDiscoveryManager());
}

QXmppClient::~QXmppClient() = default;

///
/// \fn QXmppClient::addNewExtension()
///
/// Creates a new extension and adds it to the client.
///
/// \returns the newly created extension
///
/// \since QXmpp 1.5
///

/// Registers a new \a extension with the client.
///
/// \param extension

bool QXmppClient::addExtension(QXmppClientExtension *extension)
{
    return insertExtension(d->extensions.size(), extension);
}

/// Registers a new \a extension with the client at the given \a index.
///
/// \param index
/// \param extension

bool QXmppClient::insertExtension(int index, QXmppClientExtension *extension)
{
    if (d->extensions.contains(extension)) {
        qWarning("Cannot add extension, it has already been added");
        return false;
    }

    extension->setParent(this);
    extension->setClient(this);
    d->extensions.insert(index, extension);
    return true;
}

/// Unregisters the given extension from the client. If the extension
/// is found, it will be destroyed.
///
/// \param extension

bool QXmppClient::removeExtension(QXmppClientExtension *extension)
{
    if (d->extensions.contains(extension)) {
        d->extensions.removeAll(extension);
        delete extension;
        return true;
    } else {
        qWarning("Cannot remove extension, it was never added");
        return false;
    }
}

///
/// Returns the currently used encryption extension.
///
/// \since QXmpp 1.5
///
QXmppE2eeExtension *QXmppClient::encryptionExtension() const
{
    return d->encryptionExtension;
}

///
/// Sets the extension to be used for end-to-end-encryption.
///
/// \since QXmpp 1.5
///
void QXmppClient::setEncryptionExtension(QXmppE2eeExtension *extension)
{
    d->encryptionExtension = extension;
}

/// Returns a list containing all the client's extensions.
///

QList<QXmppClientExtension *> QXmppClient::extensions()
{
    return d->extensions;
}

/// Returns a modifiable reference to the current configuration of QXmppClient.
/// \return Reference to the QXmppClient's configuration for the connection.

QXmppConfiguration &QXmppClient::configuration()
{
    return d->stream->configuration();
}

///
/// Attempts to connect to the XMPP server. Server details and other configurations
/// are specified using the config parameter. Use signals connected(), error(QXmppClient::Error)
/// and disconnected() to know the status of the connection.
///
/// \param config Specifies the configuration object for connecting the XMPP server.
/// This contains the host name, user, password etc. See QXmppConfiguration for details.
/// \param initialPresence The initial presence which will be set for this user
/// after establishing the session. The default value is QXmppPresence::Available
///
void QXmppClient::connectToServer(const QXmppConfiguration &config,
                                  const QXmppPresence &initialPresence)
{
    // reset package cache from last connection
    if (d->stream->configuration().jidBare() != config.jidBare()) {
        d->stream->resetPacketCache();
    }

    d->stream->configuration() = config;
    d->clientPresence = initialPresence;
    d->addProperCapability(d->clientPresence);

    d->stream->connectToHost();
}

/// Overloaded function to simply connect to an XMPP server with a JID and password.
///
/// \param jid JID for the account.
/// \param password Password for the account.

void QXmppClient::connectToServer(const QString &jid, const QString &password)
{
    QXmppConfiguration config;
    config.setJid(jid);
    config.setPassword(password);
    connectToServer(config);
}

///
/// After successfully connecting to the server use this function to send
/// stanzas to the server. This function can solely be used to send various kind
/// of stanzas to the server. QXmppStanza is a parent class of all the stanzas
/// QXmppMessage, QXmppPresence, QXmppIq, QXmppBind, QXmppRosterIq, QXmppSession
/// and QXmppVCard.
///
/// This function does not end-to-end encrypt the packets.
///
/// \return Returns true if the packet was sent, false otherwise.
///
/// Following code snippet illustrates how to send a message using this function:
/// \code
/// QXmppMessage message(from, to, message);
/// client.sendPacket(message);
/// \endcode
///
/// \param packet A valid XMPP stanza. It can be an iq, a message or a presence stanza.
///
bool QXmppClient::sendPacket(const QXmppNonza &packet)
{
    return d->stream->sendPacket(packet);
}

///
/// Sends a packet and reports the result via QXmppTask.
///
/// If stream management is enabled, the task continues to be active until the
/// server acknowledges the packet. On success, QXmpp::SendSuccess with
/// acknowledged == true is reported and the task finishes.
///
/// If connection errors occur, the packet is resent if possible. If
/// reconnecting is not possible, an error is reported.
///
/// \warning THIS API IS NOT FINALIZED YET!
///
/// \returns A QXmppTask that makes it possible to track the state of the packet.
///
/// \since QXmpp 1.5
///
QXmppTask<QXmpp::SendResult> QXmppClient::sendSensitive(QXmppStanza &&stanza, const std::optional<QXmppSendStanzaParams> &params)
{
    const auto sendEncrypted = [this](auto &&task) {
        QXmppPromise<QXmpp::SendResult> interface;
        task.then(this, [this, interface](auto &&result) mutable {
            std::visit(overloaded {
                           [&](std::unique_ptr<QXmppMessage> &&message) {
                               QByteArray xml;
                               QXmlStreamWriter writer(&xml);
                               message->toXml(&writer, QXmpp::ScePublic);

                               d->stream->send(QXmppPacket(xml, true, std::move(interface)));
                           },
                           [&](std::unique_ptr<QXmppIq> &&iq) {
                               d->stream->send(QXmppPacket(*iq, std::move(interface)));
                           },
                           [&](QXmppError &&error) {
                               interface.finish(std::move(error));
                           } },
                       std::move(result));
        });

        return interface.task();
    };

    if (d->encryptionExtension) {
        if (dynamic_cast<QXmppMessage *>(&stanza)) {
            return sendEncrypted(
                d->encryptionExtension->encryptMessage(
                    std::move(dynamic_cast<QXmppMessage &&>(stanza)), params));
        } else if (dynamic_cast<QXmppIq *>(&stanza)) {
            return sendEncrypted(
                d->encryptionExtension->encryptIq(
                    std::move(dynamic_cast<QXmppIq &&>(stanza)), params));
        }
    }
    return d->stream->send(stanza);
}

///
/// Sends a packet always without end-to-end-encryption.
///
/// This does the same as send(), but does not do any end-to-end encryption on
/// the stanza.
///
/// \warning THIS API IS NOT FINALIZED YET!
///
/// \returns A QXmppTask that makes it possible to track the state of the packet.
///
/// \since QXmpp 1.5
///
QXmppTask<QXmpp::SendResult> QXmppClient::send(QXmppStanza &&stanza, const std::optional<QXmppSendStanzaParams> &)
{
    return d->stream->send(stanza);
}

///
/// Sends the stanza with the same encryption as \p e2eeMetadata.
///
/// When there is no e2eeMetadata given this always sends the stanza without
/// end-to-end encryption.
/// Intended to be used for replies to IQs and messages.
///
/// \since QXmpp 1.5
///
QXmppTask<QXmpp::SendResult> QXmppClient::reply(QXmppStanza &&stanza, const std::optional<QXmppE2eeMetadata> &e2eeMetadata, const std::optional<QXmppSendStanzaParams> &params)
{
    // This should pick the right e2ee manager as soon as multiple encryptions
    // in parallel are supported.
    if (e2eeMetadata) {
        return sendSensitive(std::move(stanza), params);
    }
    return send(std::move(stanza), params);
}

///
/// Sends an IQ packet and returns the response asynchronously.
///
/// This is useful for further processing and parsing of the returned
/// QDomElement. If you don't expect a special response, you may want use
/// sendGenericIq().
///
/// This does not do any end-to-encryption on the IQ.
///
/// \sa sendSensitiveIq()
///
/// \warning THIS API IS NOT FINALIZED YET!
///
/// \since QXmpp 1.5
///
QXmppTask<QXmppClient::IqResult> QXmppClient::sendIq(QXmppIq &&iq, const std::optional<QXmppSendStanzaParams> &)
{
    return d->stream->sendIq(std::move(iq));
}

///
/// Tries to encrypt and send an IQ packet and returns the response
/// asynchronously.
///
/// This can be used for sensitive IQ requests performed from client to client.
/// Most IQ requests like service discovery requests cannot be end-to-end
/// encrypted or it only makes little sense to do so. This is why the default
/// sendIq() does not do any additional end-to-end encryption.
///
/// \warning THIS API IS NOT FINALIZED YET!
///
/// \since QXmpp 1.5
///
QXmppTask<QXmppClient::IqResult> QXmppClient::sendSensitiveIq(QXmppIq &&iq, const std::optional<QXmppSendStanzaParams> &params)
{
    if (d->encryptionExtension) {
        QXmppPromise<IqResult> p;
        auto task = p.task();
        d->encryptionExtension->encryptIq(std::move(iq), params).then(this, [this, p = std::move(p)](IqEncryptResult result) mutable {
            std::visit(overloaded {
                           [&](std::unique_ptr<QXmppIq> &&iq) {
                               // success (encrypted)
                               d->stream->sendIq(std::move(*iq)).then(this, [this, p = std::move(p)](auto &&result) mutable {
                                   // iq sent, response received
                                   std::visit(overloaded {
                                                  [&](QDomElement &&el) {
                                                      if (!isIqResponse(el)) {
                                                          p.finish(QXmppError {
                                                              QStringLiteral("Invalid IQ response received."),
                                                              QXmpp::SendError::EncryptionError });
                                                          return;
                                                      }
                                                      if (!d->encryptionExtension) {
                                                          p.finish(QXmppError {
                                                              QStringLiteral("No decryption extension found."),
                                                              QXmpp::SendError::EncryptionError });
                                                          return;
                                                      }
                                                      // try to decrypt the result (should be encrypted)
                                                      d->encryptionExtension->decryptIq(el).then(this, [p = std::move(p), encryptedEl = el](IqDecryptResult result) mutable {
                                                          std::visit(overloaded {
                                                                         [&](QDomElement &&decryptedEl) {
                                                                             p.finish(decryptedEl);
                                                                         },
                                                                         [&](QXmppE2eeExtension::NotEncrypted) {
                                                                             // the IQ response from the other entity was not encrypted
                                                                             // then report IQ response without modifications
                                                                             // TODO: should we return a QXmppError instead?
                                                                             p.finish(std::move(encryptedEl));
                                                                         },
                                                                         [&](QXmppError &&error) {
                                                                             p.finish(error);
                                                                         } },
                                                                     std::move(result));
                                                      });
                                                  },
                                                  [&](QXmppError &&e) {
                                                      p.finish(std::move(e));
                                                  } },
                                              std::move(result));
                               });
                           },
                           [&](QXmppError &&error) {
                               // error (encryption)
                               p.finish(std::move(error));
                           } },
                       std::move(result));
        });

        return task;
    }
    return d->stream->sendIq(std::move(iq));
}

///
/// Sends an IQ and returns possible stanza errors.
///
/// If you want to parse a special IQ response in the result case, you can use
/// sendIq() and parse the returned QDomElement.
///
/// \returns Returns QXmpp::Success (on response type 'result') or the contained
/// QXmppStanza::Error (on response type 'error')
///
/// \warning THIS API IS NOT FINALIZED YET!
///
/// \since QXmpp 1.5
///
QXmppTask<QXmppClient::EmptyResult> QXmppClient::sendGenericIq(QXmppIq &&iq, const std::optional<QXmppSendStanzaParams> &)
{
    return chainIq(sendIq(std::move(iq)), this, [](const QXmppIq &) -> EmptyResult {
        return QXmpp::Success();
    });
}

/// Disconnects the client and the current presence of client changes to
/// QXmppPresence::Unavailable and status text changes to "Logged out".
///
/// \note Make sure that the clientPresence is changed to
/// QXmppPresence::Available, if you are again calling connectToServer() after
/// calling the disconnectFromServer() function.
///

void QXmppClient::disconnectFromServer()
{
    // cancel reconnection
    d->reconnectionTimer->stop();

    d->clientPresence.setType(QXmppPresence::Unavailable);
    d->clientPresence.setStatusText("Logged out");
    if (d->stream->isConnected()) {
        sendPacket(d->clientPresence);
    }

    d->stream->disconnectFromHost();
}

/// Returns true if the client has authenticated with the XMPP server.

bool QXmppClient::isAuthenticated() const
{
    return d->stream->isAuthenticated();
}

/// Returns true if the client is connected to the XMPP server.
///

bool QXmppClient::isConnected() const
{
    return d->stream->isConnected();
}

///
/// Returns true if the current client state is "active", false if it is
/// "inactive". See \xep{0352}: Client State Indication for details.
///
/// On connect this is always reset to true.
///
/// \since QXmpp 1.0
///
bool QXmppClient::isActive() const
{
    return d->isActive;
}

///
/// Sets the client state as described in \xep{0352}: Client State Indication.
///
/// On connect this is always reset to true.
///
/// \since QXmpp 1.0
///
void QXmppClient::setActive(bool active)
{
    if (active != d->isActive && isConnected() && d->stream->isClientStateIndicationEnabled()) {
        d->isActive = active;
        QString packet = "<%1 xmlns='%2'/>";
        d->stream->sendData(packet.arg(active ? "active" : "inactive", ns_csi).toUtf8());
    }
}

///
/// Returns the current \xep{0198}: Stream Management state of the connection.
///
/// Upon connection of the client this can be used to check whether the
/// previous stream has been resumed.
///
/// \since QXmpp 1.4
///
QXmppClient::StreamManagementState QXmppClient::streamManagementState() const
{
    if (d->stream->isStreamManagementEnabled()) {
        if (d->stream->isStreamResumed()) {
            return ResumedStream;
        }
        return NewStream;
    }
    return NoStreamManagement;
}

/// Returns the reference to QXmppRosterManager object of the client.
///
/// \return Reference to the roster object of the connected client. Use this to
/// get the list of friends in the roster and their presence information.
///
/// \deprecated This method is deprecated since QXmpp 1.1. Use
/// \c QXmppClient::findExtension<QXmppRosterManager>() instead.

QXmppRosterManager &QXmppClient::rosterManager()
{
    return *findExtension<QXmppRosterManager>();
}

/// Utility function to send message to all the resources associated with the
/// specified bareJid. If there are no resources available, that is the contact
/// is offline or not present in the roster, it will still send a message to
/// the bareJid.
///
/// \note Usage of this method is discouraged because most modern clients use
/// carbon messages (\xep{0280}: Message Carbons) and MAM (\xep{0313}: Message
/// Archive Management) and so could possibly receive messages multiple times
/// or not receive them at all.
/// \c QXmppClient::sendPacket() should be used instead with a \c QXmppMessage.
///
/// \param bareJid bareJid of the receiving entity
/// \param message Message string to be sent.

void QXmppClient::sendMessage(const QString &bareJid, const QString &message)
{
    QXmppRosterManager *rosterManager = findExtension<QXmppRosterManager>();

    const QStringList resources = rosterManager
        ? rosterManager->getResources(bareJid)
        : QStringList();

    if (!resources.isEmpty()) {
        for (const auto &resource : resources) {
            sendPacket(
                QXmppMessage({}, bareJid + QStringLiteral("/") + resource, message));
        }
    } else {
        sendPacket(QXmppMessage({}, bareJid, message));
    }
}

QXmppClient::State QXmppClient::state() const
{
    if (d->stream->isConnected()) {
        return QXmppClient::ConnectedState;
    } else if (d->stream->socket()->state() != QAbstractSocket::UnconnectedState &&
               d->stream->socket()->state() != QAbstractSocket::ClosingState) {
        return QXmppClient::ConnectingState;
    } else {
        return QXmppClient::DisconnectedState;
    }
}

/// Returns the client's current presence.
///

QXmppPresence QXmppClient::clientPresence() const
{
    return d->clientPresence;
}

/// Changes the presence of the connected client.
///
/// The connection to the server will be updated accordingly:
///
/// \li If the presence type is QXmppPresence::Unavailable, the connection
/// to the server will be closed.
///
/// \li Otherwise, the connection to the server will be established
/// as needed.
///
/// \param presence QXmppPresence object
///

void QXmppClient::setClientPresence(const QXmppPresence &presence)
{
    d->clientPresence = presence;
    d->addProperCapability(d->clientPresence);

    if (presence.type() == QXmppPresence::Unavailable) {
        // cancel reconnection
        d->reconnectionTimer->stop();

        // NOTE: we can't call disconnect() because it alters
        // the client presence
        if (d->stream->isConnected()) {
            sendPacket(d->clientPresence);
        }

        d->stream->disconnectFromHost();
    } else if (d->stream->isConnected()) {
        sendPacket(d->clientPresence);
    } else {
        connectToServer(d->stream->configuration(), presence);
    }
}

/// Returns the socket error if error() is QXmppClient::SocketError.
///

QAbstractSocket::SocketError QXmppClient::socketError()
{
    return d->stream->socket()->error();
}

/// Returns the human-readable description of the last socket error if error() is QXmppClient::SocketError.

QString QXmppClient::socketErrorString() const
{
    return d->stream->socket()->errorString();
}

/// Returns the XMPP stream error if QXmppClient::Error is QXmppClient::XmppStreamError.

QXmppStanza::Error::Condition QXmppClient::xmppStreamError()
{
    return d->stream->xmppStreamError();
}

///
/// Returns the reference to QXmppVCardManager, implementation of \xep{0054}.
/// http://xmpp.org/extensions/xep-0054.html
///
/// \deprecated This method is deprecated since QXmpp 1.1. Use
/// \c QXmppClient::findExtension<QXmppVCardManager>() instead.
///
QXmppVCardManager &QXmppClient::vCardManager()
{
    return *findExtension<QXmppVCardManager>();
}

///
/// Returns the reference to QXmppVersionManager, implementation of \xep{0092}.
/// http://xmpp.org/extensions/xep-0092.html
///
/// \deprecated This method is deprecated since QXmpp 1.1. Use
/// \c QXmppClient::findExtension<QXmppVersionManager>() instead.
///
QXmppVersionManager &QXmppClient::versionManager()
{
    return *findExtension<QXmppVersionManager>();
}

void QXmppClient::injectIq(const QDomElement &element, const std::optional<QXmppE2eeMetadata> &e2eeMetadata)
{
    if (element.tagName() != "iq") {
        return;
    }
    if (!StanzaPipeline::process(d->extensions, element, e2eeMetadata)) {
        const auto iqType = element.attribute("type");
        if (iqType == "get" || iqType == "set") {
            // send error IQ
            using Err = QXmppStanza::Error;

            QXmppIq iq(QXmppIq::Error);
            iq.setTo(element.attribute("from"));
            iq.setId(element.attribute("id"));
            const auto errMessage = e2eeMetadata.has_value()
                ? QStringLiteral("Feature not implemented or not supported with end-to-end encryption.")
                : QStringLiteral("Feature not implemented.");
            iq.setError(Err(Err::Cancel, Err::FeatureNotImplemented, errMessage));
            reply(std::move(iq), e2eeMetadata);
        }
        // don't do anything for "result" and "error" IQs
    }
}

///
/// Processes the message with message handlers and emits messageReceived as a fallback.
///
bool QXmppClient::injectMessage(QXmppMessage &&message)
{
    auto handled = MessagePipeline::process(this, d->extensions, std::move(message));
    if (!handled) {
        // no extension handled the message
        Q_EMIT messageReceived(message);
    }
    return handled;
}

///
/// Give extensions a chance to handle incoming stanzas.
///
void QXmppClient::_q_elementReceived(const QDomElement &element, bool &handled)
{
    // The stanza comes directly from the XMPP stream, so it's not end-to-end
    // encrypted and there's no e2ee metadata (std::nullopt).
    handled = StanzaPipeline::process(d->extensions, element, std::nullopt) ||
        MessagePipeline::process(this, d->extensions, element);
}

void QXmppClient::_q_reconnect()
{
    if (d->stream->configuration().autoReconnectionEnabled()) {
        debug("Reconnecting to server");
        d->stream->connectToHost();
    }
}

void QXmppClient::_q_socketStateChanged(QAbstractSocket::SocketState socketState)
{
    Q_UNUSED(socketState);
    Q_EMIT stateChanged(state());
}

/// At connection establishment, send initial presence.

void QXmppClient::_q_streamConnected()
{
    d->receivedConflict = false;
    d->reconnectionTries = 0;
    d->isActive = true;

    // notify managers
    Q_EMIT connected();
    Q_EMIT stateChanged(QXmppClient::ConnectedState);

    // send initial presence
    if (d->stream->isAuthenticated()) {
        sendPacket(d->clientPresence);
    }
}

void QXmppClient::_q_streamDisconnected()
{
    // notify managers
    Q_EMIT disconnected();
    Q_EMIT stateChanged(QXmppClient::DisconnectedState);
}

void QXmppClient::_q_streamError(QXmppClient::Error err)
{
    if (d->stream->configuration().autoReconnectionEnabled()) {
        if (err == QXmppClient::XmppStreamError) {
            // if we receive a resource conflict, inhibit reconnection
            if (d->stream->xmppStreamError() == QXmppStanza::Error::Conflict) {
                d->receivedConflict = true;
            }
        } else if (err == QXmppClient::SocketError && !d->receivedConflict) {
            // schedule reconnect
            d->reconnectionTimer->start(d->getNextReconnectTime());
        } else if (err == QXmppClient::KeepAliveError) {
            // if we got a keepalive error, reconnect in one second
            d->reconnectionTimer->start(1000);
        }
    }

    // notify managers
    Q_EMIT error(err);
}

QXmppLogger *QXmppClient::logger() const
{
    return d->logger;
}

/// Sets the QXmppLogger associated with the current QXmppClient.

void QXmppClient::setLogger(QXmppLogger *logger)
{
    if (logger != d->logger) {
        if (d->logger) {
            disconnect(this, &QXmppLoggable::logMessage,
                       d->logger, &QXmppLogger::log);
            disconnect(this, &QXmppLoggable::setGauge,
                       d->logger, &QXmppLogger::setGauge);
            disconnect(this, &QXmppLoggable::updateCounter,
                       d->logger, &QXmppLogger::updateCounter);
        }

        d->logger = logger;
        if (d->logger) {
            connect(this, &QXmppLoggable::logMessage,
                    d->logger, &QXmppLogger::log);
            connect(this, &QXmppLoggable::setGauge,
                    d->logger, &QXmppLogger::setGauge);
            connect(this, &QXmppLoggable::updateCounter,
                    d->logger, &QXmppLogger::updateCounter);
        }

        Q_EMIT loggerChanged(d->logger);
    }
}
