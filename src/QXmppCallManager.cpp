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

static const int RTP_COMPONENT = 1;
static const int RTCP_COMPONENT = 2;

static const QLatin1String AUDIO_MEDIA("audio");
static const QLatin1String VIDEO_MEDIA("video");

class QXmppCallPrivate
{
public:
    class Stream {
    public:
        QXmppRtpChannel *channel;
        QXmppIceConnection *connection;
        QString creator;
        QString media;
        QString name;
    };

    QXmppCallPrivate(QXmppCall *qq);
    Stream *createStream(const QString &media);
    Stream *findStreamByMedia(const QString &media);
    Stream *findStreamByName(const QString &name);
    void handleAck(const QXmppIq &iq);
    bool handleDescription(QXmppCallPrivate::Stream *stream, const QXmppJingleIq::Content &content);
    void handleRequest(const QXmppJingleIq &iq);
    bool handleTransport(QXmppCallPrivate::Stream *stream, const QXmppJingleIq::Content &content);
    void setState(QXmppCall::State state);
    bool sendAck(const QXmppJingleIq &iq);
    bool sendInvite();
    bool sendRequest(const QXmppJingleIq &iq);

    QXmppCall::Direction direction;
    QString jid;
    QString ownJid;
    QXmppCallManager *manager;
    QList<QXmppJingleIq> requests;
    QString sid;
    QXmppCall::State state;

    // Media streams
    QList<Stream*> streams;
    QIODevice::OpenMode audioMode;
    QIODevice::OpenMode videoMode;

private:
    QXmppCall *q;
};

class QXmppCallManagerPrivate
{
public:
    QXmppCallManagerPrivate(QXmppCallManager *qq);
    QXmppCall *findCall(const QString &sid) const;
    QXmppCall *findCall(const QString &sid, QXmppCall::Direction direction) const;

    QList<QXmppCall*> calls;
    QHostAddress stunHost;
    quint16 stunPort;
    QHostAddress turnHost;
    quint16 turnPort;
    QString turnUser;
    QString turnPassword;

private:
    QXmppCallManager *q;
};

QXmppCallPrivate::QXmppCallPrivate(QXmppCall *qq)
    : state(QXmppCall::OfferState),
    audioMode(QIODevice::NotOpen),
    videoMode(QIODevice::NotOpen),
    q(qq)
{
}

QXmppCallPrivate::Stream *QXmppCallPrivate::findStreamByMedia(const QString &media)
{
    foreach (Stream *stream, streams)
        if (stream->media == media)
            return stream;
    return 0;
}

QXmppCallPrivate::Stream *QXmppCallPrivate::findStreamByName(const QString &name)
{
    foreach (Stream *stream, streams)
        if (stream->name == name)
            return stream;
    return 0;
}

void QXmppCallPrivate::handleAck(const QXmppIq &ack)
{
    const QString id = ack.id();
    for (int i = 0; i < requests.size(); ++i) {
        if (id == requests[i].id()) {
            // process acknowledgement
            const QXmppJingleIq request = requests.takeAt(i);
            q->debug(QString("Received ACK for packet %1").arg(id));

            // handle termination
            if (request.action() == QXmppJingleIq::SessionTerminate)
                q->terminate();
            return;
        }
    }
}

bool QXmppCallPrivate::handleDescription(QXmppCallPrivate::Stream *stream, const QXmppJingleIq::Content &content)
{
    stream->channel->setRemotePayloadTypes(content.payloadTypes());
    if (!(stream->channel->openMode() & QIODevice::ReadWrite)) {
        q->warning(QString("Remote party %1 did not provide any known %2 payloads for call %3").arg(jid, stream->media, sid));
        return false;
    }
    q->updateOpenMode();
    return true;
}

