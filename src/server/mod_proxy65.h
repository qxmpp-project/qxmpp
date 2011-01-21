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

#ifndef QXMPP_SERVER_PROXY65_H
#define QXMPP_SERVER_PROXY65_H

#include <QStringList>
#include <QTime>

#include "QXmppServerExtension.h"

class QTcpSocket;

class QTcpSocketPair : public QXmppLoggable
{
    Q_OBJECT

public:
    QTcpSocketPair(const QString &hash, QObject *parent = 0);

    bool activate();
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
    Q_PROPERTY(QStringList allowedDomains READ allowedDomains WRITE setAllowedDomains);
    Q_PROPERTY(QString jid READ jid WRITE setJid);
    Q_PROPERTY(QString host READ host WRITE setHost);
    Q_PROPERTY(quint16 port READ port WRITE setPort);

public:
    QXmppServerProxy65();
    ~QXmppServerProxy65();

    QStringList allowedDomains() const;
    void setAllowedDomains(const QStringList &allowedDomains);

    QString jid() const;
    void setJid(const QString &jid);

    QString host() const;
    void setHost(const QString &host);

    quint16 port() const;
    void setPort(quint16 port);

    /// \cond
    QStringList discoveryItems() const;
    bool handleStanza(QXmppStream *stream, const QDomElement &element);
    bool start();
    void stop();
    QVariantMap statistics() const;
    void setStatistics(const QVariantMap &statistics);
    /// \endcond

private slots:
    void slotPairFinished();
    void slotSocketConnected(QTcpSocket *socket, const QString &hostName, quint16 port);
    void slotUpdateStatistics();

private:
    QXmppServerProxy65Private * const d;
};

#endif
