/*
 * Copyright (C) 2010 Sjors Gielen, Rob ten Berge
 *
 * Authors:
 *  Sjors Gielen
 *  Rob ten Berge
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

#ifndef QXMPPSERVER_H
#define QXMPPSERVER_H

#include <QObject>
#include <QTcpServer>
#include <QSslKey>
#include <QSslCertificate>
#include <QSslSocket>

#include "QXmppConfiguration.h"
#include "QXmppPresence.h"
#include "QXmppClient.h"
#include "QXmppLogger.h"

class QXmppServerConnection;
class QXmppClientServer;

/** @brief The QXmppServer class is the server class of QXmpp.
 *
 * This class listens on a TCP port and accepts clients and/or servers (depending on
 * its configuration). It proceeds to create a QXmppServerConnection class if the
 * other peer is a server, or a QXmppClientServer class if the other peer is a client.
 */

class QXmppServer : public QTcpServer
{
    Q_OBJECT

public:
    QXmppServer(bool acceptsClients, bool acceptsServers, QObject *parent = 0);
    ~QXmppServer();

    QXmppLogger *logger();
    void setLogger(QXmppLogger *logger);

    void setAcceptsClients( bool acceptsClients );
    void setAcceptsServers( bool acceptsServers );
    bool acceptsClients() const;
    bool acceptsServers() const;

    void addCaCertificates( const QList<QSslCertificate> &certificates );
    void addCaCertificates( const QSslCertificate &certificate );
    void setLocalCertificate( const QSslCertificate &localCertificate );
    void setPrivateKey( const QSslKey &privateKey );

signals:
    void unknownClientConnected( QSslSocket *client );
    void serverConnected( QXmppServerConnection *server );
    void clientConnected( QXmppClientServer *client );

private slots:
    void determineSocketType();
    void forgetSocket( QSslSocket *socket );
    void closeSocket( QSslSocket *socket );
    bool tryOtherProtocol( QSslSocket *socket );

private:
    QList<QXmppServerConnection*> m_serverConnections;
    QList<QXmppClientServer*>     m_clientConnections;
    QList<QSslSocket*>            m_unknownClients;
    QHash<QSslSocket*, QXmlStreamReader*> m_streamReaders;
    QHash<QSslSocket*, QByteArray> m_cache;

    QXmppLogger* m_logger;
    QList<QSslCertificate> m_certificates;
    QSslCertificate m_localCertificate;
    QSslKey m_privateKey;
    bool m_acceptsClients;
    bool m_acceptsServers;

    void incomingConnection(int socketDescriptor);
};

#endif // QXMPPCLIENT_H
