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

private:
    Q_SLOT void slotError(QXmppTransferJob::Error error);
    Q_SLOT void slotFileReceived(QXmppTransferJob *job);
    Q_SLOT void slotFinished();
    Q_SLOT void slotPresenceReceived(const QXmppPresence &presence);
    Q_SLOT void slotProgress(qint64 done, qint64 total);

    QString m_recipient;
    QXmppTransferManager *transferManager;
};

#endif  // IBBCLIENT_H
