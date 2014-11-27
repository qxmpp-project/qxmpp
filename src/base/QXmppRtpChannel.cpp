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

#include <cmath>

#include <QDataStream>
#include <QMetaType>
#include <QTimer>

#include "QXmppCodec_p.h"
#include "QXmppJingleIq.h"
#include "QXmppRtpChannel.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950288
#endif

//#define QXMPP_DEBUG_RTP
//#define QXMPP_DEBUG_RTP_BUFFER
#define SAMPLE_BYTES 2

const quint8 RTP_VERSION = 0x02;

/// Parses an RTP packet.
///
/// \param ba

bool QXmppRtpPacket::decode(const QByteArray &ba)
{
    if (ba.isEmpty())
        return false;

    // fixed header
    quint8 tmp;
    QDataStream stream(ba);
    stream >> tmp;
    version = (tmp >> 6);
    const quint8 cc = (tmp >> 1) & 0xf;
    const int hlen = 12 + 4 * cc;
    if (version != RTP_VERSION || ba.size() < hlen)
        return false;
    stream >> tmp;
    marker = (tmp >> 7);
    type = tmp & 0x7f;
    stream >> sequence;
    stream >> stamp;
    stream >> ssrc;

    // contributing source IDs
    csrc.clear();
    quint32 src;
    for (int i = 0; i < cc; ++i) {
        stream >> src;
        csrc << src;
    }

    // retrieve payload
    payload = ba.right(ba.size() - hlen);
    return true;
}

/// Encodes an RTP packet.

QByteArray QXmppRtpPacket::encode() const
{
    Q_ASSERT(csrc.size() < 16);

    // fixed header
    QByteArray ba;
    ba.resize(payload.size() + 12 + 4 * csrc.size());
    QDataStream stream(&ba, QIODevice::WriteOnly);
    stream << quint8(((version & 0x3) << 6) |
                     ((csrc.size() & 0xf) << 1));
    stream << quint8((type & 0x7f) | (marker << 7));
    stream << sequence;
    stream << stamp;
    stream << ssrc;

    // contributing source ids
    foreach (const quint32 &src, csrc)
        stream << src;

    stream.writeRawData(payload.constData(), payload.size());
    return ba;
}

/// Returns a string representation of the RTP header.

QString QXmppRtpPacket::toString() const
{
    return QString("RTP packet seq %1 stamp %2 marker %3 type %4 size %5").arg(
        QString::number(sequence),
        QString::number(stamp),
        QString::number(marker),
        QString::number(type),
        QString::number(payload.size()));
}

/// Creates a new RTP channel.

QXmppRtpChannel::QXmppRtpChannel()
    : m_outgoingPayloadNumbered(false)
{
}

/// Returns the local payload types.
///

QList<QXmppJinglePayloadType> QXmppRtpChannel::localPayloadTypes()
{
    m_outgoingPayloadNumbered = true;
    return m_outgoingPayloadTypes;
}

/// Sets the remote payload types.
///
/// \param remotePayloadTypes

void QXmppRtpChannel::setRemotePayloadTypes(const QList<QXmppJinglePayloadType> &remotePayloadTypes)
{
    QList<QXmppJinglePayloadType> commonOutgoingTypes;
    QList<QXmppJinglePayloadType> commonIncomingTypes;

    foreach (const QXmppJinglePayloadType &incomingType, remotePayloadTypes) {
        // check we support this payload type
        int outgoingIndex = m_outgoingPayloadTypes.indexOf(incomingType);
        if (outgoingIndex < 0)
            continue;
        QXmppJinglePayloadType outgoingType = m_outgoingPayloadTypes[outgoingIndex];

        // be kind and try to adopt the other agent's numbering
        if (!m_outgoingPayloadNumbered && outgoingType.id() > 95) {
            outgoingType.setId(incomingType.id());
        }
        commonIncomingTypes << incomingType;
        commonOutgoingTypes << outgoingType;
    }
    if (commonOutgoingTypes.isEmpty()) {
        qWarning("QXmppRtpChannel could not negociate a common codec");
        return;
    }
    m_incomingPayloadTypes = commonIncomingTypes;
    m_outgoingPayloadTypes = commonOutgoingTypes;
    m_outgoingPayloadNumbered = true;

    // call hook
    payloadTypesChanged();
}

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
    QXmppRtpAudioChannel::Tone tone;
    quint32 incomingStart;
    quint32 outgoingStart;
    bool finished;
};

