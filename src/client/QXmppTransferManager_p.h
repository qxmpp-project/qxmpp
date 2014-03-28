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

#ifndef QXMPPTRANSFERMANAGER_P_H
#define QXMPPTRANSFERMANAGER_P_H

#include "QXmppByteStreamIq.h"
#include "QXmppTransferManager.h"

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QXmpp API.  It exists for the convenience
// of the QXmppTransferManager class.  This header file may change from
// version to version without notice, or even be removed.
//
// We mean it.
//

class QTimer;
class QXmppSocksClient;

class QXmppTransferIncomingJob : public QXmppTransferJob
{
    Q_OBJECT

public:
    QXmppTransferIncomingJob(const QString &jid, QXmppClient *client, QObject *parent);
    void checkData();
    void connectToHosts(const QXmppByteStreamIq &iq);
    bool writeData(const QByteArray &data);

private slots:
    void _q_candidateDisconnected();
    void _q_candidateReady();
    void _q_disconnected();
    void _q_receiveData();

private:
    void connectToNextHost();

    QXmppByteStreamIq::StreamHost m_candidateHost;
    QXmppSocksClient *m_candidateClient;
    QTimer *m_candidateTimer;
    QList<QXmppByteStreamIq::StreamHost> m_streamCandidates;
    QString m_streamOfferId;
    QString m_streamOfferFrom;
};

class QXmppTransferOutgoingJob : public QXmppTransferJob
{
    Q_OBJECT

public:
    QXmppTransferOutgoingJob(const QString &jid, QXmppClient *client, QObject *parent);
    void connectToProxy();
    void startSending();

private slots:
    void _q_disconnected();
    void _q_proxyReady();
    void _q_sendData();
};

#endif
