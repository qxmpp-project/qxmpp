/*
 * Copyright (C) 2008-2009 Manjeet Dahiya
 *
 * Author:
 *	Manjeet Dahiya
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


#ifndef QXMPPSTREAM_H
#define QXMPPSTREAM_H

#include <QObject>
#include <QSslSocket>
#include "QXmppConfiguration.h"
#include "QXmppRoster.h"
#include "QXmppStanza.h"

class QDomElement;

class QXmppRoster;
class QXmppClient;
class QXmppPacket;
class QXmppPresence;
class QXmppIq;
class QXmppBind;
class QXmppRosterIq;
class QXmppMessage;

class QXmppStream : public QObject
{
    Q_OBJECT

public:
    QXmppStream(QXmppClient* client);
    ~QXmppStream();
    void connect();
    void acceptSubscriptionRequest(const QString& from, bool accept = true);
    void sendSubscriptionRequest(const QString& to);
    void disconnect();
    QXmppRoster& getRoster();
    void sendPacket(const QXmppPacket&);

signals:
    void hostFound();
    void connected();
    void disconnected();
    void streamError();
    void subscriptionRequestReceived(const QString& from);
    void presenceReceived(const QXmppPresence&);
    void messageReceived(const QXmppMessage&);
    void iqReceived(const QXmppIq&);
    void rosterIqReceived(const QXmppRosterIq&);

private slots:
    void socketHostFound();
    void socketReadReady();
    void socketEncrypted();
    void socketConnected();
    void socketDisconnected();
    void socketError(QAbstractSocket::SocketError);
    void socketSslErrors(const QList<QSslError> &);

private:
    QXmppClient* m_client; // reverse pointer
    QXmppRoster m_roster;
    QString m_sessionId;
    QString m_bindId;
    QString m_rosterReqId;

    QSslSocket m_socket;
    bool m_sessionAvaliable;

    QXmppConfiguration& getConfiguration();
    void parser(const QByteArray&);
    void sendStartStream();
    void sendEndStream();
    void sendStartTls();
    void sendAuthPlain();
    void sendBindIQ();
    void sendSessionIQ();
    void sendInitialPresence();
    void sendRosterRequest();
    void sendToServer(const QByteArray&);
    bool hasStartStreamElement(const QByteArray&);
    bool hasEndStreamElement(const QByteArray&);
    QXmppStanza::Error parseStanzaError(QDomElement & errorElement);

    void processPresence(const QXmppPresence&);
    void processMessage(const QXmppMessage&);
    void processIq(const QXmppIq&);
    void processBindIq(const QXmppBind&);
    void processRosterIq(const QXmppRosterIq&);
};

#endif // QXMPPSTREAM_H
