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


#include "QXmppConstants.h"
#include "QXmppLogger.h"
#include "QXmppStanza.h"
#include "QXmppStream.h"
#include "QXmppUtils.h"

#include <QBuffer>
#include <QDomDocument>
#include <QHostAddress>
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

    // stream state
    QByteArray streamStart;
};

QXmppStreamPrivate::QXmppStreamPrivate()
    : socket(0)
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
    sendData(streamRootElementEnd);
    if (d->socket)
    {
        d->socket->flush();
        d->socket->disconnectFromHost();
    }
}

/// Handles a stream start event, which occurs when the underlying transport
/// becomes ready (socket connected, encryption started).
///
/// If you redefine handleStart(), make sure to call the base class's method.

void QXmppStream::handleStart()
{
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

    // send packet
    return sendData(data);
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
    QByteArray completeXml = d->dataBuffer;
    const QString strData = QString::fromUtf8(d->dataBuffer);
    bool streamStart = false;
    if (d->streamStart.isEmpty() && strData.contains(startStreamRegex))
        streamStart = true;
    else
        completeXml.prepend(d->streamStart);
    if (!strData.contains(endStreamRegex))
        completeXml.append(streamRootElementEnd);

    // check whether we have a valid XML document
    QDomDocument doc;
    if (!doc.setContent(completeXml, true))
        return;

    // remove data from buffer
    logReceived(strData);
    d->dataBuffer.clear();
    if (streamStart)
        d->streamStart = startStreamRegex.cap(0).toUtf8();

    // process stream start
    if (streamStart)
        handleStream(doc.documentElement());

    // process stanzas
    QDomElement nodeRecv = doc.documentElement().firstChildElement();
    while (!nodeRecv.isNull()) {
        handleStanza(nodeRecv);
        nodeRecv = nodeRecv.nextSiblingElement();
    }
}


