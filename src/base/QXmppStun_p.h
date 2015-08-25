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

#ifndef QXMPPSTUN_P_H
#define QXMPPSTUN_P_H

#include "QXmppStun.h"

class QUdpSocket;
class QTimer;

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QXmpp API.
//
// This header file may change from version to version without notice,
// or even be removed.
//
// We mean it.
//

/// \internal
///
/// The QXmppStunTransaction class represents a STUN transaction.
///

class QXMPP_EXPORT QXmppStunTransaction : public QXmppLoggable
{
    Q_OBJECT

public:
    QXmppStunTransaction(const QXmppStunMessage &request, QObject *parent);
    QXmppStunMessage request() const;
    QXmppStunMessage response() const;

signals:
    void finished();
    void writeStun(const QXmppStunMessage &request);

public slots:
    void readStun(const QXmppStunMessage &response);

private slots:
    void retry();

private:
    QXmppStunMessage m_request;
    QXmppStunMessage m_response;
    QTimer *m_retryTimer;
    int m_tries;
};

class QXMPP_EXPORT QXmppIceTransport : public QXmppLoggable
{
    Q_OBJECT

public:
    QXmppIceTransport(QObject *parent = 0);
    ~QXmppIceTransport();

    virtual QXmppJingleCandidate localCandidate(int component) const = 0;
    virtual qint64 writeDatagram(const QByteArray &data, const QHostAddress &host, quint16 port) = 0;

public slots:
    virtual void disconnectFromHost() = 0;

signals:
    /// \brief This signal is emitted when a data packet is received.
    void datagramReceived(const QByteArray &data, const QHostAddress &host, quint16 port);
};

/// \internal
///
/// The QXmppTurnAllocation class represents a TURN allocation as defined
/// by RFC 5766 Traversal Using Relays around NAT (TURN).
///

class QXMPP_EXPORT QXmppTurnAllocation : public QXmppIceTransport
{
    Q_OBJECT

public:
    enum AllocationState
    {
        UnconnectedState,
        ConnectingState,
        ConnectedState,
        ClosingState
    };

    QXmppTurnAllocation(QObject *parent = 0);
    ~QXmppTurnAllocation();

    QHostAddress relayedHost() const;
    quint16 relayedPort() const;
    AllocationState state() const;

    void setServer(const QHostAddress &host, quint16 port = 3478);
    void setUser(const QString &user);
    void setPassword(const QString &password);

    QXmppJingleCandidate localCandidate(int component) const;
    qint64 writeDatagram(const QByteArray &data, const QHostAddress &host, quint16 port);

signals:
    /// \brief This signal is emitted once TURN allocation succeeds.
    void connected();

    /// \brief This signal is emitted when TURN allocation fails.
    void disconnected();

public slots:
    void connectToHost();
    void disconnectFromHost();

private slots:
    void readyRead();
    void refresh();
    void refreshChannels();
    void transactionFinished();
    void writeStun(const QXmppStunMessage &message);

private:
    void handleDatagram(const QByteArray &datagram, const QHostAddress &host, quint16 port);
    void setState(AllocationState state);

    QUdpSocket *socket;
    QTimer *m_timer;
    QTimer *m_channelTimer;
    QString m_password;
    QString m_username;
    QHostAddress m_relayedHost;
    quint16 m_relayedPort;
    QHostAddress m_turnHost;
    quint16 m_turnPort;

    // channels
    typedef QPair<QHostAddress, quint16> Address;
    quint16 m_channelNumber;
    QMap<quint16, Address> m_channels;

    // state
    quint32 m_lifetime;
    QByteArray m_key;
    QString m_realm;
    QByteArray m_nonce;
    AllocationState m_state;
    QList<QXmppStunTransaction*> m_transactions;
};

/// \internal
///
/// The QXmppUdpTransport class represents a UDP transport.
///

class QXMPP_EXPORT QXmppUdpTransport : public QXmppIceTransport
{
    Q_OBJECT

public:
    QXmppUdpTransport(QUdpSocket *socket, QObject *parent = 0);
    ~QXmppUdpTransport();

    QXmppJingleCandidate localCandidate(int component) const;
    qint64 writeDatagram(const QByteArray &data, const QHostAddress &host, quint16 port);

public slots:
    void disconnectFromHost();

private slots:
    void readyRead();

private:
    QUdpSocket *m_socket;
};

#endif
