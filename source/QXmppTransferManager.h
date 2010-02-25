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
#include <QHostAddress>

#include "QXmppIq.h"

class QXmppByteStreamIq;
class QXmppClient;
class QXmppIbbCloseIq;
class QXmppIbbDataIq;
class QXmppIbbOpenIq;
class QXmppSocksClient;
class QXmppSocksServer;
class QXmppStreamInitiationIq;

class QXmppTransferJob : public QObject
{
    Q_OBJECT

public:
    enum Direction
    {
        IncomingDirection,
        OutgoingDirection,
    };

    enum Error
    {
        NoError = 0,
        FileCorruptError,
        ProtocolError,
    };

    enum Method
    {
        NoMethod = 0,
        InBandMethod = 1,
        SocksMethod = 2,
        AnyMethod = 3,
    };

    enum State
    {
        StartState = 0,
        TransferState = 1,
        FinishedState = 2,
    };

    void accept(QIODevice *output);

    QXmppTransferJob::Direction direction() const;
    QXmppTransferJob::Error error() const;
    QString localFilePath() const;
    void setLocalFilePath(const QString &path);
    QString jid() const;
    QXmppTransferJob::Method method() const;
    QXmppTransferJob::State state() const;

    // XEP-0096 : File transfer
    QDateTime fileDate() const;
    QString fileHash() const;
    QString fileName() const;
    int fileSize() const;

signals:
    void error(QXmppTransferJob::Error error);
    void finished();
    void progress(qint64 done, qint64 total);
    void stateChanged(QXmppTransferJob::State state);

private:
    QXmppTransferJob(const QString &jid, QXmppTransferJob::Direction direction, QObject *parent);
    void setState(QXmppTransferJob::State state);
    void terminate(QXmppTransferJob::Error error);

    int m_blockSize;
    QXmppTransferJob::Direction m_direction;
    int m_done;
    QXmppTransferJob::Error m_error;
    QIODevice *m_iodevice;
    QString m_jid;
    QString m_sid;
    Method m_method;
    QString m_mimeType;
    QString m_requestId;
    State m_state;

    // local path to file
    QString m_localFilePath;

    // file meta-data
    QDateTime m_fileDate;
    QString m_fileHash;
    QString m_fileName;
    int m_fileSize;

    // for in-band bytestreams
    int m_ibbSequence;

    // for socks5 bytestreams
    QXmppSocksClient *m_socksClient;
    QXmppSocksServer *m_socksServer;

    friend class QXmppTransferManager;
};

class QXmppTransferManager : public QObject
{
    Q_OBJECT

public:
    QXmppTransferManager(QXmppClient* client);
    QXmppTransferJob *sendFile(const QString &jid, const QString &fileName);
    int supportedMethods() const;
    void setSupportedMethods(int methods);

signals:
    void fileReceived(QXmppTransferJob *offer);

private slots:
    void byteStreamIqReceived(const QXmppByteStreamIq&);
    void ibbCloseIqReceived(const QXmppIbbCloseIq&);
    void ibbDataIqReceived(const QXmppIbbDataIq&);
    void ibbOpenIqReceived(const QXmppIbbOpenIq&);
    void iqReceived(const QXmppIq&);
    void socksClientDataReceived();
    void socksClientDisconnected();
    void socksServerDataSent();
    void socksServerDisconnected();
    void streamInitiationIqReceived(const QXmppStreamInitiationIq&);

private:
    QXmppTransferJob *getJobByRequestId(const QString &jid, const QString &id);
    QXmppTransferJob *getJobBySid(const QString &jid, const QString &sid);
    QXmppTransferJob *getJobBySocksClient(QXmppSocksClient *socksClient);
    QXmppTransferJob *getJobBySocksServer(QXmppSocksServer *socksServer);
    void byteStreamResponseReceived(const QXmppIq&);
    void byteStreamResultReceived(const QXmppByteStreamIq&);
    void byteStreamSetReceived(const QXmppByteStreamIq&);
    void ibbResponseReceived(const QXmppIq&);
    void streamInitiationResultReceived(const QXmppStreamInitiationIq&);
    void streamInitiationSetReceived(const QXmppStreamInitiationIq&);
    void socksServerSendData(QXmppTransferJob *job);

    // reference to client object (no ownership)
    QXmppClient* m_client;
    QList<QXmppTransferJob*> m_jobs;
    int m_ibbBlockSize;
    int m_supportedMethods;
};

#endif