static QPair<int, int> toneFreqs(QXmppRtpAudioChannel::Tone tone)
{
    switch (tone) {
    case QXmppRtpAudioChannel::Tone_1: return qMakePair(697, 1209);
    case QXmppRtpAudioChannel::Tone_2: return qMakePair(697, 1336);
    case QXmppRtpAudioChannel::Tone_3: return qMakePair(697, 1477);
    case QXmppRtpAudioChannel::Tone_A: return qMakePair(697, 1633);
    case QXmppRtpAudioChannel::Tone_4: return qMakePair(770, 1209);
    case QXmppRtpAudioChannel::Tone_5: return qMakePair(770, 1336);
    case QXmppRtpAudioChannel::Tone_6: return qMakePair(770, 1477);
    case QXmppRtpAudioChannel::Tone_B: return qMakePair(770, 1633);
    case QXmppRtpAudioChannel::Tone_7: return qMakePair(852, 1209);
    case QXmppRtpAudioChannel::Tone_8: return qMakePair(852, 1336);
    case QXmppRtpAudioChannel::Tone_9: return qMakePair(852, 1477);
    case QXmppRtpAudioChannel::Tone_C: return qMakePair(852, 1633);
    case QXmppRtpAudioChannel::Tone_Star: return qMakePair(941, 1209);
    case QXmppRtpAudioChannel::Tone_0: return qMakePair(941, 1336);
    case QXmppRtpAudioChannel::Tone_Pound: return qMakePair(941, 1477);
    case QXmppRtpAudioChannel::Tone_D: return qMakePair(941, 1633);
    }
    return qMakePair(0, 0);
}

QByteArray renderTone(QXmppRtpAudioChannel::Tone tone, int clockrate, quint32 clockTick, qint64 samples)
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

class QXmppRtpAudioChannelPrivate
{
public:
    QXmppRtpAudioChannelPrivate(QXmppRtpAudioChannel *qq);
    QXmppCodec *codecForPayloadType(const QXmppJinglePayloadType &payloadType);

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

    QByteArray outgoingBuffer;
    quint16 outgoingChunk;
    QXmppCodec *outgoingCodec;
    bool outgoingMarker;
    bool outgoingPayloadNumbered;
    quint16 outgoingSequence;
    quint32 outgoingStamp;
    QTimer *outgoingTimer;
    QList<ToneInfo> outgoingTones;
    QXmppJinglePayloadType outgoingTonesType;

    quint32 outgoingSsrc;
    QXmppJinglePayloadType payloadType;

private:
    QXmppRtpAudioChannel *q;
};

QXmppRtpAudioChannelPrivate::QXmppRtpAudioChannelPrivate(QXmppRtpAudioChannel *qq)
    : signalsEmitted(false),
    writtenSinceLastEmit(0),
    incomingBuffering(true),
    incomingMinimum(0),
    incomingMaximum(0),
    incomingPos(0),
    incomingSequence(0),
    outgoingCodec(0),
    outgoingMarker(true),
    outgoingPayloadNumbered(false),
    outgoingSequence(1),
    outgoingStamp(0),
    outgoingTimer(0),
    outgoingSsrc(0),
    q(qq)
{
    qRegisterMetaType<QXmppRtpAudioChannel::Tone>("QXmppRtpAudioChannel::Tone");
    outgoingSsrc = qrand();
}

/// Returns the audio codec for the given payload type.
///

QXmppCodec *QXmppRtpAudioChannelPrivate::codecForPayloadType(const QXmppJinglePayloadType &payloadType)
{
    if (payloadType.id() == G711u)
        return new QXmppG711uCodec(payloadType.clockrate());
    else if (payloadType.id() == G711a)
        return new QXmppG711aCodec(payloadType.clockrate());
#ifdef QXMPP_USE_SPEEX
    else if (payloadType.name().toLower() == "speex")
        return new QXmppSpeexCodec(payloadType.clockrate());
#endif
#ifdef QXMPP_USE_OPUS
    else if (payloadType.name().toLower() == "opus")
        return new QXmppOpusCodec(payloadType.clockrate(), payloadType.channels());
#endif
    return 0;
}