bool QXmppCallPrivate::handleTransport(QXmppCallPrivate::Stream *stream, const QXmppJingleIq::Content &content)
{
    stream->connection->setRemoteUser(content.transportUser());
    stream->connection->setRemotePassword(content.transportPassword());
    foreach (const QXmppJingleCandidate &candidate, content.transportCandidates())
        stream->connection->addRemoteCandidate(candidate);

    // perform ICE negotiation
    if (!content.transportCandidates().isEmpty())
        stream->connection->connectToHost();
    return true;
}

void QXmppCallPrivate::handleRequest(const QXmppJingleIq &iq)
{
    if (iq.action() == QXmppJingleIq::SessionAccept) {

        if (direction == QXmppCall::IncomingDirection) {
            q->warning("Ignoring Session-Accept for an incoming call");
            return;
        }

        // send ack
        sendAck(iq);

        // check content description and transport
        QXmppCallPrivate::Stream *stream = findStreamByName(iq.content().name());
        if (!stream ||
            !handleDescription(stream, iq.content()) ||
            !handleTransport(stream, iq.content())) {

            // terminate call
            QXmppJingleIq iq;
            iq.setTo(q->jid());
            iq.setType(QXmppIq::Set);
            iq.setAction(QXmppJingleIq::SessionTerminate);
            iq.setSid(q->sid());
            iq.reason().setType(QXmppJingleIq::Reason::FailedApplication);
            sendRequest(iq);

            q->terminate();
            return;
        }

        // check for call establishment
        setState(QXmppCall::ConnectingState);
        q->updateOpenMode();

    } else if (iq.action() == QXmppJingleIq::SessionInfo) {

        // notify user
        QTimer::singleShot(0, q, SIGNAL(ringing()));

    } else if (iq.action() == QXmppJingleIq::SessionTerminate) {

        // send ack
        sendAck(iq);

        // terminate
        q->info(QString("Remote party %1 terminated call %2").arg(iq.from(), iq.sid()));
        q->terminate();

    } else if (iq.action() == QXmppJingleIq::ContentAccept) {

        // send ack
        sendAck(iq);

        // check content description and transport
        QXmppCallPrivate::Stream *stream = findStreamByName(iq.content().name());
        if (!stream ||
            !handleDescription(stream, iq.content()) ||
            !handleTransport(stream, iq.content())) {

            // FIXME: what action?
            return;
        }

    } else if (iq.action() == QXmppJingleIq::ContentAdd) {

        // send ack
        sendAck(iq);

        // check media stream does not exist yet
        QXmppCallPrivate::Stream *stream = findStreamByName(iq.content().name());
        if (stream)
            return;

        // create media stream
        stream = createStream(iq.content().descriptionMedia());
        if (!stream)
            return;
        stream->creator = iq.content().creator();
        stream->name = iq.content().name();

        // check content description
        if (!handleDescription(stream, iq.content()) ||
            !handleTransport(stream, iq.content())) {

            QXmppJingleIq iq;
            iq.setTo(q->jid());
            iq.setType(QXmppIq::Set);
            iq.setAction(QXmppJingleIq::ContentReject);
            iq.setSid(q->sid());
            iq.reason().setType(QXmppJingleIq::Reason::FailedApplication);
            sendRequest(iq);
            delete stream;
            return;
        }
        streams << stream;

         // accept content
        QXmppJingleIq iq;
        iq.setTo(q->jid());
        iq.setType(QXmppIq::Set);
        iq.setAction(QXmppJingleIq::ContentAccept);
        iq.setSid(q->sid());
        iq.content().setCreator(stream->creator);
        iq.content().setName(stream->name);

        // description
        iq.content().setDescriptionMedia(stream->media);
        foreach (const QXmppJinglePayloadType &payload, stream->channel->localPayloadTypes())
            iq.content().addPayloadType(payload);

        // transport
        iq.content().setTransportUser(stream->connection->localUser());
        iq.content().setTransportPassword(stream->connection->localPassword());
        foreach (const QXmppJingleCandidate &candidate, stream->connection->localCandidates())
            iq.content().addTransportCandidate(candidate);

        sendRequest(iq);

    } else if (iq.action() == QXmppJingleIq::TransportInfo) {

        // send ack
        sendAck(iq);

        // check content transport
        QXmppCallPrivate::Stream *stream = findStreamByName(iq.content().name());
        if (!stream ||
            !handleTransport(stream, iq.content())) {
            // FIXME: what action?
            return;
        }

    }
}

