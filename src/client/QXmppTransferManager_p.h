// SPDX-FileCopyrightText: 2012 Jeremy Lainé <jeremy.laine@m4x.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

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

private Q_SLOTS:
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

public Q_SLOTS:
    void _q_disconnected();

private Q_SLOTS:
    void _q_proxyReady();
    void _q_sendData();
};
