/*
 * Copyright (C) 2010 Bolloré telecom
 *
 * Author:
 *	Jeremy Lainé
 *
 * Source:
 *	http://code.google.com/p/qxmpp
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

#include <QCryptographicHash>
#include <QDebug>
#include <QNetworkInterface>
#include <QUdpSocket>
#include <QTimer>

#include "QXmppStun.h"
#include "QXmppUtils.h"

static const quint32 STUN_MAGIC = 0x2112A442;
static const quint16 STUN_HEADER = 20;
static const quint8 STUN_IPV4 = 0x01;
static const quint8 STUN_IPV6 = 0x02;

enum MessageType {
    BindingRequest       = 0x0001,
    BindingIndication    = 0x0011,
    BindingResponse      = 0x0101,
    BindingError         = 0x0111,
    SharedSecretRequest  = 0x0002,
    SharedSecretResponse = 0x0102,
    SharedSecretError    = 0x0112,
};

enum AttributeType {
    Username         = 0x0006,
    MessageIntegrity = 0x0008,
    ErrorCode        = 0x0009,
    XorMappedAddress = 0x0020,
    Priority         = 0x0024,
    UseCandidate     = 0x0025,
    Fingerprint      = 0x8028,
    IceControlled    = 0x8029,
    IceControlling   = 0x802a,
};

static QByteArray randomByteArray(int length)
{
    QByteArray result(length, 0);
    for (int i = 0; i < length; ++i)
        result[i] = quint8(qrand() % 255);
    return result;
}

class QXmppStunMessage
{
public:
    QXmppStunMessage();

    QByteArray encode(const QString &password = QString()) const;
    bool decode(const QByteArray &buffer, const QString &password = QString());

    static quint16 peekType(const QByteArray &buffer);

    quint16 type;
    QByteArray id;

    // attributes
    int errorCode;
    QString errorPhrase;
    quint32 priority;
    QByteArray iceControlling;
    QByteArray iceControlled;
    QHostAddress mappedHost;
    quint16 mappedPort;
    QString username;
    bool useCandidate;

private:
    void setBodyLength(QByteArray &buffer, qint16 length) const;
};

/// Constructs a new QXmppStunMessage.

QXmppStunMessage::QXmppStunMessage()
    : errorCode(0), priority(0), mappedPort(0), useCandidate(false)
{
}

/// Decodes a QXmppStunMessage and checks its integrity using the given
/// password.
///
/// \param buffer
/// \param password

bool QXmppStunMessage::decode(const QByteArray &buffer, const QString &password)
{
    if (buffer.size() < STUN_HEADER)
    {
        qWarning("QXmppStunMessage received a truncated STUN packet");
        return false;
    }

    // parse STUN header
    QDataStream stream(buffer);
    quint16 length;
    quint32 cookie;
    stream >> type;
    stream >> length;
    stream >> cookie;
    id.resize(12);
    stream.readRawData(id.data(), id.size());

    if (cookie != STUN_MAGIC || length != buffer.size() - STUN_HEADER)
    {
        qWarning("QXmppStunMessage received an invalid STUN packet");
        return false;
    }

    // parse STUN attributes
    int done = 0;
    while (done < length)
    {
        quint16 a_type, a_length;
        stream >> a_type;
        stream >> a_length;
        const int pad_length = 4 * ((a_length + 3) / 4) - a_length;
        if (a_type == Priority)
        {
            if (a_length != sizeof(priority))
                return false;
            stream >> priority;
        } else if (a_type == ErrorCode) {
            if (a_length < 4)
                return false;
            quint16 reserved;
            quint8 errorCodeHigh, errorCodeLow;
            stream >> reserved;
            stream >> errorCodeHigh;
            stream >> errorCodeLow;
            errorCode = errorCodeHigh * 100 + errorCodeLow;
            QByteArray phrase(a_length - 4, 0);
            stream.readRawData(phrase.data(), phrase.size());
            errorPhrase = QString::fromUtf8(phrase);
        } else if (a_type == UseCandidate) {
            if (a_length != 0)
                return false;
            useCandidate = true;
        } else if (a_type == XorMappedAddress) {
            if (a_length < 4)
                return false;
            quint8 reserved, protocol;
            quint16 xport;
            stream >> reserved;
            stream >> protocol;
            stream >> xport;
            mappedPort = xport ^ (STUN_MAGIC >> 16);
            if (protocol == STUN_IPV4)
            {
                if (a_length != 8)
                    return false;
                quint32 xaddr;
                stream >> xaddr;
                mappedHost = QHostAddress(xaddr ^ STUN_MAGIC);
            } else if (protocol == STUN_IPV6) {
                if (a_length != 20)
                    return false;
                QByteArray xaddr(16, 0);
                stream.readRawData(xaddr.data(), xaddr.size());
                QByteArray xpad;
                QDataStream(&xpad, QIODevice::WriteOnly) << STUN_MAGIC;
                xpad += id;
                Q_IPV6ADDR addr;
                for (int i = 0; i < 16; i++)
                    addr[i] = xaddr[i] ^ xpad[i];
                mappedHost = QHostAddress(addr);
            } else {
                qWarning("QXmppStunMessage bad protocol");
                return false;
            }
        } else if (a_type == MessageIntegrity) {
            if (a_length != 20)
                return false;
            QByteArray integrity(20, 0);
            stream.readRawData(integrity.data(), integrity.size());
            // check HMAC-SHA1
            if (!password.isEmpty())
            {
                const QByteArray key = password.toUtf8();
                QByteArray copy = buffer.left(STUN_HEADER + done);
                setBodyLength(copy, done + 24);
                if (integrity != generateHmacSha1(key, copy))
                {
                    qWarning("QXmppStunMessage bad integrity");
                    return false;
                }
            }
        } else if (a_type == Fingerprint) {
            if (a_length != 4)
                return false;
            quint32 fingerprint;
            stream >> fingerprint;
            // check CRC32
            QByteArray copy = buffer.left(STUN_HEADER + done);
            setBodyLength(copy, done + 8);
            const quint32 expected = generateCrc32(copy) ^ 0x5354554eL;
            if (fingerprint != expected)
            {
                qWarning("QXmppStunMessage bad fingerprint");
                return false;
            }
        } else {
            QByteArray a_value(a_length, 0);
            stream.readRawData(a_value.data(), a_value.size());
            if (a_type == Username)
            {
                username = QString::fromUtf8(a_value);
            } else if (a_type == IceControlling) {
                iceControlling = a_value;
            } else if (a_type == IceControlled) {
                iceControlled = a_value;
            } else {
                qWarning() << "QXmppStunMessage unknown attribute type" << a_type << "length" << a_length << "padding" << pad_length << "value" << a_value.toHex();
           }
        }
        stream.skipRawData(pad_length);
        done += 4 + a_length + pad_length;
    }
    return true;
}

/// Encodes the current QXmppStunMessage, optionally calculating the
/// message integrity attribute using the given password.
/// 
/// \param password

QByteArray QXmppStunMessage::encode(const QString &password) const
{
    QByteArray buffer;
    QDataStream stream(&buffer, QIODevice::WriteOnly);

    // encode STUN header
    quint16 length = 0;
    stream << type;
    stream << length;
    stream << STUN_MAGIC;
    stream.writeRawData(id.data(), id.size());

    // XOR-MAPPED-ADDRESS
    if (mappedPort && !mappedHost.isNull() &&
        (mappedHost.protocol() == QAbstractSocket::IPv4Protocol ||
         mappedHost.protocol() == QAbstractSocket::IPv6Protocol))
    {
        stream << quint16(XorMappedAddress);
        stream << quint16(8);
        stream << quint8(0);
        if (mappedHost.protocol() == QAbstractSocket::IPv4Protocol)
        {
            stream << quint8(STUN_IPV4);
            stream << quint16(mappedPort ^ (STUN_MAGIC >> 16));
            stream << quint32(mappedHost.toIPv4Address() ^ STUN_MAGIC);
        } else {
            stream << quint8(STUN_IPV6);
            stream << quint16(mappedPort ^ (STUN_MAGIC >> 16));
            Q_IPV6ADDR addr = mappedHost.toIPv6Address();
            QByteArray xaddr;
            QDataStream(&xaddr, QIODevice::WriteOnly) << STUN_MAGIC;
            xaddr += id;
            for (int i = 0; i < 16; i++)
                xaddr[i] = xaddr[i] ^ addr[i];
            stream.writeRawData(xaddr.data(), xaddr.size());
        }
    }

    // ERROR-CODE
    if (errorCode)
    {
        const quint16 reserved = 0;
        const quint8 errorCodeHigh = errorCode / 100;
        const quint8 errorCodeLow = errorCode % 100;
        const QByteArray phrase = errorPhrase.toUtf8();
        stream << quint16(ErrorCode);
        stream << quint16(phrase.size() + 4);
        stream << reserved;
        stream << errorCodeHigh;
        stream << errorCodeLow;
        stream.writeRawData(phrase.data(), phrase.size());
        if (phrase.size() % 4)
        {
            const QByteArray padding(4 - (phrase.size() %  4), 0);
            stream.writeRawData(padding.data(), padding.size());
        }
    }

    // PRIORITY
    if (priority)
    {
        stream << quint16(Priority);
        stream << quint16(sizeof(priority));
        stream << priority;
    }

    // USE-CANDIDATE
    if (useCandidate)
    {
        stream << quint16(UseCandidate);
        stream << quint16(0);
    }

    // ICE-CONTROLLING or ICE-CONTROLLED
    if (!iceControlling.isEmpty())
    {
        stream << quint16(IceControlling);
        stream << quint16(iceControlling.size());
        stream.writeRawData(iceControlling.data(), iceControlling.size());
    } else if (!iceControlled.isEmpty()) {
        stream << quint16(IceControlled);
        stream << quint16(iceControlled.size());
        stream.writeRawData(iceControlled.data(), iceControlled.size());
    }

    // USERNAME
    if (!username.isEmpty())
    {
        const QByteArray user = username.toUtf8();
        stream << quint16(Username);
        stream << quint16(user.size());
        stream.writeRawData(user.data(), user.size());
        if (user.size() % 4)
        {
            const QByteArray padding(4 - (user.size() % 4), 0);
            stream.writeRawData(padding.data(), padding.size());
        }
    }

    // set body length
    setBodyLength(buffer, buffer.size() - STUN_HEADER);

    // integrity
    if (!password.isEmpty())
    {
        const QByteArray key = password.toUtf8();
        setBodyLength(buffer, buffer.size() - STUN_HEADER + 24);
        QByteArray integrity = generateHmacSha1(key, buffer);
        stream << quint16(MessageIntegrity);
        stream << quint16(integrity.size());
        stream.writeRawData(integrity.data(), integrity.size());
    }

    // fingerprint
    setBodyLength(buffer, buffer.size() - STUN_HEADER + 8);
    quint32 fingerprint = generateCrc32(buffer) ^ 0x5354554eL;
    stream << quint16(Fingerprint);
    stream << quint16(sizeof(fingerprint));
    stream << fingerprint;

    return buffer;
}

/// If the given packet looks like a STUN message, returns the message
/// type, otherwise returns 0.
///
/// \param buffer

quint16 QXmppStunMessage::peekType(const QByteArray &buffer)
{
    if (buffer.size() < STUN_HEADER)
        return 0;

    // parse STUN header
    QDataStream stream(buffer);
    quint16 type;
    quint16 length;
    quint32 cookie;
    stream >> type;
    stream >> length;
    stream >> cookie;

    if (cookie != STUN_MAGIC || length != buffer.size() - STUN_HEADER)
        return 0;

    return type;
}
 
void QXmppStunMessage::setBodyLength(QByteArray &buffer, qint16 length) const
{
    QDataStream stream(&buffer, QIODevice::WriteOnly);
    stream.device()->seek(2);
    stream << length;
}

/// Constructs a new QXmppStunSocket.
///

QXmppStunSocket::QXmppStunSocket(bool iceControlling, QObject *parent)
    : QObject(parent),
    m_openMode(QIODevice::NotOpen),
    m_iceControlling(iceControlling),
    m_remotePort(0)
{
    m_localUser = generateStanzaHash(4);
    m_localPassword = generateStanzaHash(22);

    m_socket = new QUdpSocket;
    connect(m_socket, SIGNAL(readyRead()), this, SLOT(slotReadyRead()));
    if (!m_socket->bind())
        qWarning("QXmppStunSocket could not start listening");
}

/// Returns the component id for the current socket, e.g. 1 for RTP
/// and 2 for RTCP.

int QXmppStunSocket::component() const
{
    return m_component;
}

/// Sets the component id for the current socket, e.g. 1 for RTP
/// and 2 for RTCP.
///
/// \param component

void QXmppStunSocket::setComponent(int component)
{
    m_component = component;
}

/// Closes the socket.

void QXmppStunSocket::close()
{
    m_socket->close();
}

/// Start ICE connectivity checks.

void QXmppStunSocket::connectToHost()
{
    if (!m_iceControlling)
        return;

    foreach (const QXmppJingleCandidate &candidate, m_remoteCandidates)
    {
        // send a binding request
        QXmppStunMessage message;
        message.id = randomByteArray(12);
        message.type = BindingRequest;
        // FIXME: calculate priority
        message.priority = 1862270975;
        message.username = QString("%1:%2").arg(m_remoteUser, m_localUser);
        message.iceControlling = QByteArray(8, 0);
        message.useCandidate = true;
        dumpMessage(message, true, candidate.host(), candidate.port());
        m_socket->writeDatagram(message.encode(m_remotePassword), candidate.host(), candidate.port());
    }
}

/// Returns the QIODevice::OpenMode which represents the socket's ability
/// to read and/or write data.

QIODevice::OpenMode QXmppStunSocket::openMode() const
{
    return m_openMode;
}

void QXmppStunSocket::dumpMessage(const QXmppStunMessage &message, bool sent, const QHostAddress &host, quint16 port)
{
#ifdef QXMPP_DEBUG_STUN
    qDebug() << "STUN(" << m_component << ")" << (sent ? "sent to" : "received from") << host.toString() << "port" << port;
    QString typeName;
    switch (message.type & 0x000f)
    {
        case 1: typeName = "Binding"; break;
        case 2: typeName = "Shared Secret"; break;
        default: typeName = "Unknown"; break;
    }
    switch (message.type & 0x0ff0)
    {
        case 0x000: typeName += " Request"; break;
        case 0x010: typeName += " Indication"; break;
        case 0x100: typeName += " Response"; break;
        case 0x110: typeName += " Error"; break;
        default: break;
    }
    qDebug() << " type" << typeName << " (" << message.type << ")";
    qDebug() << " id  " << message.id.toHex();

    // attributes
    if (!message.username.isEmpty())
        qDebug() << " * username" << message.username;
    if (message.errorCode)
        qDebug() << " * error   " << message.errorCode << message.errorPhrase;
    if (message.mappedPort)
        qDebug() << " * mapped  " << message.mappedHost.toString() << message.mappedPort;
#endif
}

/// Returns the list of local HOST CANDIDATES candidates by iterating
/// over the available network interfaces.

QList<QXmppJingleCandidate> QXmppStunSocket::localCandidates() const
{
    QList<QXmppJingleCandidate> candidates;
    foreach (const QNetworkInterface &interface, QNetworkInterface::allInterfaces())
    {
        if (!(interface.flags() & QNetworkInterface::IsRunning) ||
            interface.flags() & QNetworkInterface::IsLoopBack)
            continue;

        foreach (const QNetworkAddressEntry &entry, interface.addressEntries())
        {
            if (entry.ip().protocol() != QAbstractSocket::IPv4Protocol ||
                entry.netmask().isNull() ||
                entry.netmask() == QHostAddress::Broadcast)
                continue;

            QXmppJingleCandidate candidate;
            candidate.setComponent(m_component);
            candidate.setHost(entry.ip());
            candidate.setId(generateStanzaHash(10));
            candidate.setNetwork(interface.index());
            candidate.setPort(m_socket->localPort());
            candidate.setPriority(2130706432 - m_component);
            candidate.setProtocol("udp");
            candidate.setType("host");
            candidates << candidate;
        }
    }
    return candidates;
}

QString QXmppStunSocket::localUser() const
{
    return m_localUser;
}

void QXmppStunSocket::setLocalUser(const QString &user)
{
    m_localUser = user;
}

QString QXmppStunSocket::localPassword() const
{
    return m_localPassword;
}

void QXmppStunSocket::setLocalPassword(const QString &password)
{
    m_localPassword = password;
}

/// Adds remote STUN candidates.

void QXmppStunSocket::addRemoteCandidates(const QList<QXmppJingleCandidate> &candidates)
{
    foreach (const QXmppJingleCandidate &candidate, candidates)
    {
        if (candidate.component() == m_component &&
            candidate.type() == "host" &&
            candidate.protocol() == "udp")
            m_remoteCandidates << candidate;
    }
}

void QXmppStunSocket::setRemoteUser(const QString &user)
{
    m_remoteUser = user;
}

void QXmppStunSocket::setRemotePassword(const QString &password)
{
    m_remotePassword = password;
}

void QXmppStunSocket::slotReadyRead()
{
    const qint64 size = m_socket->pendingDatagramSize();
    QHostAddress remoteHost;
    quint16 remotePort;
    QByteArray buffer(size, 0);
    m_socket->readDatagram(buffer.data(), buffer.size(), &remoteHost, &remotePort);

    // if this is not a STUN message, emit it
    quint16 messageType = QXmppStunMessage::peekType(buffer);
    if (!messageType)
    {
        emit datagramReceived(buffer, remoteHost, remotePort);
        return;
    }

    const QString messagePassword = (messageType & 0xFF00) ? m_remotePassword : m_localPassword;
    if (messagePassword.isEmpty())
        return;
    QXmppStunMessage message;
    if (!message.decode(buffer, messagePassword))
        return;
    dumpMessage(message, false, remoteHost, remotePort);

    if (m_openMode == QIODevice::ReadWrite)
        return;

    if (message.type == BindingRequest)
    {
        // send a binding response
        QXmppStunMessage response;
        response.id = message.id;
        response.type = BindingResponse;
        response.username = message.username;
        response.mappedHost = remoteHost;
        response.mappedPort = remotePort;
        dumpMessage(response, true, remoteHost, remotePort);
        m_socket->writeDatagram(response.encode(m_localPassword), remoteHost, remotePort);

        if (m_iceControlling || message.useCandidate)
        {
            // outgoing media can flow
            qDebug() << "STUN(" << m_component << ") OUTGOING MEDIA ENABLED";
            m_openMode |= QIODevice::WriteOnly;
            m_remoteHost = remoteHost;
            m_remotePort = remotePort;
            emit ready();
        }

        if (!m_iceControlling)
        {
            // send a triggered connectivity test
            QXmppStunMessage message;
            message.id = randomByteArray(12);
            message.type = BindingRequest;
            // FIXME : calculate priority
            message.priority = 1862270975;
            message.username = QString("%1:%2").arg(m_remoteUser, m_localUser);
            message.iceControlled = QByteArray(8, 0);
            dumpMessage(message, true, remoteHost, remotePort);
            m_socket->writeDatagram(message.encode(m_remotePassword), remoteHost, remotePort);
        }
    } else if (message.type == BindingResponse) {
        // send a binding indication
        QXmppStunMessage indication;
        indication.id = randomByteArray(12);
        indication.type = BindingIndication;
        dumpMessage(indication, true, remoteHost, remotePort);
        m_socket->writeDatagram(indication.encode(), remoteHost, remotePort);

        // incoming media can flow
        qDebug() << "STUN(" << m_component << ") INCOMING MEDIA ENABLED";
        m_openMode |= QIODevice::ReadOnly;
        m_remoteHost = remoteHost;
        m_remotePort = remotePort;

        // ICE negotiation succeeded
        if (m_iceControlling)
            qDebug() << "STUN(" << m_component << ") ICE-CONTROLLING negotiation finished" << remoteHost << remotePort;
    } else if (message.type == BindingIndication) {
        // ICE negotiation succeded
        if (!m_iceControlling)
            qDebug() << "STUN(" << m_component << ") ICE-CONTROLLED negotiation finished" << remoteHost << remotePort;
    }
}

/// Sends a data packet to the remote party.
///
/// \param datagram

qint64 QXmppStunSocket::writeDatagram(const QByteArray &datagram)
{
    return m_socket->writeDatagram(datagram, m_remoteHost, m_remotePort);
}

