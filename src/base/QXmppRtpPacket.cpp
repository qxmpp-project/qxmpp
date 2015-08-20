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

#include <QDataStream>
#include <QSharedData>

#include "QXmppRtpPacket.h"

#define RTP_VERSION 2

class QXmppRtpPacketPrivate : public QSharedData
{
public:
    QXmppRtpPacketPrivate();

    /// Marker flag.
    bool marker;
    /// Payload type.
    quint8 type;
    /// Synchronization source.
    quint32 ssrc;
    /// Contributing sources.
    QList<quint32> csrc;
    /// Sequence number.
    quint16 sequence;
    /// Timestamp.
    quint32 stamp;
    /// Raw payload data.
    QByteArray payload;
};

QXmppRtpPacketPrivate::QXmppRtpPacketPrivate()
    : marker(false)
    , type(0)
    , ssrc(0)
    , sequence(0)
    , stamp(0)
{
}

/// Constructs an empty RTP packet

QXmppRtpPacket::QXmppRtpPacket()
    : d(new QXmppRtpPacketPrivate())
{
}

/// Constructs a copy of other.
///
/// \param other
///
QXmppRtpPacket::QXmppRtpPacket(const QXmppRtpPacket &other)
    : d(other.d)
{
}

QXmppRtpPacket::~QXmppRtpPacket()
{
}

/// Assigns the other packet to this one.
///
/// \param other
///
QXmppRtpPacket& QXmppRtpPacket::operator=(const QXmppRtpPacket& other)
{
    d = other.d;
    return *this;
}

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
    const quint8 cc = (tmp >> 1) & 0xf;
    const int hlen = 12 + 4 * cc;
    if ((tmp >> 6) != RTP_VERSION || ba.size() < hlen)
        return false;
    stream >> tmp;
    d->marker = (tmp >> 7);
    d->type = tmp & 0x7f;
    stream >> d->sequence;
    stream >> d->stamp;
    stream >> d->ssrc;

    // contributing source IDs
    d->csrc.clear();
    quint32 src;
    for (int i = 0; i < cc; ++i) {
        stream >> src;
        d->csrc << src;
    }

    // retrieve payload
    d->payload = ba.right(ba.size() - hlen);
    return true;
}

/// Encodes an RTP packet.

QByteArray QXmppRtpPacket::encode() const
{
    Q_ASSERT(d->csrc.size() < 16);

    // fixed header
    QByteArray ba;
    ba.resize(d->payload.size() + 12 + 4 * d->csrc.size());
    QDataStream stream(&ba, QIODevice::WriteOnly);
    stream << quint8((RTP_VERSION << 6) |
                     ((d->csrc.size() & 0xf) << 1));
    stream << quint8((d->type & 0x7f) | (d->marker << 7));
    stream << d->sequence;
    stream << d->stamp;
    stream << d->ssrc;

    // contributing source ids
    foreach (const quint32 &src, d->csrc)
        stream << src;

    stream.writeRawData(d->payload.constData(), d->payload.size());
    return ba;
}

QList<quint32> QXmppRtpPacket::csrc() const
{
    return d->csrc;
}

void QXmppRtpPacket::setCsrc(const QList<quint32> &csrc)
{
    d->csrc = csrc;
}

bool QXmppRtpPacket::marker() const
{
    return d->marker;
}

void QXmppRtpPacket::setMarker(bool marker)
{
    d->marker = marker;
}

QByteArray QXmppRtpPacket::payload() const
{
    return d->payload;
}

void QXmppRtpPacket::setPayload(const QByteArray &payload)
{
    d->payload = payload;
}

quint32 QXmppRtpPacket::ssrc() const
{
    return d->ssrc;
}

void QXmppRtpPacket::setSsrc(quint32 ssrc)
{
    d->ssrc = ssrc;
}

quint16 QXmppRtpPacket::sequence() const
{
    return d->sequence;
}

void QXmppRtpPacket::setSequence(quint16 sequence)
{
    d->sequence = sequence;
}

quint32 QXmppRtpPacket::stamp() const
{
    return d->stamp;
}

void QXmppRtpPacket::setStamp(quint32 stamp)
{
    d->stamp = stamp;
}

quint8 QXmppRtpPacket::type() const
{
    return d->type;
}

void QXmppRtpPacket::setType(quint8 type)
{
    d->type = type;
}

/// Returns a string representation of the RTP header.

QString QXmppRtpPacket::toString() const
{
    return QString("RTP packet seq %1 stamp %2 marker %3 type %4 size %5").arg(
        QString::number(d->sequence),
        QString::number(d->stamp),
        QString::number(d->marker),
        QString::number(d->type),
        QString::number(d->payload.size()));
}
