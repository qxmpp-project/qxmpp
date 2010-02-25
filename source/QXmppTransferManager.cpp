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

#include <QCryptographicHash>
#include <QDomElement>
#include <QFile>
#include <QFileInfo>
#include <QNetworkInterface>

#include "QXmppByteStreamIq.h"
#include "QXmppClient.h"
#include "QXmppConstants.h"
#include "QXmppIbbIqs.h"
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
    m_iodevice(0),
    m_jid(jid),
    m_method(NoMethod),
    m_state(StartState),
    m_fileSize(0),
    m_ibbSequence(0),
    m_socksClient(0),
    m_socksServer(0)
{
}

void QXmppTransferJob::accept(QIODevice *iodevice)
{
    if (!m_iodevice)
        m_iodevice = iodevice;
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

QString QXmppTransferJob::fileHash() const
{
    return m_fileHash;
}

QString QXmppTransferJob::fileName() const
{
    return m_fileName;
}

int QXmppTransferJob::fileSize() const
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
    if (m_socksServer)
        m_socksServer->close();

    // emit signals
    emit stateChanged(m_state);
    if (cause == NoError)
        emit finished();
    else
        emit error(m_error);
}

QXmppTransferManager::QXmppTransferManager(QXmppClient *client)
    : m_client(client),
    m_ibbBlockSize(4096),
    m_supportedMethods(QXmppTransferJob::AnyMethod)
{
}

void QXmppTransferManager::byteStreamIqReceived(const QXmppByteStreamIq &iq)
{
    if (iq.getType() == QXmppIq::Result)
        byteStreamResultReceived(iq);
    else if (iq.getType() == QXmppIq::Set)
        byteStreamSetReceived(iq);
}

void QXmppTransferManager::byteStreamResponseReceived(const QXmppIq &iq)
{
    QXmppTransferJob *job = getJobByRequestId(iq.getFrom(), iq.getId());
    if (!job ||
        job->method() != QXmppTransferJob::SocksMethod ||
        job->state() != QXmppTransferJob::StartState)
        return;

    if (iq.getType() == QXmppIq::Error)
        job->terminate(QXmppTransferJob::ProtocolError);
}

/// Handle a bytestream result, i.e. after the remote party has connected to our stream host.
void QXmppTransferManager::byteStreamResultReceived(const QXmppByteStreamIq &iq)
{
    QXmppTransferJob *job = getJobByRequestId(iq.getFrom(), iq.getId());
    if (!job ||
        job->method() != QXmppTransferJob::SocksMethod ||
        job->state() != QXmppTransferJob::StartState)
        return;

    // start sending data
    job->setState(QXmppTransferJob::TransferState);
    connect(job->m_socksServer, SIGNAL(bytesWritten(qint64)), this, SLOT(socksServerDataSent()));
    connect(job->m_socksServer, SIGNAL(disconnected()), this, SLOT(socksServerDisconnected()));
    socksServerSendData(job);
}

