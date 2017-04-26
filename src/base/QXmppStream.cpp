/*
 * Copyright (C) 2008-2014 The QXmpp developers
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


#include "QXmppConstants_p.h"
#include "QXmppLogger.h"
#include "QXmppStanza.h"
#include "QXmppStream.h"
#include "QXmppStreamManagement_p.h"
#include "QXmppUtils.h"

#include <QBuffer>
#include <QDomDocument>
#include <QHostAddress>
#include <QMap>
#include <QRegExp>
#include <QSslSocket>
#include <QStringList>
#include <QTime>
#include <QXmlStreamWriter>

static bool randomSeeded = false;
static const QByteArray streamRootElementEnd = "</stream:stream>";

class QXmppStreamPrivate
{
public:
    QXmppStreamPrivate();

    QByteArray dataBuffer;
    QSslSocket* socket;

    // incoming stream state
    QByteArray streamStart;

    bool streamManagementEnabled;
    QMap<unsigned, QByteArray> unacknowledgedStanzas;
    unsigned lastOutgoingSequenceNumber;
    unsigned lastIncomingSequenceNumber;
};

QXmppStreamPrivate::QXmppStreamPrivate()
    : socket(0), streamManagementEnabled(false), lastOutgoingSequenceNumber(0), lastIncomingSequenceNumber(0)
{
}

/// Constructs a base XMPP stream.
///
/// \param parent

QXmppStream::QXmppStream(QObject *parent)
    : QXmppLoggable(parent),
    d(new QXmppStreamPrivate)
{
    // Make sure the random number generator is seeded
    if (!randomSeeded)
    {
        qsrand(QTime(0,0,0).msecsTo(QTime::currentTime()) ^ reinterpret_cast<quintptr>(this));
        randomSeeded = true;
    }
}

/// Destroys a base XMPP stream.

QXmppStream::~QXmppStream()
{
    delete d;
}

/// Disconnects from the remote host.
///

void QXmppStream::disconnectFromHost()
{
    d->streamManagementEnabled = false;
    if (d->socket) {
        if (d->socket->state() == QAbstractSocket::ConnectedState) {
            sendData(streamRootElementEnd);
            d->socket->flush();
        }
        // FIXME: according to RFC 6120 section 4.4, we should wait for
        // the incoming stream to end before closing the socket
        d->socket->disconnectFromHost();
    }
}

/// Handles a stream start event, which occurs when the underlying transport
/// becomes ready (socket connected, encryption started).
///
/// If you redefine handleStart(), make sure to call the base class's method.

void QXmppStream::handleStart()
{
    d->streamManagementEnabled = false;
    d->dataBuffer.clear();
    d->streamStart.clear();
}

/// Returns true if the stream is connected.
///

bool QXmppStream::isConnected() const
{
    return d->socket &&
           d->socket->state() == QAbstractSocket::ConnectedState;
}

/// Sends raw data to the peer.
///
/// \param data

bool QXmppStream::sendData(const QByteArray &data)
{
    logSent(QString::fromUtf8(data));
    if (!d->socket || d->socket->state() != QAbstractSocket::ConnectedState)
        return false;
    return d->socket->write(data) == data.size();
}

/// Sends an XMPP packet to the peer.
///
/// \param packet

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

/// Returns the QSslSocket used for this stream.
///

QSslSocket *QXmppStream::socket() const
{
    return d->socket;
}

/// Sets the QSslSocket used for this stream.
///

void QXmppStream::setSocket(QSslSocket *socket)
{
    bool check;
    Q_UNUSED(check);

    d->socket = socket;
    if (!d->socket)
        return;

    // socket events
    check = connect(socket, SIGNAL(connected()),
                    this, SLOT(_q_socketConnected()));
    Q_ASSERT(check);

    check = connect(socket, SIGNAL(encrypted()),
                    this, SLOT(_q_socketEncrypted()));
    Q_ASSERT(check);

    check = connect(socket, SIGNAL(error(QAbstractSocket::SocketError)),
                    this, SLOT(_q_socketError(QAbstractSocket::SocketError)));
    Q_ASSERT(check);

    check = connect(socket, SIGNAL(readyRead()),
                    this, SLOT(_q_socketReadyRead()));
    Q_ASSERT(check);
}

void QXmppStream::_q_socketConnected()
{
    info(QString("Socket connected to %1 %2").arg(
        d->socket->peerAddress().toString(),
        QString::number(d->socket->peerPort())));
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
    warning(QString("Socket error: " + socket()->errorString()));
}

void QXmppStream::_q_socketReadyRead()
{
    d->dataBuffer.append(d->socket->readAll());

    // handle whitespace pings
    if (!d->dataBuffer.isEmpty() && d->dataBuffer.trimmed().isEmpty()) {
        d->dataBuffer.clear();
        handleStanza(QDomElement());
    }

    // FIXME : maybe these QRegExps could be static?
    QRegExp startStreamRegex("^(<\\?xml.*\\?>)?\\s*<stream:stream.*>");
    startStreamRegex.setMinimal(true);
    QRegExp endStreamRegex("</stream:stream>$");
    endStreamRegex.setMinimal(true);

    // check whether we need to add stream start / end elements
    //
    // NOTE: as we may only have partial XML content, do not alter the stream's
    // state until we have a valid XML document!
    QByteArray completeXml = d->dataBuffer;
    const QString strData = QString::fromUtf8(d->dataBuffer);
    bool streamStart = false;
    if (d->streamStart.isEmpty() && strData.contains(startStreamRegex))
        streamStart = true;
    else
        completeXml.prepend(d->streamStart);
    bool streamEnd = false;
    if (strData.contains(endStreamRegex))
        streamEnd = true;
    else
        completeXml.append(streamRootElementEnd);

    // check whether we have a valid XML document
    QDomDocument doc;
    if (!doc.setContent(completeXml, true))
        return;

    // remove data from buffer
    logReceived(strData);
    d->dataBuffer.clear();

    // process stream start
    if (streamStart) {
        d->streamStart = startStreamRegex.cap(0).toUtf8();
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
            if(nodeRecv.tagName() == QLatin1String("message") ||
               nodeRecv.tagName() == QLatin1String("presence") ||
               nodeRecv.tagName() == QLatin1String("iq"))
                ++d->lastIncomingSequenceNumber;
        }
        nodeRecv = nodeRecv.nextSiblingElement();
    }

    // process stream end
    if (streamEnd)
        disconnectFromHost();
}

/// Enables Stream Management acks / reqs (XEP-0198).
///
/// \param resetSequenceNumber Indicates if the sequence numbers should be resetted.
///                            This must be done iff the stream is not resumed.
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

/// Returns the sequence number of the last incoming stanza (XEP-0198).
unsigned QXmppStream::lastIncomingSequenceNumber() const
{
    return d->lastIncomingSequenceNumber;
}

/// Sets the last acknowledged sequence number for outgoing stanzas (XEP-0198).
void QXmppStream::setAcknowledgedSequenceNumber(unsigned sequenceNumber)
{
    for (QMap<unsigned, QByteArray>::iterator it = d->unacknowledgedStanzas.begin(); it != d->unacknowledgedStanzas.end(); ) {
        if (it.key() <= sequenceNumber)
            it = d->unacknowledgedStanzas.erase(it);
        else
            ++it;
    }
}

/// Handles an incoming acknowledgement from XEP-0198.
///
/// \param element
void QXmppStream::handleAcknowledgement(QDomElement &element)
{
    if (!d->streamManagementEnabled)
        return;

    QXmppStreamManagementAck ack;
    ack.parse(element);
    setAcknowledgedSequenceNumber(ack.seqNo());
}

/// Sends an acknowledgement as defined in XEP-0198.
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

/// Sends an acknowledgement request as defined in XEP-0198.
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
