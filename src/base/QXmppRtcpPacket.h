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
class QXmppRtcpSenderReport;
class QXmppRtcpSenderReportPrivate;
class QXmppRtcpSourceDescription;
class QXmppRtcpSourceDescriptionPrivate;

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
    ~QXmppRtcpPacket();

    bool decode(const QByteArray &ba);
    QByteArray encode() const;

    bool read(QDataStream &stream);
    void write(QDataStream &stream) const;

    quint8 type() const;
    void setType(quint8 type);

    QList<QXmppRtcpReceiverReport> receiverReports() const;
    void setReceiverReports(const QList<QXmppRtcpReceiverReport> &reports);

    QXmppRtcpSenderReport senderReport() const;
    void setSenderReport(const QXmppRtcpSenderReport &report);

    QList<QXmppRtcpSourceDescription> sourceDescriptions() const;
    void setSourceDescriptions(const QList<QXmppRtcpSourceDescription> &descriptions);

private:
    QSharedDataPointer<QXmppRtcpPacketPrivate> d;
};

class QXMPP_EXPORT QXmppRtcpReceiverReport
{
public:
    QXmppRtcpReceiverReport();
    QXmppRtcpReceiverReport(const QXmppRtcpReceiverReport &other);
    ~QXmppRtcpReceiverReport();

    quint32 ssrc() const;
    void setSsrc(const quint32 ssrc);

private:
    friend class QXmppRtcpPacket;
    QSharedDataPointer<QXmppRtcpReceiverReportPrivate> d;
};

class QXMPP_EXPORT QXmppRtcpSenderReport
{
public:
    QXmppRtcpSenderReport();
    QXmppRtcpSenderReport(const QXmppRtcpSenderReport &other);
    ~QXmppRtcpSenderReport();

    quint64 ntpStamp() const;
    void setNtpStamp(quint64 ntpStamp);

    quint32 rtpStamp() const;
    void setRtpStamp(quint32 rtpStamp);

    quint32 ssrc() const;
    void setSsrc(quint32 ssrc);

    quint32 octetCount() const;
    void setOctetCount(quint32 count);

    quint32 packetCount() const;
    void setPacketCount(quint32 count);

private:
    friend class QXmppRtcpPacket;
    QSharedDataPointer<QXmppRtcpSenderReportPrivate> d;
};

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