QXmppCallPrivate::Stream *QXmppCallPrivate::createStream(const QString &media)
{
    Q_ASSERT(manager);

    Stream *stream = new Stream;
    stream->media = media;

    // RTP channel
    QObject *channelObject = 0;
    if (media == AUDIO_MEDIA) {
        QXmppRtpAudioChannel *audioChannel = new QXmppRtpAudioChannel(q);
        stream->channel = audioChannel;
        channelObject = audioChannel;
    } else if (media == VIDEO_MEDIA) {
        QXmppRtpVideoChannel *videoChannel = new QXmppRtpVideoChannel(q);
        stream->channel = videoChannel;
        channelObject = videoChannel;
    } else {
        q->warning(QString("Unsupported media type %1").arg(media));
        delete stream;
        return 0;
    }

    // ICE connection
    stream->connection = new QXmppIceConnection(q);
    stream->connection->setIceControlling(direction == QXmppCall::OutgoingDirection);
    stream->connection->setStunServer(manager->d->stunHost, manager->d->stunPort);
    stream->connection->setTurnServer(manager->d->turnHost, manager->d->turnPort);
    stream->connection->setTurnUser(manager->d->turnUser);
    stream->connection->setTurnPassword(manager->d->turnPassword);
    stream->connection->addComponent(RTP_COMPONENT);
    stream->connection->addComponent(RTCP_COMPONENT);
    stream->connection->bind(QXmppIceComponent::discoverAddresses());

    // connect signals
    bool check = QObject::connect(stream->connection, SIGNAL(localCandidatesChanged()),
        q, SLOT(localCandidatesChanged()));
    Q_ASSERT(check);

    check = QObject::connect(stream->connection, SIGNAL(connected()),
        q, SLOT(updateOpenMode()));
    Q_ASSERT(check);

    check = QObject::connect(stream->connection, SIGNAL(disconnected()),
        q, SLOT(hangup()));
    Q_ASSERT(check);

    if (channelObject) {
        QXmppIceComponent *rtpComponent = stream->connection->component(RTP_COMPONENT);

        check = QObject::connect(rtpComponent, SIGNAL(datagramReceived(QByteArray)),
                        channelObject, SLOT(datagramReceived(QByteArray)));
        Q_ASSERT(check);

        check = QObject::connect(channelObject, SIGNAL(sendDatagram(QByteArray)),
                        rtpComponent, SLOT(sendDatagram(QByteArray)));
        Q_ASSERT(check);
    }
    return stream;
}

/// Sends an acknowledgement for a Jingle IQ.
///

bool QXmppCallPrivate::sendAck(const QXmppJingleIq &iq)
{
    QXmppIq ack;
    ack.setId(iq.id());
    ack.setTo(iq.from());
    ack.setType(QXmppIq::Result);
    return manager->client()->sendPacket(ack);
}

bool QXmppCallPrivate::sendInvite()
{
    QXmppJingleIq iq;
    iq.setTo(jid);
    iq.setType(QXmppIq::Set);
    iq.setAction(QXmppJingleIq::SessionInitiate);
    iq.setInitiator(ownJid);
    iq.setSid(sid);

    // create audio stream
    QXmppCallPrivate::Stream *stream = findStreamByMedia(AUDIO_MEDIA);
    Q_ASSERT(stream);
    iq.content().setCreator(stream->creator);
    iq.content().setName(stream->name);
    iq.content().setSenders("both");

    // description
    iq.content().setDescriptionMedia(stream->media);
    foreach (const QXmppJinglePayloadType &payload, stream->channel->localPayloadTypes())
        iq.content().addPayloadType(payload);

    // transport
    iq.content().setTransportUser(stream->connection->localUser());
    iq.content().setTransportPassword(stream->connection->localPassword());
    foreach (const QXmppJingleCandidate &candidate, stream->connection->localCandidates())
        iq.content().addTransportCandidate(candidate);

    return sendRequest(iq);
}

