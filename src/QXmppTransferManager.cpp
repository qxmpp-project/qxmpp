/*
 * Copyright (C) 2008-2010 The QXmpp developers
 *
 * Author:
 *  Jeremy Lainé
 *
 * Source:
 *  http://code.google.com/p/qxmpp
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
#include "QXmppConstants.h"
#include "QXmppIbbIq.h"
#include "QXmppSocks.h"
#include "QXmppOutgoingClient.h"
#include "QXmppStreamInitiationIq.h"
#include "QXmppTransferManager.h"
#include "QXmppUtils.h"

// time to try to connect to a SOCKS host (7 seconds)
const int socksTimeout = 7000;

static QString streamHash(const QString &sid, const QString &initiatorJid, const QString &targetJid)
{
    QCryptographicHash hash(QCryptographicHash::Sha1);
    QString str = sid + initiatorJid + targetJid;
    hash.addData(str.toAscii());
    return hash.result().toHex();
}

QXmppTransferFileInfo::QXmppTransferFileInfo()
    : m_size(0)
{
}

QDateTime QXmppTransferFileInfo::date() const
{
    return m_date;
}

void QXmppTransferFileInfo::setDate(const QDateTime &date)
{
    m_date = date;
}

QByteArray QXmppTransferFileInfo::hash() const
{
    return m_hash;
}

void QXmppTransferFileInfo::setHash(const QByteArray &hash)
{
    m_hash = hash;
}

QString QXmppTransferFileInfo::name() const
{
    return m_name;
}

void QXmppTransferFileInfo::setName(const QString &name)
{
    m_name = name;
}

qint64 QXmppTransferFileInfo::size() const
{
    return m_size;
}

void QXmppTransferFileInfo::setSize(qint64 size)
{
    m_size = size;
}

bool QXmppTransferFileInfo::operator==(const QXmppTransferFileInfo &other) const
{
    return other.m_size == m_size &&
        other.m_hash == m_hash &&
        other.m_name == m_name;
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
    m_state(OfferState),
    m_ibbSequence(0),
    m_socksSocket(0)
{
}

/// Call this method if you wish to abort on ongoing transfer job.
///

void QXmppTransferJob::abort()
{
    terminate(AbortError);
}

/// Call this method if you wish to accept an incoming transfer job.
///

void QXmppTransferJob::accept(QIODevice *iodevice)
{
    if (m_direction == IncomingDirection && m_state == OfferState && !m_iodevice)
    {
        m_iodevice = iodevice;
        setState(QXmppTransferJob::StartState);
    }
}

void QXmppTransferJob::checkData()
{
    if ((m_fileInfo.size() && m_done != m_fileInfo.size()) ||
        (!m_fileInfo.hash().isEmpty() && m_hash.result() != m_fileInfo.hash()))
        terminate(QXmppTransferJob::FileCorruptError);
    else
        terminate(QXmppTransferJob::NoError);
}

/// Returns the job's data for a given role.
///
/// You can associate arbitrary data with the role using setData().

QVariant QXmppTransferJob::data(int role) const
{
    return m_data.value(role);
}

/// Sets the data for a given role to the given value.
///
/// You can set any data you want for use in your application, this
/// data will not be used internally by QXmppTransferManager.

void QXmppTransferJob::setData(int role, const QVariant &value)
{
    m_data.insert(role, value);
}

/// Returns the job's transfer direction.
///

QXmppTransferJob::Direction QXmppTransferJob::direction() const
{
    return m_direction;
}

/// Returns the last error that was encountered.
///

QXmppTransferJob::Error QXmppTransferJob::error() const
{
    return m_error;
}

/// Returns the remote party's JID.
///

QString QXmppTransferJob::jid() const
{
    return m_jid;
}

/// Returns meta-data about the file being transferred.
///

QXmppTransferFileInfo QXmppTransferJob::fileInfo() const
{
    return m_fileInfo;
}

QDateTime QXmppTransferJob::fileDate() const
{
    return m_fileInfo.date();
}

QByteArray QXmppTransferJob::fileHash() const
{
    return m_fileInfo.hash();
}

QString QXmppTransferJob::fileName() const
{
    return m_fileInfo.name();
}

qint64 QXmppTransferJob::fileSize() const
{
    return m_fileInfo.size();
}

/// Returns the job's transfer method.
///

QXmppTransferJob::Method QXmppTransferJob::method() const
{
    return m_method;
}

/// Returns the job's session identifier.
///

QString QXmppTransferJob::sid() const
{
    return m_sid;
}

/// Returns the job's transfer speed in bytes per second.
///
/// If the transfer has not started yet or is already finished, returns 0.
///

qint64 QXmppTransferJob::speed() const
{
    qint64 elapsed = m_transferStart.elapsed();
    if (m_state != QXmppTransferJob::TransferState || !elapsed)
        return 0;
    return (m_done * 1000.0) / elapsed;
}

/// Returns the job's state.
///

QXmppTransferJob::State QXmppTransferJob::state() const
{
    return m_state;
}

void QXmppTransferJob::setState(QXmppTransferJob::State state)
{
    if (m_state != state)
    {
        m_state = state;
        if (m_state == QXmppTransferJob::TransferState)
            m_transferStart.start();
        emit stateChanged(m_state);
    }
}

void QXmppTransferJob::disconnected()
{
    if (m_state == QXmppTransferJob::FinishedState)
        return;

    // terminate transfer
    if (m_direction == QXmppTransferJob::IncomingDirection)
    {
        checkData();
    } else {
        if (fileSize() && m_done != fileSize())
            terminate(QXmppTransferJob::ProtocolError);
        else
            terminate(QXmppTransferJob::NoError);
    }
}

void QXmppTransferJob::receiveData()
{
    if (m_state != QXmppTransferJob::TransferState)
        return;

    // receive data block
    if (m_direction == QXmppTransferJob::IncomingDirection)
    {
        writeData(m_socksSocket->readAll());

        // if we have received all the data, stop here
        if (fileSize() && m_done >= fileSize())
            checkData();
    }
}

void QXmppTransferJob::sendData()
{
    if (m_state != QXmppTransferJob::TransferState)
        return;

    // don't saturate the outgoing socket
    if (m_socksSocket->bytesToWrite() > 2 * m_blockSize)
        return;

    // check whether we have written the whole file
    if (m_fileInfo.size() && m_done >= m_fileInfo.size())
    {
        if (!m_socksSocket->bytesToWrite())
            terminate(QXmppTransferJob::NoError);
        return;
    }

    char *buffer = new char[m_blockSize];
    qint64 length = m_iodevice->read(buffer, m_blockSize);
    if (length < 0)
    {
        delete [] buffer;
        terminate(QXmppTransferJob::FileAccessError);
        return;
    }
    if (length > 0)
    {
        m_socksSocket->write(buffer, length);
        delete [] buffer;
        m_done += length;
        emit progress(m_done, fileSize());
    }
}

void QXmppTransferJob::slotTerminated()
{
    emit stateChanged(m_state);
    if (m_error != NoError)
        emit error(m_error);
    emit finished();
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

    // close socket
    if (m_socksSocket)
    {
        m_socksSocket->flush();
        m_socksSocket->close();
    }

    // emit signals later
    QTimer::singleShot(0, this, SLOT(slotTerminated()));
}

bool QXmppTransferJob::writeData(const QByteArray &data)
{
    const qint64 written = m_iodevice->write(data);
    if (written < 0)
        return false;
    m_done += written;
    if (!m_fileInfo.hash().isEmpty())
        m_hash.addData(data);
    progress(m_done, m_fileInfo.size());
    return true;
}

QXmppTransferManager::QXmppTransferManager(QXmppClient *client)
    : m_ibbBlockSize(4096),
    m_proxyOnly(false),
    m_socksServer(0),
    m_supportedMethods(QXmppTransferJob::AnyMethod)
{
    // XEP-0047: In-Band Bytestreams
    bool check = QObject::connect(client, SIGNAL(iqReceived(const QXmppIq&)),
        this, SLOT(iqReceived(const QXmppIq&)));
    Q_ASSERT(check);

    // start SOCKS server
    m_socksServer = new QXmppSocksServer(this);
    if (m_socksServer->listen())
    {
        check = connect(m_socksServer, SIGNAL(newConnection(QTcpSocket*, const QString&, quint16)),
                        this, SLOT(socksServerConnected(QTcpSocket*, const QString&, quint16)));
        Q_ASSERT(check);
    } else {
        qWarning("QXmppSocksServer could not start listening");
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
    QXmppTransferJob *job = getJobByRequestId(QXmppTransferJob::IncomingDirection, iq.from(), iq.id());
    if (!job ||
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
    QXmppTransferJob *job = getJobByRequestId(QXmppTransferJob::OutgoingDirection, iq.from(), iq.id());
    if (!job ||
        job->method() != QXmppTransferJob::SocksMethod ||
        job->state() != QXmppTransferJob::StartState)
        return;

    // check the stream host
    if (iq.streamHostUsed() == job->m_socksProxy.jid())
    {
        const QXmppByteStreamIq::StreamHost streamHost = job->m_socksProxy;
        info(QString("Connecting to proxy: %1 (%2:%3)").arg(
                streamHost.jid(),
                streamHost.host().toString(),
                QString::number(streamHost.port())));

        // connect to proxy
        const QString hostName = streamHash(job->m_sid,
                                            client()->configuration().jid(),
                                            job->m_jid);

        QXmppSocksClient *socksClient = new QXmppSocksClient(streamHost.host(), streamHost.port(), job);
        socksClient->connectToHost(hostName, 0);
        // FIXME : this should probably be made asynchronous as it blocks XMPP packet handling
        if (!socksClient->waitForReady(socksTimeout))
        {
            warning(QString("Failed to connect to proxy: %1 (%2:%3)").arg(
                    streamHost.jid(),
                    streamHost.host().toString(),
                    QString::number(streamHost.port())));
            delete socksClient;
            job->terminate(QXmppTransferJob::ProtocolError);
            return;
        }
        job->m_socksSocket = socksClient;
        connect(job->m_socksSocket, SIGNAL(disconnected()), job, SLOT(disconnected()));

        // activate stream
        QXmppByteStreamIq streamIq;
        streamIq.setType(QXmppIq::Set);
        streamIq.setFrom(client()->configuration().jid());
        streamIq.setTo(streamHost.jid());
        streamIq.setSid(job->m_sid);
        streamIq.setActivate(job->m_jid);
        job->m_requestId = streamIq.id();
        client()->sendPacket(streamIq);
        return;
    }

    // direction connection, start sending data
    if (!job->m_socksSocket)
    {
        warning("Client says they connected to our SOCKS server, but they did not");
        job->terminate(QXmppTransferJob::ProtocolError);
        return;
    }
    job->setState(QXmppTransferJob::TransferState);
    connect(job->m_socksSocket, SIGNAL(disconnected()), job, SLOT(disconnected()));
    connect(job->m_socksSocket, SIGNAL(bytesWritten(qint64)), job, SLOT(sendData()));
    connect(job->m_iodevice, SIGNAL(readyRead()), job, SLOT(sendData()));
    job->sendData();
}

/// Handle a bytestream set, i.e. an invitation from the remote party to connect
/// to a stream host.
void QXmppTransferManager::byteStreamSetReceived(const QXmppByteStreamIq &iq)
{
    QXmppIq response;
    response.setId(iq.id());
    response.setTo(iq.from());

    QXmppTransferJob *job = getJobBySid(QXmppTransferJob::IncomingDirection, iq.from(), iq.sid());
    if (!job ||
        job->method() != QXmppTransferJob::SocksMethod ||
        job->state() != QXmppTransferJob::StartState)
    {
        // the stream is unknown
        QXmppStanza::Error error(QXmppStanza::Error::Auth, QXmppStanza::Error::NotAcceptable);
        error.setCode(406);
        response.setType(QXmppIq::Error);
        response.setError(error);
        client()->sendPacket(response);
        return;
    }

    // try connecting to the offered stream hosts
    foreach (const QXmppByteStreamIq::StreamHost &streamHost, iq.streamHosts())
    {
        info(QString("Connecting to streamhost: %1 (%2:%3)").arg(
                streamHost.jid(),
                streamHost.host().toString(),
                QString::number(streamHost.port())));

        const QString hostName = streamHash(job->m_sid,
                                            job->m_jid,
                                            client()->configuration().jid());

        // try to connect to stream host
        QXmppSocksClient *socksClient = new QXmppSocksClient(streamHost.host(), streamHost.port(), job);
        socksClient->connectToHost(hostName, 0);
        // FIXME : this should probably be made asynchronous as it blocks XMPP packet handling
        if (socksClient->waitForReady(socksTimeout))
        {
            job->setState(QXmppTransferJob::TransferState);
            job->m_socksSocket = socksClient;
            connect(job->m_socksSocket, SIGNAL(readyRead()), job, SLOT(receiveData()));
            connect(job->m_socksSocket, SIGNAL(disconnected()), job, SLOT(disconnected()));

            QXmppByteStreamIq ackIq;
            ackIq.setId(iq.id());
            ackIq.setTo(iq.from());
            ackIq.setType(QXmppIq::Result);
            ackIq.setSid(job->m_sid);
            ackIq.setStreamHostUsed(streamHost.jid());
            client()->sendPacket(ackIq);
            return;
        } else {
            warning(QString("Failed to connect to streamhost: %1 (%2:%3)").arg(
                    streamHost.jid(),
                    streamHost.host().toString(),
                    QString::number(streamHost.port())));
            delete socksClient;
        }
    }

    // could not connect to any stream host
    QXmppStanza::Error error(QXmppStanza::Error::Cancel, QXmppStanza::Error::ItemNotFound);
    error.setCode(404);
    response.setType(QXmppIq::Error);
    response.setError(error);
    client()->sendPacket(response);

    job->terminate(QXmppTransferJob::ProtocolError);
}

QStringList QXmppTransferManager::discoveryFeatures() const
{
    return QStringList()
        << ns_ibb               // XEP-0047: In-Band Bytestreams
        << ns_bytestreams       // XEP-0065: SOCKS5 Bytestreams
        << ns_stream_initiation // XEP-0095: Stream Initiation
        << ns_stream_initiation_file_transfer; // XEP-0096: SI File Transfer
}

bool QXmppTransferManager::handleStanza(QXmppStream *stream, const QDomElement &element)
{
    if (element.tagName() != "iq")
        return false;

    // XEP-0047 In-Band Bytestreams
    if(QXmppIbbCloseIq::isIbbCloseIq(element))
    {
        QXmppIbbCloseIq ibbCloseIq;
        ibbCloseIq.parse(element);
        ibbCloseIqReceived(ibbCloseIq);
        return true;
    }
    else if(QXmppIbbDataIq::isIbbDataIq(element))
    {
        QXmppIbbDataIq ibbDataIq;
        ibbDataIq.parse(element);
        ibbDataIqReceived(ibbDataIq);
        return true;
    }
    else if(QXmppIbbOpenIq::isIbbOpenIq(element))
    {
        QXmppIbbOpenIq ibbOpenIq;
        ibbOpenIq.parse(element);
        ibbOpenIqReceived(ibbOpenIq);
        return true;
    }
    // XEP-0065: SOCKS5 Bytestreams
    else if(QXmppByteStreamIq::isByteStreamIq(element))
    {
        QXmppByteStreamIq byteStreamIq;
        byteStreamIq.parse(element);
        byteStreamIqReceived(byteStreamIq);
        return true;
    }
    // XEP-0095: Stream Initiation
    else if(QXmppStreamInitiationIq::isStreamInitiationIq(element))
    {
        QXmppStreamInitiationIq siIq;
        siIq.parse(element);
        streamInitiationIqReceived(siIq);
        return true;
    }

    return false;
}
 
QXmppTransferJob* QXmppTransferManager::getJobByRequestId(QXmppTransferJob::Direction direction, const QString &jid, const QString &id)
{
    foreach (QXmppTransferJob *job, m_jobs)
        if (job->m_direction == direction &&
            job->m_jid == jid &&
            job->m_requestId == id)
            return job;
    return 0;
}

QXmppTransferJob* QXmppTransferManager::getJobBySid(QXmppTransferJob::Direction direction, const QString &jid, const QString &sid)
{
    foreach (QXmppTransferJob *job, m_jobs)
        if (job->m_direction == direction &&
            job->m_jid == jid &&
            job->m_sid == sid)
            return job;
    return 0;
}

void QXmppTransferManager::ibbCloseIqReceived(const QXmppIbbCloseIq &iq)
{
    QXmppIq response;
    response.setTo(iq.from());
    response.setId(iq.id());

    QXmppTransferJob *job = getJobBySid(QXmppTransferJob::IncomingDirection, iq.from(), iq.sid());
    if (!job ||
        job->method() != QXmppTransferJob::InBandMethod)
    {
        // the job is unknown, cancel it
        QXmppStanza::Error error(QXmppStanza::Error::Cancel, QXmppStanza::Error::ItemNotFound);
        response.setType(QXmppIq::Error);
        response.setError(error);
        client()->sendPacket(response);
        return;
    }

    // acknowledge the packet
    response.setType(QXmppIq::Result);
    client()->sendPacket(response);

    // check received data
    job->checkData();
}

void QXmppTransferManager::ibbDataIqReceived(const QXmppIbbDataIq &iq)
{
    QXmppIq response;
    response.setTo(iq.from());
    response.setId(iq.id());

    QXmppTransferJob *job = getJobBySid(QXmppTransferJob::IncomingDirection, iq.from(), iq.sid());
    if (!job ||
        job->method() != QXmppTransferJob::InBandMethod ||
        job->state() != QXmppTransferJob::TransferState)
    {
        // the job is unknown, cancel it
        QXmppStanza::Error error(QXmppStanza::Error::Cancel, QXmppStanza::Error::ItemNotFound);
        response.setType(QXmppIq::Error);
        response.setError(error);
        client()->sendPacket(response);
        return;
    }

    if (iq.sequence() != job->m_ibbSequence)
    {
        // the packet is out of sequence
        QXmppStanza::Error error(QXmppStanza::Error::Cancel, QXmppStanza::Error::UnexpectedRequest);
        response.setType(QXmppIq::Error);
        response.setError(error);
        client()->sendPacket(response);
        return;
    }

    // write data
    job->writeData(iq.payload());
    job->m_ibbSequence++;

    // acknowledge the packet
    response.setType(QXmppIq::Result);
    client()->sendPacket(response);
}

void QXmppTransferManager::ibbOpenIqReceived(const QXmppIbbOpenIq &iq)
{
    QXmppIq response;
    response.setTo(iq.from());
    response.setId(iq.id());

    QXmppTransferJob *job = getJobBySid(QXmppTransferJob::IncomingDirection, iq.from(), iq.sid());
    if (!job ||
        job->method() != QXmppTransferJob::InBandMethod)
    {
        // the job is unknown, cancel it
        QXmppStanza::Error error(QXmppStanza::Error::Cancel, QXmppStanza::Error::ItemNotFound);
        response.setType(QXmppIq::Error);
        response.setError(error);
        client()->sendPacket(response);
        return;
    }

    if (iq.blockSize() > m_ibbBlockSize)
    {
        // we prefer a smaller block size
        QXmppStanza::Error error(QXmppStanza::Error::Modify, QXmppStanza::Error::ResourceConstraint);
        response.setType(QXmppIq::Error);
        response.setError(error);
        client()->sendPacket(response);
        return;
    }

    job->m_blockSize = iq.blockSize();
    job->setState(QXmppTransferJob::TransferState);

    // accept transfer
    response.setType(QXmppIq::Result);
    client()->sendPacket(response);
}

void QXmppTransferManager::ibbResponseReceived(const QXmppIq &iq)
{
    QXmppTransferJob *job = getJobByRequestId(QXmppTransferJob::OutgoingDirection, iq.from(), iq.id());
    if (!job ||
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
            client()->sendPacket(dataIq);

            job->m_done += buffer.size();
            job->progress(job->m_done, job->fileSize());
        } else {
            // close the bytestream
            QXmppIbbCloseIq closeIq;
            closeIq.setTo(job->m_jid);
            closeIq.setSid(job->m_sid);
            job->m_requestId = closeIq.id();
            client()->sendPacket(closeIq);

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
        client()->sendPacket(closeIq);

        job->terminate(QXmppTransferJob::ProtocolError);
    }
}

void QXmppTransferManager::iqReceived(const QXmppIq &iq)
{
    foreach (QXmppTransferJob *job, m_jobs)
    {
        // handle IQ from proxy
        if (job->m_socksProxy.jid() == iq.from() && job->m_requestId == iq.id())
        {
            if (job->m_socksSocket)
            {
                // proxy connection activation result
                if (iq.type() == QXmppIq::Result)
                {
                    // proxy stream activated, start sending data
                    job->setState(QXmppTransferJob::TransferState);
                    connect(job->m_socksSocket, SIGNAL(bytesWritten(qint64)), job, SLOT(sendData()));
                    connect(job->m_iodevice, SIGNAL(readyRead()), job, SLOT(sendData()));
                    job->sendData();
                } else if (iq.type() == QXmppIq::Error) {
                    // proxy stream not activated, terminate
                    warning("Could not activate SOCKS5 proxy bytestream");
                    job->terminate(QXmppTransferJob::ProtocolError);
                }
            } else {
                // we could not get host/port from proxy, procede without a proxy
                if (iq.type() == QXmppIq::Error)
                    socksServerSendOffer(job);
            }
            return;
        }

        // handle IQ from peer
        else if (job->m_jid == iq.from() && job->m_requestId == iq.id())
        {
            if (job->direction() == QXmppTransferJob::OutgoingDirection &&
                job->method() == QXmppTransferJob::InBandMethod)
            {
                ibbResponseReceived(iq);
                return;
            }
            else if (job->direction() == QXmppTransferJob::IncomingDirection &&
                     job->method() == QXmppTransferJob::SocksMethod)
            {
                byteStreamResponseReceived(iq);
                return;
            }
            else if (job->direction() == QXmppTransferJob::OutgoingDirection &&
                     iq.type() == QXmppIq::Error)
            {
                // remote party cancelled stream initiation
                job->terminate(QXmppTransferJob::AbortError);
                return;
            }
        }
    }
}

void QXmppTransferManager::jobDestroyed(QObject *object)
{
    m_jobs.removeAll(static_cast<QXmppTransferJob*>(object));
}

void QXmppTransferManager::jobError(QXmppTransferJob::Error error)
{
    QXmppTransferJob *job = qobject_cast<QXmppTransferJob *>(sender());
    if (!job || !m_jobs.contains(job))
        return;

    if (job->direction() == QXmppTransferJob::OutgoingDirection &&
        job->method() == QXmppTransferJob::InBandMethod &&
        error == QXmppTransferJob::AbortError)
    {
        // close the bytestream
        QXmppIbbCloseIq closeIq;
        closeIq.setTo(job->m_jid);
        closeIq.setSid(job->m_sid);
        job->m_requestId = closeIq.id();
        client()->sendPacket(closeIq);
    }
}

void QXmppTransferManager::jobFinished()
{
    QXmppTransferJob *job = qobject_cast<QXmppTransferJob *>(sender());
    if (!job || !m_jobs.contains(job))
        return;

    emit finished(job);
}

void QXmppTransferManager::jobStateChanged(QXmppTransferJob::State state)
{
    QXmppTransferJob *job = qobject_cast<QXmppTransferJob *>(sender());
    if (!job || !m_jobs.contains(job))
        return;

    if (job->direction() != QXmppTransferJob::IncomingDirection)
        return;

    // disconnect from the signal
    disconnect(job, SIGNAL(stateChanged(QXmppTransferJob::State)), this, SLOT(jobStateChanged(QXmppTransferJob::State)));

    // the job was refused by the local party
    if (state != QXmppTransferJob::StartState || !job->m_iodevice || !job->m_iodevice->isWritable())
    {
        QXmppStanza::Error error(QXmppStanza::Error::Cancel, QXmppStanza::Error::Forbidden);
        error.setCode(403);

        QXmppIq response;
        response.setTo(job->jid());
        response.setId(job->m_offerId);
        response.setType(QXmppIq::Error);
        response.setError(error);
        client()->sendPacket(response);

        job->terminate(QXmppTransferJob::AbortError);
        return;
    }

    // the job was accepted by the local party
    connect(job, SIGNAL(error(QXmppTransferJob::Error)), this, SLOT(jobError(QXmppTransferJob::Error)));

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

    QXmppStreamInitiationIq response;
    response.setTo(job->jid());
    response.setId(job->m_offerId);
    response.setType(QXmppIq::Result);
    response.setProfile(QXmppStreamInitiationIq::FileTransfer);
    response.setSiItems(feature);

    client()->sendPacket(response);
}

/// Send file to a remote party.
///
/// The remote party will be given the choice to accept or refuse the transfer.
///
QXmppTransferJob *QXmppTransferManager::sendFile(const QString &jid, const QString &fileName, const QString &sid)
{
    QFileInfo info(fileName);

    QXmppTransferFileInfo fileInfo;
    fileInfo.setDate(info.lastModified());
    fileInfo.setName(info.fileName());
    fileInfo.setSize(info.size());

    // open file
    QIODevice *device = new QFile(fileName);
    if (!device->open(QIODevice::ReadOnly))
    {
        delete device;
        device = 0;
    }

    // hash file
    if (device && !device->isSequential())
    {
        QCryptographicHash hash(QCryptographicHash::Md5);
        QByteArray buffer;
        while (device->bytesAvailable())
        {
            buffer = device->read(16384);
            hash.addData(buffer);
        }
        device->reset();
        fileInfo.setHash(hash.result());
    }

    // create job
    return sendFile(jid, device, fileInfo, sid);
}

/// Send file to a remote party.
///
/// The remote party will be given the choice to accept or refuse the transfer.
///
QXmppTransferJob *QXmppTransferManager::sendFile(const QString &jid, QIODevice *device, const QXmppTransferFileInfo &fileInfo, const QString &sid)
{
    QXmppTransferJob *job = new QXmppTransferJob(jid, QXmppTransferJob::OutgoingDirection, this);
    if (sid.isEmpty())
        job->m_sid = generateStanzaHash();
    else
        job->m_sid = sid;
    job->m_fileInfo = fileInfo;
    job->m_iodevice = device;
    if (device)
        device->setParent(job);

    // check file is open
    if (!device || !device->isReadable())
    {
        job->terminate(QXmppTransferJob::FileAccessError);
        return job;
    }

    // check we support some methods
    if (!m_supportedMethods)
    {
        job->terminate(QXmppTransferJob::ProtocolError);
        return job;
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
    connect(job, SIGNAL(destroyed(QObject*)), this, SLOT(jobDestroyed(QObject*)));
    connect(job, SIGNAL(error(QXmppTransferJob::Error)), this, SLOT(jobError(QXmppTransferJob::Error)));
    connect(job, SIGNAL(finished()), this, SLOT(jobFinished()));

    QXmppStreamInitiationIq request;
    request.setType(QXmppIq::Set);
    request.setTo(jid);
    request.setProfile(QXmppStreamInitiationIq::FileTransfer);
    request.setSiItems(items);
    request.setSiId(job->m_sid);
    job->m_requestId = request.id();
    client()->sendPacket(request);

    return job;
}

void QXmppTransferManager::socksServerConnected(QTcpSocket *socket, const QString &hostName, quint16 port)
{
    const QString ownJid = client()->configuration().jid();
    foreach (QXmppTransferJob *job, m_jobs)
    {
        if (hostName == streamHash(job->m_sid, ownJid, job->jid()) && port == 0)
        {
            job->m_socksSocket = socket;
            return;
        }
    }
    warning("QXmppSocksServer got a connection for a unknown stream");
    socket->close();
}

void QXmppTransferManager::socksServerSendOffer(QXmppTransferJob *job)
{
    const QString ownJid = client()->configuration().jid();
    QList<QXmppByteStreamIq::StreamHost> streamHosts;

    // discover local IPs
    if (!m_proxyOnly)
    {
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
    }

    // add proxy
    if (!job->m_socksProxy.jid().isEmpty())
        streamHosts.append(job->m_socksProxy);

    // check we have some stream hosts
    if (!streamHosts.size())
    {
        warning("Could not determine local stream hosts");
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
    client()->sendPacket(streamIq);
}

void QXmppTransferManager::streamInitiationIqReceived(const QXmppStreamInitiationIq &iq)
{
    if (iq.type() == QXmppIq::Result)
        streamInitiationResultReceived(iq);
    else if (iq.type() == QXmppIq::Set)
        streamInitiationSetReceived(iq);
}

// The remote party has accepted an outgoing transfer.
void QXmppTransferManager::streamInitiationResultReceived(const QXmppStreamInitiationIq &iq)
{
    QXmppTransferJob *job = getJobByRequestId(QXmppTransferJob::OutgoingDirection, iq.from(), iq.id());
    if (!job ||
        job->state() != QXmppTransferJob::OfferState)
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

    // remote party accepted stream initiation
    job->setState(QXmppTransferJob::StartState);
    if (job->method() == QXmppTransferJob::InBandMethod)
    {
        // lower block size for IBB
        job->m_blockSize = m_ibbBlockSize;

        QXmppIbbOpenIq openIq;
        openIq.setTo(job->m_jid);
        openIq.setSid(job->m_sid);
        openIq.setBlockSize(job->m_blockSize);
        job->m_requestId = openIq.id();
        client()->sendPacket(openIq);
    } else if (job->method() == QXmppTransferJob::SocksMethod) {
        if (!m_socksServer->isListening())
        {
            warning("QXmppSocksServer is not listening");
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
            client()->sendPacket(streamIq);
        } else {
            socksServerSendOffer(job);
        }
    } else {
        warning("QXmppTransferManager received an unsupported method");
        job->terminate(QXmppTransferJob::ProtocolError);
    }
}

void QXmppTransferManager::streamInitiationSetReceived(const QXmppStreamInitiationIq &iq)
{
    QXmppIq response;
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
        client()->sendPacket(response);
        return;
    }

    // check there is a receiver connected to the fileReceived() signal
    if (!receivers(SIGNAL(fileReceived(QXmppTransferJob*))))
    {
        QXmppStanza::Error error(QXmppStanza::Error::Cancel, QXmppStanza::Error::Forbidden);
        error.setCode(403);

        response.setType(QXmppIq::Error);
        response.setError(error);
        client()->sendPacket(response);
        return;
    }

    // check the stream type
    QXmppTransferJob *job = new QXmppTransferJob(iq.from(), QXmppTransferJob::IncomingDirection, this);
    int offeredMethods = QXmppTransferJob::NoMethod;
    job->m_offerId = iq.id();
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
            job->m_fileInfo.setDate(datetimeFromString(item.attribute("date")));
            job->m_fileInfo.setHash(QByteArray::fromHex(item.attribute("hash").toAscii()));
            job->m_fileInfo.setName(item.attribute("name"));
            job->m_fileInfo.setSize(item.attribute("size").toLongLong());
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
        client()->sendPacket(response);
    
        delete job;
        return;
    }

    // register job
    m_jobs.append(job);
    connect(job, SIGNAL(destroyed(QObject*)), this, SLOT(jobDestroyed(QObject*)));
    connect(job, SIGNAL(finished()), this, SLOT(jobFinished()));
    connect(job, SIGNAL(stateChanged(QXmppTransferJob::State)), this, SLOT(jobStateChanged(QXmppTransferJob::State)));

    // allow user to accept or decline the job
    emit fileReceived(job);
}

/// Return the JID of the bytestream proxy to use for
/// outgoing transfers.
///

QString QXmppTransferManager::proxy() const
{
    return m_proxy;
}

/// Set the JID of the SOCKS5 bytestream proxy to use for
/// outgoing transfers.
///
/// If you set a proxy, when you send a file the proxy will
/// be offered to the recipient in addition to your own IP
/// addresses.
///

void QXmppTransferManager::setProxy(const QString &proxyJid)
{
    m_proxy = proxyJid;
}

/// Return whether the proxy will systematically be used for
/// outgoing SOCKS5 bytestream transfers.
///

bool QXmppTransferManager::proxyOnly() const
{
    return m_proxyOnly;
}

/// Set whether the proxy should systematically be used for
/// outgoing SOCKS5 bytestream transfers.
///
/// \note If you set this to true and do not provide a proxy
/// using setProxy(), your outgoing transfers will fail!
///

void QXmppTransferManager::setProxyOnly(bool proxyOnly)
{
    m_proxyOnly = proxyOnly;
}

/// Return the supported stream methods.
///
/// The methods are a combination of zero or more QXmppTransferJob::Method.
///

QXmppTransferJob::Methods QXmppTransferManager::supportedMethods() const
{
    return m_supportedMethods;
}

/// Set the supported stream methods. This allows you to selectively
/// enable or disable stream methods (In-Band or SOCKS5 bytestreams).
///
/// The methods argument is a combination of zero or more
/// QXmppTransferJob::Method.
///

void QXmppTransferManager::setSupportedMethods(QXmppTransferJob::Methods methods)
{
    m_supportedMethods = methods;
}
