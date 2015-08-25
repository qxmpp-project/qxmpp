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

enum DescriptionType {
    CnameType = 1,
    NameType  = 2
};

class QXmppRtcpPacketPrivate : public QSharedData
{
public:
    /// Number of report blocks.
    quint8 count;
    /// Payload type.
    quint8 type;
    /// Raw payload data.
    QByteArray payload;

    QList<QXmppRtcpSourceDescription> sourceDescriptions;
};

class QXmppRtcpSourceDescriptionPrivate : public QSharedData
{
public:
    bool read(QDataStream &stream);
    void write(QDataStream &stream) const;

    quint32 ssrc;
    QString cname;
    QString name;
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
    QDataStream stream(ba);
    return read(stream);
}

/// Encodes an RTCP packet.

QByteArray QXmppRtcpPacket::encode() const
{
    QByteArray ba;
    ba.resize(4 + d->payload.size());

    QDataStream stream(&ba, QIODevice::WriteOnly);
    write(stream);
    return ba;
}

bool QXmppRtcpPacket::read(QDataStream &stream)
{
    quint8 tmp, type;
    quint16 len;

    // fixed header
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
    if (stream.readRawData(d->payload.data(), payloadLength) != payloadLength)
        return false;

    if (d->type == SourceDescription) {
        QDataStream s(d->payload);
        for (int j = 0; j < d->count; ++j) {
            QXmppRtcpSourceDescription desc;
            if (!desc.d->read(s))
                return false;
            d->sourceDescriptions << desc;
        }
    }
    return true;
}

void QXmppRtcpPacket::write(QDataStream &stream) const
{
    QByteArray payload;
    quint8 count;

    if (d->type == SourceDescription) {
        count = d->sourceDescriptions.size();
        QDataStream s(&payload, QIODevice::WriteOnly);
        foreach (const QXmppRtcpSourceDescription &desc, d->sourceDescriptions)
            desc.d->write(s);
    } else {
        count = d->count;
        payload = d->payload;
    }

    stream << quint8((RTP_VERSION << 6) | (count & 0x1f));
    stream << d->type;
    stream << quint16(payload.size() >> 2);
    stream.writeRawData(payload.constData(), payload.size());
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

QByteArray QXmppRtcpPacket::payload() const
{
    return d->payload;
}

QList<QXmppRtcpSourceDescription> QXmppRtcpPacket::sourceDescriptions() const
{
    return d->sourceDescriptions;
}

void QXmppRtcpPacket::setSourceDescriptions(const QList<QXmppRtcpSourceDescription> &descriptions)
{
    d->sourceDescriptions = descriptions;
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

bool QXmppRtcpSourceDescriptionPrivate::read(QDataStream &stream)
{
    QByteArray buffer;
    quint8 itemType, itemLength;
    quint16 chunkLength = 0;

    stream >> ssrc;
    if (stream.status() != QDataStream::Ok)
        return false;
    while (true) {
        stream >> itemType;
        if (stream.status() != QDataStream::Ok)
            return false;
        if (!itemType) {
            chunkLength++;
            break;
        }

        stream >> itemLength;
        if (stream.status() != QDataStream::Ok)
            return false;

        buffer.resize(itemLength);
        if (stream.readRawData(buffer.data(), itemLength) != itemLength)
            return false;
        chunkLength += itemLength + 2;

        if (itemType == CnameType)
            cname = QString::fromUtf8(buffer);
        else if (itemType == NameType)
            name = QString::fromUtf8(buffer);
    }
    if (chunkLength % 4) {
        buffer.resize(4 - chunkLength % 4);
        if (stream.readRawData(buffer.data(), buffer.size()) != buffer.size())
            return false;
        if (buffer != QByteArray(buffer.size(), '\0'))
            return false;
    }
    return true;
}

void QXmppRtcpSourceDescriptionPrivate::write(QDataStream &stream) const
{
    QByteArray buffer;
    quint16 chunkLength = 0;

    stream << ssrc;
    if (!cname.isEmpty()) {
        buffer = cname.toUtf8();
        stream << quint8(CnameType);
        stream << quint8(buffer.size());
        stream.writeRawData(buffer.constData(), buffer.size());
        chunkLength += 2 + buffer.size();
    }
    if (!name.isEmpty()) {
        buffer = name.toUtf8();
        stream << quint8(NameType);
        stream << quint8(buffer.size());
        stream.writeRawData(buffer.constData(), buffer.size());
        chunkLength += 2 + buffer.size();
    }
    stream << quint8(0);
    chunkLength++;
    if (chunkLength % 4) {
        buffer = QByteArray(4 - chunkLength % 4, '\0');
        stream.writeRawData(buffer.constData(), buffer.size());
    }
}

/// Constructs an empty source description

QXmppRtcpSourceDescription::QXmppRtcpSourceDescription()
    : d(new QXmppRtcpSourceDescriptionPrivate())
{
    d->ssrc = 0;
}

/// Constructs a copy of other.
///
/// \param other
///
QXmppRtcpSourceDescription::QXmppRtcpSourceDescription(const QXmppRtcpSourceDescription &other)
    : d(other.d)
{
}

QXmppRtcpSourceDescription::~QXmppRtcpSourceDescription()
{
}

QString QXmppRtcpSourceDescription::cname() const
{
    return d->cname;
}

void QXmppRtcpSourceDescription::setCname(const QString &cname)
{
    d->cname = cname;
}

QString QXmppRtcpSourceDescription::name() const
{
    return d->name;
}

void QXmppRtcpSourceDescription::setName(const QString &name)
{
    d->name = name;
}

quint32 QXmppRtcpSourceDescription::ssrc() const
{
    return d->ssrc;
}

void QXmppRtcpSourceDescription::setSsrc(quint32 ssrc)
{
    d->ssrc = ssrc;
}
