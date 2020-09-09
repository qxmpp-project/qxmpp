/*
 * Copyright (C) 2008-2021 The QXmpp developers
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

#include "QXmppLogger.h"

#include <variant>

#include <QAbstractSocket>
#include <QObject>

class QDomElement;
template<typename T>
class QFuture;
class QSslSocket;
class QXmppIq;
class QXmppPacket;
class QXmppStanza;
class QXmppStreamPrivate;

///
/// \brief The QXmppStream class is the base class for all XMPP streams.
///
class QXMPP_EXPORT QXmppStream : public QXmppLoggable
{
    Q_OBJECT

public:
    QXmppStream(QObject *parent);
    ~QXmppStream() override;

    virtual bool isConnected() const;

    bool sendPacket(const QXmppStanza &);
    QFuture<QXmpp::PacketState> send(const QXmppStanza &);

    using IqResult = std::variant<QDomElement, QXmpp::PacketState>;
    QFuture<IqResult> sendIq(const QXmppIq &);
    void cancelOngoingIqs();

    void resetPacketCache();

Q_SIGNALS:
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

    // XEP-0198: Stream Management
    void enableStreamManagement(bool resetSequenceNumber);
    unsigned int lastIncomingSequenceNumber() const;
    void setAcknowledgedSequenceNumber(unsigned int sequenceNumber);

public Q_SLOTS:
    virtual void disconnectFromHost();
    virtual bool sendData(const QByteArray &);

private Q_SLOTS:
    void _q_socketConnected();
    void _q_socketEncrypted();
    void _q_socketError(QAbstractSocket::SocketError error);
    void _q_socketReadyRead();

private:
    friend class QXmppStreamManager;
    friend class tst_QXmppStream;

    void processData(const QString &data);
    void sendPacket(QXmppPacket &packet);
    bool handleIqResponse(const QDomElement &);

    QXmppStreamPrivate *const d;
};

#endif  // QXMPPSTREAM_H
