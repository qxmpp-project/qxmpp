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

#define QXMPP_DEBUG_STUN

#include <QCryptographicHash>
#include <QDataStream>
#include <QHostInfo>
#include <QNetworkInterface>
#include <QUdpSocket>
#include <QTimer>

#include "QXmppStun_p.h"
#include "QXmppUtils.h"

#define STUN_ID_SIZE 12
#define STUN_RTO_INTERVAL 500
#define STUN_RTO_MAX      7

static const quint32 STUN_MAGIC = 0x2112A442;
static const quint16 STUN_HEADER = 20;
static const quint8 STUN_IPV4 = 0x01;
static const quint8 STUN_IPV6 = 0x02;

static const char* gathering_states[] = {
    "new",
    "gathering",
    "complete"
};

static const char* pair_states[] = {
    "frozen",
    "waiting",
    "in-progress",
    "succeeded",
    "failed"
};

enum AttributeType {
    MappedAddress    = 0x0001, // RFC5389
    ChangeRequest    = 0x0003, // RFC5389
    SourceAddress    = 0x0004, // RFC5389
    ChangedAddress   = 0x0005, // RFC5389
    Username         = 0x0006, // RFC5389
    MessageIntegrity = 0x0008, // RFC5389
    ErrorCode        = 0x0009, // RFC5389
    ChannelNumber    = 0x000c, // RFC5766 : TURN
    Lifetime         = 0x000d, // RFC5766 : TURN
    XorPeerAddress   = 0x0012, // RFC5766 : TURN
    DataAttr         = 0x0013, // RFC5766 : TURN
    Realm            = 0x0014, // RFC5389
    Nonce            = 0x0015, // RFC5389
    XorRelayedAddress= 0x0016, // RFC5766 : TURN
    EvenPort         = 0x0018, // RFC5766 : TURN
    RequestedTransport=0x0019, // RFC5766 : TURN
    XorMappedAddress = 0x0020, // RFC5389
    ReservationToken = 0x0022, // RFC5766 : TURN
    Priority         = 0x0024, // RFC5245
    UseCandidate     = 0x0025, // RFC5245
    Software         = 0x8022, // RFC5389
    Fingerprint      = 0x8028, // RFC5389
    IceControlled    = 0x8029, // RFC5245
    IceControlling   = 0x802a, // RFC5245
    OtherAddress     = 0x802c  // RFC5780
};

// FIXME : we need to set local preference to discriminate between
// multiple IP addresses
static quint32 candidatePriority(const QXmppJingleCandidate &candidate, int localPref = 65535)
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

static QString computeFoundation(QXmppJingleCandidate::Type type, const QString &protocol, const QHostAddress &baseAddress)
{
    QCryptographicHash hash(QCryptographicHash::Md5);
    hash.addData((QString::number(type) + protocol + baseAddress.toString()).toUtf8());
    return hash.result().toHex();
}

static bool isIPv6LinkLocalAddress(const QHostAddress &addr)
{
    if (addr.protocol() != QAbstractSocket::IPv6Protocol)
        return false;
    Q_IPV6ADDR ipv6addr = addr.toIPv6Address();
    return (((ipv6addr[0] << 8) + ipv6addr[1]) & 0xffc0) == 0xfe80;
}

static bool isLoopbackAddress(const QHostAddress &addr)
{
    return (addr.toIPv4Address() & 0xff000000) == 0x7f000000;
}

