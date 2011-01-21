/*
 * Copyright (C) 2008-2011 The QXmpp developers
 *
 * Author:
 *  Jeremy Lainé
 *
 * Source:
 *  http://code.google.com/p/qxmpp
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

#include <QDomElement>
#include <QTimer>

#include "QXmppCallManager.h"
#include "QXmppClient.h"
#include "QXmppCodec.h"
#include "QXmppConstants.h"
#include "QXmppJingleIq.h"
#include "QXmppRtpChannel.h"
#include "QXmppStun.h"
#include "QXmppUtils.h"

static int typeId = qRegisterMetaType<QXmppCall::State>();

const int RTP_COMPONENT = 1;
const int RTCP_COMPONENT = 2;

class QXmppCallPrivate
{
public:
    QXmppCallPrivate(QXmppCall *qq);
    void setState(QXmppCall::State state);

    QXmppCall::Direction direction;
    QString jid;
    QString sid;
    QXmppCall::State state;
    QString contentCreator;
    QString contentName;

    QList<QXmppJingleIq> requests;

    // ICE-UDP
    QXmppIceConnection *connection;

    // RTP
    QXmppRtpChannel *audioChannel;

private:
    QXmppCall *q;
};

QXmppCallPrivate::QXmppCallPrivate(QXmppCall *qq)
    : state(QXmppCall::OfferState),
    q(qq)
{
}

void QXmppCallPrivate::setState(QXmppCall::State newState)
{
    if (state != newState)
    {
        state = newState;
        emit q->stateChanged(state);
    }
}

QXmppCall::QXmppCall(const QString &jid, QXmppCall::Direction direction, QObject *parent)
    : QXmppLoggable(parent)
{
    d = new QXmppCallPrivate(this);
    d->direction = direction;
    d->jid = jid;
    d->contentCreator = QLatin1String("initiator");
    d->contentName = QLatin1String("voice");

    // ICE connection
    bool iceControlling = (d->direction == OutgoingDirection);
    d->connection = new QXmppIceConnection(iceControlling, this);
    //d->connection->setStunServer("stun.ekiga.net");
    d->connection->addComponent(RTP_COMPONENT);
    d->connection->addComponent(RTCP_COMPONENT);
    d->connection->bind(QXmppIceComponent::discoverAddresses());

    bool check = connect(d->connection, SIGNAL(localCandidatesChanged()),
        this, SIGNAL(localCandidatesChanged()));
    Q_ASSERT(check);

    check = connect(d->connection, SIGNAL(connected()),
        this, SLOT(updateOpenMode()));
    Q_ASSERT(check);

    check = connect(d->connection, SIGNAL(disconnected()),
        this, SLOT(hangup()));
    Q_ASSERT(check);

    // RTP channel
    d->audioChannel = new QXmppRtpChannel(this);
    QXmppIceComponent *rtpComponent = d->connection->component(RTP_COMPONENT);

    check = connect(rtpComponent, SIGNAL(datagramReceived(QByteArray)),
                    d->audioChannel, SLOT(datagramReceived(QByteArray)));
    Q_ASSERT(check);

    check = connect(d->audioChannel, SIGNAL(sendDatagram(QByteArray)),
                    rtpComponent, SLOT(sendDatagram(QByteArray)));
    Q_ASSERT(check);
}

QXmppCall::~QXmppCall()
{
    delete d;
}

/// Call this method if you wish to accept an incoming call.
///

void QXmppCall::accept()
{
    if (d->direction == IncomingDirection && d->state == OfferState)
        d->setState(QXmppCall::ConnectingState);
}

/// Returns the RTP channel for the audio data.
///
/// It acts as a QIODevice so that you can read / write audio samples, for
/// instance using a QAudioOutput and a QAudioInput.
///

QXmppRtpChannel *QXmppCall::audioChannel() const
{
    return d->audioChannel;
}

/// Returns the number of bytes that are available for reading.
///

void QXmppCall::terminate()
{
    if (d->state == FinishedState)
        return;

    d->state = QXmppCall::FinishedState;

    d->audioChannel->close();
    d->connection->close();

    // emit signals later
    QTimer::singleShot(0, this, SLOT(terminated()));
}

void QXmppCall::terminated()
{
    emit stateChanged(d->state);
    emit finished();
}

/// Returns the call's direction.
///

QXmppCall::Direction QXmppCall::direction() const
{
    return d->direction;
}

/// Hangs up the call.
///

void QXmppCall::hangup()
{
    if (d->state != QXmppCall::FinishedState)
        d->setState(QXmppCall::DisconnectingState);
}

/// Returns the remote party's JID.
///

QString QXmppCall::jid() const
{
    return d->jid;
}

void QXmppCall::updateOpenMode()
{
    // determine mode
    if (d->audioChannel->isOpen() && d->connection->isConnected() && d->state != ActiveState)
    {
        d->setState(ActiveState);
        emit connected();
    }
}

/// Returns the call's session identifier.
///

QString QXmppCall::sid() const
{
    return d->sid;
}

/// Returns the call's state.
///
/// \sa stateChanged()

QXmppCall::State QXmppCall::state() const
{
    return d->state;
}

class QXmppCallManagerPrivate
{
public:
    QXmppCallManagerPrivate(QXmppCallManager *qq);
    bool checkPayloadTypes(QXmppCall *call, const QList<QXmppJinglePayloadType> &remotePayloadTypes);
    QXmppCall *findCall(const QString &sid) const;
    QXmppCall *findCall(const QString &sid, QXmppCall::Direction direction) const;
    bool sendAck(const QXmppJingleIq &iq);
    bool sendRequest(QXmppCall *call, const QXmppJingleIq &iq);

    QList<QXmppCall*> calls;

private:
    QXmppCallManager *q;
};

QXmppCallManagerPrivate::QXmppCallManagerPrivate(QXmppCallManager *qq)
    : q(qq)
{
}

QXmppCall *QXmppCallManagerPrivate::findCall(const QString &sid) const
{
    foreach (QXmppCall *call, calls)
        if (call->sid() == sid)
           return call;
    return 0;
}

QXmppCall *QXmppCallManagerPrivate::findCall(const QString &sid, QXmppCall::Direction direction) const
{
    foreach (QXmppCall *call, calls)
        if (call->sid() == sid && call->direction() == direction)
           return call;
    return 0;
}

/// Sends an acknowledgement for a Jingle IQ.
///

bool QXmppCallManagerPrivate::sendAck(const QXmppJingleIq &iq)
{
    QXmppIq ack;
    ack.setId(iq.id());
    ack.setTo(iq.from());
    ack.setType(QXmppIq::Result);
    return q->client()->sendPacket(ack);
}

/// Sends a Jingle IQ and adds it to outstanding requests.
///

bool QXmppCallManagerPrivate::sendRequest(QXmppCall *call, const QXmppJingleIq &iq)
{
    call->d->requests << iq;
    return q->client()->sendPacket(iq);
}

/// Constructs a QXmppCallManager object to handle incoming and outgoing
/// Voice-Over-IP calls.
///

QXmppCallManager::QXmppCallManager()
{
    d = new QXmppCallManagerPrivate(this);
}

/// Destroys the QXmppCallManager object.

QXmppCallManager::~QXmppCallManager()
{
    delete d;
}

QStringList QXmppCallManager::discoveryFeatures() const
{
    return QStringList()
        << ns_jingle            // XEP-0166 : Jingle
        << ns_jingle_rtp        // XEP-0167 : Jingle RTP Sessions
        << ns_jingle_rtp_audio
        << ns_jingle_ice_udp;    // XEP-0176 : Jingle ICE-UDP Transport Method
}

bool QXmppCallManager::handleStanza(const QDomElement &element)
{
    if(element.tagName() == "iq")
    {
        // XEP-0166: Jingle
        if (QXmppJingleIq::isJingleIq(element))
        {
            QXmppJingleIq jingleIq;
            jingleIq.parse(element);
            jingleIqReceived(jingleIq);
            return true;
        }
    }

    return false;
}

void QXmppCallManager::setClient(QXmppClient *client)
{
    QXmppClientExtension::setClient(client);

    bool check = connect(client, SIGNAL(iqReceived(QXmppIq)),
        this, SLOT(iqReceived(QXmppIq)));
    Q_ASSERT(check);
    Q_UNUSED(check);
}

/// Initiates a new outgoing call to the specified recipient.
///
/// \param jid

QXmppCall *QXmppCallManager::call(const QString &jid)
{
    QXmppCall *call = new QXmppCall(jid, QXmppCall::OutgoingDirection, this);
    call->d->sid = generateStanzaHash();

    // register call
    d->calls << call;
    connect(call, SIGNAL(destroyed(QObject*)),
        this, SLOT(callDestroyed(QObject*)));
    connect(call, SIGNAL(stateChanged(QXmppCall::State)),
        this, SLOT(callStateChanged(QXmppCall::State)));
    connect(call, SIGNAL(localCandidatesChanged()),
        this, SLOT(localCandidatesChanged()));

    QXmppJingleIq iq;
    iq.setTo(jid);
    iq.setType(QXmppIq::Set);
    iq.setAction(QXmppJingleIq::SessionInitiate);
    iq.setInitiator(client()->configuration().jid());
    iq.setSid(call->sid());
    iq.content().setCreator(call->d->contentCreator);
    iq.content().setName(call->d->contentName);
    iq.content().setSenders("both");

    // description
    iq.content().setDescriptionMedia("audio");
    foreach (const QXmppJinglePayloadType &payload, call->d->audioChannel->localPayloadTypes())
        iq.content().addPayloadType(payload);

    // transport
    iq.content().setTransportUser(call->d->connection->localUser());
    iq.content().setTransportPassword(call->d->connection->localPassword());
    foreach (const QXmppJingleCandidate &candidate, call->d->connection->localCandidates())
        iq.content().addTransportCandidate(candidate);

    d->sendRequest(call, iq);

    return call;
}

void QXmppCallManager::callDestroyed(QObject *object)
{
    d->calls.removeAll(static_cast<QXmppCall*>(object));
}

void QXmppCallManager::callStateChanged(QXmppCall::State state)
{
    QXmppCall *call = qobject_cast<QXmppCall*>(sender());
    if (!call || !d->calls.contains(call))
        return;

#if 0
    // disconnect from the signal
    disconnect(call, SIGNAL(stateChanged(QXmppCall::State)),
        this, SLOT(callStateChanged(QXmppCall::State)));
#endif

    if (state == QXmppCall::DisconnectingState)
    {
        // hangup up call
        QXmppJingleIq iq;
        iq.setTo(call->jid());
        iq.setType(QXmppIq::Set);
        iq.setAction(QXmppJingleIq::SessionTerminate);
        iq.setSid(call->sid());
        d->sendRequest(call, iq);

        // schedule forceful termination in 5s
        QTimer::singleShot(5000, call, SLOT(terminate()));
    }
    else if (state == QXmppCall::ConnectingState &&
             call->direction() == QXmppCall::IncomingDirection)
    {
        // accept incoming call
        QXmppJingleIq iq;
        iq.setTo(call->jid());
        iq.setType(QXmppIq::Set);
        iq.setAction(QXmppJingleIq::SessionAccept);
        iq.setResponder(client()->configuration().jid());
        iq.setSid(call->sid());
        iq.content().setCreator(call->d->contentCreator);
        iq.content().setName(call->d->contentName);

        // description
        iq.content().setDescriptionMedia("audio");
        foreach (const QXmppJinglePayloadType &payload, call->d->audioChannel->localPayloadTypes())
            iq.content().addPayloadType(payload);

        // transport
        iq.content().setTransportUser(call->d->connection->localUser());
        iq.content().setTransportPassword(call->d->connection->localPassword());
        foreach (const QXmppJingleCandidate &candidate, call->d->connection->localCandidates())
            iq.content().addTransportCandidate(candidate);

        d->sendRequest(call, iq);

        // perform ICE negotiation
        call->d->connection->connectToHost();
    }
}

/// Determine common payload types for a call.
///

bool QXmppCallManagerPrivate::checkPayloadTypes(QXmppCall *call, const QList<QXmppJinglePayloadType> &remotePayloadTypes)
{
    call->d->audioChannel->setRemotePayloadTypes(remotePayloadTypes);
    if (!call->d->audioChannel->isOpen()) {
        q->warning(QString("Remote party %1 did not provide any known payload types for call %2").arg(call->jid(), call->sid()));

        // terminate call
        QXmppJingleIq iq;
        iq.setTo(call->jid());
        iq.setType(QXmppIq::Set);
        iq.setAction(QXmppJingleIq::SessionTerminate);
        iq.setSid(call->sid());
        iq.reason().setType(QXmppJingleIq::Reason::FailedApplication);
        sendRequest(call, iq);
        return false;
    } else {
        call->updateOpenMode();
        return true;
    }
}

/// Handles acknowledgements
///

void QXmppCallManager::iqReceived(const QXmppIq &ack)
{
    if (ack.type() != QXmppIq::Result)
        return;

    // find request
    bool found = false;
    QXmppCall *call = 0;
    QXmppJingleIq request;
    foreach (call, d->calls)
    {
        for (int i = 0; i < call->d->requests.size(); i++)
        {
            if (ack.id() == call->d->requests[i].id())
            {
                request = call->d->requests.takeAt(i);
                found = true;
                break;
            }
        }
        if (found)
            break;
    }
    if (!found)
        return;

    // process acknowledgement
    debug(QString("Received ACK for packet %1").arg(ack.id()));
    if (request.action() == QXmppJingleIq::SessionTerminate)
    {
        // terminate
        call->terminate();
    }
}

/// Handle Jingle IQs.
///

void QXmppCallManager::jingleIqReceived(const QXmppJingleIq &iq)
{
    if (iq.type() != QXmppIq::Set)
        return;

    if (iq.action() == QXmppJingleIq::SessionInitiate)
    {
        // build call
        QXmppCall *call = new QXmppCall(iq.from(), QXmppCall::IncomingDirection, this);
        call->d->sid = iq.sid();
        call->d->contentCreator = iq.content().creator();
        call->d->contentName = iq.content().name();
        call->d->connection->setRemoteUser(iq.content().transportUser());
        call->d->connection->setRemotePassword(iq.content().transportPassword());
        foreach (const QXmppJingleCandidate &candidate, iq.content().transportCandidates())
            call->d->connection->addRemoteCandidate(candidate);

        // send ack
        d->sendAck(iq);

        // determine common payload types
        if (!d->checkPayloadTypes(call, iq.content().payloadTypes()))
        {
            delete call;
            return;
        }

        // register call
        d->calls << call;
        connect(call, SIGNAL(destroyed(QObject*)),
            this, SLOT(callDestroyed(QObject*)));
        connect(call, SIGNAL(stateChanged(QXmppCall::State)),
            this, SLOT(callStateChanged(QXmppCall::State)));
        connect(call, SIGNAL(localCandidatesChanged()),
            this, SLOT(localCandidatesChanged()));

        // send ringing indication
        QXmppJingleIq ringing;
        ringing.setTo(call->jid());
        ringing.setType(QXmppIq::Set);
        ringing.setAction(QXmppJingleIq::SessionInfo);
        ringing.setSid(call->sid());
        ringing.setRinging(true);
        d->sendRequest(call, ringing);

        // notify user
        emit callReceived(call);

    } else if (iq.action() == QXmppJingleIq::SessionAccept) {
        QXmppCall *call = d->findCall(iq.sid(), QXmppCall::OutgoingDirection);
        if (!call)
        {
            warning(QString("Remote party %1 accepted unknown call %2").arg(iq.from(), iq.sid()));
            return;
        }

        // send ack
        d->sendAck(iq);

        // determine common payload types
        if (!d->checkPayloadTypes(call, iq.content().payloadTypes()))
        {
            delete call;
            return;
        }

        // perform ICE negotiation
        if (!iq.content().transportCandidates().isEmpty())
        {
            call->d->connection->setRemoteUser(iq.content().transportUser());
            call->d->connection->setRemotePassword(iq.content().transportPassword());
            foreach (const QXmppJingleCandidate &candidate, iq.content().transportCandidates())
                call->d->connection->addRemoteCandidate(candidate);
        }
        call->d->connection->connectToHost();

    } else if (iq.action() == QXmppJingleIq::SessionInfo) {

        QXmppCall *call = d->findCall(iq.sid());
        if (!call)
            return;

        // notify user
        QTimer::singleShot(0, call, SIGNAL(ringing()));

    } else if (iq.action() == QXmppJingleIq::SessionTerminate) {

        QXmppCall *call = d->findCall(iq.sid());
        if (!call)
        {
            warning(QString("Remote party %1 terminated unknown call %2").arg(iq.from(), iq.sid()));
            return;
        }

        info(QString("Remote party %1 terminated call %2").arg(iq.from(), iq.sid()));

        // send ack
        d->sendAck(iq);

        // terminate
        call->terminate();

    } else if (iq.action() == QXmppJingleIq::TransportInfo) {
        QXmppCall *call = d->findCall(iq.sid());
        if (!call)
        {
            warning(QString("Remote party %1 sent transports for unknown call %2").arg(iq.from(), iq.sid()));
            return;
        }

        // send ack
        d->sendAck(iq);

        // perform ICE negotiation
        call->d->connection->setRemoteUser(iq.content().transportUser());
        call->d->connection->setRemotePassword(iq.content().transportPassword());
        foreach (const QXmppJingleCandidate &candidate, iq.content().transportCandidates())
            call->d->connection->addRemoteCandidate(candidate);
        call->d->connection->connectToHost();
    }
}

/// Sends a transport-info to inform the remote party of new local candidates.
///

void QXmppCallManager::localCandidatesChanged()
{
    QXmppCall *call = qobject_cast<QXmppCall*>(sender());
    if (!call || !d->calls.contains(call))
        return;

    QXmppJingleIq iq;
    iq.setTo(call->jid());
    iq.setType(QXmppIq::Set);
    iq.setAction(QXmppJingleIq::TransportInfo);
    iq.setInitiator(client()->configuration().jid());
    iq.setSid(call->sid());

    iq.content().setCreator(call->d->contentCreator);
    iq.content().setName(call->d->contentName);

    // transport
    iq.content().setTransportUser(call->d->connection->localUser());
    iq.content().setTransportPassword(call->d->connection->localPassword());
    foreach (const QXmppJingleCandidate &candidate, call->d->connection->localCandidates())
        iq.content().addTransportCandidate(candidate);

    d->sendRequest(call, iq);
}


