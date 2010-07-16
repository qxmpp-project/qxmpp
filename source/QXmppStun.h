/*
 * Copyright (C) 2010 Bolloré telecom
 *
 * Author:
 *	Jeremy Lainé
 *
 * Source:
 *	http://code.google.com/p/qxmpp
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

#include "QXmppJingleIq.h"

class QUdpSocket;
class QXmppStunMessage;

/// \brief The QXmppStunSocket class represents an UDP socket capable
/// of performing Interactive Connectivity Establishment (RFC 5245).
///

class QXmppStunSocket : public QObject
{
    Q_OBJECT

public:
    QXmppStunSocket(bool iceControlling, QObject *parent=0);

    QList<QXmppJingleCandidate> localCandidates() const;
    QString localUser() const;
    void setLocalUser(const QString &user);
    QString localPassword() const;
    void setLocalPassword(const QString &password);

    int component() const;
    void setComponent(int component);

    void addRemoteCandidates(const QList<QXmppJingleCandidate> &candidates);
    void setRemoteUser(const QString &user);
    void setRemotePassword(const QString &password);

    void close();
    void connectToHost();
    QIODevice::OpenMode openMode() const;
    qint64 writeDatagram(const QByteArray &datagram);

private slots:
    void slotReadyRead();

signals:
    void datagramReceived(const QByteArray &datagram, const QHostAddress &host, quint16 port);
    void ready();

private:
    void dumpMessage(const QXmppStunMessage &message, bool sent, const QHostAddress &host, quint16 port);

    int m_component;
    QIODevice::OpenMode m_openMode;

    QString m_localUser;
    QString m_localPassword;

    bool m_iceControlling;
    QList<QXmppJingleCandidate> m_remoteCandidates;
    QHostAddress m_remoteHost;
    quint16 m_remotePort;
    QString m_remoteUser;
    QString m_remotePassword;

    QUdpSocket *m_socket;
};

#endif