/// Sends a Jingle IQ and adds it to outstanding requests.
///

bool QXmppCallPrivate::sendRequest(const QXmppJingleIq &iq)
{
    requests << iq;
    return manager->client()->sendPacket(iq);
}

void QXmppCallPrivate::setState(QXmppCall::State newState)
{
    if (state != newState)
    {
        state = newState;
        emit q->stateChanged(state);
    }
}

QXmppCall::QXmppCall(const QString &jid, QXmppCall::Direction direction, QXmppCallManager *parent)
    : QXmppLoggable(parent)
{
    d = new QXmppCallPrivate(this);
    d->direction = direction;
    d->jid = jid;
    d->ownJid = parent->client()->configuration().jid();
    d->manager = parent;

    // create audio stream
    QXmppCallPrivate::Stream *stream = d->createStream(AUDIO_MEDIA);
    stream->creator = QLatin1String("initiator");
    stream->name = QLatin1String("voice");
    d->streams << stream;
}

QXmppCall::~QXmppCall()
{
    foreach (QXmppCallPrivate::Stream *stream, d->streams)
        delete stream;
    delete d;
}

/// Call this method if you wish to accept an incoming call.
///

void QXmppCall::accept()
{
    if (d->direction == IncomingDirection && d->state == OfferState)
    {
        Q_ASSERT(d->streams.size() == 1);
        QXmppCallPrivate::Stream *stream = d->streams.first();

        // accept incoming call
        QXmppJingleIq iq;
        iq.setTo(d->jid);
        iq.setType(QXmppIq::Set);
        iq.setAction(QXmppJingleIq::SessionAccept);
        iq.setResponder(d->ownJid);
        iq.setSid(d->sid);
        iq.content().setCreator(stream->creator);
        iq.content().setName(stream->name);

        // description
        iq.content().setDescriptionMedia(stream->media);
        foreach (const QXmppJinglePayloadType &payload, stream->channel->localPayloadTypes())
            iq.content().addPayloadType(payload);

        // transport
        iq.content().setTransportUser(stream->connection->localUser());
        iq.content().setTransportPassword(stream->connection->localPassword());
        foreach (const QXmppJingleCandidate &candidate, stream->connection->localCandidates())
            iq.content().addTransportCandidate(candidate);

        d->sendRequest(iq);

        // check for call establishment
        d->setState(QXmppCall::ConnectingState);
        updateOpenMode();
    }
}

/// Returns the RTP channel for the audio data.
///
/// It acts as a QIODevice so that you can read / write audio samples, for
/// instance using a QAudioOutput and a QAudioInput.
///

QXmppRtpAudioChannel *QXmppCall::audioChannel() const
{
    QXmppCallPrivate::Stream *stream = d->findStreamByMedia(AUDIO_MEDIA);
    Q_ASSERT(stream);
    return (QXmppRtpAudioChannel*)stream->channel;
}

/// Returns the RTP channel for the video data.
///

QXmppRtpVideoChannel *QXmppCall::videoChannel() const
{
    QXmppCallPrivate::Stream *stream = d->findStreamByMedia(VIDEO_MEDIA);
    Q_ASSERT(stream);
    return (QXmppRtpVideoChannel*)stream->channel;
}

