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

#include <cmath>

#include <QDataStream>
#include <QMetaType>
#include <QTimer>

#include "QXmppCodec.h"
#include "QXmppJingleIq.h"
#include "QXmppRtpChannel.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950288
#endif

//#define QXMPP_DEBUG_RTP
#define SAMPLE_BYTES 2

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

struct ToneInfo
{
    QXmppRtpChannel::Tone tone;
    quint32 incomingStart;
    quint32 outgoingStart;
    bool finished;
};

static QPair<int, int> toneFreqs(QXmppRtpChannel::Tone tone)
{
    switch (tone) {
    case QXmppRtpChannel::Tone_1: return qMakePair(697, 1209);
    case QXmppRtpChannel::Tone_2: return qMakePair(697, 1336);
    case QXmppRtpChannel::Tone_3: return qMakePair(697, 1477);
    case QXmppRtpChannel::Tone_A: return qMakePair(697, 1633);
    case QXmppRtpChannel::Tone_4: return qMakePair(770, 1209);
    case QXmppRtpChannel::Tone_5: return qMakePair(770, 1336);
    case QXmppRtpChannel::Tone_6: return qMakePair(770, 1477);
    case QXmppRtpChannel::Tone_B: return qMakePair(770, 1633);
    case QXmppRtpChannel::Tone_7: return qMakePair(852, 1209);
    case QXmppRtpChannel::Tone_8: return qMakePair(852, 1336);
    case QXmppRtpChannel::Tone_9: return qMakePair(852, 1477);
    case QXmppRtpChannel::Tone_C: return qMakePair(852, 1633);
    case QXmppRtpChannel::Tone_Star: return qMakePair(941, 1209);
    case QXmppRtpChannel::Tone_0: return qMakePair(941, 1336);
    case QXmppRtpChannel::Tone_Pound: return qMakePair(941, 1477);
    case QXmppRtpChannel::Tone_D: return qMakePair(941, 1633);
    }
    return qMakePair(0, 0);
}

QByteArray renderTone(QXmppRtpChannel::Tone tone, int clockrate, quint32 clockTick, qint64 samples)
{
    QPair<int,int> tf = toneFreqs(tone);
    const float clockMult = 2.0 * M_PI / float(clockrate);
    QByteArray chunk;
    chunk.reserve(samples * SAMPLE_BYTES);
    QDataStream output(&chunk, QIODevice::WriteOnly);
    output.setByteOrder(QDataStream::LittleEndian);
    for (quint32 i = 0; i < samples; ++i) {
        quint16 val = 16383.0 * (sin(clockMult * clockTick * tf.first) + sin(clockMult * clockTick * tf.second));
        output << val;
        clockTick++;
    }
    return chunk;
}

class QXmppRtpChannelPrivate
{
public:
    QXmppRtpChannelPrivate();
    QXmppCodec *codecForPayloadType(const QXmppJinglePayloadType &payloadType);
    QList<QXmppJinglePayloadType> supportedPayloadTypes() const;

    // signals
    bool signalsEmitted;
    qint64 writtenSinceLastEmit;

    // RTP
    QHostAddress remoteHost;
    quint16 remotePort;

    QByteArray incomingBuffer;
    bool incomingBuffering;
    QMap<int, QXmppCodec*> incomingCodecs;
    int incomingMinimum;
    int incomingMaximum;
    // position of the head of the incoming buffer, in bytes
    qint64 incomingPos;
    quint16 incomingSequence;
    QXmppJinglePayloadType incomingTonesType;

    QByteArray outgoingBuffer;
    quint16 outgoingChunk;
    QXmppCodec *outgoingCodec;
    bool outgoingMarker;
    QList<QXmppJinglePayloadType> outgoingPayloadTypes;
    quint16 outgoingSequence;
    quint32 outgoingStamp;
    QTimer *outgoingTimer;
    QList<ToneInfo> outgoingTones;
    QXmppJinglePayloadType outgoingTonesType;

    quint32 ssrc;
    QXmppJinglePayloadType payloadType;
};

