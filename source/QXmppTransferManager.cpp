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

#include <QDomElement>
#include <QFile>
#include <QFileInfo>
#include <QNetworkInterface>
#include <QTimer>

#include "QXmppByteStreamIq.h"
#include "QXmppClient.h"
#include "QXmppConstants.h"
#include "QXmppIbbIq.h"
#include "QXmppSocks.h"
#include "QXmppStreamInitiationIq.h"
#include "QXmppTransferManager.h"
#include "QXmppUtils.h"

static QString streamHash(const QString &sid, const QString &initiatorJid, const QString &targetJid)
{
    QCryptographicHash hash(QCryptographicHash::Sha1);
    QString str = sid + initiatorJid + targetJid;
    hash.addData(str.toAscii());
    return hash.result().toHex();
}

QXmppTransferJob::QXmppTransferJob(const QString &jid, QXmppTransferJob::Direction direction, QObject *parent)
    : QObject(parent),
    m_blockSize(16384),
    m_direction(direction),
    m_done(0),
    m_error(NoError),
    m_hash(QCryptographicHash::Md5),
    m_iodevice(0),
    m_jid(jid),
    m_method(NoMethod),
    m_state(StartState),
    m_fileSize(0),
    m_ibbSequence(0),
    m_socksClient(0),
    m_socksSocket(0)
{
}

void QXmppTransferJob::abort()
{
    // FIXME : actually abort job cleanly
    terminate(AbortError);
}

void QXmppTransferJob::accept(QIODevice *iodevice)
{
    if (!m_iodevice)
        m_iodevice = iodevice;
}

void QXmppTransferJob::checkData()
{
    if ((m_fileSize && m_done != m_fileSize) ||
        (!m_fileHash.isEmpty() && m_hash.result() != m_fileHash))
        terminate(QXmppTransferJob::FileCorruptError);
    else
        terminate(QXmppTransferJob::NoError);
}

QXmppTransferJob::Direction QXmppTransferJob::direction() const
{
    return m_direction;
}

QXmppTransferJob::Error QXmppTransferJob::error() const
{
    return m_error;
}

QString QXmppTransferJob::jid() const
{
    return m_jid;
}

QString QXmppTransferJob::localFilePath() const
{
    return m_localFilePath;
}

void QXmppTransferJob::setLocalFilePath(const QString &path)
{
    m_localFilePath = path;
}

QDateTime QXmppTransferJob::fileDate() const
{
    return m_fileDate;
}

QByteArray QXmppTransferJob::fileHash() const
{
    return m_fileHash;
}

QString QXmppTransferJob::fileName() const
{
    return m_fileName;
}

qint64 QXmppTransferJob::fileSize() const
{
    return m_fileSize;
}

QXmppTransferJob::Method QXmppTransferJob::method() const
{
    return m_method;
}

QXmppTransferJob::State QXmppTransferJob::state() const
{
    return m_state;
}

void QXmppTransferJob::setState(QXmppTransferJob::State state)
{
    if (m_state != state)
    {
        m_state = state;
        emit stateChanged(m_state);
    }
}

void QXmppTransferJob::slotTerminated()
{
    emit stateChanged(m_state);
    if (m_error == NoError)
        emit finished();
    else
        emit error(m_error);
}

void QXmppTransferJob::terminate(QXmppTransferJob::Error cause)
{
    if (m_state == FinishedState)
        return;

    // change state
    m_error = cause;
    m_state = FinishedState;

    // close IO device
    if (m_iodevice)
        m_iodevice->close();

    // close sockets
    if (m_socksClient)
        m_socksClient->close();
    if (m_socksSocket)
        m_socksSocket->close();

    // emit signals later
    QTimer::singleShot(0, this, SLOT(slotTerminated()));
}

bool QXmppTransferJob::writeData(const QByteArray &data)
{
    const qint64 written = m_iodevice->write(data);
    if (written < 0)
        return false;
    m_done += written;
    if (!m_fileHash.isEmpty())
        m_hash.addData(data);
    progress(m_done, m_fileSize);
    return true;
}