/// Handle a bytestream set, i.e. an invitation from the remote party to connect to their stream host.
void QXmppTransferManager::byteStreamSetReceived(const QXmppByteStreamIq &iq)
{
    QXmppIq response;
    response.setId(iq.getId());
    response.setTo(iq.getFrom());

    QXmppTransferJob *job = getJobBySid(iq.getFrom(), iq.sid());
    if (!job ||
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

        QString hostName = streamHash(job->m_sid,
                                      streamHost.jid(),
                                      m_client->getConfiguration().getJid());

        // try to connect to stream host
        job->m_socksClient = new QXmppSocksClient(streamHost.host(), streamHost.port(), job);
        job->m_socksClient->connectToHost(hostName, 0);
        if (job->m_socksClient->waitForConnected())
        {
            job->setState(QXmppTransferJob::TransferState);
            connect(job->m_socksClient, SIGNAL(readyRead()), this, SLOT(socksClientDataReceived()));
            connect(job->m_socksClient, SIGNAL(disconnected()), this, SLOT(socksClientDisconnected()));

            QXmppByteStreamIq ackIq;
            ackIq.setId(iq.getId());
            ackIq.setTo(iq.getFrom());
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

QXmppTransferJob* QXmppTransferManager::getJobBySocksServer(QXmppSocksServer *socksServer)
{
    foreach (QXmppTransferJob *job, m_jobs)
        if (job->m_socksServer == socksServer)
            return job;
    return 0;
}

void QXmppTransferManager::ibbCloseIqReceived(const QXmppIbbCloseIq &iq)
{
    QXmppIq response;
    response.setTo(iq.getFrom());
    response.setId(iq.getId());

    QXmppTransferJob *job = getJobBySid(iq.getFrom(), iq.getSid());
    if (!job || job->method() != QXmppTransferJob::InBandMethod)
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

    // terminate the transfer
    if (job->fileSize() && job->m_done != job->fileSize())
        job->terminate(QXmppTransferJob::FileCorruptError);
    else
        job->terminate(QXmppTransferJob::NoError);
}

void QXmppTransferManager::ibbDataIqReceived(const QXmppIbbDataIq &iq)
{
    QXmppIq response;
    response.setTo(iq.getFrom());
    response.setId(iq.getId());

    QXmppTransferJob *job = getJobBySid(iq.getFrom(), iq.getSid());
    if (!job || job->method() != QXmppTransferJob::InBandMethod)
    {
        // the job is unknown, cancel it
        QXmppStanza::Error error(QXmppStanza::Error::Cancel, QXmppStanza::Error::ItemNotFound);
        response.setType(QXmppIq::Error);
        response.setError(error);
        m_client->sendPacket(response);
        return;
    }

    if (iq.getSequence() != job->m_ibbSequence)
    {
        // the packet is out of sequence
        QXmppStanza::Error error(QXmppStanza::Error::Cancel, QXmppStanza::Error::UnexpectedRequest);
        response.setType(QXmppIq::Error);
        response.setError(error);
        m_client->sendPacket(response);
        return;
    }

    // write data
    const QByteArray data = iq.getPayload();
    job->m_iodevice->write(data);
    job->m_done += data.size();
    job->m_ibbSequence++;
    job->progress(job->m_done, job->fileSize());

    // acknowledge the packet
    response.setType(QXmppIq::Result);
    m_client->sendPacket(response);
}

void QXmppTransferManager::ibbOpenIqReceived(const QXmppIbbOpenIq &iq)
{
    QXmppIq response;
    response.setTo(iq.getFrom());
    response.setId(iq.getId());

    QXmppTransferJob *job = getJobBySid(iq.getFrom(), iq.getSid());
    if (!job || job->method() != QXmppTransferJob::InBandMethod)
    {
        // the job is unknown, cancel it
        QXmppStanza::Error error(QXmppStanza::Error::Cancel, QXmppStanza::Error::ItemNotFound);
        response.setType(QXmppIq::Error);
        response.setError(error);
        m_client->sendPacket(response);
        return;
    }

    if (iq.getBlockSize() > m_ibbBlockSize)
    {
        // we prefer a smaller block size
        QXmppStanza::Error error(QXmppStanza::Error::Modify, QXmppStanza::Error::ResourceConstraint);
        response.setType(QXmppIq::Error);
        response.setError(error);
        m_client->sendPacket(response);
        return;
    }

    job->m_blockSize = iq.getBlockSize();
    job->setState(QXmppTransferJob::TransferState);

    // accept transfer
    response.setType(QXmppIq::Result);
    m_client->sendPacket(response);
}

void QXmppTransferManager::ibbResponseReceived(const QXmppIq &iq)
{
    QXmppTransferJob *job = getJobByRequestId(iq.getFrom(), iq.getId());
    if (!job ||
        job->method() != QXmppTransferJob::InBandMethod ||
        job->state() == QXmppTransferJob::FinishedState)
        return;

    // if the IO device is closed, do nothing
    if (!job->m_iodevice->isOpen())
        return;

    if (iq.getType() == QXmppIq::Result)
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
            job->m_requestId = dataIq.getId();
            m_client->sendPacket(dataIq);

            job->m_done += buffer.size();
            job->progress(job->m_done, job->fileSize());
        } else {
            // close the bytestream
            QXmppIbbCloseIq closeIq;
            closeIq.setTo(job->m_jid);
            closeIq.setSid(job->m_sid);
            job->m_requestId = closeIq.getId();
            m_client->sendPacket(closeIq);

            job->terminate(QXmppTransferJob::NoError);
        }
    }
    else if (iq.getType() == QXmppIq::Error)
    {
        // close the bytestream
        QXmppIbbCloseIq closeIq;
        closeIq.setTo(job->m_jid);
        closeIq.setSid(job->m_sid);
        job->m_requestId = closeIq.getId();
        m_client->sendPacket(closeIq);

        job->terminate(QXmppTransferJob::ProtocolError);
    }
}

void QXmppTransferManager::iqReceived(const QXmppIq &iq)
{
    QXmppTransferJob *job = getJobByRequestId(iq.getFrom(), iq.getId());
    if (!job)
        return;

    if (job->method() == QXmppTransferJob::InBandMethod)
        ibbResponseReceived(iq);
    else if (job->method() == QXmppTransferJob::SocksMethod)
        byteStreamResponseReceived(iq);
    else if (iq.getType() == QXmppIq::Error) {
        // remote user cancelled stream initiation
        job->terminate(QXmppTransferJob::ProtocolError);
    }
}

QXmppTransferJob *QXmppTransferManager::sendFile(const QString &jid, const QString &fileName)
{

    // create job
    QXmppTransferJob *job = new QXmppTransferJob(jid, QXmppTransferJob::OutgoingDirection, this);

    // open file
    QFile *fileIo = new QFile(fileName, job);
    fileIo->open(QIODevice::ReadOnly);
    QFileInfo info(*fileIo);

    job->m_iodevice = fileIo;
    job->m_sid = generateStanzaHash();
    job->m_fileDate = info.lastModified();
    job->m_fileName = info.fileName();
    job->m_fileSize = info.size();
    m_jobs.append(job);

    // prepare negotiation
    QXmppElementList items;

    QXmppElement file;
    file.setTagName("file");
    file.setAttribute("xmlns", ns_stream_initiation_file_transfer);
    file.setAttribute("date", datetimeToString(job->fileDate()));
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

    QXmppStreamInitiationIq request;
    request.setType(QXmppIq::Set);
    request.setTo(jid);
    request.setProfile(QXmppStreamInitiationIq::FileTransfer);
    request.setSiItems(items);
    request.setSiId(job->m_sid);
    job->m_requestId = request.getId();
    m_client->sendPacket(request);

    return job;
}

void QXmppTransferManager::socksClientDataReceived()
{
    QXmppSocksClient *socks = qobject_cast<QXmppSocksClient*>(sender());
    QXmppTransferJob *job = getJobBySocksClient(socks);
    if (!job || job->state() != QXmppTransferJob::TransferState)
        return;

    QByteArray data = job->m_socksClient->readAll();
    job->m_iodevice->write(data);
    job->m_done += data.size();
    job->progress(job->m_done, job->fileSize());
}

void QXmppTransferManager::socksClientDisconnected()
{
    QXmppSocksClient *socks = qobject_cast<QXmppSocksClient*>(sender());
    QXmppTransferJob *job = getJobBySocksClient(socks);
    if (!job || job->state() == QXmppTransferJob::FinishedState)
        return;

    // terminate the transfer
    if (job->fileSize() && job->m_done != job->fileSize())
        job->terminate(QXmppTransferJob::FileCorruptError);
    else
        job->terminate(QXmppTransferJob::NoError);
}

void QXmppTransferManager::socksServerDataSent()
{
    QXmppSocksServer *socksServer = qobject_cast<QXmppSocksServer*>(sender());
    QXmppTransferJob *job = getJobBySocksServer(socksServer);
    if (!job || job->state() != QXmppTransferJob::TransferState)
        return;

    // send next data block
    socksServerSendData(job);
}

void QXmppTransferManager::socksServerDisconnected()
{
    QXmppSocksServer *socksServer = qobject_cast<QXmppSocksServer*>(sender());
    QXmppTransferJob *job = getJobBySocksServer(socksServer);
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
        job->m_socksServer->write(buffer);

        job->m_done += buffer.size();
        job->progress(job->m_done, job->fileSize());
    } else {
        // FIXME : close socket
        job->terminate(QXmppTransferJob::NoError);
    }
}