void QXmppCall::terminate()
{
    if (d->state == FinishedState)
        return;

    d->state = QXmppCall::FinishedState;

    foreach (QXmppCallPrivate::Stream *stream, d->streams) {
        stream->channel->close();
        stream->connection->close();
    }

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
    if (d->state == QXmppCall::DisconnectingState ||
        d->state == QXmppCall::FinishedState)
        return;

    // hangup up call
    QXmppJingleIq iq;
    iq.setTo(d->jid);
    iq.setType(QXmppIq::Set);
    iq.setAction(QXmppJingleIq::SessionTerminate);
    iq.setSid(d->sid);
    d->sendRequest(iq);

    // close streams
    foreach (QXmppCallPrivate::Stream *stream, d->streams) {
        stream->channel->close();
        stream->connection->close();
    }

    // schedule forceful termination in 5s
    QTimer::singleShot(5000, this, SLOT(terminate()));
    d->setState(QXmppCall::DisconnectingState);
}

/// Sends a transport-info to inform the remote party of new local candidates.
///

void QXmppCall::localCandidatesChanged()
{
    // find the stream
    QXmppIceConnection *conn = qobject_cast<QXmppIceConnection*>(sender());
    QXmppCallPrivate::Stream *stream = 0;
    foreach (QXmppCallPrivate::Stream *ptr, d->streams) {
        if (ptr->connection == conn) {
            stream = ptr;
            break;
        }
    }
    if (!stream)
        return;

    QXmppJingleIq iq;
    iq.setTo(d->jid);
    iq.setType(QXmppIq::Set);
    iq.setAction(QXmppJingleIq::TransportInfo);
    iq.setInitiator(d->ownJid);
    iq.setSid(d->sid);

    iq.content().setCreator(stream->creator);
    iq.content().setName(stream->name);

    // transport
    iq.content().setTransportUser(stream->connection->localUser());
    iq.content().setTransportPassword(stream->connection->localPassword());
    foreach (const QXmppJingleCandidate &candidate, stream->connection->localCandidates())
        iq.content().addTransportCandidate(candidate);

    d->sendRequest(iq);
}

/// Returns the remote party's JID.
///

QString QXmppCall::jid() const
{
    return d->jid;
}

