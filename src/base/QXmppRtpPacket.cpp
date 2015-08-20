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

#include "QXmppRtpPacket.h"

#define RTP_VERSION 2

QXmppRtpPacket::QXmppRtpPacket()
    : marker(false)
    , type(0)
    , ssrc(0)
    , sequence(0)
    , stamp(0)
{
}

QXmppRtpPacket::~QXmppRtpPacket()
{
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
    stream << quint8((RTP_VERSION << 6) |
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
