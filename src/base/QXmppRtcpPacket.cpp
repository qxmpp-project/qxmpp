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
#include <QDebug>

#include "QXmppRtcpPacket.h"

#define RTP_VERSION 2

class QXmppRtcpPacketPrivate : public QSharedData
{
public:
    /// Number of report blocks.
    quint8 count;
    /// Payload type.
    quint8 type;
    /// Raw payload data.
    QByteArray payload;
};

/// Constructs an empty RTCP packet

QXmppRtcpPacket::QXmppRtcpPacket()
    : d(new QXmppRtcpPacketPrivate())
{
}

/// Constructs a copy of other.
///
/// \param other
///
QXmppRtcpPacket::QXmppRtcpPacket(const QXmppRtcpPacket &other)
    : d(other.d)
{
}

QXmppRtcpPacket::~QXmppRtcpPacket()
{
}

/// Assigns the other packet to this one.
///
/// \param other
///
QXmppRtcpPacket& QXmppRtcpPacket::operator=(const QXmppRtcpPacket& other)
{
    d = other.d;
    return *this;
}

/// Parses an RTCP packet.
///
/// \param ba
bool QXmppRtcpPacket::decode(const QByteArray &ba)
{
    quint8 tmp, type;
    quint16 len;

    // fixed header
    QDataStream stream(ba);
    stream >> tmp;
    stream >> type;
    stream >> len;
    if (stream.status() != QDataStream::Ok)
        return false;

    // check version
    if ((tmp >> 6) != RTP_VERSION)
        return false;

    const int payloadLength = len << 2;
    d->count = (tmp & 0x1f);
    d->type = type;
    d->payload.resize(payloadLength);
    return stream.readRawData(d->payload.data(), payloadLength) == payloadLength;
}

/// Encodes an RTCP packet.

QByteArray QXmppRtcpPacket::encode() const
{
    QByteArray ba;
    ba.resize(4 + d->payload.size());

    QDataStream stream(&ba, QIODevice::WriteOnly);
    stream << quint8((RTP_VERSION << 6) | (d->count & 0x1f));
    stream << d->type;
    stream << quint16(d->payload.size() >> 2);
    stream.writeRawData(d->payload.constData(), d->payload.size());
    return ba;
}

/// Returns the number of report blocks.

quint8 QXmppRtcpPacket::count() const
{
    return d->count;
}

/// Sets the number of report blocks.
///
/// \param count

void QXmppRtcpPacket::setCount(quint8 count)
{
    d->count = count;
}

/// Returns the RTCP packet type.

quint8 QXmppRtcpPacket::type() const
{
    return d->type;
}

/// Sets the RTCP packet type.
///
/// \param type

void QXmppRtcpPacket::setType(quint8 type)
{
    d->type = type;
}
