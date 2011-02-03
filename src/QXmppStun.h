/*
 * Copyright (C) 2008-2011 The QXmpp developers
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

/// \internal
///
/// The QXmppStunMessage class represents a STUN message.
///

class QXmppStunMessage
{
public:
    enum MethodType {
        Binding      = 0x1,
        SharedSecret = 0x2,
        Allocate     = 0x3,
    };

    enum MessageType {
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

    quint16 type() const;
    void setType(quint16 type);

    // attributes

    quint32 changeRequest() const;
    void setChangeRequest(quint32 changeRequest);

    quint32 priority() const;
    void setPriority(quint32 priority);

    QString software() const;
    void setSoftware(const QString &software);

    QByteArray encode(const QString &password = QString(), bool addFingerprint = true) const;
    bool decode(const QByteArray &buffer, const QString &password = QString(), QStringList *errors = 0);
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
    QString username;
    bool useCandidate;

private:
    quint32 m_cookie;
    QByteArray m_id;
    quint16 m_type;

    QSet<quint16> m_attributes;
    quint32 m_changeRequest;
    quint32 m_priority;
    QString m_software;
};

/// \brief The QXmppIceComponent class represents a piece of a media stream
/// requiring a single transport address, as defined by RFC 5245
/// (Interactive Connectivity Establishment).

class QXmppIceComponent : public QXmppLoggable
{
    Q_OBJECT

public:
    QXmppIceComponent(bool controlling, QObject *parent=0);
    ~QXmppIceComponent();
    void setStunServer(const QHostAddress &host, quint16 port);

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
    void readyRead();

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
    qint64 writeStun(const QXmppStunMessage &message, QXmppIceComponent::Pair *pair);

    int m_component;

    QList<QXmppJingleCandidate> m_localCandidates;
    QString m_localUser;
    QString m_localPassword;

    Pair *m_activePair;
    Pair *m_fallbackPair;
    bool m_iceControlling;
    QList<Pair*> m_pairs;
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
};

/// \brief The QXmppIceConnection class represents a set of UDP sockets
/// capable of performing Interactive Connectivity Establishment (RFC 5245).
///

class QXmppIceConnection : public QXmppLoggable
{
    Q_OBJECT

public:
    QXmppIceConnection(bool controlling, QObject *parent = 0);

    QXmppIceComponent *component(int component);
    void addComponent(int component);

    QList<QXmppJingleCandidate> localCandidates() const;
    QString localUser() const;
    QString localPassword() const;

    void addRemoteCandidate(const QXmppJingleCandidate &candidate);
    void setRemoteUser(const QString &user);
    void setRemotePassword(const QString &password);

    void setStunServer(const QHostAddress &host, quint16 port = 3478);

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
    bool m_controlling;
    QMap<int, QXmppIceComponent*> m_components;
    QString m_localUser;
    QString m_localPassword;
    QHostAddress m_stunHost;
    quint16 m_stunPort;
};

#endif