QXmppTransferManager::QXmppTransferManager(QXmppClient *client)
    : m_client(client),
    m_ibbBlockSize(4096),
    m_socksServer(0),
    m_supportedMethods(QXmppTransferJob::AnyMethod)
{
    // start SOCKS server
    m_socksServer = new QXmppSocksServer(this);
    if (m_socksServer->listen())
    {
        connect(m_socksServer, SIGNAL(newConnection(QTcpSocket*, const QString&, quint16)),
            this, SLOT(socksServerConnected(QTcpSocket*, const QString&, quint16)));
    } else {
        qWarning() << "QXmppSocksServer could not start listening";
    }
}

void QXmppTransferManager::byteStreamIqReceived(const QXmppByteStreamIq &iq)
{
    // handle IQ from proxy
    foreach (QXmppTransferJob *job, m_jobs)
    {
        if (job->m_socksProxy.jid() == iq.from() && job->m_requestId == iq.id())
        {
            if (iq.type() == QXmppIq::Result && iq.streamHosts().size() > 0)
            {
                job->m_socksProxy = iq.streamHosts().first();
                socksServerSendOffer(job);
                return;
            }
        }
    }

    if (iq.type() == QXmppIq::Result)
        byteStreamResultReceived(iq);
    else if (iq.type() == QXmppIq::Set)
        byteStreamSetReceived(iq);
}

/// Handle a response to a bystream set, i.e. after we informed the remote party
/// that we connected to a stream host.
void QXmppTransferManager::byteStreamResponseReceived(const QXmppIq &iq)
{
    QXmppTransferJob *job = getJobByRequestId(iq.from(), iq.id());
    if (!job ||
        job->direction() != QXmppTransferJob::IncomingDirection ||
        job->method() != QXmppTransferJob::SocksMethod ||
        job->state() != QXmppTransferJob::StartState)
        return;

    if (iq.type() == QXmppIq::Error)
        job->terminate(QXmppTransferJob::ProtocolError);
}

/// Handle a bytestream result, i.e. after the remote party has connected to
/// a stream host.
void QXmppTransferManager::byteStreamResultReceived(const QXmppByteStreamIq &iq)
{
    QXmppTransferJob *job = getJobByRequestId(iq.from(), iq.id());
    if (!job ||
        job->direction() != QXmppTransferJob::OutgoingDirection ||
        job->method() != QXmppTransferJob::SocksMethod ||
        job->state() != QXmppTransferJob::StartState)
        return;

    // check the stream host
    if (iq.streamHostUsed() == job->m_socksProxy.jid())
    {
        const QXmppByteStreamIq::StreamHost streamHost = job->m_socksProxy;
        qDebug() << "Connecting to proxy" << streamHost.jid();
        qDebug() << " host:" << streamHost.host().toString();
        qDebug() << " port:" << streamHost.port();

        // connect to proxy
        const QString hostName = streamHash(job->m_sid,
                                            m_client->getConfiguration().jid(),
                                            job->m_jid);

        job->m_socksClient = new QXmppSocksClient(streamHost.host(), streamHost.port(), job);
        job->m_socksClient->connectToHost(hostName, 0);
        if (!job->m_socksClient->waitForConnected())
        {
            qWarning() << "Failed to connect to" << streamHost.host().toString() << streamHost.port() << ":" << job->m_socksClient->errorString();
            job->terminate(QXmppTransferJob::ProtocolError);
            return;
        }

        // activate stream
        QXmppByteStreamIq streamIq;
        streamIq.setType(QXmppIq::Set);
        streamIq.setFrom(m_client->getConfiguration().jid());
        streamIq.setTo(streamHost.jid());
        streamIq.setSid(job->m_sid);
        streamIq.setActivate(job->m_jid);
        job->m_requestId = streamIq.id();
        m_client->sendPacket(streamIq);
        return;
    }

    // direction connection, start sending data
    if (!job->m_socksSocket)
    {
        qWarning("Client says they connected to our SOCKS server, but they did not");
        job->terminate(QXmppTransferJob::ProtocolError);
        return;
    }
    job->setState(QXmppTransferJob::TransferState);
    connect(job->m_socksSocket, SIGNAL(bytesWritten(qint64)), this, SLOT(socksSocketDataSent()));
    connect(job->m_socksSocket, SIGNAL(disconnected()), this, SLOT(socksSocketDisconnected()));
    socksServerSendData(job);
}

