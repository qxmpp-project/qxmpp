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

#ifndef QXMPPSTUN_H
#define QXMPPSTUN_H

#include <QObject>

#include "QXmppLogger.h"
#include "QXmppJingleIq.h"

class QDataStream;
class QUdpSocket;
class QTimer;

/// \internal
///
/// The QXmppStunMessage class represents a STUN message.
///

class QXMPP_EXPORT QXmppStunMessage
{
public:
    enum MethodType {
        Binding      = 0x1,
        SharedSecret = 0x2,
        Allocate     = 0x3,
        Refresh      = 0x4,
        Send         = 0x6,
        Data         = 0x7,
        CreatePermission = 0x8,
        ChannelBind  = 0x9,
    };

    enum ClassType {
        Request    = 0x000,
        Indication = 0x010,
        Response   = 0x100,
        Error      = 0x110,
    };

    QXmppStunMessage();

    quint32 cookie() const;
    void setCookie(quint32 cookie);

    QByteArray id() const;
    void setId(const QByteArray &id);

    quint16 messageClass() const;
    quint16 messageMethod() const;

    quint16 type() const;
    void setType(quint16 type);

    // attributes

    quint32 changeRequest() const;
    void setChangeRequest(quint32 changeRequest);

    quint16 channelNumber() const;
    void setChannelNumber(quint16 channelNumber);

    QByteArray data() const;
    void setData(const QByteArray &data);

    quint32 lifetime() const;
    void setLifetime(quint32 changeRequest);

    QByteArray nonce() const;
    void setNonce(const QByteArray &nonce);

    quint32 priority() const;
    void setPriority(quint32 priority);

    QString realm() const;
    void setRealm(const QString &realm);

    QByteArray reservationToken() const;
    void setReservationToken(const QByteArray &reservationToken);

    quint8 requestedTransport() const;
    void setRequestedTransport(quint8 requestedTransport);

    QString software() const;
    void setSoftware(const QString &software);

    QString username() const;
    void setUsername(const QString &username);

    QByteArray encode(const QByteArray &key = QByteArray(), bool addFingerprint = true) const;
    bool decode(const QByteArray &buffer, const QByteArray &key = QByteArray(), QStringList *errors = 0);
    QString toString() const;
    static quint16 peekType(const QByteArray &buffer, quint32 &cookie, QByteArray &id);

    // attributes
    int errorCode;
    QString errorPhrase;
    QByteArray iceControlling;
    QByteArray iceControlled;
    QHostAddress changedHost;
    quint16 changedPort;
    QHostAddress mappedHost;
    quint16 mappedPort;
    QHostAddress otherHost;
    quint16 otherPort;
    QHostAddress sourceHost;
    quint16 sourcePort;
    QHostAddress xorMappedHost;
    quint16 xorMappedPort;
    QHostAddress xorPeerHost;
    quint16 xorPeerPort;
    QHostAddress xorRelayedHost;
    quint16 xorRelayedPort;
    bool useCandidate;

private:
    quint32 m_cookie;
    QByteArray m_id;
    quint16 m_type;

    // attributes
    QSet<quint16> m_attributes;
    quint32 m_changeRequest;
    quint16 m_channelNumber;
    QByteArray m_data;
    quint32 m_lifetime;
    QByteArray m_nonce;
    quint32 m_priority;
    QString m_realm;
    quint8 m_requestedTransport;
    QByteArray m_reservationToken;
    QString m_software;
    QString m_username;
};

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

/// \internal
///
/// The QXmppTurnAllocation class represents a TURN allocation as defined
/// by RFC 5766 Traversal Using Relays around NAT (TURN).
///

class QXMPP_EXPORT QXmppTurnAllocation : public QXmppLoggable
{
    Q_OBJECT

public:
    enum AllocationState
    {
        UnconnectedState,
        ConnectingState,
        ConnectedState,
        ClosingState,
    };

    QXmppTurnAllocation(QObject *parent = 0);
    ~QXmppTurnAllocation();

    QHostAddress relayedHost() const;
    quint16 relayedPort() const;
    AllocationState state() const;

    void setServer(const QHostAddress &host, quint16 port = 3478);
    void setUser(const QString &user);
    void setPassword(const QString &password);

    qint64 writeDatagram(const QByteArray &data, const QHostAddress &host, quint16 port);

signals:
    /// \brief This signal is emitted once TURN allocation succeeds.
    void connected();

    /// \brief This signal is emitted when a data packet is received.
    void datagramReceived(const QByteArray &data, const QHostAddress &host, quint16 port);

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

/// \brief The QXmppIceComponent class represents a piece of a media stream
/// requiring a single transport address, as defined by RFC 5245
/// (Interactive Connectivity Establishment).

class QXMPP_EXPORT QXmppIceComponent : public QXmppLoggable
{
    Q_OBJECT

public:
    QXmppIceComponent(QObject *parent=0);
    ~QXmppIceComponent();
    void setIceControlling(bool controlling);
    void setStunServer(const QHostAddress &host, quint16 port);
    void setTurnServer(const QHostAddress &host, quint16 port);
    void setTurnUser(const QString &user);
    void setTurnPassword(const QString &password);

