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


#ifndef QXMPPSTREAM_H
#define QXMPPSTREAM_H

#include <QAbstractSocket>
#include <QObject>
#include "QXmppLogger.h"

class QDomElement;
class QSslSocket;
class QXmppStanza;
class QXmppStreamPrivate;

/// \brief The QXmppStream class is the base class for all XMPP streams.
///

class QXMPP_EXPORT QXmppStream : public QXmppLoggable
{
    Q_OBJECT

public:
    QXmppStream(QObject *parent);
    ~QXmppStream();

    virtual bool isConnected() const;
    bool sendPacket(const QXmppStanza&);

signals:
    /// This signal is emitted when the stream is connected.
    void connected();

    /// This signal is emitted when the stream is disconnected.
    void disconnected();

protected:
    // Access to underlying socket
    QSslSocket *socket() const;
    void setSocket(QSslSocket *socket);

    // Overridable methods
    virtual void handleStart();

    /// Handles an incoming XMPP stanza.
    ///
    /// \param element
    virtual void handleStanza(const QDomElement &element) = 0;

    /// Handles an incoming XMPP stream start.
    ///
    /// \param element
    virtual void handleStream(const QDomElement &element) = 0;

    /// Enables Stream Management acks / reqs (XEP-0198).
    ///
    /// \param resetSeqno Indicates if the sequence numbers should be resetted.
    ///                   This must be done iff the stream is not resumed.
    void enableStreamManagement(bool resetSequenceNumber);

    /// Returns the sequence number of the last incoming stanza (XEP-0198).
    unsigned lastIncomingSequenceNumber() const;

    /// Sets the last acknowledged sequence number for outgoing stanzas (XEP-0198).
    void setAcknowledgedSequenceNumber(unsigned sequenceNumber);

private:
    /// Handles an incoming acknowledgement from XEP-0198.
    ///
    /// \param element
    void handleAcknowledgement(QDomElement &element);

    /// Sends an acknowledgement as defined in XEP-0198.
    void sendAcknowledgement();

    /// Sends an acknowledgement request as defined in XEP-0198.
    void sendAcknowledgementRequest();

public slots:
    virtual void disconnectFromHost();
    virtual bool sendData(const QByteArray&);

private slots:
    void _q_socketConnected();
    void _q_socketEncrypted();
    void _q_socketError(QAbstractSocket::SocketError error);
    void _q_socketReadyRead();

private:
    QXmppStreamPrivate * const d;
};

#endif // QXMPPSTREAM_H
