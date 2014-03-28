/*
 * Copyright (C) 2008-2014 The QXmpp developers
 *
 * Author:
 *  Jeremy Lainé
 *
 * Source:
 *  https://github.com/qxmpp-project/qxmpp
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
#include <QHash>
#include <QHostAddress>
#include <QNetworkInterface>
#include <QTime>
#include <QTimer>
#include <QUrl>

#include "QXmppByteStreamIq.h"
#include "QXmppClient.h"
#include "QXmppConstants.h"
#include "QXmppIbbIq.h"
#include "QXmppSocks.h"
#include "QXmppStreamInitiationIq_p.h"
#include "QXmppStun.h"
#include "QXmppTransferManager.h"
#include "QXmppTransferManager_p.h"
#include "QXmppUtils.h"

// time to try to connect to a SOCKS host (7 seconds)
const int socksTimeout = 7000;

static QString streamHash(const QString &sid, const QString &initiatorJid, const QString &targetJid)
{
    QCryptographicHash hash(QCryptographicHash::Sha1);
    QString str = sid + initiatorJid + targetJid;
    hash.addData(str.toLatin1());
    return hash.result().toHex();
}

class QXmppTransferFileInfoPrivate : public QSharedData
{
public:
    QXmppTransferFileInfoPrivate();

    QDateTime date;
    QByteArray hash;
    QString name;
    QString description;
    qint64 size;
};

QXmppTransferFileInfoPrivate::QXmppTransferFileInfoPrivate()
    : size(0)
{
}

QXmppTransferFileInfo::QXmppTransferFileInfo()
    : d(new QXmppTransferFileInfoPrivate)
{
}

QXmppTransferFileInfo::QXmppTransferFileInfo(const QXmppTransferFileInfo &other)
    : d(other.d)
{
}

QXmppTransferFileInfo::~QXmppTransferFileInfo()
{
}

QDateTime QXmppTransferFileInfo::date() const
{
    return d->date;
}

void QXmppTransferFileInfo::setDate(const QDateTime &date)
{
    d->date = date;
}

QByteArray QXmppTransferFileInfo::hash() const
{
    return d->hash;
}

void QXmppTransferFileInfo::setHash(const QByteArray &hash)
{
    d->hash = hash;
}

QString QXmppTransferFileInfo::name() const
{
    return d->name;
}

void QXmppTransferFileInfo::setName(const QString &name)
{
    d->name = name;
}

QString QXmppTransferFileInfo::description() const
{
    return d->description;
}

void QXmppTransferFileInfo::setDescription(const QString &description)
{
    d->description = description;
}

qint64 QXmppTransferFileInfo::size() const
{
    return d->size;
}

void QXmppTransferFileInfo::setSize(qint64 size)
{
    d->size = size;
}

bool QXmppTransferFileInfo::isNull() const
{
    return d->date.isNull()
        && d->description.isEmpty()
        && d->hash.isEmpty()
        && d->name.isEmpty()
        && d->size == 0;
}

QXmppTransferFileInfo& QXmppTransferFileInfo::operator=(const QXmppTransferFileInfo &other)
{
    d = other.d;
    return *this;
}

bool QXmppTransferFileInfo::operator==(const QXmppTransferFileInfo &other) const
{
    return other.d->size == d->size &&
        other.d->hash == d->hash &&
        other.d->name == d->name;
}

void QXmppTransferFileInfo::parse(const QDomElement &element)
{
    d->date = QXmppUtils::datetimeFromString(element.attribute("date"));
    d->hash = QByteArray::fromHex(element.attribute("hash").toLatin1());
    d->name = element.attribute("name");
    d->size = element.attribute("size").toLongLong();
    d->description = element.firstChildElement("desc").text();
}

void QXmppTransferFileInfo::toXml(QXmlStreamWriter *writer) const
{
    writer->writeStartElement("file");
    writer->writeAttribute("xmlns", ns_stream_initiation_file_transfer);
    if (d->date.isValid())
        writer->writeAttribute("date", QXmppUtils::datetimeToString(d->date));
    if (!d->hash.isEmpty())
        writer->writeAttribute("hash", d->hash.toHex());
    if (!d->name.isEmpty())
        writer->writeAttribute("name", d->name);
    if (d->size > 0)
        writer->writeAttribute("size", QString::number(d->size));
    if (!d->description.isEmpty())
        writer->writeTextElement("desc", d->description);
    writer->writeEndElement();
}

class QXmppTransferJobPrivate
{
public:
    QXmppTransferJobPrivate();

    int blockSize;
    QXmppClient *client;
    QXmppTransferJob::Direction direction;
    qint64 done;
    QXmppTransferJob::Error error;
    QCryptographicHash hash;
    QIODevice *iodevice;
    QString offerId;
    QString jid;
    QUrl localFileUrl;
    QString sid;
    QXmppTransferJob::Method method;
    QString mimeType;
    QString requestId;
    QXmppTransferJob::State state;
    QTime transferStart;

    // file meta-data
    QXmppTransferFileInfo fileInfo;

    // for in-band bytestreams
    int ibbSequence;

    // for socks5 bytestreams
    QTcpSocket *socksSocket;
    QXmppByteStreamIq::StreamHost socksProxy;
};

QXmppTransferJobPrivate::QXmppTransferJobPrivate()
    : blockSize(16384),
    client(0),
    done(0),
    error(QXmppTransferJob::NoError),
    hash(QCryptographicHash::Md5),
    iodevice(0),
    method(QXmppTransferJob::NoMethod),
    state(QXmppTransferJob::OfferState),
    ibbSequence(0),
    socksSocket(0)
{
}

QXmppTransferJob::QXmppTransferJob(const QString &jid, QXmppTransferJob::Direction direction, QXmppClient *client, QObject *parent)
    : QXmppLoggable(parent),
    d(new QXmppTransferJobPrivate)
{
    d->client = client;
    d->direction = direction;
    d->jid = jid;
}

QXmppTransferJob::~QXmppTransferJob()
{
    delete d;
}

/// Call this method if you wish to abort on ongoing transfer job.
///

void QXmppTransferJob::abort()
{
    terminate(AbortError);
}

/// Call this method if you wish to accept an incoming transfer job.
///

void QXmppTransferJob::accept(const QString &filePath)
{
    if (d->direction == IncomingDirection && d->state == OfferState && !d->iodevice)
    {
        QFile *file = new QFile(filePath, this);
        if (!file->open(QIODevice::WriteOnly))
        {
            warning(QString("Could not write to %1").arg(filePath));
            abort();
            return;
        }

        d->iodevice = file;
        setLocalFileUrl(QUrl::fromLocalFile(filePath));
        setState(QXmppTransferJob::StartState);
    }
}

/// Call this method if you wish to accept an incoming transfer job.
///

void QXmppTransferJob::accept(QIODevice *iodevice)
{
    if (d->direction == IncomingDirection && d->state == OfferState && !d->iodevice)
    {
        d->iodevice = iodevice;
        setState(QXmppTransferJob::StartState);
    }
}

/// Returns the job's transfer direction.
///

QXmppTransferJob::Direction QXmppTransferJob::direction() const
{
    return d->direction;
}

/// Returns the last error that was encountered.
///

QXmppTransferJob::Error QXmppTransferJob::error() const
{
    return d->error;
}

/// Returns the remote party's JID.
///

QString QXmppTransferJob::jid() const
{
    return d->jid;
}

/// Returns the local file URL.
///

QUrl QXmppTransferJob::localFileUrl() const
{
    return d->localFileUrl;
}

/// Sets the local file URL.
///
/// \note You do not need to call this method if you called accept()
///  with a file path.

void QXmppTransferJob::setLocalFileUrl(const QUrl &localFileUrl)
{
    if (localFileUrl != d->localFileUrl) {
        d->localFileUrl = localFileUrl;
        emit localFileUrlChanged(localFileUrl);
    }
}

/// Returns meta-data about the file being transferred.
///

QXmppTransferFileInfo QXmppTransferJob::fileInfo() const
{
    return d->fileInfo;
}

/// \cond
QDateTime QXmppTransferJob::fileDate() const
{
    return d->fileInfo.date();
}

QByteArray QXmppTransferJob::fileHash() const
{
    return d->fileInfo.hash();
}

QString QXmppTransferJob::fileName() const
{
    return d->fileInfo.name();
}

qint64 QXmppTransferJob::fileSize() const
{
    return d->fileInfo.size();
}
/// \endcond

/// Returns the job's transfer method.
///

QXmppTransferJob::Method QXmppTransferJob::method() const
{
    return d->method;
}

/// Returns the job's session identifier.
///

QString QXmppTransferJob::sid() const
{
    return d->sid;
}

/// Returns the job's transfer speed in bytes per second.
///
/// If the transfer has not started yet or is already finished, returns 0.
///

qint64 QXmppTransferJob::speed() const
{
    qint64 elapsed = d->transferStart.elapsed();
    if (d->state != QXmppTransferJob::TransferState || !elapsed)
        return 0;
    return (d->done * 1000.0) / elapsed;
}

/// Returns the job's state.
///

QXmppTransferJob::State QXmppTransferJob::state() const
{
    return d->state;
}

void QXmppTransferJob::setState(QXmppTransferJob::State state)
{
    if (d->state != state)
    {
        d->state = state;
        if (d->state == QXmppTransferJob::TransferState)
            d->transferStart.start();
        emit stateChanged(d->state);
    }
}

void QXmppTransferJob::_q_terminated()
{
    emit stateChanged(d->state);
    if (d->error != NoError)
        emit error(d->error);
    emit finished();
}

void QXmppTransferJob::terminate(QXmppTransferJob::Error cause)
{
    if (d->state == FinishedState)
        return;

    // change state
    d->error = cause;
    d->state = FinishedState;

    // close IO device
    if (d->iodevice)
        d->iodevice->close();

    // close socket
    if (d->socksSocket)
    {
        d->socksSocket->flush();
        d->socksSocket->close();
    }

    // emit signals later
    QTimer::singleShot(0, this, SLOT(_q_terminated()));
}

/// \cond
QXmppTransferIncomingJob::QXmppTransferIncomingJob(const QString& jid, QXmppClient* client, QObject* parent)
    : QXmppTransferJob(jid, IncomingDirection, client, parent)
    , m_candidateClient(0)
    , m_candidateTimer(0)
{
}

void QXmppTransferIncomingJob::checkData()
{
    if ((d->fileInfo.size() && d->done != d->fileInfo.size()) ||
        (!d->fileInfo.hash().isEmpty() && d->hash.result() != d->fileInfo.hash()))
        terminate(QXmppTransferJob::FileCorruptError);
    else
        terminate(QXmppTransferJob::NoError);
}

void QXmppTransferIncomingJob::connectToNextHost()
{
    bool check;
    Q_UNUSED(check);

    if (m_streamCandidates.isEmpty()) {
        // could not connect to any stream host
        QXmppByteStreamIq response;
        response.setId(m_streamOfferId);
        response.setTo(m_streamOfferFrom);
        QXmppStanza::Error error(QXmppStanza::Error::Cancel, QXmppStanza::Error::ItemNotFound);
        error.setCode(404);
        response.setType(QXmppIq::Error);
        response.setError(error);
        d->client->sendPacket(response);

        terminate(QXmppTransferJob::ProtocolError);
        return;
    }

    // try next host
    m_candidateHost = m_streamCandidates.takeFirst();
    info(QString("Connecting to streamhost: %1 (%2 %3)").arg(
            m_candidateHost.jid(),
            m_candidateHost.host(),
            QString::number(m_candidateHost.port())));

    const QString hostName = streamHash(d->sid,
                                        d->jid,
                                        d->client->configuration().jid());

    // try to connect to stream host
    m_candidateClient = new QXmppSocksClient(m_candidateHost.host(), m_candidateHost.port(), this);
    m_candidateTimer = new QTimer(this);

    check = connect(m_candidateClient, SIGNAL(disconnected()),
                    this, SLOT(_q_candidateDisconnected()));
    Q_ASSERT(check);

    check = connect(m_candidateClient, SIGNAL(ready()),
                    this, SLOT(_q_candidateReady()));
    Q_ASSERT(check);

    check = connect(m_candidateTimer, SIGNAL(timeout()),
                    this, SLOT(_q_candidateDisconnected()));
    Q_ASSERT(check);

    m_candidateTimer->setSingleShot(true);
    m_candidateTimer->start(socksTimeout);
    m_candidateClient->connectToHost(hostName, 0);
}

void QXmppTransferIncomingJob::connectToHosts(const QXmppByteStreamIq &iq)
{
    bool check;
    Q_UNUSED(check);

    m_streamCandidates = iq.streamHosts();
    m_streamOfferId = iq.id();
    m_streamOfferFrom = iq.from();

    connectToNextHost();
}

bool QXmppTransferIncomingJob::writeData(const QByteArray &data)
{
    const qint64 written = d->iodevice->write(data);
    if (written < 0)
        return false;
    d->done += written;
    if (!d->fileInfo.hash().isEmpty())
        d->hash.addData(data);
    progress(d->done, d->fileInfo.size());
    return true;
}

void QXmppTransferIncomingJob::_q_candidateReady()
{
    bool check;
    Q_UNUSED(check);

    if (!m_candidateClient)
        return;

    info(QString("Connected to streamhost: %1 (%2 %3)").arg(
            m_candidateHost.jid(),
            m_candidateHost.host(),
            QString::number(m_candidateHost.port())));

    setState(QXmppTransferJob::TransferState);
    d->socksSocket = m_candidateClient;
    m_candidateClient = 0;
    m_candidateTimer->deleteLater();
    m_candidateTimer = 0;

    check = connect(d->socksSocket, SIGNAL(readyRead()),
                    this, SLOT(_q_receiveData()));
    Q_ASSERT(check);

    check = connect(d->socksSocket, SIGNAL(disconnected()),
                    this, SLOT(_q_disconnected()));
    Q_ASSERT(check);

    QXmppByteStreamIq ackIq;
    ackIq.setId(m_streamOfferId);
    ackIq.setTo(m_streamOfferFrom);
    ackIq.setType(QXmppIq::Result);
    ackIq.setSid(d->sid);
    ackIq.setStreamHostUsed(m_candidateHost.jid());
    d->client->sendPacket(ackIq);
}

void QXmppTransferIncomingJob::_q_candidateDisconnected()
{
    if (!m_candidateClient)
        return;

    warning(QString("Failed to connect to streamhost: %1 (%2 %3)").arg(
            m_candidateHost.jid(),
            m_candidateHost.host(),
            QString::number(m_candidateHost.port())));

    m_candidateClient->deleteLater();
    m_candidateClient = 0;
    m_candidateTimer->deleteLater();
    m_candidateTimer = 0;

    // try next host
    connectToNextHost();
}

void QXmppTransferIncomingJob::_q_disconnected()
{
    if (d->state == QXmppTransferJob::FinishedState)
        return;

    checkData();
}

void QXmppTransferIncomingJob::_q_receiveData()
{
    if (d->state != QXmppTransferJob::TransferState)
        return;

    // receive data block
    if (d->direction == QXmppTransferJob::IncomingDirection)
    {
        writeData(d->socksSocket->readAll());

        // if we have received all the data, stop here
        if (fileSize() && d->done >= fileSize())
            checkData();
    }
}

QXmppTransferOutgoingJob::QXmppTransferOutgoingJob(const QString& jid, QXmppClient* client, QObject* parent)
    : QXmppTransferJob(jid, OutgoingDirection, client, parent)
{
}

void QXmppTransferOutgoingJob::connectToProxy()
{
    bool check;
    Q_UNUSED(check);

    info(QString("Connecting to proxy: %1 (%2 %3)").arg(
            d->socksProxy.jid(),
            d->socksProxy.host(),
            QString::number(d->socksProxy.port())));

    const QString hostName = streamHash(d->sid,
                                        d->client->configuration().jid(),
                                        d->jid);

    QXmppSocksClient *socksClient = new QXmppSocksClient(d->socksProxy.host(), d->socksProxy.port(), this);

    check = connect(socksClient, SIGNAL(disconnected()),
                    this, SLOT(_q_disconnected()));
    Q_ASSERT(check);

    check = connect(socksClient, SIGNAL(ready()),
                    this, SLOT(_q_proxyReady()));
    Q_ASSERT(check);

    d->socksSocket = socksClient;
    socksClient->connectToHost(hostName, 0);
}

void QXmppTransferOutgoingJob::startSending()
{
    bool check;
    Q_UNUSED(check);

    setState(QXmppTransferJob::TransferState);

    check = connect(d->socksSocket, SIGNAL(bytesWritten(qint64)),
                    this, SLOT(_q_sendData()));
    Q_ASSERT(check);

    check = connect(d->iodevice, SIGNAL(readyRead()),
                    this, SLOT(_q_sendData()));
    Q_ASSERT(check);

    _q_sendData();
}

void QXmppTransferOutgoingJob::_q_disconnected()
{
    if (d->state == QXmppTransferJob::FinishedState)
        return;

    if (fileSize() && d->done != fileSize())
        terminate(QXmppTransferJob::ProtocolError);
    else
        terminate(QXmppTransferJob::NoError);
}

void QXmppTransferOutgoingJob::_q_proxyReady()
{
    // activate stream
    QXmppByteStreamIq streamIq;
    streamIq.setType(QXmppIq::Set);
    streamIq.setFrom(d->client->configuration().jid());
    streamIq.setTo(d->socksProxy.jid());
    streamIq.setSid(d->sid);
    streamIq.setActivate(d->jid);
    d->requestId = streamIq.id();
    d->client->sendPacket(streamIq);
}

void QXmppTransferOutgoingJob::_q_sendData()
{
    if (d->state != QXmppTransferJob::TransferState)
        return;

    // don't saturate the outgoing socket
    if (d->socksSocket->bytesToWrite() > 2 * d->blockSize)
        return;

    // check whether we have written the whole file
    if (d->fileInfo.size() && d->done >= d->fileInfo.size())
    {
        if (!d->socksSocket->bytesToWrite())
            terminate(QXmppTransferJob::NoError);
        return;
    }

    char *buffer = new char[d->blockSize];
    qint64 length = d->iodevice->read(buffer, d->blockSize);
    if (length < 0)
    {
        delete [] buffer;
        terminate(QXmppTransferJob::FileAccessError);
        return;
    }
    if (length >= 0)
    {
        d->socksSocket->write(buffer, length);
        delete [] buffer;
        d->done += length;
        emit progress(d->done, fileSize());
    }
}
/// \endcond

class QXmppTransferManagerPrivate
{
public:
    QXmppTransferManagerPrivate(QXmppTransferManager *qq);

    QXmppTransferIncomingJob *getIncomingJobByRequestId(const QString &jid, const QString &id);
    QXmppTransferIncomingJob *getIncomingJobBySid(const QString &jid, const QString &sid);
    QXmppTransferOutgoingJob *getOutgoingJobByRequestId(const QString &jid, const QString &id);

    int ibbBlockSize;
    QList<QXmppTransferJob*> jobs;
    QString proxy;
    bool proxyOnly;
    QXmppSocksServer *socksServer;
    QXmppTransferJob::Methods supportedMethods;

private:
    QXmppTransferJob *getJobByRequestId(QXmppTransferJob::Direction direction, const QString &jid, const QString &id);
    QXmppTransferManager *q;
};

QXmppTransferManagerPrivate::QXmppTransferManagerPrivate(QXmppTransferManager *qq)
    : ibbBlockSize(4096)
    , proxyOnly(false)
    , socksServer(0)
    , supportedMethods(QXmppTransferJob::AnyMethod)
    , q(qq)
{
}

QXmppTransferJob* QXmppTransferManagerPrivate::getJobByRequestId(QXmppTransferJob::Direction direction, const QString &jid, const QString &id)
{
    foreach (QXmppTransferJob *job, jobs)
        if (job->d->direction == direction &&
            job->d->jid == jid &&
            job->d->requestId == id)
            return job;
    return 0;
}

QXmppTransferIncomingJob *QXmppTransferManagerPrivate::getIncomingJobByRequestId(const QString &jid, const QString &id)
{
    return static_cast<QXmppTransferIncomingJob*>(getJobByRequestId(QXmppTransferJob::IncomingDirection, jid, id));
}

QXmppTransferIncomingJob* QXmppTransferManagerPrivate::getIncomingJobBySid(const QString &jid, const QString &sid)
{
    foreach (QXmppTransferJob *job, jobs)
        if (job->d->direction == QXmppTransferJob::IncomingDirection &&
            job->d->jid == jid &&
            job->d->sid == sid)
            return static_cast<QXmppTransferIncomingJob*>(job);
    return 0;
}

QXmppTransferOutgoingJob *QXmppTransferManagerPrivate::getOutgoingJobByRequestId(const QString &jid, const QString &id)
{
    return static_cast<QXmppTransferOutgoingJob*>(getJobByRequestId(QXmppTransferJob::OutgoingDirection, jid, id));
}

/// Constructs a QXmppTransferManager to handle incoming and outgoing
/// file transfers.

QXmppTransferManager::QXmppTransferManager()
{
    bool check;
    Q_UNUSED(check);

    d = new QXmppTransferManagerPrivate(this);

    // start SOCKS server
    d->socksServer = new QXmppSocksServer(this);
    check = connect(d->socksServer, SIGNAL(newConnection(QTcpSocket*,QString,quint16)),
                    this, SLOT(_q_socksServerConnected(QTcpSocket*,QString,quint16)));
    Q_ASSERT(check);
    if (!d->socksServer->listen()) {
        qWarning("QXmppSocksServer could not start listening");
    }
}

QXmppTransferManager::~QXmppTransferManager()
{
    delete d;
}

void QXmppTransferManager::byteStreamIqReceived(const QXmppByteStreamIq &iq)
{
    // handle IQ from proxy
    foreach (QXmppTransferJob *job, d->jobs)
    {
        if (job->d->socksProxy.jid() == iq.from() && job->d->requestId == iq.id())
        {
            if (iq.type() == QXmppIq::Result && iq.streamHosts().size() > 0)
            {
                job->d->socksProxy = iq.streamHosts().first();
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
    QXmppTransferJob *job = d->getIncomingJobByRequestId(iq.from(), iq.id());
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
    bool check;
    Q_UNUSED(check);

    QXmppTransferOutgoingJob *job = d->getOutgoingJobByRequestId(iq.from(), iq.id());
    if (!job ||
        job->method() != QXmppTransferJob::SocksMethod ||
        job->state() != QXmppTransferJob::StartState)
        return;

    // check the stream host
    if (iq.streamHostUsed() == job->d->socksProxy.jid())
    {
        job->connectToProxy();
        return;
    }

    // direction connection, start sending data
    if (!job->d->socksSocket)
    {
        warning("Client says they connected to our SOCKS server, but they did not");
        job->terminate(QXmppTransferJob::ProtocolError);
        return;
    }
    check = connect(job->d->socksSocket, SIGNAL(disconnected()),
                    job, SLOT(_q_disconnected()));
    Q_ASSERT(check);

    job->startSending();
}

/// Handle a bytestream set, i.e. an invitation from the remote party to connect
/// to a stream host.
void QXmppTransferManager::byteStreamSetReceived(const QXmppByteStreamIq &iq)
{
    bool check;
    Q_UNUSED(check);

    QXmppIq response;
    response.setId(iq.id());
    response.setTo(iq.from());

    QXmppTransferIncomingJob *job = d->getIncomingJobBySid(iq.from(), iq.sid());
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

    job->connectToHosts(iq);
}

/// \cond
QStringList QXmppTransferManager::discoveryFeatures() const
{
    return QStringList()
        << ns_ibb               // XEP-0047: In-Band Bytestreams
        << ns_bytestreams       // XEP-0065: SOCKS5 Bytestreams
        << ns_stream_initiation // XEP-0095: Stream Initiation
        << ns_stream_initiation_file_transfer; // XEP-0096: SI File Transfer
}

bool QXmppTransferManager::handleStanza(const QDomElement &element)
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

void QXmppTransferManager::setClient(QXmppClient *client)
{
    bool check;
    Q_UNUSED(check);

    QXmppClientExtension::setClient(client);

    // XEP-0047: In-Band Bytestreams
    check = connect(client, SIGNAL(iqReceived(QXmppIq)),
                    this, SLOT(_q_iqReceived(QXmppIq)));
    Q_ASSERT(check);
}
/// \endcond

void QXmppTransferManager::ibbCloseIqReceived(const QXmppIbbCloseIq &iq)
{
    QXmppIq response;
    response.setTo(iq.from());
    response.setId(iq.id());

    QXmppTransferIncomingJob *job = d->getIncomingJobBySid(iq.from(), iq.sid());
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

    QXmppTransferIncomingJob *job = d->getIncomingJobBySid(iq.from(), iq.sid());
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

    if (iq.sequence() != job->d->ibbSequence)
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
    job->d->ibbSequence++;

    // acknowledge the packet
    response.setType(QXmppIq::Result);
    client()->sendPacket(response);
}

void QXmppTransferManager::ibbOpenIqReceived(const QXmppIbbOpenIq &iq)
{
    QXmppIq response;
    response.setTo(iq.from());
    response.setId(iq.id());

    QXmppTransferJob *job = d->getIncomingJobBySid(iq.from(), iq.sid());
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

    if (iq.blockSize() > d->ibbBlockSize)
    {
        // we prefer a smaller block size
        QXmppStanza::Error error(QXmppStanza::Error::Modify, QXmppStanza::Error::ResourceConstraint);
        response.setType(QXmppIq::Error);
        response.setError(error);
        client()->sendPacket(response);
        return;
    }

    job->d->blockSize = iq.blockSize();
    job->setState(QXmppTransferJob::TransferState);

    // accept transfer
    response.setType(QXmppIq::Result);
    client()->sendPacket(response);
}

void QXmppTransferManager::ibbResponseReceived(const QXmppIq &iq)
{
    QXmppTransferJob *job = d->getOutgoingJobByRequestId(iq.from(), iq.id());
    if (!job ||
        job->method() != QXmppTransferJob::InBandMethod ||
        job->state() == QXmppTransferJob::FinishedState)
        return;

    // if the IO device is closed, do nothing
    if (!job->d->iodevice->isOpen())
        return;

    if (iq.type() == QXmppIq::Result)
    {
        const QByteArray buffer = job->d->iodevice->read(job->d->blockSize);
        job->setState(QXmppTransferJob::TransferState);
        if (buffer.size())
        {
            // send next data block
            QXmppIbbDataIq dataIq;
            dataIq.setTo(job->d->jid);
            dataIq.setSid(job->d->sid);
            dataIq.setSequence(job->d->ibbSequence++);
            dataIq.setPayload(buffer);
            job->d->requestId = dataIq.id();
            client()->sendPacket(dataIq);

            job->d->done += buffer.size();
            job->progress(job->d->done, job->fileSize());
        } else {
            // close the bytestream
            QXmppIbbCloseIq closeIq;
            closeIq.setTo(job->d->jid);
            closeIq.setSid(job->d->sid);
            job->d->requestId = closeIq.id();
            client()->sendPacket(closeIq);

            job->terminate(QXmppTransferJob::NoError);
        }
    }
    else if (iq.type() == QXmppIq::Error)
    {
        // close the bytestream
        QXmppIbbCloseIq closeIq;
        closeIq.setTo(job->d->jid);
        closeIq.setSid(job->d->sid);
        job->d->requestId = closeIq.id();
        client()->sendPacket(closeIq);

        job->terminate(QXmppTransferJob::ProtocolError);
    }
}

void QXmppTransferManager::_q_iqReceived(const QXmppIq &iq)
{
    bool check;
    Q_UNUSED(check);

    foreach (QXmppTransferJob *ptr, d->jobs)
    {
        // handle IQ from proxy
        if (ptr->direction() == QXmppTransferJob::OutgoingDirection && ptr->d->socksProxy.jid() == iq.from() && ptr->d->requestId == iq.id())
        {
            QXmppTransferOutgoingJob *job = static_cast<QXmppTransferOutgoingJob*>(ptr);
            if (job->d->socksSocket)
            {
                // proxy connection activation result
                if (iq.type() == QXmppIq::Result)
                {
                    // proxy stream activated, start sending data
                    job->startSending();
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
        else if (ptr->d->jid == iq.from() && ptr->d->requestId == iq.id())
        {
            QXmppTransferJob *job = ptr;
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

void QXmppTransferManager::_q_jobDestroyed(QObject *object)
{
    d->jobs.removeAll(static_cast<QXmppTransferJob*>(object));
}

void QXmppTransferManager::_q_jobError(QXmppTransferJob::Error error)
{
    QXmppTransferJob *job = qobject_cast<QXmppTransferJob *>(sender());
    if (!job || !d->jobs.contains(job))
        return;

    if (job->direction() == QXmppTransferJob::OutgoingDirection &&
        job->method() == QXmppTransferJob::InBandMethod &&
        error == QXmppTransferJob::AbortError)
    {
        // close the bytestream
        QXmppIbbCloseIq closeIq;
        closeIq.setTo(job->d->jid);
        closeIq.setSid(job->d->sid);
        job->d->requestId = closeIq.id();
        client()->sendPacket(closeIq);
    }
}

void QXmppTransferManager::_q_jobFinished()
{
    QXmppTransferJob *job = qobject_cast<QXmppTransferJob *>(sender());
    if (!job || !d->jobs.contains(job))
        return;

    emit jobFinished(job);
}

void QXmppTransferManager::_q_jobStateChanged(QXmppTransferJob::State state)
{
    bool check;
    Q_UNUSED(check);

    QXmppTransferJob *job = qobject_cast<QXmppTransferJob *>(sender());
    if (!job || !d->jobs.contains(job))
        return;

    if (job->direction() != QXmppTransferJob::IncomingDirection)
        return;

    // disconnect from the signal
    disconnect(job, SIGNAL(stateChanged(QXmppTransferJob::State)),
               this, SLOT(_q_jobStateChanged(QXmppTransferJob::State)));

    // the job was refused by the local party
    if (state != QXmppTransferJob::StartState || !job->d->iodevice || !job->d->iodevice->isWritable())
    {
        QXmppStanza::Error error(QXmppStanza::Error::Cancel, QXmppStanza::Error::Forbidden);
        error.setCode(403);

        QXmppIq response;
        response.setTo(job->jid());
        response.setId(job->d->offerId);
        response.setType(QXmppIq::Error);
        response.setError(error);
        client()->sendPacket(response);

        job->terminate(QXmppTransferJob::AbortError);
        return;
    }

    // the job was accepted by the local party
    check = connect(job, SIGNAL(error(QXmppTransferJob::Error)),
            this, SLOT(_q_jobError(QXmppTransferJob::Error)));
    Q_ASSERT(check);

    QXmppDataForm form;
    form.setType(QXmppDataForm::Submit);

    QXmppDataForm::Field methodField(QXmppDataForm::Field::ListSingleField);
    methodField.setKey("stream-method");
    if (job->method() == QXmppTransferJob::InBandMethod)
        methodField.setValue(ns_ibb);
    else if (job->method() == QXmppTransferJob::SocksMethod)
        methodField.setValue(ns_bytestreams);
    form.setFields(QList<QXmppDataForm::Field>() << methodField);

    QXmppStreamInitiationIq response;
    response.setTo(job->jid());
    response.setId(job->d->offerId);
    response.setType(QXmppIq::Result);
    response.setProfile(QXmppStreamInitiationIq::FileTransfer);
    response.setFeatureForm(form);

    client()->sendPacket(response);

    // notify user
    emit jobStarted(job);
}

/// Send file to a remote party.
///
/// The remote party will be given the choice to accept or refuse the transfer.
///
QXmppTransferJob *QXmppTransferManager::sendFile(const QString &jid, const QString &filePath, const QString &description)
{
    if (jid.isEmpty()) {
        warning("Refusing to send file to an empty jid");
        return 0;
    }

    QFileInfo info(filePath);

    QXmppTransferFileInfo fileInfo;
    fileInfo.setDate(info.lastModified());
    fileInfo.setName(info.fileName());
    fileInfo.setSize(info.size());
    fileInfo.setDescription(description);

    // open file
    QIODevice *device = new QFile(filePath);
    if (!device->open(QIODevice::ReadOnly))
    {
        warning(QString("Could not read from %1").arg(filePath));
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
    QXmppTransferJob *job = sendFile(jid, device, fileInfo);
    job->setLocalFileUrl(filePath);
    return job;
}

/// Send file to a remote party.
///
/// The remote party will be given the choice to accept or refuse the transfer.
///
QXmppTransferJob *QXmppTransferManager::sendFile(const QString &jid, QIODevice *device, const QXmppTransferFileInfo &fileInfo, const QString &sid)
{
    bool check;
    Q_UNUSED(check);

    if (jid.isEmpty()) {
        warning("Refusing to send file to an empty jid");
        return 0;
    }

    QXmppTransferOutgoingJob *job = new QXmppTransferOutgoingJob(jid, client(), this);
    if (sid.isEmpty())
        job->d->sid = QXmppUtils::generateStanzaHash();
    else
        job->d->sid = sid;
    job->d->fileInfo = fileInfo;
    job->d->iodevice = device;
    if (device)
        device->setParent(job);

    // check file is open
    if (!device || !device->isReadable())
    {
        job->terminate(QXmppTransferJob::FileAccessError);
        return job;
    }

    // check we support some methods
    if (!d->supportedMethods)
    {
        job->terminate(QXmppTransferJob::ProtocolError);
        return job;
    }

    // collect supported stream methods
    QXmppDataForm form;
    form.setType(QXmppDataForm::Form);

    QXmppDataForm::Field methodField(QXmppDataForm::Field::ListSingleField);
    methodField.setKey("stream-method");
    if (d->supportedMethods & QXmppTransferJob::InBandMethod)
        methodField.setOptions(methodField.options() << qMakePair(QString(), QString::fromLatin1(ns_ibb)));
    if (d->supportedMethods & QXmppTransferJob::SocksMethod)
        methodField.setOptions(methodField.options() << qMakePair(QString(), QString::fromLatin1(ns_bytestreams)));
    form.setFields(QList<QXmppDataForm::Field>() << methodField);

    // start job
    d->jobs.append(job);
    check = connect(job, SIGNAL(destroyed(QObject*)),
                    this, SLOT(_q_jobDestroyed(QObject*)));
    Q_ASSERT(check);

    check = connect(job, SIGNAL(error(QXmppTransferJob::Error)),
                    this, SLOT(_q_jobError(QXmppTransferJob::Error)));
    Q_ASSERT(check);

    check = connect(job, SIGNAL(finished()),
                    this, SLOT(_q_jobFinished()));
    Q_ASSERT(check);

    QXmppStreamInitiationIq request;
    request.setType(QXmppIq::Set);
    request.setTo(jid);
    request.setProfile(QXmppStreamInitiationIq::FileTransfer);
    request.setFileInfo(job->d->fileInfo);
    request.setFeatureForm(form);
    request.setSiId(job->d->sid);
    job->d->requestId = request.id();
    client()->sendPacket(request);

    // notify user
    emit jobStarted(job);

    return job;
}

void QXmppTransferManager::_q_socksServerConnected(QTcpSocket *socket, const QString &hostName, quint16 port)
{
    const QString ownJid = client()->configuration().jid();
    foreach (QXmppTransferJob *job, d->jobs)
    {
        if (hostName == streamHash(job->d->sid, ownJid, job->jid()) && port == 0)
        {
            job->d->socksSocket = socket;
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
    if (!d->proxyOnly) {
        foreach (const QHostAddress &address, QXmppIceComponent::discoverAddresses()) {
            QXmppByteStreamIq::StreamHost streamHost;
            streamHost.setJid(ownJid);
            streamHost.setHost(address.toString());
            streamHost.setPort(d->socksServer->serverPort());
            streamHosts.append(streamHost);
        }
    }

    // add proxy
    if (!job->d->socksProxy.jid().isEmpty())
        streamHosts.append(job->d->socksProxy);

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
    streamIq.setTo(job->d->jid);
    streamIq.setSid(job->d->sid);
    streamIq.setStreamHosts(streamHosts);
    job->d->requestId = streamIq.id();
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
    QXmppTransferJob *job = d->getOutgoingJobByRequestId(iq.from(), iq.id());
    if (!job ||
        job->state() != QXmppTransferJob::OfferState)
        return;

    foreach (const QXmppDataForm::Field &field, iq.featureForm().fields()) {
        if (field.key() == "stream-method") {
            if ((field.value().toString() == ns_ibb) &&
                (d->supportedMethods & QXmppTransferJob::InBandMethod))
                job->d->method = QXmppTransferJob::InBandMethod;
            else if ((field.value().toString() == ns_bytestreams) &&
                     (d->supportedMethods & QXmppTransferJob::SocksMethod))
                job->d->method = QXmppTransferJob::SocksMethod;
        }
    }

    // remote party accepted stream initiation
    job->setState(QXmppTransferJob::StartState);
    if (job->method() == QXmppTransferJob::InBandMethod)
    {
        // lower block size for IBB
        job->d->blockSize = d->ibbBlockSize;

        QXmppIbbOpenIq openIq;
        openIq.setTo(job->d->jid);
        openIq.setSid(job->d->sid);
        openIq.setBlockSize(job->d->blockSize);
        job->d->requestId = openIq.id();
        client()->sendPacket(openIq);
    } else if (job->method() == QXmppTransferJob::SocksMethod) {
        if (!d->proxy.isEmpty())
        {
            job->d->socksProxy.setJid(d->proxy);

            // query proxy
            QXmppByteStreamIq streamIq;
            streamIq.setType(QXmppIq::Get);
            streamIq.setTo(job->d->socksProxy.jid());
            streamIq.setSid(job->d->sid);
            job->d->requestId = streamIq.id();
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
    bool check;
    Q_UNUSED(check);

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
    QXmppTransferIncomingJob *job = new QXmppTransferIncomingJob(iq.from(), client(), this);
    int offeredMethods = QXmppTransferJob::NoMethod;
    job->d->offerId = iq.id();
    job->d->sid = iq.siId();
    job->d->mimeType = iq.mimeType();
    job->d->fileInfo = iq.fileInfo();
    foreach (const QXmppDataForm::Field &field, iq.featureForm().fields()) {
        if (field.key() == "stream-method") {
            QPair<QString, QString> option;
            foreach (option, field.options()) {
                if (option.second == ns_ibb)
                    offeredMethods = offeredMethods | QXmppTransferJob::InBandMethod;
                else if (option.second == ns_bytestreams)
                    offeredMethods = offeredMethods | QXmppTransferJob::SocksMethod;
            }
        }
    }

    // select a method supported by both parties
    int sharedMethods = (offeredMethods & d->supportedMethods);
    if (sharedMethods & QXmppTransferJob::SocksMethod)
        job->d->method = QXmppTransferJob::SocksMethod;
    else if (sharedMethods & QXmppTransferJob::InBandMethod)
        job->d->method = QXmppTransferJob::InBandMethod;
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
    d->jobs.append(job);
    check = connect(job, SIGNAL(destroyed(QObject*)),
                    this, SLOT(_q_jobDestroyed(QObject*)));
    Q_ASSERT(check);

    check = connect(job, SIGNAL(finished()),
                    this, SLOT(_q_jobFinished()));
    Q_ASSERT(check);

    check = connect(job, SIGNAL(stateChanged(QXmppTransferJob::State)),
                    this, SLOT(_q_jobStateChanged(QXmppTransferJob::State)));
    Q_ASSERT(check);

    // allow user to accept or decline the job
    emit fileReceived(job);
}

/// Return the JID of the bytestream proxy to use for
/// outgoing transfers.
///

QString QXmppTransferManager::proxy() const
{
    return d->proxy;
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
    d->proxy = proxyJid;
}

/// Return whether the proxy will systematically be used for
/// outgoing SOCKS5 bytestream transfers.
///

bool QXmppTransferManager::proxyOnly() const
{
    return d->proxyOnly;
}

/// Set whether the proxy should systematically be used for
/// outgoing SOCKS5 bytestream transfers.
///
/// \note If you set this to true and do not provide a proxy
/// using setProxy(), your outgoing transfers will fail!
///

void QXmppTransferManager::setProxyOnly(bool proxyOnly)
{
    d->proxyOnly = proxyOnly;
}

/// Return the supported stream methods.
///
/// The methods are a combination of zero or more QXmppTransferJob::Method.
///

QXmppTransferJob::Methods QXmppTransferManager::supportedMethods() const
{
    return d->supportedMethods;
}

/// Set the supported stream methods. This allows you to selectively
/// enable or disable stream methods (In-Band or SOCKS5 bytestreams).
///
/// The methods argument is a combination of zero or more
/// QXmppTransferJob::Method.
///

void QXmppTransferManager::setSupportedMethods(QXmppTransferJob::Methods methods)
{
    d->supportedMethods = methods;
}
