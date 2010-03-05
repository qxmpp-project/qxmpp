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

#include <QCryptographicHash>
#include <QDateTime>
#include <QHash>
#include <QHostAddress>
#include <QVariant>

#include "QXmppIq.h"
#include "QXmppByteStreamIq.h"

class QTcpSocket;
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
        AbortError,
        FileAccessError,
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

    void abort();
    void accept(QIODevice *output);

    QVariant data(int role) const;
    void setData(int role, const QVariant &value);

    QXmppTransferJob::Direction direction() const;
    QXmppTransferJob::Error error() const;
    QString jid() const;
    QXmppTransferJob::Method method() const;
    QXmppTransferJob::State state() const;

    // XEP-0096 : File transfer
    QDateTime fileDate() const;
    QByteArray fileHash() const;
    QString fileName() const;
    qint64 fileSize() const;

signals:
    /// This signal is emitted when an error is encountered while
    /// processing the transfer job.
    void error(QXmppTransferJob::Error error);

    /// This signal is emitted when the transfer job is finished.
    ///
    /// You can determine if the job completed successfully by testing whether
    /// error() returns QXmppTransferJob::NoError.
    ///
    /// Note: Do not delete the job in the slot connected to this signal,
    /// instead use deleteLater().
    void finished();

    /// This signal is emitted to indicate the progress of this transfer job.
    void progress(qint64 done, qint64 total);

    /// This signal is emitted when the transfer job changes state.
    void stateChanged(QXmppTransferJob::State state);

private slots:
    void slotTerminated();

private:
    QXmppTransferJob(const QString &jid, QXmppTransferJob::Direction direction, QObject *parent);
    void checkData();
    void setState(QXmppTransferJob::State state);
    void terminate(QXmppTransferJob::Error error);
    bool writeData(const QByteArray &data);

    int m_blockSize;
    QXmppTransferJob::Direction m_direction;
    qint64 m_done;
    QXmppTransferJob::Error m_error;
    QCryptographicHash m_hash;
    QIODevice *m_iodevice;
    QString m_jid;
    QString m_sid;
    Method m_method;
    QString m_mimeType;
    QString m_requestId;
    State m_state;

    // arbitrary data
    QHash<int, QVariant> m_data;

    // file meta-data
    QDateTime m_fileDate;
    QByteArray m_fileHash;
    QString m_fileName;
    int m_fileSize;

    // for in-band bytestreams
    int m_ibbSequence;

    // for socks5 bytestreams
    QTcpSocket *m_socksSocket;
    QXmppByteStreamIq::StreamHost m_socksProxy;

    friend class QXmppTransferManager;
};

class QXmppTransferManager : public QObject
{
    Q_OBJECT

public:
    QXmppTransferManager(QXmppClient* client);
    QXmppTransferJob *sendFile(const QString &jid, const QString &fileName);
    QString proxy() const;
    void setProxy(const QString &proxyJid);
    int supportedMethods() const;
    void setSupportedMethods(int methods);

signals:
    /// This signal is emitted when a new file transfer offer is received.
    ///
    /// To accept the transfer job, you must call its accept() method from
    /// a slot connected to the signal. Otherwise, the offer transfer job
    /// will be refused.
    void fileReceived(QXmppTransferJob *offer);

private slots:
    void byteStreamIqReceived(const QXmppByteStreamIq&);
    void ibbCloseIqReceived(const QXmppIbbCloseIq&);
    void ibbDataIqReceived(const QXmppIbbDataIq&);
    void ibbOpenIqReceived(const QXmppIbbOpenIq&);
    void iqReceived(const QXmppIq&);
    void jobError(QXmppTransferJob::Error error);
    void socksServerConnected(QTcpSocket *socket, const QString &hostName, quint16 port);
    void socksSocketDataReceived();
    void socksSocketDataSent();
    void socksSocketDisconnected();
    void streamInitiationIqReceived(const QXmppStreamInitiationIq&);

private:
    QXmppTransferJob *getJobByRequestId(const QString &jid, const QString &id);
    QXmppTransferJob *getJobBySid(const QString &jid, const QString &sid);
    QXmppTransferJob *getJobBySocksSocket(QTcpSocket *socksSocket);
    void byteStreamResponseReceived(const QXmppIq&);
    void byteStreamResultReceived(const QXmppByteStreamIq&);
    void byteStreamSetReceived(const QXmppByteStreamIq&);
    void ibbResponseReceived(const QXmppIq&);
    void streamInitiationResultReceived(const QXmppStreamInitiationIq&);
    void streamInitiationSetReceived(const QXmppStreamInitiationIq&);
    void socksServerSendData(QXmppTransferJob *job);
    void socksServerSendOffer(QXmppTransferJob *job);

    // reference to client object (no ownership)
    QXmppClient* m_client;
    QList<QXmppTransferJob*> m_jobs;
    int m_ibbBlockSize;
    QXmppSocksServer *m_socksServer;
    int m_supportedMethods;
    QString m_proxy;
};

#endif
