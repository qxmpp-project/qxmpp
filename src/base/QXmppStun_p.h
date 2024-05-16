// SPDX-FileCopyrightText: 2015 Jeremy Lainé <jeremy.laine@m4x.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

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

//
// The QXmppStunTransaction class represents a STUN transaction.
//
class QXMPP_EXPORT QXmppStunTransaction : public QXmppLoggable
{
    Q_OBJECT

public:
    QXmppStunTransaction(const QXmppStunMessage &request, QObject *parent);
    QXmppStunMessage request() const;
    QXmppStunMessage response() const;

Q_SIGNALS:
    void finished();
    void writeStun(const QXmppStunMessage &request);

public Q_SLOTS:
    void readStun(const QXmppStunMessage &response);

private Q_SLOTS:
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
    QXmppIceTransport(QObject *parent = nullptr);
    ~QXmppIceTransport() override;

    virtual QXmppJingleCandidate localCandidate(int component) const = 0;
    virtual qint64 writeDatagram(const QByteArray &data, const QHostAddress &host, quint16 port) = 0;

public Q_SLOTS:
    virtual void disconnectFromHost() = 0;

Q_SIGNALS:
    /// \brief This signal is emitted when a data packet is received.
    void datagramReceived(const QByteArray &data, const QHostAddress &host, quint16 port);
};

//
// The QXmppTurnAllocation class represents a TURN allocation as defined
// by RFC 5766 Traversal Using Relays around NAT (TURN).
//
class QXMPP_EXPORT QXmppTurnAllocation : public QXmppIceTransport
{
    Q_OBJECT

public:
    enum AllocationState {
        UnconnectedState,
        ConnectingState,
        ConnectedState,
        ClosingState
    };

    QXmppTurnAllocation(QObject *parent = nullptr);
    ~QXmppTurnAllocation() override;

    QHostAddress relayedHost() const;
    quint16 relayedPort() const;
    AllocationState state() const;

    void setServer(const QHostAddress &host, quint16 port = 3478);
    void setUser(const QString &user);
    void setPassword(const QString &password);

    QXmppJingleCandidate localCandidate(int component) const override;
    qint64 writeDatagram(const QByteArray &data, const QHostAddress &host, quint16 port) override;

Q_SIGNALS:
    /// \brief This signal is emitted once TURN allocation succeeds.
    void connected();

    /// \brief This signal is emitted when TURN allocation fails.
    void disconnected();

public Q_SLOTS:
    void connectToHost();
    void disconnectFromHost() override;

private Q_SLOTS:
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
    QList<QXmppStunTransaction *> m_transactions;
};

//
// The QXmppUdpTransport class represents a UDP transport.
//
class QXMPP_EXPORT QXmppUdpTransport : public QXmppIceTransport
{
    Q_OBJECT

public:
    QXmppUdpTransport(QUdpSocket *socket, QObject *parent = nullptr);
    ~QXmppUdpTransport() override;

    QXmppJingleCandidate localCandidate(int component) const override;
    qint64 writeDatagram(const QByteArray &data, const QHostAddress &host, quint16 port) override;

public Q_SLOTS:
    void disconnectFromHost() override;

private Q_SLOTS:
    void readyRead();

private:
    QUdpSocket *m_socket;
};

#endif
