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

#ifndef QXMPPSERVER_H
#define QXMPPSERVER_H

#include <QTcpServer>

class QDomElement;
class QSslSocket;

class QXmppDialback;
class QXmppLogger;
class QXmppOutgoingServer;
class QXmppPasswordChecker;
class QXmppServerPrivate;
class QXmppSslServer;
class QXmppStanza;
class QXmppStream;

/// \brief The QXmppServer class represents an XMPP server.
///
/// \ingroup Core

class QXmppServer : public QObject
{
    Q_OBJECT

public:
    QXmppServer(QObject *parent = 0);
    ~QXmppServer();

    QString domain() const;
    void setDomain(const QString &domain);

    QXmppLogger *logger();
    void setLogger(QXmppLogger *logger);

    QXmppPasswordChecker *passwordChecker();
    void setPasswordChecker(QXmppPasswordChecker *checker);

    void addCaCertificates(const QString &caCertificates);
    void setLocalCertificate(const QString &sslCertificate);
    void setPrivateKey(const QString &sslKey);

    bool listenForClients(const QHostAddress &address = QHostAddress::Any, quint16 port = 5222);
    bool listenForServers(const QHostAddress &address = QHostAddress::Any, quint16 port = 5269);

    bool sendElement(const QDomElement &element);
    bool sendPacket(const QXmppStanza &stanza);

private slots:
    void slotClientConnection(QSslSocket *socket);
    void slotClientConnected();
    void slotClientDisconnected();
    void slotDialbackRequestReceived(const QXmppDialback &dialback);
    void slotElementReceived(const QDomElement &element, bool &handled);
    void slotServerConnection(QSslSocket *socket);

protected:
    QXmppOutgoingServer *connectToDomain(const QString &domain);
    // overridable methods
    virtual void handleStanza(QXmppStream *stream, const QDomElement &element);
    virtual void updateStatistics();

private:
    QList<QXmppStream*> getStreams(const QString &to);
    QXmppServerPrivate * const d;
};

class QXmppSslServerPrivate;

/// \brief The QXmppSslServer class represents an SSL-enabled TCP server.
///

class QXmppSslServer : public QTcpServer
{
    Q_OBJECT

public:
    QXmppSslServer(QObject *parent = 0);
    ~QXmppSslServer();

    void addCaCertificates(const QString &caCertificates);
    void setLocalCertificate(const QString &localCertificate);
    void setPrivateKey(const QString &privateKey);

signals:
    void newConnection(QSslSocket *socket);

private:
    void incomingConnection(int socketDescriptor);
    QXmppSslServerPrivate * const d;
};

#endif