void QXmppCall::updateOpenMode()
{
    // determine audio mode
    QXmppCallPrivate::Stream *stream = d->findStreamByMedia(AUDIO_MEDIA);
    if (stream &&
        (stream->channel->openMode() & QIODevice::ReadWrite) &&
        stream->connection->isConnected() &&
        d->state == ConnectingState)
    {
        d->setState(ActiveState);
        emit connected();
    }
    
    // determine video mode
    stream = d->findStreamByMedia(VIDEO_MEDIA);
    QIODevice::OpenMode mode = QIODevice::NotOpen;
    if (stream) {
        if (stream->connection->isConnected())
            mode = stream->channel->openMode() & QIODevice::ReadWrite;
    }
    if (mode != d->videoMode) {
        d->videoMode = mode;
        emit videoModeChanged(mode);
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

void QXmppCall::startVideo()
{
    QXmppCallPrivate::Stream *stream = d->findStreamByMedia(VIDEO_MEDIA);
    if (stream)
        return;

    // create video stream
    stream = d->createStream(VIDEO_MEDIA);
    stream->creator = QLatin1String("initiator");
    stream->name = QLatin1String("webcam");
    d->streams << stream;

    // build request
    QXmppJingleIq iq;
    iq.setTo(d->jid);
    iq.setType(QXmppIq::Set);
    iq.setAction(QXmppJingleIq::ContentAdd);
    iq.setInitiator(d->ownJid);
    iq.setSid(d->sid);
    iq.content().setCreator(stream->creator);
    iq.content().setName(stream->name);
    iq.content().setSenders("both");

    // description
    iq.content().setDescriptionMedia(stream->media);
    foreach (const QXmppJinglePayloadType &payload, stream->channel->localPayloadTypes())
        iq.content().addPayloadType(payload);

    // transport
    iq.content().setTransportUser(stream->connection->localUser());
    iq.content().setTransportPassword(stream->connection->localPassword());
    foreach (const QXmppJingleCandidate &candidate, stream->connection->localCandidates())
        iq.content().addTransportCandidate(candidate);

    d->sendRequest(iq);
}

QXmppCallManagerPrivate::QXmppCallManagerPrivate(QXmppCallManager *qq)
    : stunPort(0),
    turnPort(0),
    q(qq)
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
        << ns_jingle_rtp_video
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
    if (jid == client()->configuration().jid()) {
        warning("Refusing to call self");
        return 0;
    }

    QXmppCall *call = new QXmppCall(jid, QXmppCall::OutgoingDirection, this);
    call->d->sid = generateStanzaHash();

    // register call
    d->calls << call;
    connect(call, SIGNAL(destroyed(QObject*)),
        this, SLOT(callDestroyed(QObject*)));

    call->d->sendInvite();
    return call;
}

void QXmppCallManager::callDestroyed(QObject *object)
{
    d->calls.removeAll(static_cast<QXmppCall*>(object));
}

/// Handles acknowledgements
///

void QXmppCallManager::iqReceived(const QXmppIq &ack)
{
    if (ack.type() != QXmppIq::Result)
        return;

    // find request
    foreach (QXmppCall *call, d->calls)
        call->d->handleAck(ack);
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

        QXmppCallPrivate::Stream *stream = call->d->findStreamByMedia(iq.content().descriptionMedia());
        if (!stream)
            return;
        stream->creator = iq.content().creator();
        stream->name = iq.content().name();

        // send ack
        call->d->sendAck(iq);

        // check content description and transport
        if (!call->d->handleDescription(stream, iq.content()) ||
            !call->d->handleTransport(stream, iq.content())) {

            // terminate call
            QXmppJingleIq iq;
            iq.setTo(call->jid());
            iq.setType(QXmppIq::Set);
            iq.setAction(QXmppJingleIq::SessionTerminate);
            iq.setSid(call->sid());
            iq.reason().setType(QXmppJingleIq::Reason::FailedApplication);
            call->d->sendRequest(iq);

            delete call;
            return;
        }

        // register call
        d->calls << call;
        connect(call, SIGNAL(destroyed(QObject*)),
            this, SLOT(callDestroyed(QObject*)));

        // send ringing indication
        QXmppJingleIq ringing;
        ringing.setTo(call->jid());
        ringing.setType(QXmppIq::Set);
        ringing.setAction(QXmppJingleIq::SessionInfo);
        ringing.setSid(call->sid());
        ringing.setRinging(true);
        call->d->sendRequest(ringing);

        // notify user
        emit callReceived(call);
        return;

    } else {

        // for all other requests, require a valid call
        QXmppCall *call = d->findCall(iq.sid());
        if (!call) {
            warning(QString("Remote party %1 sent a request for an unknown call %2").arg(iq.from(), iq.sid()));
            return;
        }
        call->d->handleRequest(iq);
    }
}

/// Sets the STUN server to use to determine server-reflexive addresses
/// and ports.
///
/// \param host The address of the STUN server.
/// \param port The port of the STUN server.

void QXmppCallManager::setStunServer(const QHostAddress &host, quint16 port)
{
    d->stunHost = host;
    d->stunPort = port;
}

/// Sets the TURN server to use to relay packets in double-NAT configurations.
///
/// \param host The address of the TURN server.
/// \param port The port of the TURN server.

void QXmppCallManager::setTurnServer(const QHostAddress &host, quint16 port)
{
    d->turnHost = host;
    d->turnPort = port;
}

/// Sets the \a user used for authentication with the TURN server.
///
/// \param user

void QXmppCallManager::setTurnUser(const QString &user)
{
    d->turnUser = user;
}

/// Sets the \a password used for authentication with the TURN server.
///
/// \param password

void QXmppCallManager::setTurnPassword(const QString &password)
{
    d->turnPassword = password;
}