/// Constructs a new RTP audio channel with the given \a parent.

QXmppRtpAudioChannel::QXmppRtpAudioChannel(QObject *parent)
    : QIODevice(parent)
{
    d = new QXmppRtpAudioChannelPrivate(this);
    QXmppLoggable *logParent = qobject_cast<QXmppLoggable*>(parent);
    if (logParent) {
        connect(this, SIGNAL(logMessage(QXmppLogger::MessageType,QString)),
                logParent, SIGNAL(logMessage(QXmppLogger::MessageType,QString)));
    }
    d->outgoingTimer = new QTimer(this);
    connect(d->outgoingTimer, SIGNAL(timeout()), this, SLOT(writeDatagram()));

    // set supported codecs
    QXmppJinglePayloadType payload;

#ifdef QXMPP_USE_OPUS
    payload.setId(100); // NOTE: I don't know if this Id is ok for Opus.
    payload.setChannels(1);
    payload.setName("opus");
    payload.setClockrate(8000);
    m_outgoingPayloadTypes << payload;
#endif

#ifdef QXMPP_USE_SPEEX
    payload.setId(96);
    payload.setChannels(1);
    payload.setName("speex");
    payload.setClockrate(8000);
    m_outgoingPayloadTypes << payload;
#endif

    payload.setId(G711u);
    payload.setChannels(1);
    payload.setName("PCMU");
    payload.setClockrate(8000);
    m_outgoingPayloadTypes << payload;

    payload.setId(G711a);
    payload.setChannels(1);
    payload.setName("PCMA");
    payload.setClockrate(8000);
    m_outgoingPayloadTypes << payload;

    QMap<QString, QString> parameters;
    parameters.insert("events", "0-15");
    payload.setId(101);
    payload.setChannels(1);
    payload.setName("telephone-event");
    payload.setClockrate(8000);
    payload.setParameters(parameters);
    m_outgoingPayloadTypes << payload;
}

/// Destroys an RTP audio channel.
///

QXmppRtpAudioChannel::~QXmppRtpAudioChannel()
{
    foreach (QXmppCodec *codec, d->incomingCodecs)
        delete codec;
    if (d->outgoingCodec)
        delete d->outgoingCodec;
    delete d;
}

/// Returns the number of bytes that are available for reading.

qint64 QXmppRtpAudioChannel::bytesAvailable() const
{
    return QIODevice::bytesAvailable() + d->incomingBuffer.size();
}

/// Closes the RTP audio channel.

void QXmppRtpAudioChannel::close()
{
    d->outgoingTimer->stop();
    QIODevice::close();
}

/// Processes an incoming RTP packet.
///
/// \param ba

