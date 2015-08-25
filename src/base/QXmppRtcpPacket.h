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

#ifndef QXMPPRTCPPACKET_H
#define QXMPPRTCPPACKET_H

#include <QSharedDataPointer>

#include "QXmppGlobal.h"

class QXmppRtcpPacketPrivate;
class QXmppRtcpSourceDescriptionPrivate;

class QXMPP_EXPORT QXmppRtcpSourceDescription
{
public:
    QXmppRtcpSourceDescription();
    QXmppRtcpSourceDescription(const QXmppRtcpSourceDescription &other);
    ~QXmppRtcpSourceDescription();

    QString cname() const;
    void setCname(const QString &name);

    QString name() const;
    void setName(const QString &name);

    quint32 ssrc() const;
    void setSsrc(const quint32 ssrc);

private:
    friend class QXmppRtcpPacket;
    QSharedDataPointer<QXmppRtcpSourceDescriptionPrivate> d;
};

/// \brief The QXmppRtpPacket class represents an RTCP packet.
///

class QXMPP_EXPORT QXmppRtcpPacket
{
public:
    enum Type {
        SenderReport        = 200,
        ReceiverReport      = 201,
        SourceDescription   = 202,
        Goodbye             = 203,
    };

    QXmppRtcpPacket();
    QXmppRtcpPacket(const QXmppRtcpPacket &other);
    virtual ~QXmppRtcpPacket();

    QXmppRtcpPacket& operator=(const QXmppRtcpPacket &other);

    bool decode(const QByteArray &ba);
    QByteArray encode() const;

    bool read(QDataStream &stream);
    void write(QDataStream &stream) const;

    quint8 count() const;
    void setCount(quint8 count);

    QByteArray payload() const;
    void setPayload(const QByteArray &payload);

    quint8 type() const;
    void setType(quint8 type);

    QList<QXmppRtcpSourceDescription> sourceDescriptions() const;
    void setSourceDescriptions(const QList<QXmppRtcpSourceDescription> &descriptions);

private:
    QSharedDataPointer<QXmppRtcpPacketPrivate> d;
};

#endif
