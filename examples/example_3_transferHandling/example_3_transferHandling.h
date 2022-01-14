// SPDX-FileCopyrightText: 2012 Ian Reinhart Geiser <geiseri@kde.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef IBBCLIENT_H
#define IBBCLIENT_H

#include "QXmppClient.h"
#include "QXmppTransferManager.h"

class xmppClient : public QXmppClient
{
    Q_OBJECT

public:
    xmppClient(QObject *parent = nullptr);
    void setRecipient(const QString &recipient);

private slots:
    void slotError(QXmppTransferJob::Error error);
    void slotFileReceived(QXmppTransferJob *job);
    void slotFinished();
    void slotPresenceReceived(const QXmppPresence &presence);
    void slotProgress(qint64 done, qint64 total);

private:
    QString m_recipient;
    QXmppTransferManager *transferManager;
};

#endif  // IBBCLIENT_H
