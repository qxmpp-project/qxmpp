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

/// \internal
///
/// The QXmppRtpPacket class represents an RTP packet.

class QXMPP_EXPORT QXmppRtpPacket
{
public:
    QXmppRtpPacket();
    QXmppRtpPacket(const QXmppRtpPacket &other);
    ~QXmppRtpPacket();

    QXmppRtpPacket& operator=(const QXmppRtpPacket &other);

    bool decode(const QByteArray &ba);
    QByteArray encode() const;
    QString toString() const;

    QList<quint32> csrc() const;
    void setCsrc(const QList<quint32> &csrc);

    bool marker() const;
    void setMarker(bool marker);

    QByteArray payload() const;
    void setPayload(const QByteArray &payload);

    quint16 sequence() const;
    void setSequence(quint16 sequence);

    quint32 ssrc() const;
    void setSsrc(quint32 ssrc);
    
    quint32 stamp() const;
    void setStamp(quint32 stamp);

    quint8 type() const;
    void setType(quint8 type);

private:
    QSharedDataPointer<QXmppRtpPacketPrivate> d;
};

#endif