void QXmppRtpAudioChannel::datagramReceived(const QByteArray &ba)
{
    QXmppRtpPacket packet;
    if (!packet.decode(ba))
        return;

#ifdef QXMPP_DEBUG_RTP
    logReceived(packet.toString());
#endif

    // check sequence number
#if 0
    if (d->incomingSequence && packet.sequence != d->incomingSequence + 1)
        warning(QString("RTP packet seq %1 is out of order, previous was %2")
                .arg(QString::number(packet.sequence))
                .arg(QString::number(d->incomingSequence)));
#endif
    d->incomingSequence = packet.sequence;

    // get or create codec
    QXmppCodec *codec = 0;
    if (!d->incomingCodecs.contains(packet.type)) {
        foreach (const QXmppJinglePayloadType &payload, m_incomingPayloadTypes) {
            if (packet.type == payload.id()) {
                codec = d->codecForPayloadType(payload);
                break;
            }
        }
        if (codec)
            d->incomingCodecs.insert(packet.type, codec);
        else
            warning(QString("Could not find codec for RTP type %1").arg(QString::number(packet.type)));
    } else {
        codec = d->incomingCodecs.value(packet.type);
    }
    if (!codec)
        return;

    // determine packet's position in the buffer (in bytes)
    qint64 packetOffset = 0;
    if (!d->incomingBuffer.isEmpty()) {
        packetOffset = packet.stamp * SAMPLE_BYTES - d->incomingPos;
        if (packetOffset < 0) {
#ifdef QXMPP_DEBUG_RTP_BUFFER
            warning(QString("RTP packet stamp %1 is too old, buffer start is %2")
                    .arg(QString::number(packet.stamp))
                    .arg(QString::number(d->incomingPos)));
#endif
            return;
        }
    } else {
        d->incomingPos = packet.stamp * SAMPLE_BYTES + (d->incomingPos % SAMPLE_BYTES);
    }

    // allocate space for new packet
    // FIXME: this is wrong, we want the decoded data size!
    qint64 packetLength = packet.payload.size();
    if (packetOffset + packetLength > d->incomingBuffer.size())
        d->incomingBuffer += QByteArray(packetOffset + packetLength - d->incomingBuffer.size(), 0);
    QDataStream input(packet.payload);
    QDataStream output(&d->incomingBuffer, QIODevice::WriteOnly);
    output.device()->seek(packetOffset);
    output.setByteOrder(QDataStream::LittleEndian);
    codec->decode(input, output);

    // check whether we are running late
    if (d->incomingBuffer.size() > d->incomingMaximum)
    {
        qint64 droppedSize = d->incomingBuffer.size() - d->incomingMinimum;
        const int remainder = droppedSize % SAMPLE_BYTES;
        if (remainder)
            droppedSize -= remainder;
#ifdef QXMPP_DEBUG_RTP_BUFFER
        warning(QString("Incoming RTP buffer is too full, dropping %1 bytes")
                .arg(QString::number(droppedSize)));
#endif
        d->incomingBuffer.remove(0, droppedSize);
        d->incomingPos += droppedSize;
    }
    // check whether we have filled the initial buffer
    if (d->incomingBuffer.size() >= d->incomingMinimum)
        d->incomingBuffering = false;
    if (!d->incomingBuffering)
        emit readyRead();
}

void QXmppRtpAudioChannel::emitSignals()
{
    emit bytesWritten(d->writtenSinceLastEmit);
    d->writtenSinceLastEmit = 0;
    d->signalsEmitted = false;
}

/// Returns true, as the RTP channel is a sequential device.
///

bool QXmppRtpAudioChannel::isSequential() const
{
    return true;
}

/// Returns the mode in which the channel has been opened.

QIODevice::OpenMode QXmppRtpAudioChannel::openMode() const
{
    return QIODevice::openMode();
}

/// Returns the RTP channel's payload type.
///
/// You can use this to determine the QAudioFormat to use with your
/// QAudioInput/QAudioOutput.

QXmppJinglePayloadType QXmppRtpAudioChannel::payloadType() const
{
    return d->payloadType;
}

/// \cond
qint64 QXmppRtpAudioChannel::readData(char * data, qint64 maxSize)
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
        debug(QString("QXmppRtpAudioChannel::readData missing %1 bytes").arg(QString::number(maxSize - readSize)));
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

void QXmppRtpAudioChannel::payloadTypesChanged()
{
    // delete incoming codecs
    foreach (QXmppCodec *codec, d->incomingCodecs)
        delete codec;
    d->incomingCodecs.clear();

    // delete outgoing codec
    if (d->outgoingCodec) {
        delete d->outgoingCodec;
        d->outgoingCodec = 0;
    }

    // create outgoing codec
    foreach (const QXmppJinglePayloadType &outgoingType, m_outgoingPayloadTypes) {
        // check for telephony events
        if (outgoingType.name() == "telephone-event") {
            d->outgoingTonesType = outgoingType;
        }
        else if (!d->outgoingCodec) {
            QXmppCodec *codec = d->codecForPayloadType(outgoingType);
            if (codec) {
                d->payloadType = outgoingType;
                d->outgoingCodec = codec;
            }
        }
    }

    // size in bytes of an decoded packet
    d->outgoingChunk = SAMPLE_BYTES * d->payloadType.ptime() * d->payloadType.clockrate() / 1000;
    d->outgoingTimer->setInterval(d->payloadType.ptime());

    d->incomingMinimum = d->outgoingChunk * 5;
    d->incomingMaximum = d->outgoingChunk * 15;

    open(QIODevice::ReadWrite | QIODevice::Unbuffered);
}
/// \endcond

