/*
 * Copyright (C) 2008-2011 The QXmpp developers
 *
 * Author:
 *  Jeremy Lainé
 *
 * Source:
 *  http://code.google.com/p/qxmpp
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

#define QXMPP_DEBUG_STUN

#include <QCryptographicHash>
#include <QHostInfo>
#include <QNetworkInterface>
#include <QUdpSocket>
#include <QTimer>

#include "QXmppStun.h"
#include "QXmppUtils.h"

#define ID_SIZE 12

static const quint32 STUN_MAGIC = 0x2112A442;
static const quint16 STUN_HEADER = 20;
static const quint8 STUN_IPV4 = 0x01;
static const quint8 STUN_IPV6 = 0x02;

enum AttributeType {
    MappedAddress    = 0x0001, // RFC5389
    ChangeRequest    = 0x0003, // RFC5389
    SourceAddress    = 0x0004, // RFC5389
    ChangedAddress   = 0x0005, // RFC5389
    Username         = 0x0006, // RFC5389
    MessageIntegrity = 0x0008, // RFC5389
    ErrorCode        = 0x0009, // RFC5389
    ChannelNumber    = 0x000c, // RFC5766
    Lifetime         = 0x000d, // RFC5766
    XorPeerAddress   = 0x0012, // RFC5766
    XorMappedAddress = 0x0020, // RFC5389
    Priority         = 0x0024, // RFC5245
    UseCandidate     = 0x0025, // RFC5245
    Software         = 0x8022, // RFC5389
    Fingerprint      = 0x8028, // RFC5389
    IceControlled    = 0x8029, // RFC5245
    IceControlling   = 0x802a, // RFC5245
    OtherAddress     = 0x802c, // RFC5780
};

// FIXME : we need to set local preference to discriminate between
// multiple IP addresses
static int candidatePriority(const QXmppJingleCandidate &candidate, int localPref = 65535)
{
    int typePref;
    switch (candidate.type())
    {
    case QXmppJingleCandidate::HostType:
        typePref = 126;
        break;
    case QXmppJingleCandidate::PeerReflexiveType:
        typePref = 110;
        break;
    case QXmppJingleCandidate::ServerReflexiveType:
        typePref = 100;
        break;
    default:
        typePref = 0;
    }

    return (1 << 24) * typePref + \
           (1 << 8) * localPref + \
           (256 - candidate.component());
}

static bool isIPv6LinkLocalAddress(const QHostAddress &addr)
{
    if (addr.protocol() != QAbstractSocket::IPv6Protocol)
        return false;
    Q_IPV6ADDR ipv6addr = addr.toIPv6Address();
    return (((ipv6addr[0] << 8) + ipv6addr[1]) & 0xffc0) == 0xfe80;
}

static bool decodeAddress(QDataStream &stream, quint16 a_length, QHostAddress &address, quint16 &port, const QByteArray &xorId = QByteArray())
{
    if (a_length < 4)
        return false;
    quint8 reserved, protocol;
    quint16 rawPort;
    stream >> reserved;
    stream >> protocol;
    stream >> rawPort;
    if (xorId.isEmpty())
        port = rawPort;
    else
        port = rawPort ^ (STUN_MAGIC >> 16);
    if (protocol == STUN_IPV4)
    {
        if (a_length != 8)
            return false;
        quint32 addr;
        stream >> addr;
        if (xorId.isEmpty())
            address = QHostAddress(addr);
        else
            address = QHostAddress(addr ^ STUN_MAGIC);
    } else if (protocol == STUN_IPV6) {
        if (a_length != 20)
            return false;
        Q_IPV6ADDR addr;
        stream.readRawData((char*)&addr, sizeof(addr));
        if (!xorId.isEmpty())
        {
            QByteArray xpad;
            QDataStream(&xpad, QIODevice::WriteOnly) << STUN_MAGIC;
            xpad += xorId;
            for (int i = 0; i < 16; i++)
                addr[i] ^= xpad[i];
        }
        address = QHostAddress(addr);
    } else {
        return false;
    }
    return true;
}

static void encodeAddress(QDataStream &stream, quint16 type, const QHostAddress &address, quint16 port, const QByteArray &xorId = QByteArray())
{
    const quint8 reserved = 0;
    if (address.protocol() == QAbstractSocket::IPv4Protocol)
    {
        stream << type;
        stream << quint16(8);
        stream << reserved;
        stream << quint8(STUN_IPV4);
        quint32 addr = address.toIPv4Address();
        if (!xorId.isEmpty())
        {
            port ^= (STUN_MAGIC >> 16);
            addr ^= STUN_MAGIC;
        }
        stream << port;
        stream << addr;
    } else if (address.protocol() == QAbstractSocket::IPv6Protocol) {
        stream << type;
        stream << quint16(20);
        stream << reserved;
        stream << quint8(STUN_IPV6);
        Q_IPV6ADDR addr = address.toIPv6Address();
        if (!xorId.isEmpty())
        {
            port ^= (STUN_MAGIC >> 16);
            QByteArray xpad;
            QDataStream(&xpad, QIODevice::WriteOnly) << STUN_MAGIC;
            xpad += xorId;
            for (int i = 0; i < 16; i++)
                addr[i] ^= xpad[i];
        }
        stream << port;
        stream.writeRawData((char*)&addr, sizeof(addr));
    } else {
        qWarning("Cannot write STUN attribute for unknown IP version");
    }
}

static void addAddress(QDataStream &stream, quint16 type, const QHostAddress &host, quint16 port, const QByteArray &xorId = QByteArray())
{
    if (port && !host.isNull() &&
        (host.protocol() == QAbstractSocket::IPv4Protocol ||
         host.protocol() == QAbstractSocket::IPv6Protocol))
    {
        encodeAddress(stream, type, host, port, xorId);
    }
}

static void encodeString(QDataStream &stream, quint16 type, const QString &string)
{
    const QByteArray utf8string = string.toUtf8();
    stream << type;
    stream << quint16(utf8string.size());
    stream.writeRawData(utf8string.data(), utf8string.size());
    if (utf8string.size() % 4)
    {
        const QByteArray padding(4 - (utf8string.size() % 4), 0);
        stream.writeRawData(padding.data(), padding.size());
    }
}

static void setBodyLength(QByteArray &buffer, qint16 length)
{
    QDataStream stream(&buffer, QIODevice::WriteOnly);
    stream.device()->seek(2);
    stream << length;
}

/// Constructs a new QXmppStunMessage.

QXmppStunMessage::QXmppStunMessage()
    : errorCode(0),
    changedPort(0),
    mappedPort(0),
    otherPort(0),
    sourcePort(0),
    xorMappedPort(0),
    useCandidate(false),
    m_cookie(STUN_MAGIC),
    m_type(0),
    m_changeRequest(0),
    m_priority(0)
{
    m_id = QByteArray(ID_SIZE, 0);
}

quint32 QXmppStunMessage::cookie() const
{
    return m_cookie;
}

void QXmppStunMessage::setCookie(quint32 cookie)
{
    m_cookie = cookie;
}

QByteArray QXmppStunMessage::id() const
{
    return m_id;
}

void QXmppStunMessage::setId(const QByteArray &id)
{
    Q_ASSERT(id.size() == ID_SIZE);
    m_id = id;
}

quint16 QXmppStunMessage::type() const
{
    return m_type;
}

void QXmppStunMessage::setType(quint16 type)
{
    m_type = type;
}

/// Returns the CHANGE-REQUEST attribute, indicating whether to change
/// the IP and / or port from which the response is sent.

quint32 QXmppStunMessage::changeRequest() const
{
    return m_changeRequest;
}

/// Sets the CHANGE-REQUEST attribute, indicating whether to change
/// the IP and / or port from which the response is sent.
///
/// \param changeRequest

void QXmppStunMessage::setChangeRequest(quint32 changeRequest)
{
    m_changeRequest = changeRequest;
    m_attributes << ChangeRequest;
}

/// Returns the SOFTWARE attribute, containing a textual description of the
/// software being used.

QString QXmppStunMessage::software() const
{
    return m_software;
}

/// Sets the SOFTWARE attribute, containing a textual description of the
/// software being used.
///
/// \param software

void QXmppStunMessage::setSoftware(const QString &software)
{
    m_software = software;
    m_attributes << Software;
}

/// Returns the PRIORITY attribute, the priority that would be assigned to
/// a peer reflexive candidate discovered during the ICE check.

quint32 QXmppStunMessage::priority() const
{
    return m_priority;
}

/// Sets the PRIORITY attribute, the priority that would be assigned to
/// a peer reflexive candidate discovered during the ICE check.
///
/// \param priority

void QXmppStunMessage::setPriority(quint32 priority)
{
    m_priority = priority;
    m_attributes << Priority;
}

/// Decodes a QXmppStunMessage and checks its integrity using the given
/// password.
///
/// \param buffer
/// \param password
/// \param errors

bool QXmppStunMessage::decode(const QByteArray &buffer, const QString &password, QStringList *errors)
{
    QStringList silent;
    if (!errors)
        errors = &silent;

    if (buffer.size() < STUN_HEADER)
    {
        *errors << QLatin1String("Received a truncated STUN packet");
        return false;
    }

    // parse STUN header
    QDataStream stream(buffer);
    quint16 length;
    stream >> m_type;
    stream >> length;
    stream >> m_cookie;
    stream.readRawData(m_id.data(), m_id.size());

    if (length != buffer.size() - STUN_HEADER)
    {
        *errors << QLatin1String("Received an invalid STUN packet");
        return false;
    }

    // parse STUN attributes
    int done = 0;
    bool after_integrity = false;
    while (done < length)
    {
        quint16 a_type, a_length;
        stream >> a_type;
        stream >> a_length;
        const int pad_length = 4 * ((a_length + 3) / 4) - a_length;

        // only FINGERPRINT is allowed after MESSAGE-INTEGRITY
        if (after_integrity && a_type != Fingerprint)
        {
            *errors << QString("Skipping attribute %1 after MESSAGE-INTEGRITY").arg(QString::number(a_type));
            stream.skipRawData(a_length + pad_length);
            done += 4 + a_length + pad_length;
            continue;
        }

        if (a_type == Priority)
        {
            // PRIORITY
            if (a_length != sizeof(m_priority))
                return false;
            stream >> m_priority;
            m_attributes << Priority;

        } else if (a_type == ErrorCode) {

            // ERROR-CODE
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

            // USE-CANDIDATE
            if (a_length != 0)
                return false;
            useCandidate = true;

        } else if (a_type == Software) {

            // SOFTWARE
            QByteArray utf8Software(a_length, 0);
            stream.readRawData(utf8Software.data(), utf8Software.size());
            m_software = QString::fromUtf8(utf8Software);
            m_attributes << Software;

        } else if (a_type == MappedAddress) {

            // MAPPED-ADDRESS
            if (!decodeAddress(stream, a_length, mappedHost, mappedPort))
            {
                *errors << QLatin1String("Bad MAPPED-ADDRESS");
                return false;
            }

        } else if (a_type == ChangeRequest) {

            // CHANGE-REQUEST
            if (a_length != sizeof(m_changeRequest))
                return false;
            stream >> m_changeRequest;
            m_attributes << ChangeRequest;

        } else if (a_type == SourceAddress) {

            // SOURCE-ADDRESS
            if (!decodeAddress(stream, a_length, sourceHost, sourcePort))
            {
                *errors << QLatin1String("Bad SOURCE-ADDRESS");
                return false;
            }

        } else if (a_type == ChangedAddress) {

            // CHANGED-ADDRESS
            if (!decodeAddress(stream, a_length, changedHost, changedPort))
            {
                *errors << QLatin1String("Bad CHANGED-ADDRESS");
                return false;
            }

        } else if (a_type == OtherAddress) {

            // OTHER-ADDRESS
            if (!decodeAddress(stream, a_length, otherHost, otherPort))
            {
                *errors << QLatin1String("Bad OTHER-ADDRESS");
                return false;
            }

        } else if (a_type == XorMappedAddress) {

            // XOR-MAPPED-ADDRESS
            if (!decodeAddress(stream, a_length, xorMappedHost, xorMappedPort, m_id))
            {
                *errors << QLatin1String("Bad XOR-MAPPED-ADDRESS");
                return false;
            }

        } else if (a_type == MessageIntegrity) {

            // MESSAGE-INTEGRITY
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
                    *errors << QLatin1String("Bad message integrity");
                    return false;
                }
            }

            // from here onwards, only FINGERPRINT is allowed
            after_integrity = true;

        } else if (a_type == Fingerprint) {

            // FINGERPRINT
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
                *errors << QLatin1String("Bad fingerprint");
                return false;
            }

            // stop parsing, no more attributes are allowed
            return true;

        } else if (a_type == IceControlling) {

            /// ICE-CONTROLLING
            if (a_length != 8)
                return false;
            iceControlling.resize(8);
            stream.readRawData(iceControlling.data(), iceControlling.size());

         } else if (a_type == IceControlled) {

            /// ICE-CONTROLLED
            if (a_length != 8)
                return false;
            iceControlled.resize(8);
            stream.readRawData(iceControlled.data(), iceControlled.size());

        } else if (a_type == Username) {

            // USERNAME
            QByteArray utf8Username(a_length, 0);
            stream.readRawData(utf8Username.data(), utf8Username.size());
            username = QString::fromUtf8(utf8Username);

        } else {

            // Unknown attribute
            stream.skipRawData(a_length);
            *errors << QString("Skipping unknown attribute %1").arg(QString::number(a_type));

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
/// \param addFingerprint

QByteArray QXmppStunMessage::encode(const QString &password, bool addFingerprint) const
{
    QByteArray buffer;
    QDataStream stream(&buffer, QIODevice::WriteOnly);

    // encode STUN header
    quint16 length = 0;
    stream << m_type;
    stream << length;
    stream << m_cookie;
    stream.writeRawData(m_id.data(), m_id.size());

    // MAPPED-ADDRESS
    addAddress(stream, MappedAddress, mappedHost, mappedPort);

    // CHANGE-REQUEST
    if (m_attributes.contains(ChangeRequest)) {
        stream << quint16(ChangeRequest);
        stream << quint16(sizeof(m_changeRequest));
        stream << m_changeRequest;
    }

    // SOURCE-ADDRESS
    addAddress(stream, SourceAddress, sourceHost, sourcePort);

    // CHANGED-ADDRESS
    addAddress(stream, ChangedAddress, changedHost, changedPort);

    // OTHER-ADDRESS
    addAddress(stream, OtherAddress, otherHost, otherPort);

    // XOR-MAPPED-ADDRESS
    addAddress(stream, XorMappedAddress, xorMappedHost, xorMappedPort, m_id);

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
    if (m_attributes.contains(Priority))
    {
        stream << quint16(Priority);
        stream << quint16(sizeof(m_priority));
        stream << m_priority;
    }

    // USE-CANDIDATE
    if (useCandidate)
    {
        stream << quint16(UseCandidate);
        stream << quint16(0);
    }

    // SOFTWARE
    if (m_attributes.contains(Software))
        encodeString(stream, Software, m_software);

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
        encodeString(stream, Username, username);

    // set body length
    setBodyLength(buffer, buffer.size() - STUN_HEADER);

    // MESSAGE-INTEGRITY
    if (!password.isEmpty())
    {
        const QByteArray key = password.toUtf8();
        setBodyLength(buffer, buffer.size() - STUN_HEADER + 24);
        QByteArray integrity = generateHmacSha1(key, buffer);
        stream << quint16(MessageIntegrity);
        stream << quint16(integrity.size());
        stream.writeRawData(integrity.data(), integrity.size());
    }

    // FINGERPRINT
    if (addFingerprint)
    {
        setBodyLength(buffer, buffer.size() - STUN_HEADER + 8);
        quint32 fingerprint = generateCrc32(buffer) ^ 0x5354554eL;
        stream << quint16(Fingerprint);
        stream << quint16(sizeof(fingerprint));
        stream << fingerprint;
    }

    return buffer;
}

/// If the given packet looks like a STUN message, returns the message
/// type, otherwise returns 0.
///
/// \param buffer
/// \param cookie
/// \param id

quint16 QXmppStunMessage::peekType(const QByteArray &buffer, quint32 &cookie, QByteArray &id)
{
    if (buffer.size() < STUN_HEADER)
        return 0;

    // parse STUN header
    QDataStream stream(buffer);
    quint16 type;
    quint16 length;
    stream >> type;
    stream >> length;
    stream >> cookie;

    if (length != buffer.size() - STUN_HEADER)
        return 0;

    id.resize(ID_SIZE);
    stream.readRawData(id.data(), id.size());
    return type;
}
 
QString QXmppStunMessage::toString() const
{
    QStringList dumpLines;
    QString typeName;
    switch (m_type & 0x000f)
    {
        case Binding: typeName = "Binding"; break;
        case SharedSecret: typeName = "Shared Secret"; break;
        case Allocate: typeName = "Allocate"; break;
        default: typeName = "Unknown"; break;
    }
    switch (m_type & 0x0ff0)
    {
        case Request: typeName += " Request"; break;
        case Indication: typeName += " Indication"; break;
        case Response: typeName += " Response"; break;
        case Error: typeName += " Error"; break;
        default: break;
    }
    dumpLines << QString(" type %1 (%2)")
        .arg(typeName)
        .arg(QString::number(m_type));
    dumpLines << QString(" id %1").arg(QString::fromAscii(m_id.toHex()));

    // attributes
    if (!username.isEmpty())
        dumpLines << QString(" * USERNAME %1").arg(username);
    if (errorCode)
        dumpLines << QString(" * ERROR-CODE %1 %2")
            .arg(QString::number(errorCode), errorPhrase);
    if (m_attributes.contains(Software))
        dumpLines << QString(" * SOFTWARE %1").arg(m_software);
    if (mappedPort)
        dumpLines << QString(" * MAPPED-ADDRESS %1 %2")
            .arg(mappedHost.toString(), QString::number(mappedPort));
    if (m_attributes.contains(ChangeRequest))
        dumpLines << QString(" * CHANGE-REQUEST %1")
            .arg(QString::number(m_changeRequest));
    if (sourcePort)
        dumpLines << QString(" * SOURCE-ADDRESS %1 %2")
            .arg(sourceHost.toString(), QString::number(sourcePort));
    if (changedPort)
        dumpLines << QString(" * CHANGED-ADDRESS %1 %2")
            .arg(changedHost.toString(), QString::number(changedPort));
    if (otherPort)
        dumpLines << QString(" * OTHER-ADDRESS %1 %2")
            .arg(otherHost.toString(), QString::number(otherPort));
    if (xorMappedPort)
        dumpLines << QString(" * XOR-MAPPED-ADDRESS %1 %2")
            .arg(xorMappedHost.toString(), QString::number(xorMappedPort));
    if (m_attributes.contains(Priority))
        dumpLines << QString(" * PRIORITY %1").arg(QString::number(m_priority));
    if (!iceControlling.isEmpty())
        dumpLines << QString(" * ICE-CONTROLLING %1")
            .arg(QString::fromAscii(iceControlling.toHex()));
    if (!iceControlled.isEmpty())
        dumpLines << QString(" * ICE-CONTROLLED %1")
            .arg(QString::fromAscii(iceControlled.toHex()));

    return dumpLines.join("\n");
}

QXmppIceComponent::Pair::Pair()
    : checked(QIODevice::NotOpen),
    socket(0)
{
    // FIXME : calculate priority
    priority = 1862270975;
    transaction = generateRandomBytes(ID_SIZE);
}

QString QXmppIceComponent::Pair::toString() const
{
    QString str = QString("%1 %2").arg(remote.host().toString(), QString::number(remote.port()));
    if (socket)
        str += QString(" (local %1 %2)").arg(socket->localAddress().toString(), QString::number(socket->localPort()));
    if (!reflexive.host().isNull() && reflexive.port())
        str += QString(" (reflexive %1 %2)").arg(reflexive.host().toString(), QString::number(reflexive.port()));
    return str;
}

/// Constructs a new QXmppIceComponent.
///
/// \param controlling
/// \param parent

QXmppIceComponent::QXmppIceComponent(bool controlling, QObject *parent)
    : QXmppLoggable(parent),
    m_activePair(0),
    m_fallbackPair(0),
    m_iceControlling(controlling),
    m_stunPort(0),
    m_stunTries(0)
{
    m_localUser = generateStanzaHash(4);
    m_localPassword = generateStanzaHash(22);

    m_timer = new QTimer(this);
    m_timer->setInterval(500);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(checkCandidates())); 

    m_stunTimer = new QTimer(this);
    m_stunTimer->setInterval(500);
    connect(m_stunTimer, SIGNAL(timeout()), this, SLOT(checkStun()));
}

/// Destroys the QXmppIceComponent.

QXmppIceComponent::~QXmppIceComponent()
{
    foreach (Pair *pair, m_pairs)
        delete pair;
}

/// Returns the component id for the current socket, e.g. 1 for RTP
/// and 2 for RTCP.

int QXmppIceComponent::component() const
{
    return m_component;
}

/// Sets the component id for the current socket, e.g. 1 for RTP
/// and 2 for RTCP.
///
/// \param component

void QXmppIceComponent::setComponent(int component)
{
    m_component = component;
    setObjectName(QString("STUN(%1)").arg(QString::number(m_component)));
}

void QXmppIceComponent::checkCandidates()
{
    debug("Checking remote candidates");
    foreach (Pair *pair, m_pairs)
    {
        if (m_remoteUser.isEmpty())
            continue;

        // send a binding request
        QXmppStunMessage message;
        message.setId(pair->transaction);
        message.setType(QXmppStunMessage::Binding | QXmppStunMessage::Request);
        message.setPriority(pair->priority);
        message.username = QString("%1:%2").arg(m_remoteUser, m_localUser);
        if (m_iceControlling)
        {
            message.iceControlling = QByteArray(8, 0);
            message.useCandidate = true;
        } else {
            message.iceControlled = QByteArray(8, 0);
        }
        writeStun(message, pair);
    }

}

void QXmppIceComponent::checkStun()
{
    if (m_stunHost.isNull() || !m_stunPort || m_stunTries > 10) {
        m_stunTimer->stop();
        return;
    }

    // Send a request to STUN server to determine server-reflexive candidate
    foreach (QUdpSocket *socket, m_sockets)
    {
        QXmppStunMessage msg;
        msg.setType(QXmppStunMessage::Binding | QXmppStunMessage::Request);
        msg.setId(m_stunId);
#ifdef QXMPP_DEBUG_STUN
        logSent(QString("STUN packet to %1 %2\n%3").arg(m_stunHost.toString(),
                QString::number(m_stunPort), msg.toString()));
#endif
        socket->writeDatagram(msg.encode(), m_stunHost, m_stunPort);
    }
    m_stunTries++;
}

/// Stops ICE connectivity checks and closes the underlying sockets.

void QXmppIceComponent::close()
{
    foreach (QUdpSocket *socket, m_sockets)
        socket->close();
    m_timer->stop();
    m_stunTimer->stop();
}

/// Starts ICE connectivity checks.

void QXmppIceComponent::connectToHost()
{
    if (m_activePair)
        return;

    checkCandidates();
    m_timer->start();
}

/// Returns true if ICE negotiation completed, false otherwise.

bool QXmppIceComponent::isConnected() const
{
    return m_activePair != 0;
}

/// Returns the list of local candidates.

QList<QXmppJingleCandidate> QXmppIceComponent::localCandidates() const
{
    return m_localCandidates;
}

/// Sets the local user fragment.
///
/// \param user

void QXmppIceComponent::setLocalUser(const QString &user)
{
    m_localUser = user;
}

/// Sets the local password.
///
/// \param password

void QXmppIceComponent::setLocalPassword(const QString &password)
{
    m_localPassword = password;
}

/// Adds a remote STUN candidate.

bool QXmppIceComponent::addRemoteCandidate(const QXmppJingleCandidate &candidate)
{
    if (candidate.component() != m_component ||
        (candidate.type() != QXmppJingleCandidate::HostType &&
         candidate.type() != QXmppJingleCandidate::ServerReflexiveType) ||
        candidate.protocol() != "udp" ||
        (candidate.host().protocol() != QAbstractSocket::IPv4Protocol &&
        candidate.host().protocol() != QAbstractSocket::IPv6Protocol))
        return false;

    foreach (Pair *pair, m_pairs)
        if (pair->remote.host() == candidate.host() &&
            pair->remote.port() == candidate.port())
            return false;

    foreach (QUdpSocket *socket, m_sockets)
    {
        // do not pair IPv4 with IPv6 or global with link-local addresses
        if (socket->localAddress().protocol() != candidate.host().protocol() ||
            isIPv6LinkLocalAddress(socket->localAddress()) != isIPv6LinkLocalAddress(candidate.host()))
            continue;

        Pair *pair = new Pair;
        pair->remote = candidate;
        if (isIPv6LinkLocalAddress(pair->remote.host()))
        {
            QHostAddress remoteHost = pair->remote.host();
            remoteHost.setScopeId(socket->localAddress().scopeId());
            pair->remote.setHost(remoteHost);
        }
        pair->socket = socket;
        m_pairs << pair;

        if (!m_fallbackPair)
            m_fallbackPair = pair;
    }
    return true;
}

/// Adds a discovered STUN candidate.

QXmppIceComponent::Pair *QXmppIceComponent::addRemoteCandidate(QUdpSocket *socket, const QHostAddress &host, quint16 port)
{
    foreach (Pair *pair, m_pairs)
        if (pair->remote.host() == host &&
            pair->remote.port() == port &&
            pair->socket == socket)
            return pair;

    QXmppJingleCandidate candidate;
    candidate.setComponent(m_component);
    candidate.setHost(host);
    candidate.setId(generateStanzaHash(10));
    candidate.setPort(port);
    candidate.setProtocol("udp");
    candidate.setType(QXmppJingleCandidate::PeerReflexiveType);

    // FIXME : what priority?
    // candidate.setPriority(candidatePriority(candidate));

    Pair *pair = new Pair;
    pair->remote = candidate;
    pair->socket = socket;
    m_pairs << pair;

    debug(QString("Added candidate %1").arg(pair->toString()));
    return pair;
}

/// Sets the remote user fragment.
///
/// \param user

void QXmppIceComponent::setRemoteUser(const QString &user)
{
    m_remoteUser = user;
}

/// Sets the remote password.
///
/// \param password

void QXmppIceComponent::setRemotePassword(const QString &password)
{
    m_remotePassword = password;
}

/// Sets the list of sockets to use for this component.
///
/// \param sockets

void QXmppIceComponent::setSockets(QList<QUdpSocket*> sockets)
{
    // clear previous candidates and sockets
    m_localCandidates.clear();
    foreach (QUdpSocket *socket, m_sockets)
        delete socket;
    m_sockets.clear();

    // store candidates
    int foundation = 0;
    foreach (QUdpSocket *socket, sockets)
    {
        socket->setParent(this);
        connect(socket, SIGNAL(readyRead()), this, SLOT(readyRead()));

        QXmppJingleCandidate candidate;
        candidate.setComponent(m_component);
        candidate.setFoundation(foundation++);
        candidate.setHost(socket->localAddress());
        candidate.setId(generateStanzaHash(10));
        candidate.setPort(socket->localPort());
        candidate.setProtocol("udp");
        candidate.setType(QXmppJingleCandidate::HostType);
        candidate.setPriority(candidatePriority(candidate));

        m_sockets << socket;
        m_localCandidates << candidate;
    }

    // start STUN checks
    if (!m_stunHost.isNull() && m_stunPort) {
        m_stunTries = 0;
        checkStun();
        m_stunTimer->start();
    }
}

/// Sets the STUN server to use to determine server-reflexive addresses
/// and ports.
///
/// \param host The address of the STUN server.
/// \param port The port of the STUN server.

void QXmppIceComponent::setStunServer(const QHostAddress &host, quint16 port)
{
    m_stunHost = host;
    m_stunPort = port;
    m_stunId = generateRandomBytes(ID_SIZE);
}

void QXmppIceComponent::readyRead()
{
    QUdpSocket *socket = qobject_cast<QUdpSocket*>(sender());
    if (!socket)
        return;

    const qint64 size = socket->pendingDatagramSize();
    QHostAddress remoteHost;
    quint16 remotePort;
    QByteArray buffer(size, 0);
    socket->readDatagram(buffer.data(), buffer.size(), &remoteHost, &remotePort);

    // if this is not a STUN message, emit it
    quint32 messageCookie;
    QByteArray messageId;
    quint16 messageType = QXmppStunMessage::peekType(buffer, messageCookie, messageId);
    if (!messageType || messageCookie != STUN_MAGIC)
    {
        // use this as an opportunity to flag a potential pair
        foreach (Pair *pair, m_pairs) {
            if (pair->remote.host() == remoteHost &&
                pair->remote.port() == remotePort) {
                m_fallbackPair = pair;
                break;
            }
        }
        emit datagramReceived(buffer);
        return;
    }

    // determine password to use
    QString messagePassword;
    if (messageId != m_stunId)
    {
        messagePassword = (messageType & 0xFF00) ? m_remotePassword : m_localPassword;
        if (messagePassword.isEmpty())
            return;
    }

    // parse STUN message
    QXmppStunMessage message;
    QStringList errors;
    if (!message.decode(buffer, messagePassword, &errors))
    {
        foreach (const QString &error, errors)
            warning(error);
        return;
    }
#ifdef QXMPP_DEBUG_STUN
    logReceived(QString("STUN packet from %1 port %2\n%3").arg(remoteHost.toString(),
            QString::number(remotePort),
            message.toString()));
#endif

    // check how to handle message
    if (message.id() == m_stunId)
    {
        m_stunTimer->stop();

        // determine server-reflexive address
        QHostAddress reflexiveHost;
        quint16 reflexivePort = 0;
        if (!message.xorMappedHost.isNull() && message.xorMappedPort != 0)
        {
            reflexiveHost = message.xorMappedHost;
            reflexivePort = message.xorMappedPort;
        }
        else if (!message.mappedHost.isNull() && message.mappedPort != 0)
        {
            reflexiveHost = message.mappedHost;
            reflexivePort = message.mappedPort;
        } else {
            warning("STUN server did not provide a reflexive address");
            return;
        }

        // check whether this candidates is already known
        foreach (const QXmppJingleCandidate &candidate, m_localCandidates)
        {
            if (candidate.host() == reflexiveHost &&
                candidate.port() == reflexivePort &&
                candidate.type() == QXmppJingleCandidate::ServerReflexiveType)
                return;
        }

        // add the new local candidate
        debug(QString("Adding server-reflexive candidate %1 %2").arg(reflexiveHost.toString(), QString::number(reflexivePort)));
        QXmppJingleCandidate candidate;
        candidate.setComponent(m_component);
        candidate.setHost(reflexiveHost);
        candidate.setId(generateStanzaHash(10));
        candidate.setPort(reflexivePort);
        candidate.setProtocol("udp");
        candidate.setType(QXmppJingleCandidate::ServerReflexiveType);

        candidate.setPriority(candidatePriority(candidate));
        m_localCandidates << candidate;

        emit localCandidatesChanged();
        return;
    }

    // process message from peer
    Pair *pair = 0;
    if (message.type() == (QXmppStunMessage::Binding | QXmppStunMessage::Request))
    {
        // add remote candidate
        pair = addRemoteCandidate(socket, remoteHost, remotePort);

        // send a binding response
        QXmppStunMessage response;
        response.setId(message.id());
        response.setType(QXmppStunMessage::Binding | QXmppStunMessage::Response);
        response.username = message.username;
        response.xorMappedHost = pair->remote.host();
        response.xorMappedPort = pair->remote.port();
        writeStun(response, pair);

        // update state
        if (m_iceControlling || message.useCandidate)
        {
            debug(QString("ICE reverse check %1").arg(pair->toString()));
            pair->checked |= QIODevice::ReadOnly;
        }

        if (!m_iceControlling && !m_activePair && !m_remoteUser.isEmpty())
        {
            // send a triggered connectivity test
            QXmppStunMessage message;
            message.setId(pair->transaction);
            message.setType(QXmppStunMessage::Binding | QXmppStunMessage::Request);
            message.setPriority(pair->priority);
            message.username = QString("%1:%2").arg(m_remoteUser, m_localUser);
            message.iceControlled = QByteArray(8, 0);
            writeStun(message, pair);
        }

    } else if (message.type() == (QXmppStunMessage::Binding | QXmppStunMessage::Response)) {

        // find the pair for this transaction
        foreach (Pair *ptr, m_pairs)
        {
            if (ptr->transaction == message.id())
            {
                pair = ptr;
                break;
            }
        }
        if (!pair)
        {
            debug(QString("Unknown transaction %1").arg(QString::fromAscii(message.id().toHex())));
            return;
        }
        // store peer-reflexive address
        pair->reflexive.setHost(message.xorMappedHost);
        pair->reflexive.setPort(message.xorMappedPort);

        // FIXME : add the new remote candidate?
        //addRemoteCandidate(socket, remoteHost, remotePort);

#if 0
        // send a binding indication
        QXmppStunMessage indication;
        indication.setId(generateRandomBytes(ID_SIZE));
        indication.setType(BindingIndication);
        m_socket->writeStun(indication, pair);
#endif
        // outgoing media can flow
        debug(QString("ICE forward check %1").arg(pair->toString()));
        pair->checked |= QIODevice::WriteOnly;
    }

    // signal completion
    if (pair && pair->checked == QIODevice::ReadWrite)
    { 
        debug(QString("ICE completed %1").arg(pair->toString()));
        m_activePair = pair;
        m_timer->stop();
        emit connected();
    }
}

static QList<QUdpSocket*> reservePort(const QList<QHostAddress> &addresses, quint16 port, QObject *parent)
{
    QList<QUdpSocket*> sockets;
    foreach (const QHostAddress &address, addresses) {
        QUdpSocket *socket = new QUdpSocket(parent);
        sockets << socket;
        if (!socket->bind(address, port)) {
            for (int i = 0; i < sockets.size(); ++i)
                delete sockets[i];
            sockets.clear();
            break;
        }
    }
    return sockets;
}

/// Returns the list of local network addresses.

QList<QHostAddress> QXmppIceComponent::discoverAddresses()
{
    QList<QHostAddress> addresses;
    foreach (const QNetworkInterface &interface, QNetworkInterface::allInterfaces())
    {
        if (!(interface.flags() & QNetworkInterface::IsRunning) ||
            interface.flags() & QNetworkInterface::IsLoopBack)
            continue;

        foreach (const QNetworkAddressEntry &entry, interface.addressEntries())
        {
            if ((entry.ip().protocol() != QAbstractSocket::IPv4Protocol &&
                 entry.ip().protocol() != QAbstractSocket::IPv6Protocol) ||
                entry.netmask().isNull() ||
                entry.netmask() == QHostAddress::Broadcast)
                continue;

#ifdef Q_OS_MAC
            // FIXME: on Mac OS X, sending IPv6 UDP packets fails
            if (entry.ip().protocol() == QAbstractSocket::IPv6Protocol)
                continue;
#endif

            QHostAddress ip = entry.ip();
            if (isIPv6LinkLocalAddress(ip))
               ip.setScopeId(interface.name());
            addresses << ip;
        }
    }
    return addresses;
}

/// Tries to bind \a count UDP sockets on each of the given \a addresses.
///
/// The port numbers are chosen so that they are consecutive, starting at
/// an even port. This makes them suitable for RTP/RTCP sockets pairs.
///
/// \param addresses The network address on which to bind the sockets.
/// \param count     The number of ports to reserve.
/// \param parent    The parent object for the sockets.

QList<QUdpSocket*> QXmppIceComponent::reservePorts(const QList<QHostAddress> &addresses, int count, QObject *parent)
{
    QList<QUdpSocket*> sockets;
    if (addresses.isEmpty() || !count)
        return sockets;

    const int expectedSize = addresses.size() * count;
    quint16 port = 40000;
    while (sockets.size() != expectedSize) {
        // reserve first port (even number)
        if (port % 2)
            port++;
        QList<QUdpSocket*> socketChunk;
        while (socketChunk.isEmpty() && port <= 65536 - count) {
            socketChunk = reservePort(addresses, port, parent);
            if (socketChunk.isEmpty())
                port += 2;
        }
        if (socketChunk.isEmpty())
            return sockets;

        // reserve other ports
        sockets << socketChunk;
        for (int i = 1; i < count; ++i) {
            socketChunk = reservePort(addresses, ++port, parent);
            if (socketChunk.isEmpty())
                break;
            sockets << socketChunk;
        }

        // cleanup if we failed
        if (sockets.size() != expectedSize) {
            for (int i = 0; i < sockets.size(); ++i)
                delete sockets[i];
            sockets.clear();
        }
    }
    return sockets;
}

/// Sends a data packet to the remote party.
///
/// \param datagram

qint64 QXmppIceComponent::sendDatagram(const QByteArray &datagram)
{
    Pair *pair = m_activePair ? m_activePair : m_fallbackPair;
    if (!pair)
        return -1;
    return pair->socket->writeDatagram(datagram, pair->remote.host(), pair->remote.port());
}

/// Sends a STUN packet to the remote party.

qint64 QXmppIceComponent::writeStun(const QXmppStunMessage &message, QXmppIceComponent::Pair *pair)
{
    const QString messagePassword = (message.type() & 0xFF00) ? m_localPassword : m_remotePassword;
    qint64 ret = pair->socket->writeDatagram(message.encode(messagePassword), pair->remote.host(), pair->remote.port());
#ifdef QXMPP_DEBUG_STUN
    logSent(QString("Sent to %1\n%2").arg(pair->toString(), message.toString()));
#endif
    return ret;
}

/// Constructs a new ICE connection.
///
/// \param controlling
/// \param parent

QXmppIceConnection::QXmppIceConnection(bool controlling, QObject *parent)
    : QXmppLoggable(parent),
    m_controlling(controlling),
    m_stunPort(0)
{
    bool check;

    m_localUser = generateStanzaHash(4);
    m_localPassword = generateStanzaHash(22);

    // timer to limit connection time to 30 seconds
    m_connectTimer = new QTimer(this);
    m_connectTimer->setInterval(30000);
    m_connectTimer->setSingleShot(true);
    check = connect(m_connectTimer, SIGNAL(timeout()),
                    this, SLOT(slotTimeout()));
    Q_ASSERT(check);
    Q_UNUSED(check);
}

/// Returns the given component of this ICE connection.
///
/// \param component

QXmppIceComponent *QXmppIceConnection::component(int component)
{
    return m_components.value(component);
}

/// Adds a component to this ICE connection, for instance 1 for RTP
/// or 2 for RTCP.
///
/// \param component

void QXmppIceConnection::addComponent(int component)
{
    if (m_components.contains(component))
    {
        warning(QString("Already have component %1").arg(QString::number(component)));
        return;
    }

    QXmppIceComponent *socket = new QXmppIceComponent(m_controlling, this);
    socket->setComponent(component);
    socket->setLocalUser(m_localUser);
    socket->setLocalPassword(m_localPassword);
    socket->setStunServer(m_stunHost, m_stunPort);

    bool check = connect(socket, SIGNAL(localCandidatesChanged()),
        this, SIGNAL(localCandidatesChanged()));
    Q_ASSERT(check);

    check = connect(socket, SIGNAL(connected()),
        this, SLOT(slotConnected()));
    Q_ASSERT(check);

    m_components[component] = socket;
}

/// Adds a candidate for one of the remote components.
///
/// \param candidate

void QXmppIceConnection::addRemoteCandidate(const QXmppJingleCandidate &candidate)
{
    QXmppIceComponent *socket = m_components.value(candidate.component());
    if (!socket)
    {
        warning(QString("Not adding candidate for unknown component %1").arg(
                QString::number(candidate.component())));
        return;
    }
    socket->addRemoteCandidate(candidate);
}

/// Binds the local sockets to the specified addresses.
///
/// \param addresses The addresses on which to listen.

bool QXmppIceConnection::bind(const QList<QHostAddress> &addresses)
{
    // reserve ports
    QList<QUdpSocket*> sockets = QXmppIceComponent::reservePorts(addresses, m_components.size());
    if (sockets.isEmpty())
        return false;

    // assign sockets
    QList<int> keys = m_components.keys();
    qSort(keys);
    int s = 0;
    foreach (int k, keys) {
        m_components[k]->setSockets(sockets.mid(s, addresses.size()));
        s += addresses.size();
    }

    return true;
}

/// Closes the ICE connection.

void QXmppIceConnection::close()
{
    foreach (QXmppIceComponent *socket, m_components.values())
        socket->close();
}

/// Starts ICE connectivity checks.

void QXmppIceConnection::connectToHost()
{
    foreach (QXmppIceComponent *socket, m_components.values())
        socket->connectToHost();
    m_connectTimer->start();
}

/// Returns true if ICE negotiation completed, false otherwise.

bool QXmppIceConnection::isConnected() const
{
    foreach (QXmppIceComponent *socket, m_components.values())
        if (!socket->isConnected())
            return false;
    return true;
}

/// Returns the list of local HOST CANDIDATES candidates by iterating
/// over the available network interfaces.

QList<QXmppJingleCandidate> QXmppIceConnection::localCandidates() const
{
    QList<QXmppJingleCandidate> candidates;
    foreach (QXmppIceComponent *socket, m_components.values())
        candidates += socket->localCandidates();
    return candidates;
}

/// Returns the local user fragment.

QString QXmppIceConnection::localUser() const
{
    return m_localUser;
}

/// Returns the local password.

QString QXmppIceConnection::localPassword() const
{
    return m_localPassword;
}

/// Sets the remote user fragment.
///
/// \param user

void QXmppIceConnection::setRemoteUser(const QString &user)
{
    foreach (QXmppIceComponent *socket, m_components.values())
        socket->setRemoteUser(user);
}

/// Sets the remote password.
///
/// \param password

void QXmppIceConnection::setRemotePassword(const QString &password)
{
    foreach (QXmppIceComponent *socket, m_components.values())
        socket->setRemotePassword(password);
}

/// Sets the STUN server to use to determine server-reflexive addresses
/// and ports.
///
/// \param host The address of the STUN server.
/// \param port The port of the STUN server.

void QXmppIceConnection::setStunServer(const QHostAddress &host, quint16 port)
{
    m_stunHost = host;
    m_stunPort = port;
    foreach (QXmppIceComponent *socket, m_components.values())
        socket->setStunServer(host, port);
}

void QXmppIceConnection::slotConnected()
{
    foreach (QXmppIceComponent *socket, m_components.values())
        if (!socket->isConnected())
            return;
    m_connectTimer->stop();
    emit connected();
}

void QXmppIceConnection::slotTimeout()
{
    warning(QString("ICE negotiation timed out"));
    foreach (QXmppIceComponent *socket, m_components.values())
        socket->close();
    emit disconnected();
}