/// Handle a bytestream set, i.e. an invitation from the remote party to connect
/// to a stream host.
void QXmppTransferManager::byteStreamSetReceived(const QXmppByteStreamIq &iq)
{
    QXmppIq response;
    response.setId(iq.id());
    response.setTo(iq.from());

    QXmppTransferJob *job = getJobBySid(iq.from(), iq.sid());
    if (!job ||
        job->direction() != QXmppTransferJob::IncomingDirection ||
        job->method() != QXmppTransferJob::SocksMethod ||
        job->state() != QXmppTransferJob::StartState)
    {
        // the stream is unknown
        QXmppStanza::Error error(QXmppStanza::Error::Auth, QXmppStanza::Error::NotAcceptable);
        error.setCode(406);
        response.setType(QXmppIq::Error);
        response.setError(error);
        m_client->sendPacket(response);
        return;
    }

    // try connecting to the offered stream hosts
    foreach (const QXmppByteStreamIq::StreamHost &streamHost, iq.streamHosts())
    {
        qDebug() << "Connecting to streamhost" << streamHost.jid();
        qDebug() << " host:" << streamHost.host().toString();
        qDebug() << " port:" << streamHost.port();

        const QString hostName = streamHash(job->m_sid,
                                            job->m_jid,
                                            m_client->getConfiguration().jid());

        // try to connect to stream host
        job->m_socksClient = new QXmppSocksClient(streamHost.host(), streamHost.port(), job);
        job->m_socksClient->connectToHost(hostName, 0);
        if (job->m_socksClient->waitForConnected())
        {
            job->setState(QXmppTransferJob::TransferState);
            connect(job->m_socksClient, SIGNAL(readyRead()), this, SLOT(socksClientDataReceived()));
            connect(job->m_socksClient, SIGNAL(disconnected()), this, SLOT(socksClientDisconnected()));

            QXmppByteStreamIq ackIq;
            ackIq.setId(iq.id());
            ackIq.setTo(iq.from());
            ackIq.setType(QXmppIq::Result);
            ackIq.setSid(job->m_sid);
            ackIq.setStreamHostUsed(streamHost.jid());
            m_client->sendPacket(ackIq);
            return;
        } else {
            qWarning() << "Failed to connect to" << streamHost.host().toString() << streamHost.port() << ":" << job->m_socksClient->errorString();
            delete job->m_socksClient;
            job->m_socksClient = 0;
        }
    }

    // could not connect to any stream host
    QXmppStanza::Error error(QXmppStanza::Error::Cancel, QXmppStanza::Error::ItemNotFound);
    error.setCode(404);
    response.setType(QXmppIq::Error);
    response.setError(error);
    m_client->sendPacket(response);

    job->terminate(QXmppTransferJob::ProtocolError);
}

QXmppTransferJob* QXmppTransferManager::getJobByRequestId(const QString &jid, const QString &id)
{
    foreach (QXmppTransferJob *job, m_jobs)
        if (job->m_jid == jid && job->m_requestId == id)
            return job;
    return 0;
}

QXmppTransferJob* QXmppTransferManager::getJobBySid(const QString &jid, const QString &sid)
{
    foreach (QXmppTransferJob *job, m_jobs)
        if (job->m_jid == jid && job->m_sid == sid)
            return job;
    return 0;
}

QXmppTransferJob* QXmppTransferManager::getJobBySocksClient(QXmppSocksClient *socksClient)
{
    foreach (QXmppTransferJob *job, m_jobs)
        if (job->m_socksClient == socksClient)
            return job;
    return 0;
}

QXmppTransferJob* QXmppTransferManager::getJobBySocksSocket(QTcpSocket *socksSocket)
{
    foreach (QXmppTransferJob *job, m_jobs)
        if (job->m_socksSocket == socksSocket)
            return job;
    return 0;
}

void QXmppTransferManager::ibbCloseIqReceived(const QXmppIbbCloseIq &iq)
{
    QXmppIq response;
    response.setTo(iq.from());
    response.setId(iq.id());

    QXmppTransferJob *job = getJobBySid(iq.from(), iq.sid());
    if (!job ||
        job->direction() != QXmppTransferJob::IncomingDirection ||
        job->method() != QXmppTransferJob::InBandMethod)
    {
        // the job is unknown, cancel it
        QXmppStanza::Error error(QXmppStanza::Error::Cancel, QXmppStanza::Error::ItemNotFound);
        response.setType(QXmppIq::Error);
        response.setError(error);
        m_client->sendPacket(response);
        return;
    }

    // acknowledge the packet
    response.setType(QXmppIq::Result);
    m_client->sendPacket(response);

    // check received data
    job->checkData();
}