QXmppRtpChannelPrivate::QXmppRtpChannelPrivate()
    : signalsEmitted(false),
    writtenSinceLastEmit(0),
    incomingBuffering(true),
    incomingMinimum(0),
    incomingMaximum(0),
    incomingPos(0),
    incomingSequence(0),
    outgoingCodec(0),
    outgoingMarker(true),
    outgoingSequence(1),
    outgoingStamp(0),
    ssrc(0)
{
    qRegisterMetaType<QXmppRtpChannel::Tone>("QXmppRtpChannel::Tone");

    outgoingPayloadTypes = supportedPayloadTypes();
    ssrc = qrand();
}

/// Returns the audio codec for the given payload type.
///

QXmppCodec *QXmppRtpChannelPrivate::codecForPayloadType(const QXmppJinglePayloadType &payloadType)
{
    if (payloadType.id() == G711u)
        return new QXmppG711uCodec(payloadType.clockrate());
    else if (payloadType.id() == G711a)
        return new QXmppG711aCodec(payloadType.clockrate());
#ifdef QXMPP_USE_SPEEX
    else if (payloadType.name().toLower() == "speex")
        return new QXmppSpeexCodec(payloadType.clockrate());
#endif
    return 0;
}

/// Returns the list of supported payload types.
///

QList<QXmppJinglePayloadType> QXmppRtpChannelPrivate::supportedPayloadTypes() const
{
    QList<QXmppJinglePayloadType> payloads;
    QXmppJinglePayloadType payload;

#ifdef QXMPP_USE_SPEEX
    payload.setId(96);
    payload.setChannels(1);
    payload.setName("speex");
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

    payload.setId(101);
    payload.setChannels(1);
    payload.setName("telephone-event");
    payload.setClockrate(8000);
    payloads << payload;

    return payloads;
}

/// Creates a new RTP channel.
///
/// \param parent

QXmppRtpChannel::QXmppRtpChannel(QObject *parent)
    : QIODevice(parent),
    d(new QXmppRtpChannelPrivate)
{
    QXmppLoggable *logParent = qobject_cast<QXmppLoggable*>(parent);
    if (logParent) {
        connect(this, SIGNAL(logMessage(QXmppLogger::MessageType,QString)),
                logParent, SIGNAL(logMessage(QXmppLogger::MessageType,QString)));
    }
    d->outgoingTimer = new QTimer(this);
    connect(d->outgoingTimer, SIGNAL(timeout()), this, SLOT(writeDatagram()));
}

/// Destroys an RTP channel.
///

QXmppRtpChannel::~QXmppRtpChannel()
{
    foreach (QXmppCodec *codec, d->incomingCodecs)
        delete codec;
    delete d;
}

/// Returns the number of bytes that are available for reading.
///

qint64 QXmppRtpChannel::bytesAvailable() const
{
    return d->incomingBuffer.size();
}

/// Processes an incoming RTP packet.
///
/// \param ba

