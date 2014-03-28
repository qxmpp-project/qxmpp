/*
 * Copyright (C) 2008-2014 The QXmpp developers
 *
 * Author:
 *	Ian Reinhart Geiser
 *
 * Source:
 *	https://github.com/qxmpp-project/qxmpp
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


#ifndef IBBCLIENT_H
#define IBBCLIENT_H

#include "QXmppClient.h"
#include "QXmppTransferManager.h"

class QBuffer;

class xmppClient : public QXmppClient
{
    Q_OBJECT

public:
    xmppClient(QObject *parent = 0);
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

#endif // IBBCLIENT_H