void QXmppTransferManager::ibbDataIqReceived(const QXmppIbbDataIq &iq)
{
    QXmppIq response;
    response.setTo(iq.from());
    response.setId(iq.id());

    QXmppTransferJob *job = getJobBySid(iq.from(), iq.sid());
    if (!job ||
        job->direction() != QXmppTransferJob::IncomingDirection ||
        job->method() != QXmppTransferJob::InBandMethod)
    {
        // the job is unknown, cancel it
        QXmppStanza::Error error(QXmppStanza::Error::Cancel, QXmppStanza::Error::ItemNotFound);
        response.setType(QXmppIq::Error);
        response.setError(error);
        m_client->sendPacket(response);
        return;
    }

    if (iq.sequence() != job->m_ibbSequence)
    {
        // the packet is out of sequence
        QXmppStanza::Error error(QXmppStanza::Error::Cancel, QXmppStanza::Error::UnexpectedRequest);
        response.setType(QXmppIq::Error);
        response.setError(error);
        m_client->sendPacket(response);
        return;
    }

    // write data
    job->writeData(iq.payload());
    job->m_ibbSequence++;

    // acknowledge the packet
    response.setType(QXmppIq::Result);
    m_client->sendPacket(response);
}

void QXmppTransferManager::ibbOpenIqReceived(const QXmppIbbOpenIq &iq)
{
    QXmppIq response;
    response.setTo(iq.from());
    response.setId(iq.id());

    QXmppTransferJob *job = getJobBySid(iq.from(), iq.sid());
    if (!job ||
        job->direction() != QXmppTransferJob::IncomingDirection ||
        job->method() != QXmppTransferJob::InBandMethod)
    {
        // the job is unknown, cancel it
        QXmppStanza::Error error(QXmppStanza::Error::Cancel, QXmppStanza::Error::ItemNotFound);
        response.setType(QXmppIq::Error);
        response.setError(error);
        m_client->sendPacket(response);
        return;
    }

    if (iq.blockSize() > m_ibbBlockSize)
    {
        // we prefer a smaller block size
        QXmppStanza::Error error(QXmppStanza::Error::Modify, QXmppStanza::Error::ResourceConstraint);
        response.setType(QXmppIq::Error);
        response.setError(error);
        m_client->sendPacket(response);
        return;
    }

    job->m_blockSize = iq.blockSize();
    job->setState(QXmppTransferJob::TransferState);

    // accept transfer
    response.setType(QXmppIq::Result);
    m_client->sendPacket(response);
}

void QXmppTransferManager::ibbResponseReceived(const QXmppIq &iq)
{
    QXmppTransferJob *job = getJobByRequestId(iq.from(), iq.id());
    if (!job ||
        job->direction() != QXmppTransferJob::OutgoingDirection ||
        job->method() != QXmppTransferJob::InBandMethod ||
        job->state() == QXmppTransferJob::FinishedState)
        return;

    // if the IO device is closed, do nothing
    if (!job->m_iodevice->isOpen())
        return;

    if (iq.type() == QXmppIq::Result)
    {
        const QByteArray buffer = job->m_iodevice->read(job->m_blockSize);
        job->setState(QXmppTransferJob::TransferState);
        if (buffer.size())
        {
            // send next data block
            QXmppIbbDataIq dataIq;
            dataIq.setTo(job->m_jid);
            dataIq.setSid(job->m_sid);
            dataIq.setSequence(job->m_ibbSequence++);
            dataIq.setPayload(buffer);
            job->m_requestId = dataIq.id();
            m_client->sendPacket(dataIq);

            job->m_done += buffer.size();
            job->progress(job->m_done, job->fileSize());
        } else {
            // close the bytestream
            QXmppIbbCloseIq closeIq;
            closeIq.setTo(job->m_jid);
            closeIq.setSid(job->m_sid);
            job->m_requestId = closeIq.id();
            m_client->sendPacket(closeIq);

            job->terminate(QXmppTransferJob::NoError);
        }
    }
    else if (iq.type() == QXmppIq::Error)
    {
        // close the bytestream
        QXmppIbbCloseIq closeIq;
        closeIq.setTo(job->m_jid);
        closeIq.setSid(job->m_sid);
        job->m_requestId = closeIq.id();
        m_client->sendPacket(closeIq);

        job->terminate(QXmppTransferJob::ProtocolError);
    }
}