void QXmppRtpChannel::datagramReceived(const QByteArray &ba)
{
    if (ba.size() < 12 || (quint8(ba.at(0)) >> 6) != RTP_VERSION)
    {
        warning("QXmppRtpChannel::datagramReceived got an invalid RTP packet");
        return;
    }

    // parse RTP header
    QDataStream stream(ba);
    quint8 version, marker_type;
    quint32 ssrc;
    quint16 sequence;
    quint32 stamp;
    stream >> version;
    stream >> marker_type;
    stream >> sequence;
    stream >> stamp;
    stream >> ssrc;
    const quint8 type = marker_type & 0x7f;
    const qint64 packetLength = ba.size() - 12;

#ifdef QXMPP_DEBUG_RTP
    const bool marker = marker_type & 0x80;
    logReceived(QString("RTP packet seq %1 stamp %2 marker %3 type %4 size %5").arg(
            QString::number(sequence),
            QString::number(stamp),
            QString::number(marker),
            QString::number(type),
            QString::number(packetLength)));
#endif

    // check type
    QXmppCodec *codec = d->incomingCodecs.value(type);
    if (!codec) {
        warning(QString("RTP packet seq %1 has unknown type %2")
                .arg(QString::number(sequence))
                .arg(QString::number(type)));
        return;
    }

    // check sequence number
#if 0
    if (d->incomingSequence && sequence != d->incomingSequence + 1)
        warning(QString("RTP packet seq %1 is out of order, previous was %2")
                .arg(QString::number(sequence))
                .arg(QString::number(d->incomingSequence)));
#endif
    d->incomingSequence = sequence;

    // determine packet's position in the buffer (in bytes)
    qint64 packetOffset = 0;
    if (!d->incomingBuffer.isEmpty())
    {
        packetOffset = stamp * SAMPLE_BYTES - d->incomingPos;
        if (packetOffset < 0)
        {
            warning(QString("RTP packet stamp %1 is too old, buffer start is %2")
                    .arg(QString::number(stamp))
                    .arg(QString::number(d->incomingPos)));
            return;
        }
    } else {
        d->incomingPos = stamp * SAMPLE_BYTES + (d->incomingPos % SAMPLE_BYTES);
    }

    // allocate space for new packet
    if (packetOffset + packetLength > d->incomingBuffer.size())
        d->incomingBuffer += QByteArray(packetOffset + packetLength - d->incomingBuffer.size(), 0);
    QDataStream output(&d->incomingBuffer, QIODevice::WriteOnly);
    output.device()->seek(packetOffset);
    output.setByteOrder(QDataStream::LittleEndian);
    codec->decode(stream, output);

    // check whether we are running late
    if (d->incomingBuffer.size() > d->incomingMaximum)
    {
        qint64 droppedSize = d->incomingBuffer.size() - d->incomingMinimum;
        const int remainder = droppedSize % SAMPLE_BYTES;
        if (remainder)
            droppedSize -= remainder;
        warning(QString("Incoming RTP buffer is too full, dropping %1 bytes")
                .arg(QString::number(droppedSize)));
        d->incomingBuffer.remove(0, droppedSize);
        d->incomingPos += droppedSize;
    }
    // check whether we have filled the initial buffer
    if (d->incomingBuffer.size() >= d->incomingMinimum)
        d->incomingBuffering = false;
    if (!d->incomingBuffering)
        emit readyRead();
}

void QXmppRtpChannel::emitSignals()
{
    emit bytesWritten(d->writtenSinceLastEmit);
    d->writtenSinceLastEmit = 0;
    d->signalsEmitted = false;
}

/// Returns true, as the RTP channel is a sequential device.
///

bool QXmppRtpChannel::isSequential() const
{
    return true;
}

qint64 QXmppRtpChannel::readData(char * data, qint64 maxSize)
{
    // if we are filling the buffer, return empty samples
    if (d->incomingBuffering)
    {
        // FIXME: if we are asked for a non-integer number of samples,
        // we will return junk on next read as we don't increment d->incomingPos
        memset(data, 0, maxSize);
        return maxSize;
    }

    qint64 readSize = qMin(maxSize, qint64(d->incomingBuffer.size()));
    memcpy(data, d->incomingBuffer.constData(), readSize);
    d->incomingBuffer.remove(0, readSize);
    if (readSize < maxSize)
    {
#ifdef QXMPP_DEBUG_RTP
        debug(QString("QXmppRtpChannel::readData missing %1 bytes").arg(QString::number(maxSize - readSize)));
#endif
        memset(data + readSize, 0, maxSize - readSize);
    }

    // add local DTMF echo
    if (!d->outgoingTones.isEmpty()) {
        const int headOffset = d->incomingPos % SAMPLE_BYTES;
        const int samples = (headOffset + maxSize + SAMPLE_BYTES - 1) / SAMPLE_BYTES;
        const QByteArray chunk = renderTone(
            d->outgoingTones[0].tone,
            d->payloadType.clockrate(),
            d->incomingPos / SAMPLE_BYTES - d->outgoingTones[0].incomingStart,
            samples);
        memcpy(data, chunk.constData() + headOffset, maxSize);
    }

    d->incomingPos += maxSize;
    return maxSize;
}

