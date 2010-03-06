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

/// \brief The QXmppTransferJob class represents a single file transfer job.
///
/// \sa QXmppTransferManager
///

class QXmppTransferJob : public QObject
{
    Q_OBJECT

public:
    enum Direction
    {
        IncomingDirection, ///< The file is being received.
        OutgoingDirection, ///< The file is being sent.
    };

    enum Error
    {
        NoError = 0,      ///< No error occured.
        AbortError,       ///< The file transfer was aborted.
        FileAccessError,  ///< An error was encountered trying to access a local file.
        FileCorruptError, ///< The file is corrupt: the file size or hash do not match.
        ProtocolError,    ///< An error was encountered in the file transfer protocol.
    };

    enum Method
    {
        NoMethod = 0,     ///< No transfer method.
        InBandMethod = 1, ///< XEP-0047: In-Band Bytestreams
        SocksMethod = 2,  ///< XEP-0065: SOCKS5 Bytestreams
        AnyMethod = 3,    ///< Any supported transfer method.
    };

    enum State
    {
        OfferState = 0,
        StartState = 1,
        TransferState = 2,
        FinishedState = 3,
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
    QString m_offerId;
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

/// \brief The QXmppTransferManager class provides support for sending and
/// receiving files.
///
/// Stream initiation is performed as described in XEP-0095: Stream Initiation
/// and XEP-0096: SI File Transfer. The actual file transfer is then performed
/// using either XEP-0065: SOCKS5 Bytestreams or XEP-0047: In-Band Bytestreams.
/// 
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
    /// To accept the transfer job, call the job's QXmppTransferJob::accept() method.
    /// To refuse the transfer job, call the job's QXmppTransferJob::abort() method.
    void fileReceived(QXmppTransferJob *offer);

private slots:
    void byteStreamIqReceived(const QXmppByteStreamIq&);
    void ibbCloseIqReceived(const QXmppIbbCloseIq&);
    void ibbDataIqReceived(const QXmppIbbDataIq&);
    void ibbOpenIqReceived(const QXmppIbbOpenIq&);
    void iqReceived(const QXmppIq&);
    void jobError(QXmppTransferJob::Error error);
    void jobStateChanged(QXmppTransferJob::State state);
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
