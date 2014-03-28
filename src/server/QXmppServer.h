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

#ifndef QXMPPSERVER_H
#define QXMPPSERVER_H

#include <QTcpServer>
#include <QVariantMap>

#include "QXmppLogger.h"

class QDomElement;
class QSslCertificate;
class QSslKey;
class QSslSocket;

class QXmppDialback;
class QXmppIncomingClient;
class QXmppOutgoingServer;
class QXmppPasswordChecker;
class QXmppPresence;
class QXmppServerExtension;
class QXmppServerPrivate;
class QXmppSslServer;
class QXmppStanza;
class QXmppStream;

/// \brief The QXmppServer class represents an XMPP server.
///
/// It provides support for both client-to-server and server-to-server
/// communications, SSL encryption and logging facilities.
///
/// QXmppServer comes with a number of modules for service discovery,
/// XMPP ping, statistics and file transfer proxy support. You can write
/// your own extensions for QXmppServer by subclassing QXmppServerExtension.
///
/// \ingroup Core

class QXMPP_EXPORT QXmppServer : public QXmppLoggable
{
    Q_OBJECT
    Q_PROPERTY(QXmppLogger* logger READ logger WRITE setLogger NOTIFY loggerChanged)

public:
    QXmppServer(QObject *parent = 0);
    ~QXmppServer();

    void addExtension(QXmppServerExtension *extension);
    QList<QXmppServerExtension*> extensions();

    QString domain() const;
    void setDomain(const QString &domain);

    QXmppLogger *logger();
    void setLogger(QXmppLogger *logger);

    QXmppPasswordChecker *passwordChecker();
    void setPasswordChecker(QXmppPasswordChecker *checker);

    QVariantMap statistics() const;

    void addCaCertificates(const QString &caCertificates);
    void setLocalCertificate(const QString &path);
    void setPrivateKey(const QString &path);

    void close();
    bool listenForClients(const QHostAddress &address = QHostAddress::Any, quint16 port = 5222);
    bool listenForServers(const QHostAddress &address = QHostAddress::Any, quint16 port = 5269);

    bool sendElement(const QDomElement &element);
    bool sendPacket(const QXmppStanza &stanza);

    void addIncomingClient(QXmppIncomingClient *stream);

signals:
    /// This signal is emitted when a client has connected.
    void clientConnected(const QString &jid);

    /// This signal is emitted when a client has disconnected.
    void clientDisconnected(const QString &jid);

    /// This signal is emitted when the logger changes.
    void loggerChanged(QXmppLogger *logger);

public slots:
    void handleElement(const QDomElement &element);

private slots:
    void _q_clientConnection(QSslSocket *socket);
    void _q_clientConnected();
    void _q_clientDisconnected();
    void _q_dialbackRequestReceived(const QXmppDialback &dialback);
    void _q_outgoingServerDisconnected();
    void _q_serverConnection(QSslSocket *socket);
    void _q_serverDisconnected();

private:
    friend class QXmppServerPrivate;
    QXmppServerPrivate *d;
};

class QXmppSslServerPrivate;

/// \brief The QXmppSslServer class represents an SSL-enabled TCP server.
///

class QXMPP_EXPORT QXmppSslServer : public QTcpServer
{
    Q_OBJECT

public:
    QXmppSslServer(QObject *parent = 0);
    ~QXmppSslServer();

    void addCaCertificates(const QList<QSslCertificate> &certificates);
    void setLocalCertificate(const QSslCertificate &certificate);
    void setPrivateKey(const QSslKey &key);

signals:
    /// This signal is emitted when a new connection is established.
    void newConnection(QSslSocket *socket);

private:
    #if QT_VERSION < 0x050000
    void incomingConnection(int socketDescriptor);
    #else
    void incomingConnection(qintptr socketDescriptor);
    #endif
    QXmppSslServerPrivate * const d;
};

#endif