    QList<QXmppJingleCandidate> localCandidates() const;
    void setLocalUser(const QString &user);
    void setLocalPassword(const QString &password);

    int component() const;
    void setComponent(int component);

    bool addRemoteCandidate(const QXmppJingleCandidate &candidate);
    void setRemoteUser(const QString &user);
    void setRemotePassword(const QString &password);

    bool isConnected() const;
    void setSockets(QList<QUdpSocket*> sockets);

    static QList<QHostAddress> discoverAddresses();
    static QList<QUdpSocket*> reservePorts(const QList<QHostAddress> &addresses, int count, QObject *parent = 0);

public slots:
    void close();
    void connectToHost();
    qint64 sendDatagram(const QByteArray &datagram);

private slots:
    void checkCandidates();
    void checkStun();
    void handleDatagram(const QByteArray &datagram, const QHostAddress &host, quint16 port, QUdpSocket *socket = 0);
    void readyRead();
    void turnConnected();

signals:
    /// \brief This signal is emitted once ICE negotiation succeeds.
    void connected();

    /// \brief This signal is emitted when a data packet is received.
    void datagramReceived(const QByteArray &datagram);

    /// \brief This signal is emitted when the list of local candidates changes.
    void localCandidatesChanged();

private:
    class Pair {
    public:
        Pair(int component, bool controlling);
        quint64 priority() const;
        QString toString() const;

        QIODevice::OpenMode checked;
        QXmppJingleCandidate remote;
        QXmppJingleCandidate reflexive;
        QByteArray transaction;
        QUdpSocket *socket;

    private:
        int m_component;
        bool m_controlling;
    };

    Pair *addRemoteCandidate(QUdpSocket *socket, const QHostAddress &host, quint16 port, quint32 priority);
    qint64 writeStun(const QXmppStunMessage &message, QXmppIceComponent::Pair *pair);

    int m_component;

    QList<QXmppJingleCandidate> m_localCandidates;
    QString m_localUser;
    QString m_localPassword;

    Pair *m_activePair;
    Pair *m_fallbackPair;
    bool m_iceControlling;
    QList<Pair*> m_pairs;
    quint32 m_peerReflexivePriority;
    QString m_remoteUser;
    QString m_remotePassword;

    QList<QUdpSocket*> m_sockets;
    QTimer *m_timer;

    // STUN server
    QByteArray m_stunId;
    QHostAddress m_stunHost;
    quint16 m_stunPort;
    QTimer *m_stunTimer;
    int m_stunTries;

    // TURN server
    QXmppTurnAllocation *m_turnAllocation;
    bool m_turnConfigured;
};

/// \brief The QXmppIceConnection class represents a set of UDP sockets
/// capable of performing Interactive Connectivity Establishment (RFC 5245).
///

class QXMPP_EXPORT QXmppIceConnection : public QXmppLoggable
{
    Q_OBJECT

public:
    QXmppIceConnection(QObject *parent = 0);

    QXmppIceComponent *component(int component);
    void addComponent(int component);
    void setIceControlling(bool controlling);

    QList<QXmppJingleCandidate> localCandidates() const;
    QString localUser() const;
    void setLocalUser(const QString &user);
    QString localPassword() const;
    void setLocalPassword(const QString &password);

    void addRemoteCandidate(const QXmppJingleCandidate &candidate);
    void setRemoteUser(const QString &user);
    void setRemotePassword(const QString &password);

    void setStunServer(const QHostAddress &host, quint16 port = 3478);
    void setTurnServer(const QHostAddress &host, quint16 port = 3478);
    void setTurnUser(const QString &user);
    void setTurnPassword(const QString &password);

    bool bind(const QList<QHostAddress> &addresses);
    bool isConnected() const;

signals:
    /// \brief This signal is emitted once ICE negotiation succeeds.
    void connected();

    /// \brief This signal is emitted when ICE negotiation fails.
    void disconnected();

    /// \brief This signal is emitted when the list of local candidates changes.
    void localCandidatesChanged();

public slots:
    void close();
    void connectToHost();

private slots:
    void slotConnected();
    void slotTimeout();

private:
    QTimer *m_connectTimer;
    bool m_iceControlling;
    QMap<int, QXmppIceComponent*> m_components;
    QString m_localUser;
    QString m_localPassword;
    QHostAddress m_stunHost;
    quint16 m_stunPort;
    QHostAddress m_turnHost;
    quint16 m_turnPort;
    QString m_turnUser;
    QString m_turnPassword;
};

#endif