void QXmppTransferManager::iqReceived(const QXmppIq &iq)
{
    // handle IQ from proxy
    foreach (QXmppTransferJob *job, m_jobs)
    {
        if (job->m_socksProxy.jid() == iq.from() && job->m_requestId == iq.id())
        {
            if (job->m_socksClient)
            {
                // proxy connection activation result
                if (iq.type() == QXmppIq::Result)
                {
                    // proxy stream activated, start sending data
                    job->setState(QXmppTransferJob::TransferState);
                    job->m_socksSocket = job->m_socksClient->socket();
                    connect(job->m_socksSocket, SIGNAL(bytesWritten(qint64)), this, SLOT(socksSocketDataSent()));
                    connect(job->m_socksSocket, SIGNAL(disconnected()), this, SLOT(socksSocketDisconnected()));
                    socksServerSendData(job);
                } else if (iq.type() == QXmppIq::Error) {
                    // proxy stream not activated, terminate
                    qWarning("Could not activate SOCKS5 proxy bytestream");
                    job->terminate(QXmppTransferJob::ProtocolError);
                }
            } else {
                // we could not get host/port from proxy, procede without a proxy
                if (iq.type() == QXmppIq::Error)
                    socksServerSendOffer(job);
            }
            return;
        }
    }

    QXmppTransferJob *job = getJobByRequestId(iq.from(), iq.id());
    if (!job)
        return;

    if (job->method() == QXmppTransferJob::InBandMethod)
        ibbResponseReceived(iq);
    else if (job->method() == QXmppTransferJob::SocksMethod)
        byteStreamResponseReceived(iq);
    else if (iq.type() == QXmppIq::Error) {
        // remote user cancelled stream initiation
        job->terminate(QXmppTransferJob::ProtocolError);
    }
}

QXmppTransferJob *QXmppTransferManager::sendFile(const QString &jid, const QString &fileName)
{
    QFileInfo info(fileName);

    // create job
    QXmppTransferJob *job = new QXmppTransferJob(jid, QXmppTransferJob::OutgoingDirection, this);
    job->m_sid = generateStanzaHash();
    job->m_fileDate = info.lastModified();
    job->m_fileName = info.fileName();
    job->m_fileSize = info.size();

    // check we support some methods
    if (m_supportedMethods == QXmppTransferJob::NoMethod)
    {
        job->terminate(QXmppTransferJob::ProtocolError);
        return job;
    }

    // open file
    job->m_iodevice = new QFile(fileName, job);
    if (!job->m_iodevice->open(QIODevice::ReadOnly))
    {
        job->terminate(QXmppTransferJob::FileAccessError);
        return job;
    }

    // hash file
    if (!job->m_iodevice->isSequential())
    {
        QByteArray buffer;
        while (job->m_iodevice->bytesAvailable())
        {
            buffer = job->m_iodevice->read(16384);
            job->m_hash.addData(buffer);
        }
        job->m_iodevice->reset();
        job->m_fileHash = job->m_hash.result();
        job->m_hash.reset();
    }

    // prepare negotiation
    QXmppElementList items;

    QXmppElement file;
    file.setTagName("file");
    file.setAttribute("xmlns", ns_stream_initiation_file_transfer);
    file.setAttribute("date", datetimeToString(job->fileDate()));
    file.setAttribute("hash", job->fileHash().toHex());
    file.setAttribute("name", job->fileName());
    file.setAttribute("size", QString::number(job->fileSize()));
    items.append(file);
 
    QXmppElement feature;
    feature.setTagName("feature");
    feature.setAttribute("xmlns", ns_feature_negotiation);

    QXmppElement x;
    x.setTagName("x");
    x.setAttribute("xmlns", "jabber:x:data");
    x.setAttribute("type", "form");
    feature.appendChild(x);

    QXmppElement field;
    field.setTagName("field");
    field.setAttribute("var", "stream-method");
    field.setAttribute("type", "list-single");
    x.appendChild(field);

    // add supported stream methods
    if (m_supportedMethods & QXmppTransferJob::InBandMethod)
    {
        QXmppElement option;
        option.setTagName("option");
        field.appendChild(option);

        QXmppElement value;
        value.setTagName("value");
        value.setValue(ns_ibb);
        option.appendChild(value);
    }
    if (m_supportedMethods & QXmppTransferJob::SocksMethod)
    {
        QXmppElement option;
        option.setTagName("option");
        field.appendChild(option);

        QXmppElement value;
        value.setTagName("value");
        value.setValue(ns_bytestreams);
        option.appendChild(value);
    }

    items.append(feature);

    // start job
    m_jobs.append(job);

    QXmppStreamInitiationIq request;
    request.setType(QXmppIq::Set);
    request.setTo(jid);
    request.setProfile(QXmppStreamInitiationIq::FileTransfer);
    request.setSiItems(items);
    request.setSiId(job->m_sid);
    job->m_requestId = request.id();
    m_client->sendPacket(request);

    return job;
}