/// Returns the RTP channel's payload type.
///
/// You can use this to determine the QAudioFormat to use with your
/// QAudioInput/QAudioOutput.

QXmppJinglePayloadType QXmppRtpChannel::payloadType() const
{
    return d->payloadType;
}

/// Returns the local payload types.
///

QList<QXmppJinglePayloadType> QXmppRtpChannel::localPayloadTypes() const
{
    return d->outgoingPayloadTypes;
}

/// Returns the position in the received audio data.

qint64 QXmppRtpChannel::pos() const
{
    return d->incomingPos;
}

/// Seeks in the received audio data.
///
/// Seeking backwards will result in empty samples being added at the start
/// of the buffer.
///
/// \param pos

bool QXmppRtpChannel::seek(qint64 pos)
{
    qint64 delta = pos - d->incomingPos;
    if (delta < 0)
        d->incomingBuffer.prepend(QByteArray(-delta, 0));
    else
        d->incomingBuffer.remove(0, delta);
    d->incomingPos = pos;
    return true;
}

/// Sets the remote payload types.
///
/// \param remotePayloadTypes

void QXmppRtpChannel::setRemotePayloadTypes(const QList<QXmppJinglePayloadType> &remotePayloadTypes)
{
    QList<QXmppJinglePayloadType> commonPayloadTypes;

    foreach (const QXmppJinglePayloadType &payloadType, remotePayloadTypes) {

        // check we support this payload type
        int index = d->outgoingPayloadTypes.indexOf(payloadType);
        if (index < 0)
            continue;
        commonPayloadTypes << d->outgoingPayloadTypes[index];

        // check for telephony events
        if (payloadType.name() == "telephone-event") {
            d->incomingTonesType = payloadType;
            d->outgoingTonesType = d->outgoingPayloadTypes[index];
            continue;
        }

        // create codec for this payload type
        QXmppCodec *codec = d->codecForPayloadType(payloadType);
        if (!codec)
            continue;

        if (commonPayloadTypes.size() == 1) {

            // store outgoing codec
            d->payloadType = d->outgoingPayloadTypes[index];
            d->outgoingCodec = codec;

        } else if (payloadType.ptime() != d->payloadType.ptime() ||
                   payloadType.clockrate() != d->payloadType.clockrate()) {

            warning(QString("QXmppRtpChannel skipping payload due to ptime or clockrate mismatch : %1 (%2)")
                .arg(QString::number(payloadType.id()))
                .arg(payloadType.name()));
            delete codec;
            continue;
        }

        // store incoming codec
        d->incomingCodecs[payloadType.id()] = codec;
    }
    d->outgoingPayloadTypes = commonPayloadTypes;
    if (d->outgoingPayloadTypes.isEmpty()) {
        warning("QXmppRtpChannel could not negociate a common codec");
        return;
    }

    // size in bytes of an decoded packet
    d->outgoingChunk = SAMPLE_BYTES * d->payloadType.ptime() * d->payloadType.clockrate() / 1000;
    d->outgoingTimer->setInterval(d->payloadType.ptime());

    d->incomingMinimum = d->outgoingChunk * 5;
    d->incomingMaximum = d->outgoingChunk * 15;

    open(QIODevice::ReadWrite | QIODevice::Unbuffered);
}

/// Starts sending the specified DTMF tone.
///
/// \param tone

void QXmppRtpChannel::startTone(QXmppRtpChannel::Tone tone)
{
    ToneInfo info;
    info.tone = tone;
    info.incomingStart = d->incomingPos / SAMPLE_BYTES;
    info.outgoingStart = d->outgoingStamp;
    info.finished = false;
    d->outgoingTones << info;
}

/// Stops sending the specified DTMF tone.
///
/// \param tone

void QXmppRtpChannel::stopTone(QXmppRtpChannel::Tone tone)
{
    for (int i = 0; i < d->outgoingTones.size(); ++i) {
        if (d->outgoingTones[i].tone == tone) {
            d->outgoingTones[i].finished = true;
            break;
        }
    }
}

