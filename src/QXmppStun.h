/*
 * Copyright (C) 2008-2010 The QXmpp developers
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

#ifndef QXMPPSTUN_H
#define QXMPPSTUN_H

#include <QObject>

#include "QXmppLogger.h"
#include "QXmppJingleIq.h"

class QDataStream;
class QUdpSocket;
class QTimer;

/// \brief The QXmppStunMessage class represents a STUN message.
///

class QXmppStunMessage
{
public:
    QXmppStunMessage();

    QByteArray id() const;
    void setId(const QByteArray &id);

    quint16 type() const;
    void setType(quint16 type);

    QByteArray encode(const QString &password = QString(), bool addFingerprint = true) const;
    bool decode(const QByteArray &buffer, const QString &password = QString(), QStringList *errors = 0);
    QString toString() const;
    static quint16 peekType(const QByteArray &buffer, QByteArray &id);

    // attributes
    int errorCode;
    QString errorPhrase;
    quint32 priority;
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
    QString software;
    QString username;
    bool useCandidate;

private:
    QByteArray m_id;
    quint16 m_type;
};

class QXmppStunSocket : public QXmppLoggable
{
    Q_OBJECT

public:
    QXmppStunSocket(bool iceControlling, QObject *parent=0);
    ~QXmppStunSocket();
    void setStunServer(const QHostAddress &host, quint16 port);

    QList<QXmppJingleCandidate> localCandidates() const;
    void setLocalUser(const QString &user);
    void setLocalPassword(const QString &password);

    int component() const;
    void setComponent(int component);

    bool addRemoteCandidate(const QXmppJingleCandidate &candidate);
    void setRemoteUser(const QString &user);
    void setRemotePassword(const QString &password);

    bool bind();
    void close();
    void connectToHost();
    bool isConnected() const;
    qint64 writeDatagram(const QByteArray &datagram);

private slots:
    void checkCandidates();
    void readyRead();

signals:
    // This signal is emitted once ICE negotiation succeeds.
    void connected();

    // This signal is emitted when a data packet is received.
    void datagramReceived(const QByteArray &datagram);

    // This signal is emitted when the list of local candidates changes.
    void localCandidatesChanged();

private:
    class Pair {
    public:
        Pair();
        QString toString() const;

        QIODevice::OpenMode checked;
        quint32 priority;
        QXmppJingleCandidate remote;
        QXmppJingleCandidate reflexive;
        QByteArray transaction;
        QUdpSocket *socket;
    };

    Pair *addRemoteCandidate(QUdpSocket *socket, const QHostAddress &host, quint16 port);
    qint64 writeStun(const QXmppStunMessage &message, QXmppStunSocket::Pair *pair);

    int m_component;

    QList<QXmppJingleCandidate> m_localCandidates;
    QString m_localUser;
    QString m_localPassword;

    Pair *m_activePair;
    bool m_iceControlling;
    QList<Pair*> m_pairs;
    QString m_remoteUser;
    QString m_remotePassword;

    QList<QUdpSocket*> m_sockets;
    QTimer *m_timer;

    // STUN server
    bool m_stunDone;
    QByteArray m_stunId;
    QHostAddress m_stunHost;
    quint16 m_stunPort;
};

/// \brief The QXmppIceConnection class represents an UDP socket capable
/// of performing Interactive Connectivity Establishment (RFC 5245).
///

class QXmppIceConnection : public QXmppLoggable
{
    Q_OBJECT

public:
    QXmppIceConnection(bool controlling, QObject *parent = 0);
    void addComponent(int component);
    void setStunServer(const QString &hostName, quint16 port = 3478);

    QList<QXmppJingleCandidate> localCandidates() const;
    QString localUser() const;
    QString localPassword() const;

    void addRemoteCandidate(const QXmppJingleCandidate &candidate);
    void setRemoteUser(const QString &user);
    void setRemotePassword(const QString &password);

    void close();
    void connectToHost();
    bool isConnected() const;
    qint64 writeDatagram(int, const QByteArray &datagram);

signals:
    // This signal is emitted once ICE negotiation succeeds.
    void connected();

    // This signal is emitted when ICE negotiation fails.
    void disconnected();

    // This signal is emitted when a data packet is received.
    void datagramReceived(int component, const QByteArray &datagram);

    // This signal is emitted when the list of local candidates changes.
    void localCandidatesChanged();

private slots:
    void slotConnected();
    void slotDatagramReceived(const QByteArray &datagram);
    void slotTimeout();

private:
    QTimer *m_connectTimer;
    bool m_controlling;
    QMap<int, QXmppStunSocket*> m_components;
    QString m_localUser;
    QString m_localPassword;
    QHostAddress m_stunHost;
    quint16 m_stunPort;
};

#endif