void QXmppTransferManager::socksClientDataReceived()
{
    QXmppSocksClient *socks = qobject_cast<QXmppSocksClient*>(sender());
    QXmppTransferJob *job = getJobBySocksClient(socks);
    if (!job || job->state() != QXmppTransferJob::TransferState)
        return;

    job->writeData(job->m_socksClient->readAll());
}

void QXmppTransferManager::socksClientDisconnected()
{
    QXmppSocksClient *socks = qobject_cast<QXmppSocksClient*>(sender());
    QXmppTransferJob *job = getJobBySocksClient(socks);
    if (!job || job->state() == QXmppTransferJob::FinishedState)
        return;

    // check received data
    job->checkData();
}

void QXmppTransferManager::socksServerConnected(QTcpSocket *socket, const QString &hostName, quint16 port)
{
    const QString ownJid = m_client->getConfiguration().jid();
    foreach (QXmppTransferJob *job, m_jobs)
    {
        if (hostName == streamHash(job->m_sid, ownJid, job->jid()) && port == 0)
        {
            job->m_socksSocket = socket;
            return;
        }
    }
    qWarning("QXmppSocksServer got a connection for a unknown stream");
    socket->close();
}

void QXmppTransferManager::socksSocketDataSent()
{
    QTcpSocket *socksSocket = qobject_cast<QTcpSocket*>(sender());
    QXmppTransferJob *job = getJobBySocksSocket(socksSocket);
    if (!job || job->state() != QXmppTransferJob::TransferState)
        return;

    // send next data block
    socksServerSendData(job);
}

void QXmppTransferManager::socksSocketDisconnected()
{
    QTcpSocket *socksSocket = qobject_cast<QTcpSocket*>(sender());
    QXmppTransferJob *job = getJobBySocksSocket(socksSocket);
    if (!job || job->state() == QXmppTransferJob::FinishedState)
        return;

    // terminate transfer
    job->terminate(QXmppTransferJob::ProtocolError);
}

void QXmppTransferManager::socksServerSendData(QXmppTransferJob *job)
{
    const QByteArray buffer = job->m_iodevice->read(job->m_blockSize);
    if (buffer.size())
    {
        job->m_socksSocket->write(buffer);
        job->m_done += buffer.size();
        job->progress(job->m_done, job->fileSize());
    } else {
        // FIXME : close socket
        job->terminate(QXmppTransferJob::NoError);
    }
}

void QXmppTransferManager::socksServerSendOffer(QXmppTransferJob *job)
{
    const QString ownJid = m_client->getConfiguration().jid();

    // discover local IPs
    QList<QXmppByteStreamIq::StreamHost> streamHosts;
    foreach (const QNetworkInterface &interface, QNetworkInterface::allInterfaces())
    {
        if (!(interface.flags() & QNetworkInterface::IsRunning) ||
            interface.flags() & QNetworkInterface::IsLoopBack)
            continue;

        foreach (const QNetworkAddressEntry &entry, interface.addressEntries())
        {
            if (entry.ip().protocol() != QAbstractSocket::IPv4Protocol ||
                entry.netmask().isNull() ||
                entry.netmask() == QHostAddress::Broadcast)
                continue;

            QXmppByteStreamIq::StreamHost streamHost;
            streamHost.setHost(entry.ip());
            streamHost.setPort(m_socksServer->serverPort());
            streamHost.setJid(ownJid);
            streamHosts.append(streamHost);
        }
    }
    if (!job->m_socksProxy.jid().isEmpty())
        streamHosts.append(job->m_socksProxy);

    // check we have some stream hosts
    if (!streamHosts.size())
    {
        qWarning("Could not determine local stream hosts");
        job->terminate(QXmppTransferJob::ProtocolError);
        return;
    }

    // send offer
    QXmppByteStreamIq streamIq;
    streamIq.setType(QXmppIq::Set);
    streamIq.setTo(job->m_jid);
    streamIq.setSid(job->m_sid);
    streamIq.setStreamHosts(streamHosts);
    job->m_requestId = streamIq.id();
    m_client->sendPacket(streamIq);
}