qint64 QXmppRtpChannel::writeData(const char * data, qint64 maxSize)
{
    if (!d->outgoingCodec)
    {
        warning("QXmppRtpChannel::writeData before codec was set");
        return -1;
    }

    d->outgoingBuffer += QByteArray::fromRawData(data, maxSize);

    // start sending audio chunks
    if (!d->outgoingTimer->isActive())
        d->outgoingTimer->start();

    return maxSize;
}

void QXmppRtpChannel::writeDatagram()
{
    // read audio chunk
    QByteArray chunk;
    if (d->outgoingBuffer.size() < d->outgoingChunk) {
        warning("Outgoing RTP buffer is starved");
        chunk = QByteArray(d->outgoingChunk, 0);
    } else {
        chunk = d->outgoingBuffer.left(d->outgoingChunk);
        d->outgoingBuffer.remove(0, d->outgoingChunk);
    }

    bool sendAudio = true;
    if (!d->outgoingTones.isEmpty()) {
        const quint32 packetTicks = (d->payloadType.clockrate() * d->payloadType.ptime()) / 1000;
        const ToneInfo info = d->outgoingTones[0];

        if (d->outgoingTonesType.id()) {
            // send RFC 2833 DTMF
            QByteArray header;
            QDataStream stream(&header, QIODevice::WriteOnly);
            quint8 marker_type = d->outgoingTonesType.id();
            if (info.outgoingStart == d->outgoingStamp)
                marker_type |= 0x80;

            stream << quint8(RTP_VERSION << 6);
            stream << marker_type;
            stream << d->outgoingSequence;
            stream << info.outgoingStart;
            stream << d->ssrc;

            stream << quint8(info.tone);
            stream << quint8(info.finished ? 0x80 : 0x00);
            stream << quint16(d->outgoingStamp + packetTicks - info.outgoingStart);
#ifdef QXMPP_DEBUG_RTP
            logSent(QString("RTP packet seq %1 stamp %2 marker %3 type %4 size %5").arg(
                        QString::number(d->outgoingSequence),
                        QString::number(d->outgoingStamp),
                        QString::number(marker_type & 0x80 != 0),
                        QString::number(marker_type & 0x7f),
                        QString::number(header.size() - 12)));
#endif
            emit sendDatagram(header);
            d->outgoingSequence++;
            d->outgoingStamp += packetTicks;

            sendAudio = false;
        } else {
            // generate in-band DTMF
            chunk = renderTone(info.tone, d->payloadType.clockrate(), d->outgoingStamp - info.outgoingStart, packetTicks);
        }

        // if the tone is finished, remove it
        if (info.finished)
            d->outgoingTones.removeFirst();
    }

    if (sendAudio) {
        // send audio data
        QByteArray header;
        QDataStream stream(&header, QIODevice::WriteOnly);
        quint8 marker_type = d->payloadType.id();
        if (d->outgoingMarker)
        {
            marker_type |= 0x80;
            d->outgoingMarker= false;
        }
        stream << quint8(RTP_VERSION << 6);
        stream << marker_type;
        stream << d->outgoingSequence;
        stream << d->outgoingStamp;
        stream << d->ssrc;

        // encode audio chunk
        QDataStream input(chunk);
        input.setByteOrder(QDataStream::LittleEndian);
        const qint64 packetTicks = d->outgoingCodec->encode(input, stream);

#ifdef QXMPP_DEBUG_RTP
        logSent(QString("RTP packet seq %1 stamp %2 marker %3 type %4 size %5").arg(
                    QString::number(d->outgoingSequence),
                    QString::number(d->outgoingStamp),
                    QString::number(marker_type & 0x80 != 0),
                    QString::number(marker_type & 0x7f),
                    QString::number(header.size() - 12)));
#endif
        emit sendDatagram(header);
        d->outgoingSequence++;
        d->outgoingStamp += packetTicks;
    }

    // queue signals
    d->writtenSinceLastEmit += chunk.size();
    if (!d->signalsEmitted && !signalsBlocked()) {
        d->signalsEmitted = true;
        QMetaObject::invokeMethod(this, "emitSignals", Qt::QueuedConnection);
    }
}