/// Returns the position in the received audio data.

qint64 QXmppRtpAudioChannel::pos() const
{
    return d->incomingPos;
}

/// Seeks in the received audio data.
///
/// Seeking backwards will result in empty samples being added at the start
/// of the buffer.
///
/// \param pos

bool QXmppRtpAudioChannel::seek(qint64 pos)
{
    qint64 delta = pos - d->incomingPos;
    if (delta < 0)
        d->incomingBuffer.prepend(QByteArray(-delta, 0));
    else
        d->incomingBuffer.remove(0, delta);
    d->incomingPos = pos;
    return true;
}

/// Starts sending the specified DTMF tone.
///
/// \param tone

void QXmppRtpAudioChannel::startTone(QXmppRtpAudioChannel::Tone tone)
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

void QXmppRtpAudioChannel::stopTone(QXmppRtpAudioChannel::Tone tone)
{
    for (int i = 0; i < d->outgoingTones.size(); ++i) {
        if (d->outgoingTones[i].tone == tone) {
            d->outgoingTones[i].finished = true;
            break;
        }
    }
}

/// \cond
qint64 QXmppRtpAudioChannel::writeData(const char * data, qint64 maxSize)
{
    if (!d->outgoingCodec) {
        warning("QXmppRtpAudioChannel::writeData before codec was set");
        return -1;
    }

    d->outgoingBuffer += QByteArray::fromRawData(data, maxSize);

    // start sending audio chunks
    if (!d->outgoingTimer->isActive())
        d->outgoingTimer->start();

    return maxSize;
}
/// \endcond