void QXmppTransferManager::streamInitiationIqReceived(const QXmppStreamInitiationIq &iq)
{
    if (iq.type() == QXmppIq::Result)
        streamInitiationResultReceived(iq);
    else if (iq.type() == QXmppIq::Set)
        streamInitiationSetReceived(iq);
}

void QXmppTransferManager::streamInitiationResultReceived(const QXmppStreamInitiationIq &iq)
{
    QXmppTransferJob *job = getJobByRequestId(iq.from(), iq.id());
    if (!job ||
        job->direction() != QXmppTransferJob::OutgoingDirection)
        return;

    foreach (const QXmppElement &item, iq.siItems())
    {
        if (item.tagName() == "feature" && item.attribute("xmlns") == ns_feature_negotiation)
        {
            QXmppElement field = item.firstChildElement("x").firstChildElement("field");
            while (!field.isNull())
            {
                if (field.attribute("var") == "stream-method")
                {
                    if ((field.firstChildElement("value").value() == ns_ibb) &&
                        (m_supportedMethods & QXmppTransferJob::InBandMethod))
                        job->m_method = QXmppTransferJob::InBandMethod;
                    else if ((field.firstChildElement("value").value() == ns_bytestreams) &&
                             (m_supportedMethods & QXmppTransferJob::SocksMethod))
                        job->m_method = QXmppTransferJob::SocksMethod;
                }
                field = field.nextSiblingElement("field");
            }
        }
    }

    if (job->method() == QXmppTransferJob::InBandMethod)
    {
        // lower block size for IBB
        job->m_blockSize = m_ibbBlockSize;

        QXmppIbbOpenIq openIq;
        openIq.setTo(job->m_jid);
        openIq.setSid(job->m_sid);
        openIq.setBlockSize(job->m_blockSize);
        job->m_requestId = openIq.id();
        m_client->sendPacket(openIq);
    } else if (job->method() == QXmppTransferJob::SocksMethod) {
        if (!m_socksServer->isListening())
        {
            qWarning() << "QXmppSocksServer is not listening";
            job->terminate(QXmppTransferJob::ProtocolError);
            return;
        }
        if (!m_proxy.isEmpty())
        {
            job->m_socksProxy.setJid(m_proxy);

            // query proxy
            QXmppByteStreamIq streamIq;
            streamIq.setType(QXmppIq::Get);
            streamIq.setTo(job->m_socksProxy.jid());
            streamIq.setSid(job->m_sid);
            job->m_requestId = streamIq.id();
            m_client->sendPacket(streamIq);
        } else {
            socksServerSendOffer(job);
        }
    } else {
        qWarning("We received an unsupported method");
        job->terminate(QXmppTransferJob::ProtocolError);
    }
}