// Returns true if the two addresses are compatible.
static bool isCompatibleAddress(const QHostAddress &a1, const QHostAddress &a2)
{
    return a1.protocol() == a2.protocol() &&
           isIPv6LinkLocalAddress(a1) == isIPv6LinkLocalAddress(a2);
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
    xorPeerPort(0),
    xorRelayedPort(0),
    useCandidate(false),
    m_cookie(STUN_MAGIC),
    m_type(0),
    m_changeRequest(0),
    m_channelNumber(0),
    m_lifetime(0),
    m_priority(0)
{
    m_id = QByteArray(STUN_ID_SIZE, 0);
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
    Q_ASSERT(id.size() == STUN_ID_SIZE);
    m_id = id;
}

quint16 QXmppStunMessage::messageClass() const
{
    return m_type & 0x0110;
}

quint16 QXmppStunMessage::messageMethod() const
{
    return m_type & 0x3eef;
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

/// Returns the CHANNEL-NUMBER attribute.

quint16 QXmppStunMessage::channelNumber() const
{
    return m_channelNumber;
}

/// Sets the CHANNEL-NUMBER attribute.
///
/// \param channelNumber

void QXmppStunMessage::setChannelNumber(quint16 channelNumber)
{
    m_channelNumber = channelNumber;
    m_attributes << ChannelNumber;
}

/// Returns the DATA attribute.

QByteArray QXmppStunMessage::data() const
{
    return m_data;
}

/// Sets the DATA attribute.

void QXmppStunMessage::setData(const QByteArray &data)
{
    m_data = data;
    m_attributes << DataAttr;
}

/// Returns the LIFETIME attribute, indicating the duration in seconds for
/// which the server will maintain an allocation.

quint32 QXmppStunMessage::lifetime() const
{
    return m_lifetime;
}

/// Sets the LIFETIME attribute, indicating the duration in seconds for
/// which the server will maintain an allocation.
///
/// \param lifetime

void QXmppStunMessage::setLifetime(quint32 lifetime)
{
    m_lifetime = lifetime;
    m_attributes << Lifetime;
}

/// Returns the NONCE attribute.

QByteArray QXmppStunMessage::nonce() const
{
    return m_nonce;
}

/// Sets the NONCE attribute.
///
/// \param nonce

void QXmppStunMessage::setNonce(const QByteArray &nonce)
{
    m_nonce = nonce;
    m_attributes << Nonce;
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

/// Returns the REALM attribute.

QString QXmppStunMessage::realm() const
{
    return m_realm;
}

/// Sets the REALM attribute.
///
/// \param realm

void QXmppStunMessage::setRealm(const QString &realm)
{
    m_realm = realm;
    m_attributes << Realm;
}

/// Returns the REQUESTED-TRANSPORT attribute.

quint8 QXmppStunMessage::requestedTransport() const
{
    return m_requestedTransport;
}

/// Sets the REQUESTED-TRANSPORT attribute.
///
/// \param requestedTransport

void QXmppStunMessage::setRequestedTransport(quint8 requestedTransport)
{
    m_requestedTransport = requestedTransport;
    m_attributes << RequestedTransport;
}

/// Returns the RESERVATION-TOKEN attribute.

QByteArray QXmppStunMessage::reservationToken() const
{
    return m_reservationToken;
}

/// Sets the RESERVATION-TOKEN attribute.
///
/// \param reservationToken

void QXmppStunMessage::setReservationToken(const QByteArray &reservationToken)
{
    m_reservationToken = reservationToken;
    m_reservationToken.resize(8);
    m_attributes << ReservationToken;
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

/// Returns the USERNAME attribute, containing the username to use for
/// authentication.

QString QXmppStunMessage::username() const
{
    return m_username;
}

/// Sets the USERNAME attribute, containing the username to use for
/// authentication.
///
/// \param username

void QXmppStunMessage::setUsername(const QString &username)
{
    m_username = username;
    m_attributes << Username;
}

/// Decodes a QXmppStunMessage and checks its integrity using the given key.
///
/// \param buffer
/// \param key
/// \param errors

bool QXmppStunMessage::decode(const QByteArray &buffer, const QByteArray &key, QStringList *errors)
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

        } else if (a_type == ChannelNumber) {

            // CHANNEL-NUMBER
            if (a_length != 4)
                return false;
            stream >> m_channelNumber;
            stream.skipRawData(2);
            m_attributes << ChannelNumber;

        } else if (a_type == DataAttr) {

            // DATA
            m_data.resize(a_length);
            stream.readRawData(m_data.data(), m_data.size());
            m_attributes << DataAttr;

        } else if (a_type == Lifetime) {

            // LIFETIME
            if (a_length != sizeof(m_lifetime))
                return false;
            stream >> m_lifetime;
            m_attributes << Lifetime;

        } else if (a_type == Nonce) {

            // NONCE
            m_nonce.resize(a_length);
            stream.readRawData(m_nonce.data(), m_nonce.size());
            m_attributes << Nonce;

        } else if (a_type == Realm) {

            // REALM
            QByteArray utf8Realm(a_length, 0);
            stream.readRawData(utf8Realm.data(), utf8Realm.size());
            m_realm = QString::fromUtf8(utf8Realm);
            m_attributes << Realm;

        } else if (a_type == RequestedTransport) {

            // REQUESTED-TRANSPORT
            if (a_length != 4)
                return false;
            stream >> m_requestedTransport;
            stream.skipRawData(3);
            m_attributes << RequestedTransport;

        } else if (a_type == ReservationToken) {

            // RESERVATION-TOKEN
            if (a_length != 8)
                return false;
            m_reservationToken.resize(a_length);
            stream.readRawData(m_reservationToken.data(), m_reservationToken.size());
            m_attributes << ReservationToken;

        } else if (a_type == Software) {

            // SOFTWARE
            QByteArray utf8Software(a_length, 0);
            stream.readRawData(utf8Software.data(), utf8Software.size());
            m_software = QString::fromUtf8(utf8Software);
            m_attributes << Software;

        } else if (a_type == Username) {

            // USERNAME
            QByteArray utf8Username(a_length, 0);
            stream.readRawData(utf8Username.data(), utf8Username.size());
            m_username = QString::fromUtf8(utf8Username);
            m_attributes << Username;

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

        } else if (a_type == XorPeerAddress) {

            // XOR-PEER-ADDRESS
            if (!decodeAddress(stream, a_length, xorPeerHost, xorPeerPort, m_id))
            {
                *errors << QLatin1String("Bad XOR-PEER-ADDRESS");
                return false;
            }

        } else if (a_type == XorRelayedAddress) {

            // XOR-RELAYED-ADDRESS
            if (!decodeAddress(stream, a_length, xorRelayedHost, xorRelayedPort, m_id))
            {
                *errors << QLatin1String("Bad XOR-RELAYED-ADDRESS");
                return false;
            }

        } else if (a_type == MessageIntegrity) {

            // MESSAGE-INTEGRITY
            if (a_length != 20)
                return false;
            QByteArray integrity(20, 0);
            stream.readRawData(integrity.data(), integrity.size());

            // check HMAC-SHA1
            if (!key.isEmpty())
            {
                QByteArray copy = buffer.left(STUN_HEADER + done);
                setBodyLength(copy, done + 24);
                if (integrity != QXmppUtils::generateHmacSha1(key, copy))
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
            const quint32 expected = QXmppUtils::generateCrc32(copy) ^ 0x5354554eL;
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
            iceControlling.resize(a_length);
            stream.readRawData(iceControlling.data(), iceControlling.size());

         } else if (a_type == IceControlled) {

            /// ICE-CONTROLLED
            if (a_length != 8)
                return false;
            iceControlled.resize(a_length);
            stream.readRawData(iceControlled.data(), iceControlled.size());

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
/// message integrity attribute using the given key.
///
/// \param key
/// \param addFingerprint

QByteArray QXmppStunMessage::encode(const QByteArray &key, bool addFingerprint) const
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

    // XOR-PEER-ADDRESS
    addAddress(stream, XorPeerAddress, xorPeerHost, xorPeerPort, m_id);

    // XOR-RELAYED-ADDRESS
    addAddress(stream, XorRelayedAddress, xorRelayedHost, xorRelayedPort, m_id);

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

    // CHANNEL-NUMBER
    if (m_attributes.contains(ChannelNumber)) {
        stream << quint16(ChannelNumber);
        stream << quint16(4);
        stream << m_channelNumber;
        stream << quint16(0);
    }

    // DATA
    if (m_attributes.contains(DataAttr)) {
        stream << quint16(DataAttr);
        stream << quint16(m_data.size());
        stream.writeRawData(m_data.data(), m_data.size());
        if (m_data.size() % 4) {
            const QByteArray padding(4 - (m_data.size() %  4), 0);
            stream.writeRawData(padding.data(), padding.size());
        }
    }

    // LIFETIME
    if (m_attributes.contains(Lifetime)) {
        stream << quint16(Lifetime);
        stream << quint16(sizeof(m_lifetime));
        stream << m_lifetime;
    }

    // NONCE
    if (m_attributes.contains(Nonce)) {
        stream << quint16(Nonce);
        stream << quint16(m_nonce.size());
        stream.writeRawData(m_nonce.data(), m_nonce.size());
    }

    // REALM
    if (m_attributes.contains(Realm))
        encodeString(stream, Realm, m_realm);

    // REQUESTED-TRANSPORT
    if (m_attributes.contains(RequestedTransport)) {
        const QByteArray reserved(3, 0);
        stream << quint16(RequestedTransport);
        stream << quint16(4);
        stream << m_requestedTransport;
        stream.writeRawData(reserved.data(), reserved.size());
    }

    // RESERVATION-TOKEN
    if (m_attributes.contains(ReservationToken)) {
        stream << quint16(ReservationToken);
        stream << quint16(m_reservationToken.size());
        stream.writeRawData(m_reservationToken.data(), m_reservationToken.size());
    }

    // SOFTWARE
    if (m_attributes.contains(Software))
        encodeString(stream, Software, m_software);

    // USERNAME
    if (m_attributes.contains(Username))
        encodeString(stream, Username, m_username);

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

    // set body length
    setBodyLength(buffer, buffer.size() - STUN_HEADER);

    // MESSAGE-INTEGRITY
    if (!key.isEmpty())
    {
        setBodyLength(buffer, buffer.size() - STUN_HEADER + 24);
        QByteArray integrity = QXmppUtils::generateHmacSha1(key, buffer);
        stream << quint16(MessageIntegrity);
        stream << quint16(integrity.size());
        stream.writeRawData(integrity.data(), integrity.size());
    }

    // FINGERPRINT
    if (addFingerprint)
    {
        setBodyLength(buffer, buffer.size() - STUN_HEADER + 8);
        quint32 fingerprint = QXmppUtils::generateCrc32(buffer) ^ 0x5354554eL;
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

    id.resize(STUN_ID_SIZE);
    stream.readRawData(id.data(), id.size());
    return type;
}

QString QXmppStunMessage::toString() const
{
    QStringList dumpLines;
    QString typeName;
    switch (messageMethod())
    {
        case Binding: typeName = "Binding"; break;
        case SharedSecret: typeName = "Shared Secret"; break;
        case Allocate: typeName = "Allocate"; break;
        case Refresh: typeName = "Refresh"; break;
        case Send: typeName = "Send"; break;
        case Data: typeName = "Data"; break;
        case CreatePermission: typeName = "CreatePermission"; break;
        case ChannelBind: typeName = "ChannelBind"; break;
        default: typeName = "Unknown"; break;
    }
    switch (messageClass())
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
    dumpLines << QString(" id %1").arg(QString::fromLatin1(m_id.toHex()));

    // attributes
    if (m_attributes.contains(ChannelNumber))
        dumpLines << QString(" * CHANNEL-NUMBER %1").arg(QString::number(m_channelNumber));
    if (errorCode)
        dumpLines << QString(" * ERROR-CODE %1 %2")
            .arg(QString::number(errorCode), errorPhrase);
    if (m_attributes.contains(Lifetime))
        dumpLines << QString(" * LIFETIME %1").arg(QString::number(m_lifetime));
    if (m_attributes.contains(Nonce))
        dumpLines << QString(" * NONCE %1").arg(QString::fromLatin1(m_nonce));
    if (m_attributes.contains(Realm))
        dumpLines << QString(" * REALM %1").arg(m_realm);
    if (m_attributes.contains(RequestedTransport))
        dumpLines << QString(" * REQUESTED-TRANSPORT 0x%1").arg(QString::number(m_requestedTransport, 16));
    if (m_attributes.contains(ReservationToken))
        dumpLines << QString(" * RESERVATION-TOKEN %1").arg(QString::fromLatin1(m_reservationToken.toHex()));
    if (m_attributes.contains(Software))
        dumpLines << QString(" * SOFTWARE %1").arg(m_software);
    if (m_attributes.contains(Username))
        dumpLines << QString(" * USERNAME %1").arg(m_username);
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
    if (xorPeerPort)
        dumpLines << QString(" * XOR-PEER-ADDRESS %1 %2")
            .arg(xorPeerHost.toString(), QString::number(xorPeerPort));
    if (xorRelayedPort)
        dumpLines << QString(" * XOR-RELAYED-ADDRESS %1 %2")
            .arg(xorRelayedHost.toString(), QString::number(xorRelayedPort));
    if (m_attributes.contains(Priority))
        dumpLines << QString(" * PRIORITY %1").arg(QString::number(m_priority));
    if (!iceControlling.isEmpty())
        dumpLines << QString(" * ICE-CONTROLLING %1")
            .arg(QString::fromLatin1(iceControlling.toHex()));
    if (!iceControlled.isEmpty())
        dumpLines << QString(" * ICE-CONTROLLED %1")
            .arg(QString::fromLatin1(iceControlled.toHex()));
    if (useCandidate)
        dumpLines << QString(" * USE-CANDIDATE");

    return dumpLines.join("\n");
}

/// Constructs a new QXmppStunTransaction.
///
/// \param request
/// \param receiver

QXmppStunTransaction::QXmppStunTransaction(const QXmppStunMessage &request, QObject *receiver)
    : QXmppLoggable(receiver),
    m_request(request),
    m_tries(0)
{
    bool check;
    Q_UNUSED(check);

    check = connect(this, SIGNAL(writeStun(QXmppStunMessage)),
                    receiver, SLOT(writeStun(QXmppStunMessage)));
    Q_ASSERT(check);

    check = connect(this, SIGNAL(finished()),
                    receiver, SLOT(transactionFinished()));
    Q_ASSERT(check);

    // RTO timer
    m_retryTimer = new QTimer(this);
    m_retryTimer->setSingleShot(true);
    check = connect(m_retryTimer, SIGNAL(timeout()),
                    this, SLOT(retry()));

    // send packet immediately
    m_retryTimer->start(0);
}

void QXmppStunTransaction::readStun(const QXmppStunMessage &response)
{
    if (response.messageClass() == QXmppStunMessage::Error ||
        response.messageClass() == QXmppStunMessage::Response) {
        m_response = response;
        m_retryTimer->stop();
        emit finished();
    }
}

/// Returns the STUN request.

QXmppStunMessage QXmppStunTransaction::request() const
{
    return m_request;
}

/// Returns the STUN response.

QXmppStunMessage QXmppStunTransaction::response() const
{
    return m_response;
}

void QXmppStunTransaction::retry()
{
    if (m_tries >= STUN_RTO_MAX) {
        m_response.setType(QXmppStunMessage::Error);
        m_response.errorPhrase = QLatin1String("Request timed out");
        emit finished();
        return;
    }

    // resend request
    emit writeStun(m_request);
    m_retryTimer->start(m_tries ? 2 * m_retryTimer->interval() : STUN_RTO_INTERVAL);
    m_tries++;
}

/// Constructs a new QXmppTurnAllocation.
///
/// \param parent

QXmppTurnAllocation::QXmppTurnAllocation(QObject *parent)
    : QXmppIceTransport(parent),
    m_relayedPort(0),
    m_turnPort(0),
    m_channelNumber(0x4000),
    m_lifetime(600),
    m_state(UnconnectedState)
{
    bool check;
    Q_UNUSED(check);

    socket = new QUdpSocket(this);
    check = connect(socket, SIGNAL(readyRead()),
                    this, SLOT(readyRead()));
    Q_ASSERT(check);

    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);
    check = connect(m_timer, SIGNAL(timeout()),
                    this, SLOT(refresh()));
    Q_ASSERT(check);

    // channels are valid 600s, we refresh every 500s
    m_channelTimer = new QTimer(this);
    m_channelTimer->setInterval(500 * 1000);
    check = connect(m_channelTimer, SIGNAL(timeout()),
                    this, SLOT(refreshChannels()));
    Q_ASSERT(check);
}

/// Destroys the TURN allocation.

QXmppTurnAllocation::~QXmppTurnAllocation()
{
    if (m_state == ConnectedState)
        disconnectFromHost();
}

/// Allocates the TURN allocation.

void QXmppTurnAllocation::connectToHost()
{
    if (m_state != UnconnectedState)
        return;

    // start listening for UDP
    if (socket->state() == QAbstractSocket::UnconnectedState) {
        if (!socket->bind()) {
            warning("Could not start listening for TURN");
            return;
        }
    }

    // send allocate request
    QXmppStunMessage request;
    request.setType(QXmppStunMessage::Allocate | QXmppStunMessage::Request);
    request.setId(QXmppUtils::generateRandomBytes(STUN_ID_SIZE));
    request.setLifetime(m_lifetime);
    request.setRequestedTransport(0x11);
    m_transactions << new QXmppStunTransaction(request, this);

    // update state
    setState(ConnectingState);
}

/// Releases the TURN allocation.

void QXmppTurnAllocation::disconnectFromHost()
{
    m_channelTimer->stop();
    m_timer->stop();

    // clear channels and any outstanding transactions
    m_channels.clear();
    foreach (QXmppStunTransaction *transaction, m_transactions)
        delete transaction;
    m_transactions.clear();

    // end allocation
    if (m_state == ConnectedState) {
        QXmppStunMessage request;
        request.setType(QXmppStunMessage::Refresh | QXmppStunMessage::Request);
        request.setId(QXmppUtils::generateRandomBytes(STUN_ID_SIZE));
        request.setNonce(m_nonce);
        request.setRealm(m_realm);
        request.setUsername(m_username);
        request.setLifetime(0);
        m_transactions << new QXmppStunTransaction(request, this);

        setState(ClosingState);
    } else {
        setState(UnconnectedState);
    }
}

QXmppJingleCandidate QXmppTurnAllocation::localCandidate(int component) const
{
    QXmppJingleCandidate candidate;
    candidate.setComponent(component);
    candidate.setHost(relayedHost());
    candidate.setId(QXmppUtils::generateStanzaHash(10));
    candidate.setPort(relayedPort());
    candidate.setProtocol("udp");
    candidate.setType(QXmppJingleCandidate::RelayedType);
    candidate.setPriority(candidatePriority(candidate));
    candidate.setFoundation(computeFoundation(
        candidate.type(),
        candidate.protocol(),
        candidate.host()));
    return candidate;
}

void QXmppTurnAllocation::readyRead()
{
    QByteArray buffer;
    QHostAddress remoteHost;
    quint16 remotePort;
    while (socket->hasPendingDatagrams()) {
        const qint64 size = socket->pendingDatagramSize();
        buffer.resize(size);
        socket->readDatagram(buffer.data(), buffer.size(), &remoteHost, &remotePort);
        handleDatagram(buffer, remoteHost, remotePort);
    }
}

void QXmppTurnAllocation::handleDatagram(const QByteArray &buffer, const QHostAddress &remoteHost, quint16 remotePort)
{
    // demultiplex channel data
    if (buffer.size() >= 4 && (buffer[0] & 0xc0) == 0x40) {
        QDataStream stream(buffer);
        quint16 channel, length;
        stream >> channel;
        stream >> length;
        if (m_state == ConnectedState && m_channels.contains(channel) && length <= buffer.size() - 4) {
            emit datagramReceived(buffer.mid(4, length), m_channels[channel].first,
                m_channels[channel].second);
        }
        return;
    }

    // parse STUN message
    QXmppStunMessage message;
    QStringList errors;
    if (!message.decode(buffer, QByteArray(), &errors)) {
        foreach (const QString &error, errors)
            warning(error);
        return;
    }

#ifdef QXMPP_DEBUG_STUN
    logReceived(QString("TURN packet from %1 port %2\n%3").arg(
            remoteHost.toString(),
            QString::number(remotePort),
            message.toString()));
#endif

    // find transaction
    foreach (QXmppStunTransaction *transaction, m_transactions) {
        if (transaction->request().id() == message.id() &&
            transaction->request().messageMethod() == message.messageMethod()) {
            transaction->readStun(message);
            return;
        }
    }
}

/// Refresh allocation.

void QXmppTurnAllocation::refresh()
{
    QXmppStunMessage request;
    request.setType(QXmppStunMessage::Refresh | QXmppStunMessage::Request);
    request.setId(QXmppUtils::generateRandomBytes(STUN_ID_SIZE));
    request.setNonce(m_nonce);
    request.setRealm(m_realm);
    request.setUsername(m_username);
    m_transactions << new QXmppStunTransaction(request, this);
}

/// Refresh channel bindings.

void QXmppTurnAllocation::refreshChannels()
{
    foreach (quint16 channel, m_channels.keys()) {
        QXmppStunMessage request;
        request.setType(QXmppStunMessage::ChannelBind | QXmppStunMessage::Request);
        request.setId(QXmppUtils::generateRandomBytes(STUN_ID_SIZE));
        request.setNonce(m_nonce);
        request.setRealm(m_realm);
        request.setUsername(m_username);
        request.setChannelNumber(channel);
        request.xorPeerHost = m_channels[channel].first;
        request.xorPeerPort = m_channels[channel].second;
        m_transactions << new QXmppStunTransaction(request, this);
    }
}

/// Returns the relayed host address, i.e. the address on the server
/// used to communicate with peers.

QHostAddress QXmppTurnAllocation::relayedHost() const
{
    return m_relayedHost;
}

/// Returns the relayed port, i.e. the port on the server used to communicate
/// with peers.

quint16 QXmppTurnAllocation::relayedPort() const
{
    return m_relayedPort;
}

/// Sets the password used to authenticate with the TURN server.
///
/// \param password

void QXmppTurnAllocation::setPassword(const QString &password)
{
    m_password = password;
}

/// Sets the TURN server to use.
///
/// \param host The address of the TURN server.
/// \param port The port of the TURN server.

void QXmppTurnAllocation::setServer(const QHostAddress &host, quint16 port)
{
    m_turnHost = host;
    m_turnPort = port;
}

/// Sets the \a user used for authentication with the TURN server.
///
/// \param user

void QXmppTurnAllocation::setUser(const QString &user)
{
    m_username = user;
}

/// Returns the current state of the allocation.
///

QXmppTurnAllocation::AllocationState QXmppTurnAllocation::state() const
{
    return m_state;
}

void QXmppTurnAllocation::setState(AllocationState state)
{
    if (state == m_state)
        return;
    m_state = state;
    if (m_state == ConnectedState) {
        emit connected();
    } else if (m_state == UnconnectedState) {
        m_timer->stop();
        emit disconnected();
    }
}

void QXmppTurnAllocation::transactionFinished()
{
    QXmppStunTransaction *transaction = qobject_cast<QXmppStunTransaction*>(sender());
    if (!transaction || !m_transactions.removeAll(transaction))
        return;
    transaction->deleteLater();

    // handle authentication
    const QXmppStunMessage reply = transaction->response();
    if (reply.messageClass() == QXmppStunMessage::Error &&
        reply.errorCode == 401 &&
        (reply.nonce() != m_nonce && reply.realm() != m_realm))
    {
        // update long-term credentials
        m_nonce = reply.nonce();
        m_realm = reply.realm();
        QCryptographicHash hash(QCryptographicHash::Md5);
        hash.addData((m_username + ":" + m_realm + ":" + m_password).toUtf8());
        m_key = hash.result();

        // retry request
        QXmppStunMessage request(transaction->request());
        request.setId(QXmppUtils::generateRandomBytes(STUN_ID_SIZE));
        request.setNonce(m_nonce);
        request.setRealm(m_realm);
        request.setUsername(m_username);
        m_transactions << new QXmppStunTransaction(request, this);
        return;
    }

    const quint16 method = transaction->request().messageMethod();
    if (method == QXmppStunMessage::Allocate) {

        if (reply.messageClass() == QXmppStunMessage::Error) {
            warning(QString("Allocation failed: %1 %2").arg(
                QString::number(reply.errorCode), reply.errorPhrase));
            setState(UnconnectedState);
            return;
        }
        if (reply.xorRelayedHost.isNull() ||
            reply.xorRelayedHost.protocol() != QAbstractSocket::IPv4Protocol ||
            !reply.xorRelayedPort) {
            warning("Allocation did not yield a valid relayed address");
            setState(UnconnectedState);
            return;
        }

        // store relayed address
        m_relayedHost = reply.xorRelayedHost;
        m_relayedPort = reply.xorRelayedPort;

        // schedule refresh
        m_lifetime = reply.lifetime();
        m_timer->start((m_lifetime - 60) * 1000);

        setState(ConnectedState);

    } else if (method == QXmppStunMessage::ChannelBind) {

        if (reply.messageClass() == QXmppStunMessage::Error) {
            warning(QString("ChannelBind failed: %1 %2").arg(
                QString::number(reply.errorCode), reply.errorPhrase));

            // remove channel
            m_channels.remove(transaction->request().channelNumber());
            if (m_channels.isEmpty())
                m_channelTimer->stop();
            return;
        }

    } else if (method == QXmppStunMessage::Refresh) {

        if (reply.messageClass() == QXmppStunMessage::Error) {
            warning(QString("Refresh failed: %1 %2").arg(
                QString::number(reply.errorCode), reply.errorPhrase));
            setState(UnconnectedState);
            return;
        }

        if (m_state == ClosingState) {
            setState(UnconnectedState);
            return;
        }

        // schedule refresh
        m_lifetime = reply.lifetime();
        m_timer->start((m_lifetime - 60) * 1000);

    }
}

qint64 QXmppTurnAllocation::writeDatagram(const QByteArray &data, const QHostAddress &host, quint16 port)
{
    if (m_state != ConnectedState)
        return -1;

    const Address addr = qMakePair(host, port);
    quint16 channel = m_channels.key(addr);

    if (!channel) {
        channel = m_channelNumber++;
        m_channels.insert(channel, addr);

        // bind channel
        QXmppStunMessage request;
        request.setType(QXmppStunMessage::ChannelBind | QXmppStunMessage::Request);
        request.setId(QXmppUtils::generateRandomBytes(STUN_ID_SIZE));
        request.setNonce(m_nonce);
        request.setRealm(m_realm);
        request.setUsername(m_username);
        request.setChannelNumber(channel);
        request.xorPeerHost = host;
        request.xorPeerPort = port;
        m_transactions << new QXmppStunTransaction(request, this);

        // schedule refresh
        if (!m_channelTimer->isActive())
            m_channelTimer->start();
    }

    // send data
    QByteArray channelData;
    channelData.reserve(4 + data.size());
    QDataStream stream(&channelData, QIODevice::WriteOnly);
    stream << channel;
    stream << quint16(data.size());
    stream.writeRawData(data.data(), data.size());
    if (socket->writeDatagram(channelData, m_turnHost, m_turnPort) == channelData.size())
        return data.size();
    else
        return -1;
}

void QXmppTurnAllocation::writeStun(const QXmppStunMessage &message)
{
    socket->writeDatagram(message.encode(m_key), m_turnHost, m_turnPort);
#ifdef QXMPP_DEBUG_STUN
    logSent(QString("TURN packet to %1 port %2\n%3").arg(
            m_turnHost.toString(),
            QString::number(m_turnPort),
            message.toString()));
#endif
}

QXmppUdpTransport::QXmppUdpTransport(QUdpSocket *socket, QObject *parent)
    : QXmppIceTransport(parent)
    , m_socket(socket)
{
    bool check;
    Q_UNUSED(check);

    check = connect(m_socket, SIGNAL(readyRead()), this, SLOT(readyRead()));
    Q_ASSERT(check);
}

QXmppUdpTransport::~QXmppUdpTransport()
{
}

void QXmppUdpTransport::disconnectFromHost()
{
    m_socket->close();
}

QXmppJingleCandidate QXmppUdpTransport::localCandidate(int component) const
{
    QXmppJingleCandidate candidate;
    candidate.setComponent(component);
    // remove scope ID from IPv6 non-link local addresses
    QHostAddress addr(m_socket->localAddress());
    if (addr.protocol() == QAbstractSocket::IPv6Protocol &&
        !isIPv6LinkLocalAddress(addr)) {
        addr.setScopeId(QString());
    }
    candidate.setHost(addr);
    candidate.setId(QXmppUtils::generateStanzaHash(10));
    candidate.setPort(m_socket->localPort());
    candidate.setProtocol("udp");
    candidate.setType(QXmppJingleCandidate::HostType);
    candidate.setPriority(candidatePriority(candidate));
    candidate.setFoundation(computeFoundation(
        candidate.type(),
        candidate.protocol(),
        candidate.host()));
    return candidate;
}

void QXmppUdpTransport::readyRead()
{
    QByteArray buffer;
    QHostAddress remoteHost;
    quint16 remotePort;
    while (m_socket->hasPendingDatagrams()) {
        const qint64 size = m_socket->pendingDatagramSize();
        buffer.resize(size);
        m_socket->readDatagram(buffer.data(), buffer.size(), &remoteHost, &remotePort);
        emit datagramReceived(buffer, remoteHost, remotePort);
    }
}

qint64 QXmppUdpTransport::writeDatagram(const QByteArray &data, const QHostAddress &host, quint16 port)
{
    QHostAddress remoteHost = host;
    if (isIPv6LinkLocalAddress(host))
        remoteHost.setScopeId(m_socket->localAddress().scopeId());
    return m_socket->writeDatagram(data, remoteHost, port);
}

class CandidatePair : public QXmppLoggable
{
public:
    enum State {
        FrozenState,
        WaitingState,
        InProgressState,
        SucceededState,
        FailedState
    };
    CandidatePair(int component, bool controlling, QObject *parent);
    quint64 priority() const;
    State state() const;
    void setState(State state);
    QString toString() const;

    bool nominated;
    bool nominating;
    QXmppJingleCandidate remote;
    QXmppJingleCandidate reflexive;
    QXmppIceTransport *transport;
    QXmppStunTransaction *transaction;

private:
    int m_component;
    bool m_controlling;
    State m_state;
};

static bool candidatePairPtrLessThan(const CandidatePair *p1, const CandidatePair *p2)
{
    return p1->priority() > p2->priority();
}

CandidatePair::CandidatePair(int component, bool controlling, QObject *parent)
    : QXmppLoggable(parent)
    , nominated(false)
    , nominating(false)
    , transport(0)
    , transaction(0)
    , m_component(component)
    , m_controlling(controlling)
    , m_state(WaitingState)
{
}

quint64 CandidatePair::priority() const
{
    const QXmppJingleCandidate local = transport->localCandidate(m_component);

    // see RFC 5245 - 5.7.2. Computing Pair Priority and Ordering Pairs
    const quint32 G = m_controlling ? local.priority() : remote.priority();
    const quint32 D = m_controlling ? remote.priority() : local.priority();
    return (quint64(1) << 32) * qMin(G, D) + 2 * qMax(G, D) + (G > D ? 1 : 0);
}

CandidatePair::State CandidatePair::state() const
{
    return m_state;
}

void CandidatePair::setState(CandidatePair::State state)
{
    m_state = state;
    info(QString("ICE pair changed to state %1 %2").arg(QLatin1String(pair_states[state]), toString()));
}

QString CandidatePair::toString() const
{
    const QXmppJingleCandidate candidate = transport->localCandidate(m_component);
    QString str = QString("%1 port %2").arg(remote.host().toString(), QString::number(remote.port()));
    if (candidate.type() == QXmppJingleCandidate::HostType)
        str += QString(" (local %1 port %2)").arg(candidate.host().toString(), QString::number(candidate.port()));
    else
        str += QString(" (relayed)");
    if (!reflexive.host().isNull() && reflexive.port())
        str += QString(" (reflexive %1 port %2)").arg(reflexive.host().toString(), QString::number(reflexive.port()));
    return str;
}

class QXmppIcePrivate
{
public:
    QXmppIcePrivate();

    bool iceControlling;
    QString localUser;
    QString localPassword;
    QString remoteUser;
    QString remotePassword;
    QHostAddress stunHost;
    quint16 stunPort;
    QByteArray tieBreaker;
};

QXmppIcePrivate::QXmppIcePrivate()
    : iceControlling(false)
    , stunPort(0)
{
    localUser = QXmppUtils::generateStanzaHash(4);
    localPassword = QXmppUtils::generateStanzaHash(22);
    tieBreaker = QXmppUtils::generateRandomBytes(8);
}

class QXmppIceComponentPrivate
{
public:
    QXmppIceComponentPrivate(int component, QXmppIcePrivate *config, QXmppIceComponent *qq);
    bool addRemoteCandidate(const QXmppJingleCandidate &candidate);
    CandidatePair* findPair(QXmppStunTransaction *transaction);
    void performCheck(CandidatePair *pair, bool nominate);
    void setSockets(QList<QUdpSocket*> sockets);
    void setTurnServer(const QHostAddress &host, quint16 port);
    void setTurnUser(const QString &user);
    void setTurnPassword(const QString &password);
    void writeStun(const QXmppStunMessage &message, QXmppIceTransport *transport, const QHostAddress &remoteHost, quint16 remotePort);

    CandidatePair *activePair;
    const int component;
    const QXmppIcePrivate* const config;
    CandidatePair *fallbackPair;

    QXmppIceConnection::GatheringState gatheringState;

    QList<QXmppJingleCandidate> localCandidates;

    quint32 peerReflexivePriority;
    QList<QXmppJingleCandidate> remoteCandidates;

    QList<CandidatePair*> pairs;
    QList<QXmppIceTransport*> transports;
    QTimer *timer;

    // STUN server
    QMap<QXmppStunTransaction*, QXmppIceTransport*> stunTransactions;

    // TURN server
    QXmppTurnAllocation *turnAllocation;
    bool turnConfigured;

private:
    QXmppIceComponent *q;
};

QXmppIceComponentPrivate::QXmppIceComponentPrivate(int component_, QXmppIcePrivate *config_, QXmppIceComponent *qq)
    : activePair(0)
    , component(component_)
    , config(config_)
    , fallbackPair(0)
    , gatheringState(QXmppIceConnection::NewGatheringState)
    , peerReflexivePriority(0)
    , timer(0)
    , turnAllocation(0)
    , turnConfigured(false)
    , q(qq)
{
}

bool QXmppIceComponentPrivate::addRemoteCandidate(const QXmppJingleCandidate &candidate)
{
    if (candidate.component() != component ||
        (candidate.type() != QXmppJingleCandidate::HostType &&
         candidate.type() != QXmppJingleCandidate::RelayedType &&
         candidate.type() != QXmppJingleCandidate::ServerReflexiveType) ||
        candidate.protocol() != "udp" ||
        (candidate.host().protocol() != QAbstractSocket::IPv4Protocol &&
        candidate.host().protocol() != QAbstractSocket::IPv6Protocol))
        return false;

    foreach (const QXmppJingleCandidate &c, remoteCandidates)
        if (c.host() == candidate.host() && c.port() == candidate.port())
            return false;
    remoteCandidates << candidate;

    foreach (QXmppIceTransport *transport, transports) {
        // only pair compatible addresses
        const QXmppJingleCandidate local = transport->localCandidate(component);
        if (!isCompatibleAddress(local.host(), candidate.host()))
            continue;

        CandidatePair *pair = new CandidatePair(component, config->iceControlling, q);
        pair->remote = candidate;
        pair->transport = transport;
        pairs << pair;

        if (!fallbackPair && local.type() == QXmppJingleCandidate::HostType)
            fallbackPair = pair;
    }

    qSort(pairs.begin(), pairs.end(), candidatePairPtrLessThan);

    return true;
}

CandidatePair* QXmppIceComponentPrivate::findPair(QXmppStunTransaction *transaction)
{
    foreach (CandidatePair *pair, pairs) {
        if (pair->transaction == transaction)
            return pair;
    }
    return 0;
}

void QXmppIceComponentPrivate::performCheck(CandidatePair *pair, bool nominate)
{
    QXmppStunMessage message;
    message.setId(QXmppUtils::generateRandomBytes(STUN_ID_SIZE));
    message.setType(QXmppStunMessage::Binding | QXmppStunMessage::Request);
    message.setPriority(peerReflexivePriority);
    message.setUsername(QString("%1:%2").arg(config->remoteUser, config->localUser));
    if (config->iceControlling) {
        message.iceControlling = config->tieBreaker;
        message.useCandidate = true;
    } else {
        message.iceControlled = config->tieBreaker;
    }
    pair->nominating = nominate;
    pair->setState(CandidatePair::InProgressState);
    pair->transaction = new QXmppStunTransaction(message, q);
}

void QXmppIceComponentPrivate::setSockets(QList<QUdpSocket*> sockets)
{
    bool check;
    Q_UNUSED(check);

    // clear previous candidates and sockets
    localCandidates.clear();
    foreach (CandidatePair *pair, pairs)
        delete pair;
    pairs.clear();
    foreach (QXmppIceTransport *transport, transports)
        if (transport != turnAllocation)
            delete transport;
    transports.clear();

    // store candidates
    foreach (QUdpSocket *socket, sockets) {
        socket->setParent(q);

        QXmppUdpTransport *transport = new QXmppUdpTransport(socket, q);
        check = QObject::connect(transport, SIGNAL(datagramReceived(QByteArray,QHostAddress,quint16)),
                                 q, SLOT(handleDatagram(QByteArray,QHostAddress,quint16)));
        Q_ASSERT(check);

        QXmppJingleCandidate candidate = transport->localCandidate(component);

        transports << transport;
        localCandidates << candidate;
    }

    // start STUN checks
    if (!config->stunHost.isNull() && config->stunPort) {
        stunTransactions.clear();

        QXmppStunMessage request;
        request.setType(QXmppStunMessage::Binding | QXmppStunMessage::Request);
        foreach (QXmppIceTransport *transport, transports) {
            const QXmppJingleCandidate local = transport->localCandidate(component);
            if (!isCompatibleAddress(local.host(), config->stunHost))
                continue;

            request.setId(QXmppUtils::generateRandomBytes(STUN_ID_SIZE));
            QXmppStunTransaction *transaction = new QXmppStunTransaction(request, q);
            stunTransactions.insert(transaction, transport);
        }
    }

    // connect to TURN server
    if (turnConfigured) {
        transports << turnAllocation;
        turnAllocation->connectToHost();
    }

    q->updateGatheringState();
}

void QXmppIceComponentPrivate::setTurnServer(const QHostAddress &host, quint16 port)
{
    turnAllocation->setServer(host, port);
    turnConfigured = !host.isNull() && port;
}

void QXmppIceComponentPrivate::setTurnUser(const QString &user)
{
    turnAllocation->setUser(user);
}

void QXmppIceComponentPrivate::setTurnPassword(const QString &password)
{
    turnAllocation->setPassword(password);
}

void QXmppIceComponentPrivate::writeStun(const QXmppStunMessage &message, QXmppIceTransport *transport, const QHostAddress &address, quint16 port)
{
    const QString messagePassword = (message.type() & 0xFF00) ? config->localPassword : config->remotePassword;
    const QByteArray data = message.encode(messagePassword.toUtf8());
    transport->writeDatagram(data, address, port);
#ifdef QXMPP_DEBUG_STUN
    q->logSent(QString("STUN packet to %1 port %2\n%3").arg(
               address.toString(),
               QString::number(port),
               message.toString()));
#endif
}

/// Constructs a new QXmppIceComponent.
///
/// \param parent

QXmppIceComponent::QXmppIceComponent(int component, QXmppIcePrivate *config, QObject *parent)
    : QXmppLoggable(parent)
{
    bool check;
    Q_UNUSED(check);

    d = new QXmppIceComponentPrivate(component, config, this);

    d->timer = new QTimer(this);
    d->timer->setInterval(500);
    check = connect(d->timer, SIGNAL(timeout()),
                    this, SLOT(checkCandidates()));
    Q_ASSERT(check);

    d->turnAllocation = new QXmppTurnAllocation(this);
    check = connect(d->turnAllocation, SIGNAL(connected()),
                    this, SLOT(turnConnected()));
    Q_ASSERT(check);
    check = connect(d->turnAllocation, SIGNAL(datagramReceived(QByteArray,QHostAddress,quint16)),
                    this, SLOT(handleDatagram(QByteArray,QHostAddress,quint16)));
    Q_ASSERT(check);
    check = connect(d->turnAllocation, SIGNAL(disconnected()),
                    this, SLOT(updateGatheringState()));
    Q_ASSERT(check);

    // calculate peer-reflexive candidate priority
    // see RFC 5245 -  7.1.2.1. PRIORITY and USE-CANDIDATE
    QXmppJingleCandidate reflexive;
    reflexive.setComponent(d->component);
    reflexive.setType(QXmppJingleCandidate::PeerReflexiveType);
    d->peerReflexivePriority = candidatePriority(reflexive);

    setObjectName(QString("STUN(%1)").arg(QString::number(d->component)));
}

/// Destroys the QXmppIceComponent.

QXmppIceComponent::~QXmppIceComponent()
{
    foreach (CandidatePair *pair, d->pairs)
        delete pair;
    delete d;
}

/// Returns the component id for the current socket, e.g. 1 for RTP
/// and 2 for RTCP.

int QXmppIceComponent::component() const
{
    return d->component;
}

void QXmppIceComponent::checkCandidates()
{
    if (d->config->remoteUser.isEmpty())
        return;
    debug("Checking remote candidates");

    foreach (CandidatePair *pair, d->pairs) {
        if (pair->state() == CandidatePair::WaitingState) {
            d->performCheck(pair, d->config->iceControlling);
            break;
        }
    }
}

/// Stops ICE connectivity checks and closes the underlying sockets.

void QXmppIceComponent::close()
{
    foreach (QXmppIceTransport *transport, d->transports)
        transport->disconnectFromHost();
    d->turnAllocation->disconnectFromHost();
    d->timer->stop();
    d->activePair = 0;
}

/// Starts ICE connectivity checks.

void QXmppIceComponent::connectToHost()
{
    if (d->activePair)
        return;

    checkCandidates();
    d->timer->start();
}

/// Returns true if ICE negotiation completed, false otherwise.

bool QXmppIceComponent::isConnected() const
{
    return d->activePair != 0;
}

/// Returns the list of local candidates.

QList<QXmppJingleCandidate> QXmppIceComponent::localCandidates() const
{
    return d->localCandidates;
}

void QXmppIceComponent::handleDatagram(const QByteArray &buffer, const QHostAddress &remoteHost, quint16 remotePort)
{
    QXmppIceTransport *transport = qobject_cast<QXmppIceTransport*>(sender());
    if (!transport)
        return;

    // if this is not a STUN message, emit it
    quint32 messageCookie;
    QByteArray messageId;
    quint16 messageType = QXmppStunMessage::peekType(buffer, messageCookie, messageId);
    if (!messageType || messageCookie != STUN_MAGIC)
    {
        // use this as an opportunity to flag a potential pair
        foreach (CandidatePair *pair, d->pairs) {
            if (pair->remote.host() == remoteHost &&
                pair->remote.port() == remotePort) {
                d->fallbackPair = pair;
                break;
            }
        }
        emit datagramReceived(buffer);
        return;
    }

    // check if it's STUN
    QXmppStunTransaction *stunTransaction = 0;
    foreach (QXmppStunTransaction *t, d->stunTransactions.keys()) {
        if (t->request().id() == messageId &&
            d->stunTransactions.value(t) == transport) {
            stunTransaction = t;
            break;
        }
    }

    // determine password to use
    QString messagePassword;
    if (!stunTransaction) {
        messagePassword = (messageType & 0xFF00) ? d->config->remotePassword : d->config->localPassword;
        if (messagePassword.isEmpty())
            return;
    }

    // parse STUN message
    QXmppStunMessage message;
    QStringList errors;
    if (!message.decode(buffer, messagePassword.toUtf8(), &errors)) {
        foreach (const QString &error, errors)
            warning(error);
        return;
    }
#ifdef QXMPP_DEBUG_STUN
    logReceived(QString("STUN packet from %1 port %2\n%3").arg(
            remoteHost.toString(),
            QString::number(remotePort),
            message.toString()));
#endif

    // we only want binding requests and responses
    if (message.messageMethod() != QXmppStunMessage::Binding)
        return;

    // STUN checks
    if (stunTransaction) {
        stunTransaction->readStun(message);
        return;
    }

    // process message from peer
    CandidatePair *pair = 0;
    if (message.messageClass() == QXmppStunMessage::Request)
    {
        // check for role conflict
        if (d->config->iceControlling && (!message.iceControlling.isEmpty() || message.useCandidate)) {
            warning("Role conflict, expected to be controlling");
            return;
        } else if (!d->config->iceControlling && !message.iceControlled.isEmpty()) {
            warning("Role conflict, expected to be controlled");
            return;
        }

        // send a binding response
        QXmppStunMessage response;
        response.setId(message.id());
        response.setType(QXmppStunMessage::Binding | QXmppStunMessage::Response);
        response.xorMappedHost = remoteHost;
        response.xorMappedPort = remotePort;
        d->writeStun(response, transport, remoteHost, remotePort);

        // find or create remote candidate
        QXmppJingleCandidate remoteCandidate;
        bool remoteCandidateFound = false;
        foreach (const QXmppJingleCandidate &c, d->remoteCandidates) {
            if (c.host() == remoteHost && c.port() == remotePort) {
                remoteCandidate = c;
                remoteCandidateFound = true;
                break;
            }
        }
        if (!remoteCandidateFound) {
            // 7.2.1.3. Learning Peer Reflexive Candidates
            remoteCandidate.setComponent(d->component);
            remoteCandidate.setHost(remoteHost);
            remoteCandidate.setId(QXmppUtils::generateStanzaHash(10));
            remoteCandidate.setPort(remotePort);
            remoteCandidate.setPriority(message.priority());
            remoteCandidate.setProtocol("udp");
            remoteCandidate.setType(QXmppJingleCandidate::PeerReflexiveType);
            remoteCandidate.setFoundation(QXmppUtils::generateStanzaHash(32));

            d->remoteCandidates << remoteCandidate;
        }

        // construct pair
        foreach (CandidatePair *ptr, d->pairs) {
            if (ptr->transport == transport
             && ptr->remote.host() == remoteHost
             && ptr->remote.port() == remotePort) {
                pair = ptr;
                break;
            }
        }
        if (!pair) {
            pair = new CandidatePair(d->component, d->config->iceControlling, this);
            pair->remote = remoteCandidate;
            pair->transport = transport;
            d->pairs << pair;

            qSort(d->pairs.begin(), d->pairs.end(), candidatePairPtrLessThan);
        }

        switch (pair->state()) {
        case CandidatePair::FrozenState:
        case CandidatePair::WaitingState:
        case CandidatePair::FailedState:
            // send a triggered connectivity test
            if (!d->config->remoteUser.isEmpty())
                d->performCheck(pair, pair->nominating || d->config->iceControlling || message.useCandidate);
            break;
        case CandidatePair::InProgressState:
            // FIXME: force retransmit now
            pair->nominating = pair->nominating || message.useCandidate;
            break;
        case CandidatePair::SucceededState:
            if (message.useCandidate)
                pair->nominated = true;
            break;
        }

    } else if (message.messageClass() == QXmppStunMessage::Response
            || message.messageClass() == QXmppStunMessage::Error) {

        // find the pair for this transaction
        foreach (CandidatePair *ptr, d->pairs) {
            if (ptr->transaction && ptr->transaction->request().id() == message.id()) {
                pair = ptr;
                break;
            }
        }
        if (!pair)
            return;

        // check remote host and port
        if (remoteHost != pair->remote.host() || remotePort != pair->remote.port()) {
            QXmppStunMessage error;
            error.setType(QXmppStunMessage::Error);
            error.errorPhrase = QString("Received response from unexpected %1:%1").arg(
                remoteHost.toString(),
                QString::number(remotePort));
            pair->transaction->readStun(error);
            return;
        }

        pair->transaction->readStun(message);
    }

    // signal completion
    if (pair && pair->nominated) {
        d->timer->stop();
        if (!d->activePair || pair->priority() > d->activePair->priority()) {
            info(QString("ICE pair selected %1 (priority: %2)").arg(
                pair->toString(), QString::number(pair->priority())));
            const bool wasConnected = (d->activePair != 0);
            d->activePair = pair;
            if (!wasConnected)
                emit connected();
        }
    }
}

void QXmppIceComponent::transactionFinished()
{
    QXmppStunTransaction *transaction = qobject_cast<QXmppStunTransaction*>(sender());
    transaction->deleteLater();

    // ICE checks
    CandidatePair *pair = d->findPair(transaction);
    if (pair) {
        const QXmppStunMessage response = transaction->response();
        if (response.messageClass() == QXmppStunMessage::Response) {
            // store peer-reflexive address
            if (!response.xorMappedHost.isNull() && response.xorMappedPort != 0) {
                pair->reflexive.setHost(response.xorMappedHost);
                pair->reflexive.setPort(response.xorMappedPort);
            }

            pair->setState(CandidatePair::SucceededState);
            if (pair->nominating) {
                // outgoing media can flow
                pair->nominated = true;
            }
        } else {
            debug(QString("ICE forward check failed %1 (error %2)").arg(
                pair->toString(),
                transaction->response().errorPhrase));
            pair->setState(CandidatePair::FailedState);
        }
        pair->transaction = 0;
        return;
    }

    // STUN checks
    QXmppIceTransport *transport = d->stunTransactions.value(transaction);
    if (transport) {
        const QXmppStunMessage response = transaction->response();
        if (response.messageClass() == QXmppStunMessage::Response) {
            // determine server-reflexive address
            QHostAddress reflexiveHost;
            quint16 reflexivePort = 0;
            if (!response.xorMappedHost.isNull() && response.xorMappedPort != 0) {
                reflexiveHost = response.xorMappedHost;
                reflexivePort = response.xorMappedPort;
            } else if (!response.mappedHost.isNull() && response.mappedPort != 0) {
                reflexiveHost = response.mappedHost;
                reflexivePort = response.mappedPort;
            } else {
                warning("STUN server did not provide a reflexive address");
                return;
            }

            // check whether this candidates is already known
            foreach (const QXmppJingleCandidate &candidate, d->localCandidates) {
                if (candidate.host() == reflexiveHost &&
                    candidate.port() == reflexivePort &&
                    candidate.type() == QXmppJingleCandidate::ServerReflexiveType)
                    return;
            }

            // add the new local candidate
            debug(QString("Adding server-reflexive candidate %1 port %2").arg(reflexiveHost.toString(), QString::number(reflexivePort)));
            QXmppJingleCandidate candidate;
            candidate.setComponent(d->component);
            candidate.setHost(reflexiveHost);
            candidate.setId(QXmppUtils::generateStanzaHash(10));
            candidate.setPort(reflexivePort);
            candidate.setProtocol("udp");
            candidate.setType(QXmppJingleCandidate::ServerReflexiveType);
            candidate.setPriority(candidatePriority(candidate));
            candidate.setFoundation(computeFoundation(
                candidate.type(),
                candidate.protocol(),
                transport->localCandidate(d->component).host()));

            d->localCandidates << candidate;

            emit localCandidatesChanged();
        } else {
            debug(QString("STUN test failed (error %1)").arg(
                transaction->response().errorPhrase));
        }
        d->stunTransactions.remove(transaction);
        updateGatheringState();
        return;
    }
}

void QXmppIceComponent::turnConnected()
{
    const QXmppJingleCandidate candidate = d->turnAllocation->localCandidate(d->component);

    // add the new local candidate
    debug(QString("Adding relayed candidate %1 port %2").arg(
        candidate.host().toString(),
        QString::number(candidate.port())));
    d->localCandidates << candidate;

    emit localCandidatesChanged();
    updateGatheringState();
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
            QHostAddress ip = entry.ip();
            if ((ip.protocol() != QAbstractSocket::IPv4Protocol &&
                 ip.protocol() != QAbstractSocket::IPv6Protocol) ||
                entry.netmask().isNull())
                continue;

            // FIXME: for some reason we can have loopback addresses
            // even if the interface does not have the loopback flag
            if (isLoopbackAddress(ip))
                continue;

            // FIXME: for now skip IPv6 link-local addresses, seems to upset
            // clients such as empathy
            if (isIPv6LinkLocalAddress(ip)) {
                ip.setScopeId(interface.name());
                continue;
            }
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
    quint16 port = 49152;
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
    CandidatePair *pair = d->activePair ? d->activePair : d->fallbackPair;
    if (!pair)
        return -1;
    return pair->transport->writeDatagram(datagram, pair->remote.host(), pair->remote.port());
}

void QXmppIceComponent::updateGatheringState()
{
    QXmppIceConnection::GatheringState newGatheringState;
    if (d->transports.isEmpty())
        newGatheringState = QXmppIceConnection::NewGatheringState;
    else if (!d->stunTransactions.isEmpty()
           || d->turnAllocation->state() == QXmppTurnAllocation::ConnectingState)
        newGatheringState = QXmppIceConnection::BusyGatheringState;
    else
        newGatheringState = QXmppIceConnection::CompleteGatheringState;

    if (newGatheringState != d->gatheringState) {
        d->gatheringState = newGatheringState;
        emit gatheringStateChanged();
    }
}

void QXmppIceComponent::writeStun(const QXmppStunMessage &message)
{
    QXmppStunTransaction *transaction = qobject_cast<QXmppStunTransaction*>(sender());

    // ICE checks
    CandidatePair *pair = d->findPair(transaction);
    if (pair) {
        d->writeStun(message, pair->transport, pair->remote.host(), pair->remote.port());
        return;
    }

    // STUN checks
    QXmppIceTransport *transport = d->stunTransactions.value(transaction);
    if (transport) {
        transport->writeDatagram(message.encode(), d->config->stunHost, d->config->stunPort);
#ifdef QXMPP_DEBUG_STUN
        logSent(QString("STUN packet to %1 port %2\n%3").arg(
                   d->config->stunHost.toString(),
                   QString::number(d->config->stunPort),
                   message.toString()));
#endif
        return;
    }
}

class QXmppIceConnectionPrivate : public QXmppIcePrivate
{
public:
    QXmppIceConnectionPrivate();

    QMap<int, QXmppIceComponent*> components;
    QTimer *connectTimer;

    QXmppIceConnection::GatheringState gatheringState;

    QHostAddress turnHost;
    quint16 turnPort;
    QString turnUser;
    QString turnPassword;
};

QXmppIceConnectionPrivate::QXmppIceConnectionPrivate()
    : connectTimer(NULL)
    , gatheringState(QXmppIceConnection::NewGatheringState)
    , turnPort(0)
{
}

/// Constructs a new ICE connection.
///
/// \param parent

QXmppIceConnection::QXmppIceConnection(QObject *parent)
    : QXmppLoggable(parent)
    , d(new QXmppIceConnectionPrivate())
{
    bool check;

    // timer to limit connection time to 30 seconds
    d->connectTimer = new QTimer(this);
    d->connectTimer->setInterval(30000);
    d->connectTimer->setSingleShot(true);
    check = connect(d->connectTimer, SIGNAL(timeout()),
                    this, SLOT(slotTimeout()));
    Q_ASSERT(check);
    Q_UNUSED(check);
}

QXmppIceConnection::~QXmppIceConnection()
{
    delete d;
}

/// Returns the given component of this ICE connection.
///
/// \param component

QXmppIceComponent *QXmppIceConnection::component(int component)
{
    return d->components.value(component);
}

/// Adds a component to this ICE connection, for instance 1 for RTP
/// or 2 for RTCP.
///
/// \param component

void QXmppIceConnection::addComponent(int component)
{
    bool check;
    Q_UNUSED(check);

    if (d->components.contains(component)) {
        warning(QString("Already have component %1").arg(QString::number(component)));
        return;
    }

    QXmppIceComponent *socket = new QXmppIceComponent(component, d, this);
    socket->d->setTurnServer(d->turnHost, d->turnPort);
    socket->d->setTurnUser(d->turnUser);
    socket->d->setTurnPassword(d->turnPassword);

    check = connect(socket, SIGNAL(localCandidatesChanged()),
                    this, SIGNAL(localCandidatesChanged()));
    Q_ASSERT(check);

    check = connect(socket, SIGNAL(connected()),
                    this, SLOT(slotConnected()));
    Q_ASSERT(check);

    check = connect(socket, SIGNAL(gatheringStateChanged()),
                    this, SLOT(slotGatheringStateChanged()));
    Q_ASSERT(check);

    d->components[component] = socket;
}

/// Adds a candidate for one of the remote components.
///
/// \param candidate

void QXmppIceConnection::addRemoteCandidate(const QXmppJingleCandidate &candidate)
{
    QXmppIceComponent *socket = d->components.value(candidate.component());
    if (!socket) {
        warning(QString("Not adding candidate for unknown component %1").arg(
                QString::number(candidate.component())));
        return;
    }
    socket->d->addRemoteCandidate(candidate);
}

/// Binds the local sockets to the specified addresses.
///
/// \param addresses The addresses on which to listen.

bool QXmppIceConnection::bind(const QList<QHostAddress> &addresses)
{
    // reserve ports
    QList<QUdpSocket*> sockets = QXmppIceComponent::reservePorts(addresses, d->components.size());
    if (sockets.isEmpty() && !addresses.isEmpty())
        return false;

    // assign sockets
    QList<int> keys = d->components.keys();
    qSort(keys);
    int s = 0;
    foreach (int k, keys) {
        d->components[k]->d->setSockets(sockets.mid(s, addresses.size()));
        s += addresses.size();
    }

    return true;
}

/// Closes the ICE connection.

void QXmppIceConnection::close()
{
    d->connectTimer->stop();
    foreach (QXmppIceComponent *socket, d->components.values())
        socket->close();
}

/// Starts ICE connectivity checks.

void QXmppIceConnection::connectToHost()
{
    if (isConnected() || d->connectTimer->isActive())
        return;

    foreach (QXmppIceComponent *socket, d->components.values())
        socket->connectToHost();
    d->connectTimer->start();
}


/// Returns true if ICE negotiation completed, false otherwise.

bool QXmppIceConnection::isConnected() const
{
    foreach (QXmppIceComponent *socket, d->components.values())
        if (!socket->isConnected())
            return false;
    return true;
}

/// Returns the ICE gathering state, that is the discovery of
/// local candidates.

QXmppIceConnection::GatheringState QXmppIceConnection::gatheringState() const
{
    return d->gatheringState;
}

/// Sets whether the local party has the ICE controlling role.
///
/// \a note This must be called only once, immediately after creating
/// the connection.

void QXmppIceConnection::setIceControlling(bool controlling)
{
    d->iceControlling = controlling;
}

/// Returns the list of local HOST CANDIDATES candidates by iterating
/// over the available network interfaces.

QList<QXmppJingleCandidate> QXmppIceConnection::localCandidates() const
{
    QList<QXmppJingleCandidate> candidates;
    foreach (QXmppIceComponent *socket, d->components.values())
        candidates += socket->localCandidates();
    return candidates;
}

/// Returns the local user fragment.

QString QXmppIceConnection::localUser() const
{
    return d->localUser;
}

/// Returns the local password.

QString QXmppIceConnection::localPassword() const
{
    return d->localPassword;
}

/// Sets the remote user fragment.
///
/// \param user

void QXmppIceConnection::setRemoteUser(const QString &user)
{
    d->remoteUser = user;
}

/// Sets the remote password.
///
/// \param password

void QXmppIceConnection::setRemotePassword(const QString &password)
{
    d->remotePassword = password;
}

/// Sets the STUN server to use to determine server-reflexive addresses
/// and ports.
///
/// \note This may only be called prior to calling bind().
///
/// \param host The address of the STUN server.
/// \param port The port of the STUN server.

void QXmppIceConnection::setStunServer(const QHostAddress &host, quint16 port)
{
    d->stunHost = host;
    d->stunPort = port;
}

/// Sets the TURN server to use to relay packets in double-NAT configurations.
///
/// \note This may only be called prior to calling bind().
///
/// \param host The address of the TURN server.
/// \param port The port of the TURN server.

void QXmppIceConnection::setTurnServer(const QHostAddress &host, quint16 port)
{
    d->turnHost = host;
    d->turnPort = port;
    foreach (QXmppIceComponent *socket, d->components.values())
        socket->d->setTurnServer(host, port);
}

/// Sets the \a user used for authentication with the TURN server.
///
/// \note This may only be called prior to calling bind().
///
/// \param user

void QXmppIceConnection::setTurnUser(const QString &user)
{
    d->turnUser = user;
    foreach (QXmppIceComponent *socket, d->components.values())
        socket->d->setTurnUser(user);
}

/// Sets the \a password used for authentication with the TURN server.
///
/// \note This may only be called prior to calling bind().
///
/// \param password

void QXmppIceConnection::setTurnPassword(const QString &password)
{
    d->turnPassword = password;
    foreach (QXmppIceComponent *socket, d->components.values())
        socket->d->setTurnPassword(password);
}

void QXmppIceConnection::slotConnected()
{
    foreach (QXmppIceComponent *socket, d->components.values())
        if (!socket->isConnected())
            return;
    info(QString("ICE negotiation completed"));
    d->connectTimer->stop();
    emit connected();
}

void QXmppIceConnection::slotGatheringStateChanged()
{
    GatheringState newGatheringState;
    bool allComplete = true;
    bool allNew = true;
    foreach (QXmppIceComponent *socket, d->components.values()) {
        if (socket->d->gatheringState != CompleteGatheringState)
            allComplete = false;
        if (socket->d->gatheringState != NewGatheringState)
            allNew = false;
    }
    if (allNew)
        newGatheringState = NewGatheringState;
    else if (allComplete)
        newGatheringState = CompleteGatheringState;
    else
        newGatheringState = BusyGatheringState;

    if (newGatheringState != d->gatheringState) {
        info(QString("ICE gathering state changed from '%1' to '%2'").arg(
            gathering_states[d->gatheringState],
            gathering_states[newGatheringState]));
        d->gatheringState = newGatheringState;
        emit gatheringStateChanged();
    }
}

void QXmppIceConnection::slotTimeout()
{
    warning(QString("ICE negotiation timed out"));
    foreach (QXmppIceComponent *socket, d->components.values())
        socket->close();
    emit disconnected();
}

QXmppIceTransport::QXmppIceTransport(QObject *parent)
    : QXmppLoggable(parent)
{
}

QXmppIceTransport::~QXmppIceTransport()
{
}
