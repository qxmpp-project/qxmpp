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
class QXmppServerExtension;
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

    void addExtension(QXmppServerExtension *extension);
    QList<QXmppServerExtension*> loadedExtensions();

    QString domain() const;
    void setDomain(const QString &domain);

    QXmppLogger *logger();
    void setLogger(QXmppLogger *logger);

    QXmppPasswordChecker *passwordChecker();
    void setPasswordChecker(QXmppPasswordChecker *checker);

    void addCaCertificates(const QString &caCertificates);
    void setLocalCertificate(const QString &sslCertificate);
    void setPrivateKey(const QString &sslKey);

    void close();
    bool listenForClients(const QHostAddress &address = QHostAddress::Any, quint16 port = 5222);
    bool listenForServers(const QHostAddress &address = QHostAddress::Any, quint16 port = 5269);

    bool sendElement(const QDomElement &element);
    bool sendPacket(const QXmppStanza &stanza);

signals:
    /// This signal is emitted when an XMPP stream is added.
    void streamAdded(QXmppStream *stream);

    /// This signal is emitted when an XMPP stream is removed.
    void streamRemoved(QXmppStream *stream);

private slots:
    void slotClientConnection(QSslSocket *socket);
    void slotClientConnected();
    void slotClientDisconnected();
    void slotDialbackRequestReceived(const QXmppDialback &dialback);
    void slotElementReceived(const QDomElement &element);
    void slotServerConnection(QSslSocket *socket);
    void slotServerDisconnected();

private:
    QXmppOutgoingServer *connectToDomain(const QString &domain);
    QList<QXmppStream*> getStreams(const QString &to);
    virtual void handleStanza(QXmppStream *stream, const QDomElement &element);
    virtual QStringList subscribers(const QString &jid);
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
