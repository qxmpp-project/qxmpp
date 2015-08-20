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

#ifndef QXMPPRTPPACKET_H
#define QXMPPRTPPACKET_H

#include <QSharedDataPointer>

#include "QXmppGlobal.h"

class QXmppRtpPacketPrivate;

/// \brief The QXmppRtpPacket class represents an RTP packet.
///

class QXMPP_EXPORT QXmppRtpPacket
{
public:
    QXmppRtpPacket();
    ~QXmppRtpPacket();

    bool decode(const QByteArray &ba);
    QByteArray encode() const;
    QString toString() const;

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

#endif
