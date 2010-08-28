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

#ifndef QXMPP_SERVER_PROXY65_H
#define QXMPP_SERVER_PROXY65_H

#include <QTime>

#include "QXmppServerExtension.h"

class QTcpSocket;

class QTcpSocketPair : public QObject
{
    Q_OBJECT

public:
    QTcpSocketPair(const QString &hash);

    void activate();
    void addSocket(QTcpSocket *socket);

    QString key;
    QTime time;
    qint64 transfer;

signals:
    void finished();

private slots:
    void disconnected();
    void sendData();

private:
    QTcpSocket *target;
    QTcpSocket *source;
};

class QXmppServerProxy65Private;

/// \brief QXmppServer extension for XEP-0065: SOCKS5 Bytestreams.
///

class QXmppServerProxy65 : public QXmppServerExtension
{
    Q_OBJECT
    Q_CLASSINFO("ExtensionName", "proxy65");
    Q_PROPERTY(QString jid READ jid WRITE setJid);
    Q_PROPERTY(QString statisticsFile READ statisticsFile WRITE setStatisticsFile);

public:
    QXmppServerProxy65();
    ~QXmppServerProxy65();

    QString jid() const;
    void setJid(const QString &jid);

    QString statisticsFile() const;
    void setStatisticsFile(const QString &statisticsFile);

    QStringList discoveryItems() const;
    bool handleStanza(QXmppStream *stream, const QDomElement &element);
    bool start();
    void stop();

private slots:
    void slotPairFinished();
    void slotSocketConnected(QTcpSocket *socket, const QString &hostName, quint16 port);
    void slotUpdateStatistics();

private:
    QXmppServerProxy65Private * const d;
};

#endif
