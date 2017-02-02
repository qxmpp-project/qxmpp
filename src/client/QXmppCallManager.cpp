/*
 * Copyright (C) 2008-2014 The QXmpp developers
 *
 * Author:
 *  Jeremy Lainé
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

#include <QDomElement>
#include <QTimer>

#include "QXmppCallManager.h"
#include "QXmppClient.h"
#include "QXmppConstants_p.h"
#include "QXmppJingleIq.h"
#include "QXmppRtpChannel.h"
#include "QXmppStun.h"
#include "QXmppUtils.h"

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
    QXmppJingleIq::Content localContent(QXmppCallPrivate::Stream *stream) const;

    void handleAck(const QXmppIq &iq);
    bool handleDescription(QXmppCallPrivate::Stream *stream, const QXmppJingleIq::Content &content);
    void handleRequest(const QXmppJingleIq &iq);
    bool handleTransport(QXmppCallPrivate::Stream *stream, const QXmppJingleIq::Content &content);
    void setState(QXmppCall::State state);
    bool sendAck(const QXmppJingleIq &iq);
    bool sendInvite();
    bool sendRequest(const QXmppJingleIq &iq);
    void terminate(QXmppJingleIq::Reason::Type reasonType);

    QXmppCall::Direction direction;
    QString jid;
    QString ownJid;
    QXmppCallManager *manager;
    QList<QXmppJingleIq> requests;
    QString sid;
    QXmppCall::State state;

    // Media streams
    bool sendVideo;
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
    : direction(QXmppCall::IncomingDirection),
    manager(0),
    state(QXmppCall::ConnectingState),
    sendVideo(false),
    audioMode(QIODevice::NotOpen),
    videoMode(QIODevice::NotOpen),
    q(qq)
{
    qRegisterMetaType<QXmppCall::State>();
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
                q->terminated();
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
    const QXmppJingleIq::Content content = iq.contents().isEmpty() ? QXmppJingleIq::Content() : iq.contents().first();

    if (iq.action() == QXmppJingleIq::SessionAccept) {

        if (direction == QXmppCall::IncomingDirection) {
            q->warning("Ignoring Session-Accept for an incoming call");
            return;
        }

        // send ack
        sendAck(iq);

        // check content description and transport
        QXmppCallPrivate::Stream *stream = findStreamByName(content.name());
        if (!stream ||
            !handleDescription(stream, content) ||
            !handleTransport(stream, content)) {

            // terminate call
            terminate(QXmppJingleIq::Reason::FailedApplication);
            return;
        }

        // check for call establishment
        setState(QXmppCall::ActiveState);

    } else if (iq.action() == QXmppJingleIq::SessionInfo) {

        // notify user
        QTimer::singleShot(0, q, SIGNAL(ringing()));

    } else if (iq.action() == QXmppJingleIq::SessionTerminate) {

        // send ack
        sendAck(iq);

        // terminate
        q->info(QString("Remote party %1 terminated call %2").arg(iq.from(), iq.sid()));
        q->terminated();

    } else if (iq.action() == QXmppJingleIq::ContentAccept) {

        // send ack
        sendAck(iq);

        // check content description and transport
        QXmppCallPrivate::Stream *stream = findStreamByName(content.name());
        if (!stream ||
            !handleDescription(stream, content) ||
            !handleTransport(stream, content)) {

            // FIXME: what action?
            return;
        }

    } else if (iq.action() == QXmppJingleIq::ContentAdd) {

        // send ack
        sendAck(iq);

        // check media stream does not exist yet
        QXmppCallPrivate::Stream *stream = findStreamByName(content.name());
        if (stream)
            return;

        // create media stream
        stream = createStream(content.descriptionMedia());
        if (!stream)
            return;
        stream->creator = content.creator();
        stream->name = content.name();

        // check content description
        if (!handleDescription(stream, content) ||
            !handleTransport(stream, content)) {

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
        iq.addContent(localContent(stream));
        sendRequest(iq);

    } else if (iq.action() == QXmppJingleIq::TransportInfo) {

        // send ack
        sendAck(iq);

        // check content transport
        QXmppCallPrivate::Stream *stream = findStreamByName(content.name());
        if (!stream ||
            !handleTransport(stream, content)) {
            // FIXME: what action?
            return;
        }

    }
}

QXmppCallPrivate::Stream *QXmppCallPrivate::createStream(const QString &media)
{
    bool check;
    Q_UNUSED(check);
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
    check = QObject::connect(stream->connection, SIGNAL(localCandidatesChanged()),
        q, SLOT(localCandidatesChanged()));
    Q_ASSERT(check);

    check = QObject::connect(stream->connection, SIGNAL(connected()),
        q, SLOT(updateOpenMode()));
    Q_ASSERT(check);

    check = QObject::connect(q, SIGNAL(stateChanged(QXmppCall::State)),
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

QXmppJingleIq::Content QXmppCallPrivate::localContent(QXmppCallPrivate::Stream *stream) const
{
    QXmppJingleIq::Content content;
    content.setCreator(stream->creator);
    content.setName(stream->name);
    content.setSenders("both");

    // description
    content.setDescriptionMedia(stream->media);
    content.setDescriptionSsrc(stream->channel->localSsrc());
    content.setPayloadTypes(stream->channel->localPayloadTypes());

    // transport
    content.setTransportUser(stream->connection->localUser());
    content.setTransportPassword(stream->connection->localPassword());
    content.setTransportCandidates(stream->connection->localCandidates());

    return content;
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
    // create audio stream
    QXmppCallPrivate::Stream *stream = findStreamByMedia(AUDIO_MEDIA);
    Q_ASSERT(stream);

    QXmppJingleIq iq;
    iq.setTo(jid);
    iq.setType(QXmppIq::Set);
    iq.setAction(QXmppJingleIq::SessionInitiate);
    iq.setInitiator(ownJid);
    iq.setSid(sid);
    iq.addContent(localContent(stream));
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

        if (state == QXmppCall::ActiveState)
            emit q->connected();
        else if (state == QXmppCall::FinishedState)
            emit q->finished();
    }
}

/// Request graceful call termination

void QXmppCallPrivate::terminate(QXmppJingleIq::Reason::Type reasonType)
{
    if (state == QXmppCall::DisconnectingState ||
        state == QXmppCall::FinishedState)
        return;

    // hangup call
    QXmppJingleIq iq;
    iq.setTo(jid);
    iq.setType(QXmppIq::Set);
    iq.setAction(QXmppJingleIq::SessionTerminate);
    iq.setSid(sid);
    iq.reason().setType(reasonType);
    sendRequest(iq);
    setState(QXmppCall::DisconnectingState);

    // schedule forceful termination in 5s
    QTimer::singleShot(5000, q, SLOT(terminated()));
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
    if (d->direction == IncomingDirection && d->state == ConnectingState)
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
        iq.addContent(d->localContent(stream));
        d->sendRequest(iq);

        // notify user
        d->manager->callStarted(this);

        // check for call establishment
        d->setState(QXmppCall::ActiveState);
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
    if (stream)
        return static_cast<QXmppRtpAudioChannel*>(stream->channel);
    else
        return 0;
}

/// Returns the audio mode.

QIODevice::OpenMode QXmppCall::audioMode() const
{
    return d->audioMode;
}

/// Returns the RTP channel for the video data.
///

QXmppRtpVideoChannel *QXmppCall::videoChannel() const
{
    QXmppCallPrivate::Stream *stream = d->findStreamByMedia(VIDEO_MEDIA);
    if (stream)
        return static_cast<QXmppRtpVideoChannel*>(stream->channel);
    else
        return 0;
}

/// Returns the video mode.

QIODevice::OpenMode QXmppCall::videoMode() const
{
    return d->videoMode;
}

void QXmppCall::terminated()
{
    // close streams
    foreach (QXmppCallPrivate::Stream *stream, d->streams) {
        stream->channel->close();
        stream->connection->close();
    }

    // update state
    d->setState(QXmppCall::FinishedState);
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
    d->terminate(QXmppJingleIq::Reason::None);
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
    iq.setSid(d->sid);
    iq.addContent(d->localContent(stream));
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
    QXmppCallPrivate::Stream *stream;
    QIODevice::OpenMode mode;

    // determine audio mode
    mode = QIODevice::NotOpen;
    stream = d->findStreamByMedia(AUDIO_MEDIA);
    if (d->state == QXmppCall::ActiveState && stream && stream->connection->isConnected())
        mode = stream->channel->openMode() & QIODevice::ReadWrite;
    if (mode != d->audioMode) {
        d->audioMode = mode;
        emit audioModeChanged(mode);
    }

    // determine video mode
    mode = QIODevice::NotOpen;
    stream = d->findStreamByMedia(VIDEO_MEDIA);
    if (d->state == QXmppCall::ActiveState && stream && stream->connection->isConnected()) {
        mode |= (stream->channel->openMode() & QIODevice::ReadOnly);
        if (d->sendVideo)
            mode |= (stream->channel->openMode() & QIODevice::WriteOnly);
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

/// Starts sending video to the remote party.

void QXmppCall::startVideo()
{
    if (d->state != QXmppCall::ActiveState) {
        warning("Cannot start video, call is not active");
        return;
    }

    d->sendVideo = true;
    QXmppCallPrivate::Stream *stream = d->findStreamByMedia(VIDEO_MEDIA);
    if (stream) {
        updateOpenMode();
        return;
    }

    // create video stream
    stream = d->createStream(VIDEO_MEDIA);
    stream->creator = (d->direction == QXmppCall::OutgoingDirection) ? QLatin1String("initiator") : QLatin1String("responder");
    stream->name = QLatin1String("webcam");
    d->streams << stream;

    // build request
    QXmppJingleIq iq;
    iq.setTo(d->jid);
    iq.setType(QXmppIq::Set);
    iq.setAction(QXmppJingleIq::ContentAdd);
    iq.setSid(d->sid);
    iq.addContent(d->localContent(stream));
    d->sendRequest(iq);
}

/// Stops sending video to the remote party.

void QXmppCall::stopVideo()
{
    if (!d->sendVideo)
        return;

    d->sendVideo = false;
    QXmppCallPrivate::Stream *stream = d->findStreamByMedia(VIDEO_MEDIA);
    if (stream)
        updateOpenMode();
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

/// \cond
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
            _q_jingleIqReceived(jingleIq);
            return true;
        }
    }

    return false;
}

void QXmppCallManager::setClient(QXmppClient *client)
{
    bool check;
    Q_UNUSED(check);

    QXmppClientExtension::setClient(client);

    check = connect(client, SIGNAL(disconnected()),
                    this, SLOT(_q_disconnected()));
    Q_ASSERT(check);

    check = connect(client, SIGNAL(iqReceived(QXmppIq)),
                    this, SLOT(_q_iqReceived(QXmppIq)));
    Q_ASSERT(check);

    check = connect(client, SIGNAL(presenceReceived(QXmppPresence)),
                    this, SLOT(_q_presenceReceived(QXmppPresence)));
    Q_ASSERT(check);
}
/// \endcond

/// Initiates a new outgoing call to the specified recipient.
///
/// \param jid

QXmppCall *QXmppCallManager::call(const QString &jid)
{
    bool check;
    Q_UNUSED(check);

    if (jid.isEmpty()) {
        warning("Refusing to call an empty jid");
        return 0;
    }

    if (jid == client()->configuration().jid()) {
        warning("Refusing to call self");
        return 0;
    }

    QXmppCall *call = new QXmppCall(jid, QXmppCall::OutgoingDirection, this);
    call->d->sid = QXmppUtils::generateStanzaHash();

    // register call
    d->calls << call;
    check = connect(call, SIGNAL(destroyed(QObject*)),
                    this, SLOT(_q_callDestroyed(QObject*)));
    Q_ASSERT(check);
    emit callStarted(call);

    call->d->sendInvite();

    return call;
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

/// Handles call destruction.

void QXmppCallManager::_q_callDestroyed(QObject *object)
{
    d->calls.removeAll(static_cast<QXmppCall*>(object));
}

/// Handles disconnection from server.

void QXmppCallManager::_q_disconnected()
{
    foreach (QXmppCall *call, d->calls)
        call->d->terminate(QXmppJingleIq::Reason::Gone);
}

/// Handles acknowledgements.
///

void QXmppCallManager::_q_iqReceived(const QXmppIq &ack)
{
    if (ack.type() != QXmppIq::Result)
        return;

    // find request
    foreach (QXmppCall *call, d->calls)
        call->d->handleAck(ack);
}

/// Handles a Jingle IQ.
///

void QXmppCallManager::_q_jingleIqReceived(const QXmppJingleIq &iq)
{
    bool check;
    Q_UNUSED(check);

    if (iq.type() != QXmppIq::Set)
        return;

    if (iq.action() == QXmppJingleIq::SessionInitiate)
    {
        // build call
        QXmppCall *call = new QXmppCall(iq.from(), QXmppCall::IncomingDirection, this);
        call->d->sid = iq.sid();

        const QXmppJingleIq::Content content = iq.contents().isEmpty() ? QXmppJingleIq::Content() : iq.contents().first();
        QXmppCallPrivate::Stream *stream = call->d->findStreamByMedia(content.descriptionMedia());
        if (!stream)
            return;
        stream->creator = content.creator();
        stream->name = content.name();

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
        check = connect(call, SIGNAL(destroyed(QObject*)),
                        this, SLOT(_q_callDestroyed(QObject*)));
        Q_ASSERT(check);

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

/// Handles a presence.

void QXmppCallManager::_q_presenceReceived(const QXmppPresence &presence)
{
    if (presence.type() != QXmppPresence::Unavailable)
        return;

    foreach (QXmppCall *call, d->calls) {
        if (presence.from() == call->jid()) {
            // the remote party has gone away, terminate call
            call->d->terminate(QXmppJingleIq::Reason::Gone);
        }
    }
}

