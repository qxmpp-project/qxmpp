/*
 * Copyright (C) 2008-2020 The QXmpp developers
 *
 * Authors:
 *  Manjeet Dahiya
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

#include "QXmppStream.h"

#include "QXmppConstants_p.h"
#include "QXmppLogger.h"
#include "QXmppStanza.h"
#include "QXmppStreamManagement_p.h"
#include "QXmppUtils.h"

#include <QBuffer>
#include <QDomDocument>
#include <QHostAddress>
#include <QMap>
#include <QRegularExpression>
#include <QSslSocket>
#include <QStringList>
#include <QTime>
#include <QXmlStreamWriter>

#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
static bool randomSeeded = false;
#endif

class QXmppStreamPrivate
{
public:
    QXmppStreamPrivate();

    QString dataBuffer;
    QSslSocket *socket;

    // incoming stream state
    QString streamOpenElement;

    bool streamManagementEnabled;
    QMap<unsigned, QByteArray> unacknowledgedStanzas;
    unsigned lastOutgoingSequenceNumber;
    unsigned lastIncomingSequenceNumber;
};

QXmppStreamPrivate::QXmppStreamPrivate()
    : socket(nullptr), streamManagementEnabled(false), lastOutgoingSequenceNumber(0), lastIncomingSequenceNumber(0)
{
}

///
/// Constructs a base XMPP stream.
///
/// \param parent
///
QXmppStream::QXmppStream(QObject *parent)
    : QXmppLoggable(parent),
      d(new QXmppStreamPrivate)
{
#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
    // Make sure the random number generator is seeded
    if (!randomSeeded) {
        qsrand(QTime(0, 0, 0).msecsTo(QTime::currentTime()) ^ reinterpret_cast<quintptr>(this));
        randomSeeded = true;
    }
#endif
}

///
/// Destroys a base XMPP stream.
///
QXmppStream::~QXmppStream()
{
    delete d;
}

///
/// Disconnects from the remote host.
///
void QXmppStream::disconnectFromHost()
{
    d->streamManagementEnabled = false;
    if (d->socket) {
        if (d->socket->state() == QAbstractSocket::ConnectedState) {
            sendData(QByteArrayLiteral("</stream:stream>"));
            d->socket->flush();
        }
        // FIXME: according to RFC 6120 section 4.4, we should wait for
        // the incoming stream to end before closing the socket
        d->socket->disconnectFromHost();
    }
}

///
/// Handles a stream start event, which occurs when the underlying transport
/// becomes ready (socket connected, encryption started).
///
/// If you redefine handleStart(), make sure to call the base class's method.
///
void QXmppStream::handleStart()
{
    d->streamManagementEnabled = false;
    d->dataBuffer.clear();
    d->streamOpenElement.clear();
}

///
/// Returns true if the stream is connected.
///
bool QXmppStream::isConnected() const
{
    return d->socket &&
        d->socket->state() == QAbstractSocket::ConnectedState;
}

///
/// Sends raw data to the peer.
///
/// \param data
///
bool QXmppStream::sendData(const QByteArray &data)
{
    logSent(QString::fromUtf8(data));
    if (!d->socket || d->socket->state() != QAbstractSocket::ConnectedState)
        return false;
    return d->socket->write(data) == data.size();
}

///
/// Sends an XMPP packet to the peer.
///
/// \param packet
///
bool QXmppStream::sendPacket(const QXmppStanza &packet)
{
    // prepare packet
    QByteArray data;
    QXmlStreamWriter xmlStream(&data);
    packet.toXml(&xmlStream);

    bool isXmppStanza = packet.isXmppStanza();
    if (isXmppStanza && d->streamManagementEnabled)
        d->unacknowledgedStanzas[++d->lastOutgoingSequenceNumber] = data;

    // send packet
    bool success = sendData(data);
    if (isXmppStanza)
        sendAcknowledgementRequest();
    return success;
}

///
/// Returns the QSslSocket used for this stream.
///
QSslSocket *QXmppStream::socket() const
{
    return d->socket;
}

///
/// Sets the QSslSocket used for this stream.
///
void QXmppStream::setSocket(QSslSocket *socket)
{
    d->socket = socket;
    if (!d->socket)
        return;

    // socket events
    connect(socket, &QAbstractSocket::connected, this, &QXmppStream::_q_socketConnected);
    connect(socket, &QSslSocket::encrypted, this, &QXmppStream::_q_socketEncrypted);
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    connect(socket, &QSslSocket::errorOccurred, this, &QXmppStream::_q_socketError);
#else
    connect(socket, QOverload<QAbstractSocket::SocketError>::of(&QSslSocket::error), this, &QXmppStream::_q_socketError);
#endif
    connect(socket, &QIODevice::readyRead, this, &QXmppStream::_q_socketReadyRead);
}

void QXmppStream::_q_socketConnected()
{
    info(QStringLiteral("Socket connected to %1 %2").arg(d->socket->peerAddress().toString(), QString::number(d->socket->peerPort())));
    handleStart();
}

void QXmppStream::_q_socketEncrypted()
{
    debug("Socket encrypted");
    handleStart();
}

void QXmppStream::_q_socketError(QAbstractSocket::SocketError socketError)
{
    Q_UNUSED(socketError);
    warning(QStringLiteral("Socket error: ") + socket()->errorString());
}

void QXmppStream::_q_socketReadyRead()
{
    processData(QString::fromUtf8(d->socket->readAll()));
}

void QXmppStream::processData(const QString &data)
{
    // As we may only have partial XML content, we need to cache the received
    // data until it has been successfully parsed. In case it can't be parsed,
    //
    // There are only two small problems with the current strategy:
    //  * When we receive a full stanza + a partial one, we can't parse the
    //    first stanza until another stanza arrives that is complete.
    //  * We don't know when we received invalid XML (would cause a growing
    //    cache and a timeout after some time).
    // However, both issues could only be solved using an XML stream reader
    // which would cause many other problems since we don't actually use it for
    // parsing the content.
    d->dataBuffer.append(data);

    //
    // Check for whitespace pings
    //
    if (d->dataBuffer.isEmpty() || d->dataBuffer.trimmed().isEmpty()) {
        d->dataBuffer.clear();

        logReceived({});
        handleStanza({});
        return;
    }

    //
    // Check whether we received a stream open or closing tag
    //
    static const QRegularExpression streamStartRegex(R"(^(<\?xml.*\?>)?\s*<stream:stream[^>]*>)");
    static const QRegularExpression streamEndRegex("</stream:stream>$");

    QRegularExpressionMatch streamOpenMatch;
    bool hasStreamOpen = d->streamOpenElement.isEmpty() &&
        (streamOpenMatch = streamStartRegex.match(d->dataBuffer)).hasMatch();

    bool hasStreamClose = streamEndRegex.match(d->dataBuffer).hasMatch();

    //
    // The stream start/end and stanza packets can't be parsed without any
    // modifications with QDomDocument. This is because of multiple reasons:
    //  * The <stream:stream> open element is not considered valid without the
    //    closing tag.
    //  * Only the closing tag is of course not valid too.
    //  * Stanzas/Nonzas need to have the correct stream namespaces set:
    //     * For being able to parse <stream:features/>
    //     * For having the correct namespace (e.g. 'jabber:client') set to
    //       stanzas and their child elements (e.g. <body/> of a message).
    //
    // The wrapping strategy looks like this:
    //  * The stream open tag is cached once it arrives, for later access
    //  * Incoming XML that has no <stream> open tag will be prepended by the
    //    cached <stream> tag.
    //  * Incoming XML that has no <stream> close tag will be appended by a
    //    generic string "</stream:stream>"
    //
    // The result is parsed by QDomDocument and the child elements of the stream
    // are processed. In case the received data contained a stream open tag,
    // the stream is processed (before the stanzas are processed). In case we
    // received a </stream> closing tag, the connection is closed.
    //
    auto wrappedStanzas = d->dataBuffer;
    if (!hasStreamOpen) {
        wrappedStanzas.prepend(d->streamOpenElement);
    }
    if (!hasStreamClose) {
        wrappedStanzas.append(QStringLiteral("</stream:stream>"));
    }

    //
    // Try to parse the wrapped XML
    //
    QDomDocument doc;
    if (!doc.setContent(wrappedStanzas, true))
        return;

    //
    // Success: We can clear the buffer and send a 'received' log message
    //
    d->dataBuffer.clear();
    logReceived(d->dataBuffer);

    // process stream start
    if (hasStreamOpen) {
        d->streamOpenElement = streamOpenMatch.captured();
        handleStream(doc.documentElement());
    }

    // process stanzas
    QDomElement nodeRecv = doc.documentElement().firstChildElement();
    while (!nodeRecv.isNull()) {
        if (QXmppStreamManagementAck::isStreamManagementAck(nodeRecv))
            handleAcknowledgement(nodeRecv);
        else if (QXmppStreamManagementReq::isStreamManagementReq(nodeRecv))
            sendAcknowledgement();
        else {
            handleStanza(nodeRecv);
            if (nodeRecv.tagName() == QLatin1String("message") ||
                nodeRecv.tagName() == QLatin1String("presence") ||
                nodeRecv.tagName() == QLatin1String("iq"))
                ++d->lastIncomingSequenceNumber;
        }
        nodeRecv = nodeRecv.nextSiblingElement();
    }

    // process stream end
    if (hasStreamClose) {
        disconnectFromHost();
    }
}

///
/// Enables Stream Management acks / reqs (\xep{0198}).
///
/// \param resetSequenceNumber Indicates if the sequence numbers should be
/// reset. This must be done if the stream is not resumed.
///
/// \since QXmpp 1.0
///
void QXmppStream::enableStreamManagement(bool resetSequenceNumber)
{
    d->streamManagementEnabled = true;

    if (resetSequenceNumber) {
        d->lastOutgoingSequenceNumber = 0;
        d->lastIncomingSequenceNumber = 0;

        // resend unacked stanzas
        if (!d->unacknowledgedStanzas.empty()) {
            QMap<unsigned, QByteArray> oldUnackedStanzas = d->unacknowledgedStanzas;
            d->unacknowledgedStanzas.clear();
            for (QMap<unsigned, QByteArray>::iterator it = oldUnackedStanzas.begin(); it != oldUnackedStanzas.end(); ++it) {
                d->unacknowledgedStanzas[++d->lastOutgoingSequenceNumber] = it.value();
                sendData(it.value());
            }
            sendAcknowledgementRequest();
        }
    } else {
        // resend unacked stanzas
        if (!d->unacknowledgedStanzas.empty()) {
            for (QMap<unsigned, QByteArray>::iterator it = d->unacknowledgedStanzas.begin(); it != d->unacknowledgedStanzas.end(); ++it)
                sendData(it.value());
            sendAcknowledgementRequest();
        }
    }
}

///
/// Returns the sequence number of the last incoming stanza (\xep{0198}).
///
/// \since QXmpp 1.0
///
unsigned QXmppStream::lastIncomingSequenceNumber() const
{
    return d->lastIncomingSequenceNumber;
}

///
/// Sets the last acknowledged sequence number for outgoing stanzas
/// (\xep{0198}).
///
/// \since QXmpp 1.0
///
void QXmppStream::setAcknowledgedSequenceNumber(unsigned sequenceNumber)
{
    for (QMap<unsigned, QByteArray>::iterator it = d->unacknowledgedStanzas.begin(); it != d->unacknowledgedStanzas.end();) {
        if (it.key() <= sequenceNumber)
            it = d->unacknowledgedStanzas.erase(it);
        else
            ++it;
    }
}

///
/// Handles an incoming acknowledgement from \xep{0198}.
///
/// \param element
///
/// \since QXmpp 1.0
///
void QXmppStream::handleAcknowledgement(QDomElement &element)
{
    if (!d->streamManagementEnabled)
        return;

    QXmppStreamManagementAck ack;
    ack.parse(element);
    setAcknowledgedSequenceNumber(ack.seqNo());
}

///
/// Sends an acknowledgement as defined in \xep{0198}.
///
/// \since QXmpp 1.0
///
void QXmppStream::sendAcknowledgement()
{
    if (!d->streamManagementEnabled)
        return;

    // prepare packet
    QByteArray data;
    QXmlStreamWriter xmlStream(&data);
    QXmppStreamManagementAck ack(d->lastIncomingSequenceNumber);
    ack.toXml(&xmlStream);

    // send packet
    sendData(data);
}

///
/// Sends an acknowledgement request as defined in \xep{0198}.
///
/// \since QXmpp 1.0
///
void QXmppStream::sendAcknowledgementRequest()
{
    if (!d->streamManagementEnabled)
        return;

    // prepare packet
    QByteArray data;
    QXmlStreamWriter xmlStream(&data);
    QXmppStreamManagementReq::toXml(&xmlStream);

    // send packet
    sendData(data);
}
