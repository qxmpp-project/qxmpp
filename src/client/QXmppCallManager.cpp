// SPDX-FileCopyrightText: 2010 Jeremy Lainé <jeremy.laine@m4x.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppCallManager.h"

#include "QXmppCall.h"
#include "QXmppCallManager_p.h"
#include "QXmppCall_p.h"
#include "QXmppClient.h"
#include "QXmppConstants_p.h"
#include "QXmppJingleIq.h"
#include "QXmppUtils.h"

#include "StringLiterals.h"

#include <gst/gst.h>

#include <QDomElement>
#include <QTimer>

/// \cond
QXmppCallManagerPrivate::QXmppCallManagerPrivate(QXmppCallManager *qq)
    : turnPort(0),
      q(qq)
{
    // Initialize GStreamer
    gst_init(nullptr, nullptr);
}

QXmppCall *QXmppCallManagerPrivate::findCall(const QString &sid) const
{
    for (auto *call : calls) {
        if (call->sid() == sid) {
            return call;
        }
    }
    return nullptr;
}

QXmppCall *QXmppCallManagerPrivate::findCall(const QString &sid, QXmppCall::Direction direction) const
{
    for (auto *call : calls) {
        if (call->sid() == sid && call->direction() == direction) {
            return call;
        }
    }
    return nullptr;
}
/// \endcond

///
/// Constructs a QXmppCallManager object to handle incoming and outgoing
/// Voice-Over-IP calls.
///
QXmppCallManager::QXmppCallManager()
    : d(std::make_unique<QXmppCallManagerPrivate>(this))
{
}

///
/// Destroys the QXmppCallManager object.
///
QXmppCallManager::~QXmppCallManager() = default;

/// \cond
QStringList QXmppCallManager::discoveryFeatures() const
{
    return {
        ns_jingle.toString(),      // XEP-0166: Jingle
        ns_jingle_rtp.toString(),  // XEP-0167: Jingle RTP Sessions
        ns_jingle_rtp_audio.toString(),
        ns_jingle_rtp_video.toString(),
        ns_jingle_ice_udp.toString(),  // XEP-0176: Jingle ICE-UDP Transport Method
    };
}

bool QXmppCallManager::handleStanza(const QDomElement &element)
{
    if (element.tagName() == u"iq") {
        // XEP-0166: Jingle
        if (QXmppJingleIq::isJingleIq(element)) {
            QXmppJingleIq jingleIq;
            jingleIq.parse(element);
            _q_jingleIqReceived(jingleIq);
            return true;
        }
    }

    return false;
}

void QXmppCallManager::onRegistered(QXmppClient *client)
{
    connect(client, &QXmppClient::disconnected,
            this, &QXmppCallManager::_q_disconnected);

    connect(client, &QXmppClient::iqReceived,
            this, &QXmppCallManager::_q_iqReceived);

    connect(client, &QXmppClient::presenceReceived,
            this, &QXmppCallManager::_q_presenceReceived);
}

void QXmppCallManager::onUnregistered(QXmppClient *client)
{
    disconnect(client, &QXmppClient::disconnected,
               this, &QXmppCallManager::_q_disconnected);

    disconnect(client, &QXmppClient::iqReceived,
               this, &QXmppCallManager::_q_iqReceived);

    disconnect(client, &QXmppClient::presenceReceived,
               this, &QXmppCallManager::_q_presenceReceived);
}
/// \endcond

///
/// Initiates a new outgoing call to the specified recipient.
///
/// \param jid
///
QXmppCall *QXmppCallManager::call(const QString &jid)
{
    if (jid.isEmpty()) {
        warning(u"Refusing to call an empty jid"_s);
        return nullptr;
    }

    if (jid == client()->configuration().jid()) {
        warning(u"Refusing to call self"_s);
        return nullptr;
    }

    QXmppCall *call = new QXmppCall(jid, QXmppCall::OutgoingDirection, this);
    QXmppCallStream *stream = call->d->createStream(u"audio"_s, u"initiator"_s, u"microphone"_s);
    call->d->streams << stream;
    call->d->sid = QXmppUtils::generateStanzaHash();

    // register call
    d->calls << call;
    connect(call, &QObject::destroyed,
            this, &QXmppCallManager::_q_callDestroyed);
    Q_EMIT callStarted(call);

    call->d->sendInvite();

    return call;
}