void QXmppTransferManager::streamInitiationSetReceived(const QXmppStreamInitiationIq &iq)
{
    QXmppStreamInitiationIq response;
    response.setTo(iq.from());
    response.setId(iq.id());

    // check we support the profile
    if (iq.profile() != QXmppStreamInitiationIq::FileTransfer)
    {
        // FIXME : we should add:
        // <bad-profile xmlns='http://jabber.org/protocol/si'/>
        QXmppStanza::Error error(QXmppStanza::Error::Cancel, QXmppStanza::Error::BadRequest);
        error.setCode(400);

        response.setType(QXmppIq::Error);
        response.setError(error);
        m_client->sendPacket(response);
        return;
    }

    // check the stream type
    QXmppTransferJob *job = new QXmppTransferJob(iq.from(), QXmppTransferJob::IncomingDirection, this);
    int offeredMethods = QXmppTransferJob::NoMethod;
    job->m_sid = iq.siId();
    job->m_mimeType = iq.mimeType();
    foreach (const QXmppElement &item, iq.siItems())
    {
        if (item.tagName() == "feature" && item.attribute("xmlns") == ns_feature_negotiation)
        {
            QXmppElement field = item.firstChildElement("x").firstChildElement("field");
            while (!field.isNull())
            {
                if (field.attribute("var") == "stream-method" && field.attribute("type") == "list-single")
                {
                    QXmppElement option = field.firstChildElement("option");
                    while (!option.isNull())
                    {
                        if (option.firstChildElement("value").value() == ns_ibb)
                            offeredMethods = offeredMethods | QXmppTransferJob::InBandMethod;
                        else if (option.firstChildElement("value").value() == ns_bytestreams)
                            offeredMethods = offeredMethods | QXmppTransferJob::SocksMethod;
                        option = option.nextSiblingElement("option");
                    }
                }
                field = field.nextSiblingElement("field");
            }
        }
        else if (item.tagName() == "file" && item.attribute("xmlns") == ns_stream_initiation_file_transfer)
        {
            job->m_fileDate = datetimeFromString(item.attribute("date"));
            job->m_fileHash = QByteArray::fromHex(item.attribute("hash").toAscii());
            job->m_fileName = item.attribute("name");
            job->m_fileSize = item.attribute("size").toInt();
        }
    }

    // select a method supported by both parties
    int sharedMethods = (offeredMethods & m_supportedMethods);
    if (sharedMethods & QXmppTransferJob::SocksMethod)
        job->m_method = QXmppTransferJob::SocksMethod;
    else if (sharedMethods & QXmppTransferJob::InBandMethod)
        job->m_method = QXmppTransferJob::InBandMethod;
    else
    {
        // FIXME : we should add:
        // <no-valid-streams xmlns='http://jabber.org/protocol/si'/>
        QXmppStanza::Error error(QXmppStanza::Error::Cancel, QXmppStanza::Error::BadRequest);
        error.setCode(400);

        response.setType(QXmppIq::Error);
        response.setError(error);
        m_client->sendPacket(response);
    
        delete job;
        return;
    }

    // allow user to accept or decline the job
    emit fileReceived(job);
    if (!job->m_iodevice)
    {
        QXmppStanza::Error error(QXmppStanza::Error::Cancel, QXmppStanza::Error::Forbidden);
        error.setCode(403);

        response.setType(QXmppIq::Error);
        response.setError(error);
        m_client->sendPacket(response);

        delete job;
        return;
    }

    // the job was accepted
    m_jobs.append(job);

    QXmppElement value;
    value.setTagName("value");
    if (job->method() == QXmppTransferJob::InBandMethod)
        value.setValue(ns_ibb);
    else if (job->method() == QXmppTransferJob::SocksMethod)
        value.setValue(ns_bytestreams);

    QXmppElement field;
    field.setTagName("field");
    field.setAttribute("var", "stream-method");
    field.appendChild(value);

    QXmppElement x;
    x.setTagName("x");
    x.setAttribute("xmlns", "jabber:x:data");
    x.setAttribute("type", "submit");
    x.appendChild(field);

    QXmppElement feature;
    feature.setTagName("feature");
    feature.setAttribute("xmlns", ns_feature_negotiation);
    feature.appendChild(x);

    response.setType(QXmppIq::Result);
    response.setProfile(iq.profile());
    response.setSiItems(feature);

    m_client->sendPacket(response);
}

/// Return the bytestream proxy.
QString QXmppTransferManager::proxy() const
{
    return m_proxy;
}

/// Set the bytestream proxy.
void QXmppTransferManager::setProxy(const QString &proxy)
{
    m_proxy = proxy;
}

/// Return the supported stream methods.
int QXmppTransferManager::supportedMethods() const
{
    return m_supportedMethods;
}

/// Set the supported stream methods. This allows you to selectively
/// enable or disable stream methods (In-Band or SOCKS5 bytestreams).
///
/// The methods argument is a combination of zero or more
/// QXmppTransferJob::Method.
///
void QXmppTransferManager::setSupportedMethods(int methods)
{
    m_supportedMethods = (methods & QXmppTransferJob::AnyMethod);
}