void QXmppTransferManager::streamInitiationIqReceived(const QXmppStreamInitiationIq &iq)
{
    if (iq.getType() == QXmppIq::Result)
        streamInitiationResultReceived(iq);
    else if (iq.getType() == QXmppIq::Set)
        streamInitiationSetReceived(iq);
}

void QXmppTransferManager::streamInitiationResultReceived(const QXmppStreamInitiationIq &iq)
{
    QXmppTransferJob *job = getJobByRequestId(iq.getFrom(), iq.getId());
    if (!job)
        return;

    foreach (const QXmppElement &item, iq.getSiItems())
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
        m_client->sendPacket(openIq);
    } else if (job->method() == QXmppTransferJob::SocksMethod) {
        QXmppByteStreamIq streamIq;
        streamIq.setType(QXmppIq::Set);
        streamIq.setTo(job->m_jid);
        streamIq.setSid(job->m_sid);

        const QString ownJid = m_client->getConfiguration().getJid();
        QList<QXmppByteStreamIq::StreamHost> streamHosts;

        // find interface to bind to
        job->m_socksServer = new QXmppSocksServer(this);
        job->m_socksServer->setHostName(streamHash(job->m_sid, ownJid, job->jid()));
        job->m_socksServer->setHostPort(0);
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

                // we let the server pick a port
                if (!job->m_socksServer->listen(entry.ip()))
                {
                    qWarning() << "QXmppSocksServer could not listen on address" << entry.ip();
                    continue;
                }

                qDebug() << "QXmppSocksServer listening on" << job->m_socksServer->serverAddress() << job->m_socksServer->serverPort();

                QXmppByteStreamIq::StreamHost streamHost;
                streamHost.setHost(job->m_socksServer->serverAddress());
                streamHost.setPort(job->m_socksServer->serverPort());
                streamHost.setJid(ownJid);
                streamHosts.append(streamHost);
                streamIq.setStreamHosts(streamHosts);
                job->m_requestId = streamIq.getId();
                m_client->sendPacket(streamIq);
                return;
            }
        }
        qWarning("Could not determine public IP");
        job->terminate(QXmppTransferJob::ProtocolError);
    } else {
        qWarning("We received an unsupported method");
        job->terminate(QXmppTransferJob::ProtocolError);
    }
}

void QXmppTransferManager::streamInitiationSetReceived(const QXmppStreamInitiationIq &iq)
{
    QXmppStreamInitiationIq response;
    response.setTo(iq.getFrom());
    response.setId(iq.getId());

    // check we support the profile
    if (iq.getProfile() != QXmppStreamInitiationIq::FileTransfer)
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
    QXmppTransferJob *job = new QXmppTransferJob(iq.getFrom(), QXmppTransferJob::IncomingDirection, this);
    int offeredMethods = QXmppTransferJob::NoMethod;
    job->m_sid = iq.getSiId();
    job->m_mimeType = iq.getMimeType();
    foreach (const QXmppElement &item, iq.getSiItems())
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
            job->m_fileHash = item.attribute("hash");
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
    response.setProfile(iq.getProfile());
    response.setSiItems(feature);

    m_client->sendPacket(response);
}

int QXmppTransferManager::supportedMethods() const
{
    return m_supportedMethods;
}

void QXmppTransferManager::setSupportedMethods(int methods)
{
    m_supportedMethods = methods;
}