///
/// Sets multiple STUN servers to use to determine server-reflexive addresses
/// and ports.
///
/// \note This may only be called prior to calling bind().
///
/// \param servers List of the STUN servers.
///
/// \since QXmpp 1.3
///
void QXmppCallManager::setStunServers(const QList<QPair<QHostAddress, quint16>> &servers)
{
    d->stunServers = servers;
}

///
/// Sets a single STUN server to use to determine server-reflexive addresses
/// and ports.
///
/// \note This may only be called prior to calling bind().
///
/// \param host The address of the STUN server.
/// \param port The port of the STUN server.
///
void QXmppCallManager::setStunServer(const QHostAddress &host, quint16 port)
{
    d->stunServers.clear();
    d->stunServers.push_back(qMakePair(host, port));
}

///
/// Sets the TURN server to use to relay packets in double-NAT configurations.
///
/// \param host The address of the TURN server.
/// \param port The port of the TURN server.
///
void QXmppCallManager::setTurnServer(const QHostAddress &host, quint16 port)
{
    d->turnHost = host;
    d->turnPort = port;
}

///
/// Sets the \a user used for authentication with the TURN server.
///
/// \param user
///
void QXmppCallManager::setTurnUser(const QString &user)
{
    d->turnUser = user;
}

///
/// Sets the \a password used for authentication with the TURN server.
///
/// \param password
///
void QXmppCallManager::setTurnPassword(const QString &password)
{
    d->turnPassword = password;
}

///
/// Handles call destruction.
///
void QXmppCallManager::_q_callDestroyed(QObject *object)
{
    d->calls.removeAll(static_cast<QXmppCall *>(object));
}

///
/// Handles disconnection from server.
///
void QXmppCallManager::_q_disconnected()
{
    for (auto *call : std::as_const(d->calls)) {
        call->d->terminate(QXmppJingleIq::Reason::Gone);
    }
}

///
/// Handles acknowledgements.
///
void QXmppCallManager::_q_iqReceived(const QXmppIq &ack)
{
    if (ack.type() != QXmppIq::Result) {
        return;
    }

    // find request
    for (auto *call : std::as_const(d->calls)) {
        call->d->handleAck(ack);
    }
}

///
/// Handles a Jingle IQ.
///
void QXmppCallManager::_q_jingleIqReceived(const QXmppJingleIq &iq)
{

    if (iq.type() != QXmppIq::Set) {
        return;
    }

    if (iq.action() == QXmppJingleIq::SessionInitiate) {
        // build call
        QXmppCall *call = new QXmppCall(iq.from(), QXmppCall::IncomingDirection, this);
        call->d->sid = iq.sid();

        const auto content = iq.contents().isEmpty() ? QXmppJingleIq::Content()
                                                     : iq.contents().constFirst();
        auto *stream = call->d->createStream(content.descriptionMedia(), content.creator(), content.name());
        if (!stream) {
            // FIXME: delete call here?
            return;
        }
        call->d->streams << stream;

        // send ack
        call->d->sendAck(iq);

        // check content description and transport
        if (!call->d->handleDescription(stream, content) ||
            !call->d->handleTransport(stream, content)) {

            // terminate call
            call->d->terminate(QXmppJingleIq::Reason::FailedApplication);
            call->terminated();
            delete call;
            return;
        }

        // register call
        d->calls << call;
        connect(call, &QObject::destroyed,
                this, &QXmppCallManager::_q_callDestroyed);

        // send ringing indication
        QXmppJingleIq ringing;
        ringing.setTo(call->jid());
        ringing.setType(QXmppIq::Set);
        ringing.setSid(call->sid());
        ringing.setRtpSessionState(QXmppJingleIq::RtpSessionStateRinging());
        call->d->sendRequest(ringing);

        // notify user
        Q_EMIT callReceived(call);
        return;

    } else {

        // for all other requests, require a valid call
        QXmppCall *call = d->findCall(iq.sid());
        if (!call) {
            warning(u"Remote party %1 sent a request for an unknown call %2"_s.arg(iq.from(), iq.sid()));
            return;
        }
        call->d->handleRequest(iq);
    }
}

///
/// Handles a presence.
///
void QXmppCallManager::_q_presenceReceived(const QXmppPresence &presence)
{
    if (presence.type() != QXmppPresence::Unavailable) {
        return;
    }

    for (auto *call : std::as_const(d->calls)) {
        if (presence.from() == call->jid()) {
            // the remote party has gone away, terminate call
            call->d->terminate(QXmppJingleIq::Reason::Gone);
        }
    }
}
