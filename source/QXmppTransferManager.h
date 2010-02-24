/*
 * Copyright (C) 2010 Bolloré telecom
 *
 * Author:
 *	Jeremy Lainé
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

#ifndef QXMPPTRANSFERMANAGER_H
#define QXMPPTRANSFERMANAGER_H

#include <QDateTime>

#include "QXmppIq.h"

class QXmppClient;
class QXmppIbbCloseIq;
class QXmppIbbDataIq;
class QXmppIbbOpenIq;
class QXmppStreamInitiationIq;

class QXmppTransferManager;

class QXmppTransferJob : public QObject
{
    Q_OBJECT

public:
    enum Error
    {
        NoError = 0,
        FileCorruptError,
        ProtocolError,
    };

    enum Method
    {
        InBandByteStream = 1,
        SocksByteStream = 2,
    };

    void accept(QIODevice *output);

    QXmppTransferJob::Error error() const;
    QString jid() const;

    // XEP-0096 : File transfer
    QDateTime fileDate() const;
    QString fileHash() const;
    QString fileName() const;
    int fileSize() const;

signals:
    void error(QXmppTransferJob::Error error);
    void finished();
    void progress(qint64 done, qint64 total);

private:
    QXmppTransferJob(const QString &jid, QXmppTransferManager *manager);
    void terminate(QXmppTransferJob::Error error);

    int m_done;
    QXmppTransferJob::Error m_error;
    QIODevice *m_iodevice;
    QString m_jid;
    QString m_sid;
    int m_methods;
    QString m_mimeType;

    QDateTime m_fileDate;
    QString m_fileHash;
    QString m_fileName;
    int m_fileSize;

    QString m_requestId;
    int m_ibbSequence;
    friend class QXmppTransferManager;
};

class QXmppTransferManager : public QObject
{
    Q_OBJECT

public:
    QXmppTransferManager(QXmppClient* client);
    QXmppTransferJob *sendFile(const QString &jid, const QString &fileName);

signals:
    void fileReceived(QXmppTransferJob *offer);

private slots:
    void ibbCloseIqReceived(const QXmppIbbCloseIq&);
    void ibbDataIqReceived(const QXmppIbbDataIq&);
    void ibbOpenIqReceived(const QXmppIbbOpenIq&);
    void iqReceived(const QXmppIq&);
    void streamInitiationIqReceived(const QXmppStreamInitiationIq&);

private:
    QXmppTransferJob *getJobByRequestId(const QString &jid, const QString &id);
    QXmppTransferJob *getJobBySid(const QString &jid, const QString &sid);
    void streamInitiationResultReceived(const QXmppStreamInitiationIq&);
    void streamInitiationSetReceived(const QXmppStreamInitiationIq&);

    // reference to client object (no ownership)
    QXmppClient* m_client;
    QList<QXmppTransferJob*> m_jobs;
    int m_ibbBlockSize;
};

#endif
