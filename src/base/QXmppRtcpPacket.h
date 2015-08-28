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
class QXmppRtcpReceiverReport;
class QXmppRtcpReceiverReportPrivate;
class QXmppRtcpSenderInfo;
class QXmppRtcpSenderInfoPrivate;
class QXmppRtcpSourceDescription;
class QXmppRtcpSourceDescriptionPrivate;

/// \internal
///
/// The QXmppRtcpPacket class represents an RTCP packet.

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
    ~QXmppRtcpPacket();

    bool decode(const QByteArray &ba);
    QByteArray encode() const;

    bool read(QDataStream &stream);
    void write(QDataStream &stream) const;

    QString goodbyeReason() const;
    void setGoodbyeReason(const QString &goodbyeReason);

    QList<quint32> goodbyeSsrcs() const;
    void setGoodbyeSsrcs(const QList<quint32> &goodbyeSsrcs);

    QList<QXmppRtcpReceiverReport> receiverReports() const;
    void setReceiverReports(const QList<QXmppRtcpReceiverReport> &reports);

    QXmppRtcpSenderInfo senderInfo() const;
    void setSenderInfo(const QXmppRtcpSenderInfo &senderInfo);

    QList<QXmppRtcpSourceDescription> sourceDescriptions() const;
    void setSourceDescriptions(const QList<QXmppRtcpSourceDescription> &descriptions);

    quint32 ssrc() const;
    void setSsrc(quint32 ssrc);

    quint8 type() const;
    void setType(quint8 type);

private:
    QSharedDataPointer<QXmppRtcpPacketPrivate> d;
};

/// \internal

class QXMPP_EXPORT QXmppRtcpReceiverReport
{
public:
    QXmppRtcpReceiverReport();
    QXmppRtcpReceiverReport(const QXmppRtcpReceiverReport &other);
    ~QXmppRtcpReceiverReport();

    quint32 dlsr() const;
    void setDlsr(quint32 dlsr);

    quint8 fractionLost() const;
    void setFractionLost(quint8 fractionLost);

    quint32 jitter() const;
    void setJitter(quint32 jitter);

    quint32 lsr() const;
    void setLsr(quint32 lsr);

    quint32 ssrc() const;
    void setSsrc(quint32 ssrc);

    quint32 totalLost() const;
    void setTotalLost(quint32 totalLost);

private:
    friend class QXmppRtcpPacket;
    QSharedDataPointer<QXmppRtcpReceiverReportPrivate> d;
};

/// \internal

class QXMPP_EXPORT QXmppRtcpSenderInfo
{
public:
    QXmppRtcpSenderInfo();
    QXmppRtcpSenderInfo(const QXmppRtcpSenderInfo &other);
    ~QXmppRtcpSenderInfo();

    quint64 ntpStamp() const;
    void setNtpStamp(quint64 ntpStamp);

    quint32 rtpStamp() const;
    void setRtpStamp(quint32 rtpStamp);

    quint32 octetCount() const;
    void setOctetCount(quint32 count);

    quint32 packetCount() const;
    void setPacketCount(quint32 count);

private:
    friend class QXmppRtcpPacket;
    QSharedDataPointer<QXmppRtcpSenderInfoPrivate> d;
};

/// \internal

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

#endif