void QXmppRtpAudioChannel::writeDatagram()
{
    // read audio chunk
    QByteArray chunk;
    if (d->outgoingBuffer.size() < d->outgoingChunk) {
#ifdef QXMPP_DEBUG_RTP_BUFFER
        warning("Outgoing RTP buffer is starved");
#endif
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
            QXmppRtpPacket packet;
            packet.version = RTP_VERSION;
            packet.marker = (info.outgoingStart == d->outgoingStamp);
            packet.type = d->outgoingTonesType.id();
            packet.sequence = d->outgoingSequence;
            packet.stamp = info.outgoingStart;
            packet.ssrc = d->outgoingSsrc;

            QDataStream output(&packet.payload, QIODevice::WriteOnly);
            output << quint8(info.tone);
            output << quint8(info.finished ? 0x80 : 0x00);
            output << quint16(d->outgoingStamp + packetTicks - info.outgoingStart);
#ifdef QXMPP_DEBUG_RTP
            logSent(packet.toString());
#endif
            emit sendDatagram(packet.encode());
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
        QXmppRtpPacket packet;
        packet.version = RTP_VERSION;
        if (d->outgoingMarker)
        {
            packet.marker = true;
            d->outgoingMarker = false;
        } else {
            packet.marker = false;
        }
        packet.type = d->payloadType.id();
        packet.sequence = d->outgoingSequence;
        packet.stamp = d->outgoingStamp;
        packet.ssrc = d->outgoingSsrc;

        // encode audio chunk
        QDataStream input(chunk);
        input.setByteOrder(QDataStream::LittleEndian);
        QDataStream output(&packet.payload, QIODevice::WriteOnly);
        const qint64 packetTicks = d->outgoingCodec->encode(input, output);

#ifdef QXMPP_DEBUG_RTP
        logSent(packet.toString());
#endif
        emit sendDatagram(packet.encode());
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

/** Constructs a null video frame.
 */
QXmppVideoFrame::QXmppVideoFrame()
    : m_bytesPerLine(0),
    m_height(0),
    m_mappedBytes(0),
    m_pixelFormat(Format_Invalid),
    m_width(0)
{
}

/** Constructs a video frame of the given pixel format and size in pixels.
 *
 * @param bytes
 * @param size
 * @param bytesPerLine
 * @param format
 */
QXmppVideoFrame::QXmppVideoFrame(int bytes, const QSize &size, int bytesPerLine, PixelFormat format)
    : m_bytesPerLine(bytesPerLine),
    m_height(size.height()),
    m_mappedBytes(bytes),
    m_pixelFormat(format),
    m_width(size.width())
{
    m_data.resize(bytes);
}

/// Returns a pointer to the start of the frame data buffer.

uchar *QXmppVideoFrame::bits()
{
    return (uchar*)m_data.data();
}

/// Returns a pointer to the start of the frame data buffer.

const uchar *QXmppVideoFrame::bits() const
{
    return (const uchar*)m_data.constData();
}

/// Returns the number of bytes in a scan line.

int QXmppVideoFrame::bytesPerLine() const
{
    return m_bytesPerLine;
}

/// Returns the height of a video frame.

int QXmppVideoFrame::height() const
{
    return m_height;
}

/// Returns true if the frame is valid.

bool QXmppVideoFrame::isValid() const
{
    return m_pixelFormat != Format_Invalid &&
           m_height > 0 && m_width > 0 &&
           m_mappedBytes > 0;
}

/// Returns the number of bytes occupied by the mapped frame data.

int QXmppVideoFrame::mappedBytes() const
{
    return m_mappedBytes;
}

/// Returns the color format of a video frame.

QXmppVideoFrame::PixelFormat QXmppVideoFrame::pixelFormat() const
{
    return m_pixelFormat;
}

/// Returns the size of a video frame.

QSize QXmppVideoFrame::size() const
{
    return QSize(m_width, m_height);
}

/// Returns the width of a video frame.

int QXmppVideoFrame::width() const
{
    return m_width;
}

class QXmppRtpVideoChannelPrivate
{
public:
    QXmppRtpVideoChannelPrivate();
    QMap<int, QXmppVideoDecoder*> decoders;
    QXmppVideoEncoder *encoder;
    QList<QXmppVideoFrame> frames;

    // local
    QXmppVideoFormat outgoingFormat;
    quint8 outgoingId;
    quint16 outgoingSequence;
    quint32 outgoingStamp;
    quint32 outgoingSsrc;
};

QXmppRtpVideoChannelPrivate::QXmppRtpVideoChannelPrivate()
    : encoder(0),
    outgoingId(0),
    outgoingSequence(1),
    outgoingStamp(0),
    outgoingSsrc(0)
{
    outgoingSsrc = qrand();
}

/// Constructs a new RTP video channel with the given \a parent.

QXmppRtpVideoChannel::QXmppRtpVideoChannel(QObject *parent)
    : QXmppLoggable(parent)
{
    d = new QXmppRtpVideoChannelPrivate;
    d->outgoingFormat.setFrameRate(15.0);
    d->outgoingFormat.setFrameSize(QSize(320, 240));
    d->outgoingFormat.setPixelFormat(QXmppVideoFrame::Format_YUYV);

    // set supported codecs
    QXmppVideoEncoder *encoder;
    QXmppJinglePayloadType payload;
    Q_UNUSED(encoder);
    Q_UNUSED(payload);

#ifdef QXMPP_USE_VPX
    encoder = new QXmppVpxEncoder;
    encoder->setFormat(d->outgoingFormat);
    payload.setId(96);
    payload.setName("vp8");
    payload.setClockrate(256000);
    payload.setParameters(encoder->parameters());
    m_outgoingPayloadTypes << payload;
    delete encoder;
#endif

#ifdef QXMPP_USE_THEORA
    encoder = new QXmppTheoraEncoder;
    encoder->setFormat(d->outgoingFormat);
    payload.setId(97);
    payload.setName("theora");
    payload.setClockrate(90000);
    payload.setParameters(encoder->parameters());
    m_outgoingPayloadTypes << payload;
    delete encoder;
#endif
}

QXmppRtpVideoChannel::~QXmppRtpVideoChannel()
{
    foreach (QXmppVideoDecoder *decoder, d->decoders)
        delete decoder;
    if (d->encoder)
        delete d->encoder;
    delete d;
}

/// Closes the RTP video channel.

void QXmppRtpVideoChannel::close()
{
}

/// Processes an incoming RTP video packet.
///
/// \param ba

void QXmppRtpVideoChannel::datagramReceived(const QByteArray &ba)
{
    QXmppRtpPacket packet;
    if (!packet.decode(ba))
        return;

#ifdef QXMPP_DEBUG_RTP
    logReceived(packet.toString());
#endif

    // get codec
    QXmppVideoDecoder *decoder = d->decoders.value(packet.type);
    if (!decoder)
        return;
    d->frames << decoder->handlePacket(packet);
}

/// Returns the video format used by the encoder.

QXmppVideoFormat QXmppRtpVideoChannel::decoderFormat() const
{
    if (d->decoders.isEmpty())
        return QXmppVideoFormat();
    const int key = d->decoders.keys().first();
    return d->decoders.value(key)->format();
}

/// Returns the video format used by the encoder.

QXmppVideoFormat QXmppRtpVideoChannel::encoderFormat() const
{
    return d->outgoingFormat;
}

/// Sets the video format used by the encoder.

void QXmppRtpVideoChannel::setEncoderFormat(const QXmppVideoFormat &format)
{
    if (d->encoder && !d->encoder->setFormat(format))
        return;
    d->outgoingFormat = format;
}

/// Returns the mode in which the channel has been opened.

QIODevice::OpenMode QXmppRtpVideoChannel::openMode() const
{
    QIODevice::OpenMode mode = QIODevice::NotOpen;
    if (!d->decoders.isEmpty())
        mode |= QIODevice::ReadOnly;
    if (d->encoder)
        mode |= QIODevice::WriteOnly;
    return mode;
}

/// \cond
void QXmppRtpVideoChannel::payloadTypesChanged()
{
    // refresh decoders
    foreach (QXmppVideoDecoder *decoder, d->decoders)
        delete decoder;
    d->decoders.clear();
    foreach (const QXmppJinglePayloadType &payload, m_incomingPayloadTypes) {
        QXmppVideoDecoder *decoder = 0;
        if (false)
            {}
#ifdef QXMPP_USE_THEORA
        else if (payload.name().toLower() == "theora")
            decoder = new QXmppTheoraDecoder;
#endif
#ifdef QXMPP_USE_VPX
        else if (payload.name().toLower() == "vp8")
            decoder = new QXmppVpxDecoder;
#endif
        if (decoder) {
            decoder->setParameters(payload.parameters());
            d->decoders.insert(payload.id(), decoder);
        }
    }

    // refresh encoder
    if (d->encoder) {
        delete d->encoder;
        d->encoder = 0;
    }
    foreach (const QXmppJinglePayloadType &payload, m_outgoingPayloadTypes) {
        QXmppVideoEncoder *encoder = 0;
        if (false)
            {}
#ifdef QXMPP_USE_THEORA
        else if (payload.name().toLower() == "theora")
            encoder = new QXmppTheoraEncoder;
#endif
#ifdef QXMPP_USE_VPX
        else if (payload.name().toLower() == "vp8") {
            encoder = new QXmppVpxEncoder(payload.clockrate());
        }
#endif
        if (encoder) {
            encoder->setFormat(d->outgoingFormat);
            d->encoder = encoder;
            d->outgoingId = payload.id();
            break;
        }
    }
}
/// \endcond

/// Decodes buffered RTP packets and returns a list of video frames.

QList<QXmppVideoFrame> QXmppRtpVideoChannel::readFrames()
{
    const QList<QXmppVideoFrame> frames = d->frames;
    d->frames.clear();
    return frames;
}

/// Encodes a video \a frame and sends RTP packets.

void QXmppRtpVideoChannel::writeFrame(const QXmppVideoFrame &frame)
{
    if (!d->encoder) {
        warning("QXmppRtpVideoChannel::writeFrame before codec was set");
        return;
    }

    QXmppRtpPacket packet;
    packet.version = RTP_VERSION;
    packet.marker = false;
    packet.type = d->outgoingId;
    packet.ssrc = d->outgoingSsrc;
    foreach (const QByteArray &payload, d->encoder->handleFrame(frame)) {
        packet.sequence = d->outgoingSequence++;
        packet.stamp = d->outgoingStamp;
        packet.payload = payload;
#ifdef QXMPP_DEBUG_RTP
        logSent(packet.toString());
#endif
        emit sendDatagram(packet.encode());
    }
    d->outgoingStamp += 1;
}

