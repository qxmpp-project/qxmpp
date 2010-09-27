/*
 * Copyright (C) 2008-2010 The QXmpp developers
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
#include "QXmppStun.h"
#include "QXmppUtils.h"

const int RTP_COMPONENT = 1;
const int RTCP_COMPONENT = 2;

const quint8 RTP_VERSION = 0x02;

enum CodecId {
    G711u = 0,
    GSM = 3,
    G723 = 4,
    G711a = 8,
    G722 = 9,
    L16Stereo = 10,
    L16Mono = 11,
    G728 = 15,
    G729 = 18,
};

#define SAMPLE_BYTES 2

QXmppCall::QXmppCall(const QString &jid, QXmppCall::Direction direction, QObject *parent)
    : QIODevice(parent),
    m_direction(direction),
    m_jid(jid),
    m_state(OfferState),
    m_signalsEmitted(false),
    m_writtenSinceLastEmit(0),
    m_codec(0),
    m_incomingBuffering(true),
    m_incomingMinimum(0),
    m_incomingSequence(0),
    m_incomingStamp(0),
    m_outgoingMarker(true),
    m_outgoingSequence(0),
    m_outgoingStamp(0)
{
    m_contentCreator = QLatin1String("initiator");
    m_contentName = QLatin1String("voice");

    // ICE connection
    bool iceControlling = (m_direction == OutgoingDirection);
    m_connection = new QXmppIceConnection(iceControlling, this);
    //m_connection->setStunServer("stun.ekiga.net");
    m_connection->addComponent(RTP_COMPONENT);
    m_connection->addComponent(RTCP_COMPONENT);

    bool check = connect(m_connection, SIGNAL(logMessage(QXmppLogger::MessageType, QString)),
        this, SIGNAL(logMessage(QXmppLogger::MessageType, QString)));
    Q_ASSERT(check);

    check = connect(m_connection, SIGNAL(localCandidatesChanged()),
        this, SIGNAL(localCandidatesChanged()));
    Q_ASSERT(check);

    check = connect(m_connection, SIGNAL(connected()),
        this, SLOT(updateOpenMode()));
    Q_ASSERT(check);

    check = connect(m_connection, SIGNAL(datagramReceived(int,QByteArray)),
        this, SLOT(datagramReceived(int, QByteArray)));
    Q_ASSERT(check);
}

/// Call this method if you wish to accept an incoming call.
///

void QXmppCall::accept()
{
    if (m_direction == IncomingDirection && m_state == OfferState)
        setState(QXmppCall::ConnectingState);
}

/// Returns the number of bytes that are available for reading.
///

qint64 QXmppCall::bytesAvailable() const
{
    return m_incomingBuffer.size();
}

void QXmppCall::terminate()
{
    if (m_state == FinishedState)
        return;

    m_state = QXmppCall::FinishedState;

    close();
    m_connection->close();

    // emit signals later
    QTimer::singleShot(0, this, SLOT(terminated()));
}

void QXmppCall::terminated()
{
    emit stateChanged(m_state);
    emit finished();
}

/// Returns the call's direction.
///

QXmppCall::Direction QXmppCall::direction() const
{
    return m_direction;
}

void QXmppCall::emitSignals()
{
    emit bytesWritten(m_writtenSinceLastEmit);
    m_writtenSinceLastEmit = 0;
    m_signalsEmitted = false;
}

/// Hangs up the call.
///

void QXmppCall::hangup()
{
    if (m_state != QXmppCall::FinishedState)
        setState(QXmppCall::DisconnectingState);
}

bool QXmppCall::isSequential() const
{
    return true;
}

/// Returns the remote party's JID.
///

QString QXmppCall::jid() const
{
    return m_jid;
}

/// Returns the negociated payload type.
///
/// You can use this to determine the QAudioFormat to use with your
/// QAudioInput/QAudioOutput.

QXmppJinglePayloadType QXmppCall::payloadType() const
{
    return m_payloadType;
}

void QXmppCall::setPayloadType(const QXmppJinglePayloadType &payloadType)
{
    m_payloadType = payloadType;
    if (payloadType.id() == G711u)
        m_codec = new QXmppG711uCodec(payloadType.clockrate());
    else if (payloadType.id() == G711a)
        m_codec = new QXmppG711aCodec(payloadType.clockrate());
#ifdef QXMPP_USE_SPEEX
    else if (payloadType.name().toLower() == "speex")
        m_codec = new QXmppSpeexCodec(payloadType.clockrate());
#endif
    else
    {
        emit logMessage(QXmppLogger::WarningMessage,
            QString("QXmppCall got an unknown codec : %1 (%2)")
                .arg(QString::number(payloadType.id()))
                .arg(payloadType.name()));
        return;
    }

    // size in bytes of an unencoded packet
    m_outgoingChunk = SAMPLE_BYTES * payloadType.ptime() * payloadType.clockrate() / 1000;

    // initial number of bytes to buffer
    m_incomingMinimum = m_outgoingChunk * 5;

    updateOpenMode();
}

void QXmppCall::addRemoteCandidates(const QList<QXmppJingleCandidate> &candidates)
{
    foreach (const QXmppJingleCandidate &candidate, candidates)
        m_connection->addRemoteCandidate(candidate);
}

void QXmppCall::updateOpenMode()
{
    // determine mode
    if (m_codec && m_connection->isConnected() && m_state != ActiveState)
    {
        open(QIODevice::ReadWrite);
        setState(ActiveState);
        emit connected();
    }
}

void QXmppCall::datagramReceived(int component, const QByteArray &buffer)
{
    if (component != RTP_COMPONENT)
        return;

    if (!m_codec)
    {
        emit logMessage(QXmppLogger::WarningMessage,
            QLatin1String("QXmppCall:datagramReceived before codec selection"));
        return;
    }

    if (buffer.size() < 12 || (quint8(buffer.at(0)) >> 6) != RTP_VERSION)
    {
        emit logMessage(QXmppLogger::WarningMessage,
            QLatin1String("QXmppCall::datagramReceived got an invalid RTP packet"));
        return;
    }

    // parse RTP header
    QDataStream stream(buffer);
    quint8 version, type;
    quint32 ssrc;
    quint16 sequence;
    quint32 stamp;
    stream >> version;
    stream >> type;
    stream >> sequence;
    stream >> stamp;
    stream >> ssrc;
    const qint64 packetLength = buffer.size() - 12;

#ifdef QXMPP_DEBUG_RTP
    emit logMessage(QXmppLogger::ReceivedMessage,
        QString("RTP packet seq %1 stamp %2 size %3")
            .arg(QString::number(sequence))
            .arg(QString::number(stamp))
            .arg(QString::number(packetLength)));
#endif

    // check sequence number
    if (sequence != m_incomingSequence + 1)
        emit logMessage(QXmppLogger::WarningMessage,
            QString("RTP packet seq %1 is out of order, previous was %2")
                .arg(QString::number(sequence))
                .arg(QString::number(m_incomingSequence)));
    m_incomingSequence = sequence;

    // determine packet's position in the buffer (in bytes)
    qint64 packetOffset = 0;
    if (!buffer.isEmpty())
    {
        packetOffset = (stamp - m_incomingStamp) * SAMPLE_BYTES;
        if (packetOffset < 0)
        {
            emit logMessage(QXmppLogger::WarningMessage,
                QString("RTP packet stamp %1 is too old, buffer start is %2")
                    .arg(QString::number(stamp))
                    .arg(QString::number(m_incomingStamp)));
            return;
        }
    } else {
        m_incomingStamp = stamp;
    }

    // allocate space for new packet
    if (packetOffset + packetLength > m_incomingBuffer.size())
        m_incomingBuffer += QByteArray(packetOffset + packetLength - m_incomingBuffer.size(), 0);
    QDataStream output(&m_incomingBuffer, QIODevice::WriteOnly);
    output.device()->seek(packetOffset);
    output.setByteOrder(QDataStream::LittleEndian);
    m_codec->decode(stream, output);

    // check whether we have filled the initial buffer
    if (m_incomingBuffer.size() >= m_incomingMinimum)
        m_incomingBuffering = false;
    if (!m_incomingBuffering)
        emit readyRead();
}

/// Returns the call's session identifier.
///

QString QXmppCall::sid() const
{
    return m_sid;
}

/// Returns the call's state.
///
/// \sa stateChanged()

QXmppCall::State QXmppCall::state() const
{
    return m_state;
}

void QXmppCall::setState(QXmppCall::State state)
{
    if (m_state != state)
    {
        m_state = state;
        emit stateChanged(m_state);
    }
}

qint64 QXmppCall::readData(char * data, qint64 maxSize)
{
    // if we are filling the buffer, return empty samples
    if (m_incomingBuffering)
    {
        memset(data, 0, maxSize);
        return maxSize;
    }

    qint64 readSize = qMin(maxSize, qint64(m_incomingBuffer.size()));
    memcpy(data, m_incomingBuffer.constData(), readSize);
    m_incomingBuffer.remove(0, readSize);
    if (readSize < maxSize)
    {
        emit logMessage(QXmppLogger::InformationMessage,
            QString("QXmppCall::readData missing %1 bytes").arg(QString::number(maxSize - readSize)));
        memset(data + readSize, 0, maxSize - readSize);
    }
    m_incomingStamp += readSize / SAMPLE_BYTES;
    return maxSize;
}

qint64 QXmppCall::writeData(const char * data, qint64 maxSize)
{
    if (!m_codec)
    {
        emit logMessage(QXmppLogger::WarningMessage,
            QLatin1String("QXmppCall::writeData before codec was set"));
        return -1;
    }

    m_outgoingBuffer += QByteArray::fromRawData(data, maxSize);
    while (m_outgoingBuffer.size() >= m_outgoingChunk)
    {
        QByteArray header;
        QDataStream stream(&header, QIODevice::WriteOnly);
        quint8 version = RTP_VERSION << 6;
        stream << version;
        quint8 type = m_payloadType.id();
        if (m_outgoingMarker)
        {
            type |= 0x80;
            m_outgoingMarker= false;
        }
        stream << type;
        stream << ++m_outgoingSequence;
        stream << m_outgoingStamp;
        const quint32 ssrc = 0;
        stream << ssrc;

        QByteArray chunk = m_outgoingBuffer.left(m_outgoingChunk);
        QDataStream input(chunk);
        input.setByteOrder(QDataStream::LittleEndian);
        m_outgoingStamp += m_codec->encode(input, stream);

        if (m_connection->writeDatagram(RTP_COMPONENT, header) < 0)
            emit logMessage(QXmppLogger::WarningMessage,
                QLatin1String("QXmppCall:writeData could not send audio data"));
#ifdef QXMPP_DEBUG_RTP
        else
            emit logMessage(QXmppLogger::SentMessage,
                QString("RTP packet seq %1 stamp %2 size %3")
                    .arg(QString::number(m_outgoingSequence))
                    .arg(QString::number(m_outgoingStamp))
                    .arg(QString::number(header.size() - 12)));
#endif

        m_outgoingBuffer.remove(0, chunk.size());
    }

    m_writtenSinceLastEmit += maxSize;
    if (!m_signalsEmitted && !signalsBlocked()) {
        m_signalsEmitted = true;
        QMetaObject::invokeMethod(this, "emitSignals", Qt::QueuedConnection);
    }

    return maxSize;
}

QXmppCallManager::QXmppCallManager(QXmppClient *client)
{
    bool check = connect(client, SIGNAL(iqReceived(QXmppIq)),
        this, SLOT(iqReceived(QXmppIq)));
    Q_ASSERT(check);
    Q_UNUSED(check);
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

/// Initiates a new outgoing call to the specified recipient.
///
/// \param jid

QXmppCall *QXmppCallManager::call(const QString &jid)
{
    QXmppCall *call = new QXmppCall(jid, QXmppCall::OutgoingDirection, this);
    call->m_sid = generateStanzaHash();
    m_calls << call;
    connect(call, SIGNAL(destroyed(QObject*)), this, SLOT(callDestroyed(QObject*)));
    connect(call, SIGNAL(stateChanged(QXmppCall::State)),
        this, SLOT(callStateChanged(QXmppCall::State)));
    connect(call, SIGNAL(localCandidatesChanged()),
        this, SLOT(localCandidatesChanged()));
    connect(call, SIGNAL(logMessage(QXmppLogger::MessageType, QString)),
        this, SIGNAL(logMessage(QXmppLogger::MessageType, QString)));

    QXmppJingleIq iq;
    iq.setTo(jid);
    iq.setType(QXmppIq::Set);
    iq.setAction(QXmppJingleIq::SessionInitiate);
    iq.setInitiator(client()->configuration().jid());
    iq.setSid(call->m_sid);
    iq.content().setCreator(call->m_contentCreator);
    iq.content().setName(call->m_contentName);
    iq.content().setSenders("both");

    // description
    iq.content().setDescriptionMedia("audio");
    foreach (const QXmppJinglePayloadType &payload, localPayloadTypes())
        iq.content().addPayloadType(payload);

    // transport
    iq.content().setTransportUser(call->m_connection->localUser());
    iq.content().setTransportPassword(call->m_connection->localPassword());
    foreach (const QXmppJingleCandidate &candidate, call->m_connection->localCandidates())
        iq.content().addTransportCandidate(candidate);

    sendRequest(call, iq);

    return call;
}

void QXmppCallManager::callDestroyed(QObject *object)
{
    m_calls.removeAll(static_cast<QXmppCall*>(object));
}

void QXmppCallManager::callStateChanged(QXmppCall::State state)
{
    QXmppCall *call = qobject_cast<QXmppCall*>(sender());
    if (!call || !m_calls.contains(call))
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
        iq.setSid(call->m_sid);
        sendRequest(call, iq);

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
        iq.setSid(call->m_sid);
        iq.content().setCreator(call->m_contentCreator);
        iq.content().setName(call->m_contentName);

        // description
        iq.content().setDescriptionMedia("audio");
        foreach (const QXmppJinglePayloadType &payload, call->m_commonPayloadTypes)
            iq.content().addPayloadType(payload);

        // transport
        iq.content().setTransportUser(call->m_connection->localUser());
        iq.content().setTransportPassword(call->m_connection->localPassword());
        foreach (const QXmppJingleCandidate &candidate, call->m_connection->localCandidates())
            iq.content().addTransportCandidate(candidate);

        sendRequest(call, iq);

        // perform ICE negotiation
        call->m_connection->connectToHost();
    }
}

/// Determine common payload types for a call.
///

bool QXmppCallManager::checkPayloadTypes(QXmppCall *call, const QList<QXmppJinglePayloadType> &remotePayloadTypes)
{
    foreach (const QXmppJinglePayloadType &payload, localPayloadTypes())
    {
        int payloadIndex = remotePayloadTypes.indexOf(payload);
        if (payloadIndex >= 0)
            call->m_commonPayloadTypes << remotePayloadTypes[payloadIndex];
    }
    if (call->m_commonPayloadTypes.isEmpty())
    {
         emit logMessage(QXmppLogger::WarningMessage,
            QString("Remote party %1 did not provide any known payload types for call %2").arg(call->jid(), call->sid()));

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
        call->setPayloadType(call->m_commonPayloadTypes.first());
        return true;
    }
}

QXmppCall *QXmppCallManager::findCall(const QString &sid) const
{
    foreach (QXmppCall *call, m_calls)
        if (call->m_sid == sid)
           return call;
    return 0;
}

QXmppCall *QXmppCallManager::findCall(const QString &sid, QXmppCall::Direction direction) const
{
    foreach (QXmppCall *call, m_calls)
        if (call->m_sid == sid && call->m_direction == direction)
           return call;
    return 0;
}

/// Returns the list of locally supported payload types.
///

QList<QXmppJinglePayloadType> QXmppCallManager::localPayloadTypes() const
{
    QList<QXmppJinglePayloadType> payloads;
    QXmppJinglePayloadType payload;

#ifdef QXMPP_USE_SPEEX
    payload.setId(96);
    payload.setChannels(1);
    payload.setName("SPEEX");
    payload.setClockrate(16000);
    payloads << payload;

    payload.setId(97);
    payload.setChannels(1);
    payload.setName("SPEEX");
    payload.setClockrate(8000);
    payloads << payload;
#endif

    payload.setId(G711u);
    payload.setChannels(1);
    payload.setName("PCMU");
    payload.setClockrate(8000);
    payloads << payload;

    payload.setId(G711a);
    payload.setChannels(1);
    payload.setName("PCMA");
    payload.setClockrate(8000);
    payloads << payload;

    return payloads;
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
    foreach (call, m_calls)
    {
        for (int i = 0; i < call->m_requests.size(); i++)
        {
            if (ack.id() == call->m_requests[i].id())
            {
                request = call->m_requests.takeAt(i);
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
    emit logMessage(QXmppLogger::DebugMessage, QString("Received ACK for packet %1").arg(ack.id()));
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
        call->m_sid = iq.sid();
        call->m_contentCreator = iq.content().creator();
        call->m_contentName = iq.content().name();
        call->m_connection->setRemoteUser(iq.content().transportUser());
        call->m_connection->setRemotePassword(iq.content().transportPassword());
        call->addRemoteCandidates(iq.content().transportCandidates());

        // send ack
        sendAck(iq);

        // determine common payload types
        if (!checkPayloadTypes(call, iq.content().payloadTypes()))
        {
            delete call;
            return;
        }

        // register call
        m_calls.append(call);
        connect(call, SIGNAL(stateChanged(QXmppCall::State)),
            this, SLOT(callStateChanged(QXmppCall::State)));
        connect(call, SIGNAL(localCandidatesChanged()),
            this, SLOT(localCandidatesChanged()));
        connect(call, SIGNAL(logMessage(QXmppLogger::MessageType, QString)),
            this, SIGNAL(logMessage(QXmppLogger::MessageType, QString)));

        // send ringing indication
        QXmppJingleIq ringing;
        ringing.setTo(call->jid());
        ringing.setType(QXmppIq::Set);
        ringing.setAction(QXmppJingleIq::SessionInfo);
        ringing.setSid(call->sid());
        ringing.setRinging(true);
        sendRequest(call, ringing);

        // notify user
        emit callReceived(call);

    } else if (iq.action() == QXmppJingleIq::SessionAccept) {
        QXmppCall *call = findCall(iq.sid(), QXmppCall::OutgoingDirection);
        if (!call)
        {
            emit logMessage(QXmppLogger::WarningMessage,
                QString("Remote party %1 accepted unknown call %2").arg(iq.from(), iq.sid()));
            return;
        }

        // send ack
        sendAck(iq);

        // determine common payload types
        if (!checkPayloadTypes(call, iq.content().payloadTypes()))
        {
            delete call;
            return;
        }

        // perform ICE negotiation
        if (!iq.content().transportCandidates().isEmpty())
        {
            call->m_connection->setRemoteUser(iq.content().transportUser());
            call->m_connection->setRemotePassword(iq.content().transportPassword());
            call->addRemoteCandidates(iq.content().transportCandidates());
        }
        call->m_connection->connectToHost();

    } else if (iq.action() == QXmppJingleIq::SessionInfo) {

        QXmppCall *call = findCall(iq.sid());
        if (!call)
            return;

        // notify user
        QTimer::singleShot(0, call, SIGNAL(ringing()));

    } else if (iq.action() == QXmppJingleIq::SessionTerminate) {

        QXmppCall *call = findCall(iq.sid());
        if (!call)
        {
            emit logMessage(QXmppLogger::WarningMessage,
                QString("Remote party %1 terminated unknown call %2").arg(iq.from(), iq.sid()));
            return;
        }

        emit logMessage(QXmppLogger::InformationMessage,
            QString("Remote party %1 terminated call %2").arg(iq.from(), iq.sid()));

        // send ack
        sendAck(iq);

        // terminate
        call->terminate();

    } else if (iq.action() == QXmppJingleIq::TransportInfo) {
        QXmppCall *call = findCall(iq.sid());
        if (!call)
        {
            emit logMessage(QXmppLogger::WarningMessage,
                QString("Remote party %1 sent transports for unknown call %2").arg(iq.from(), iq.sid()));
            return;
        }

        // send ack
        sendAck(iq);

        // perform ICE negotiation
        call->m_connection->setRemoteUser(iq.content().transportUser());
        call->m_connection->setRemotePassword(iq.content().transportPassword());
        call->addRemoteCandidates(iq.content().transportCandidates());
        call->m_connection->connectToHost();
    }
}

/// Sends a transport-info to inform the remote party of new local candidates.
///

void QXmppCallManager::localCandidatesChanged()
{
    QXmppCall *call = qobject_cast<QXmppCall*>(sender());
    if (!call || !m_calls.contains(call))
        return;

    QXmppJingleIq iq;
    iq.setTo(call->jid());
    iq.setType(QXmppIq::Set);
    iq.setAction(QXmppJingleIq::TransportInfo);
    iq.setInitiator(client()->configuration().jid());
    iq.setSid(call->sid());

    iq.content().setCreator(call->m_contentCreator);
    iq.content().setName(call->m_contentName);
    foreach (const QXmppJingleCandidate &candidate, call->m_connection->localCandidates())
        iq.content().addTransportCandidate(candidate);

    sendRequest(call, iq);
}

/// Sends an acknowledgement for a Jingle IQ.
///

bool QXmppCallManager::sendAck(const QXmppJingleIq &iq)
{
    QXmppIq ack;
    ack.setId(iq.id());
    ack.setTo(iq.from());
    ack.setType(QXmppIq::Result);
    return client()->sendPacket(ack);
}

/// Sends a Jingle IQ and adds it to outstanding requests.
///

bool QXmppCallManager::sendRequest(QXmppCall *call, const QXmppJingleIq &iq)
{
    call->m_requests << iq;
    return client()->sendPacket(iq);
}

