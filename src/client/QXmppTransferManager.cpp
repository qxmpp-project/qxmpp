// SPDX-FileCopyrightText: 2010 Jeremy Lainé <jeremy.laine@m4x.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppTransferManager.h"

#include "QXmppByteStreamIq.h"
#include "QXmppClient.h"
#include "QXmppConstants_p.h"
#include "QXmppIbbIq.h"
#include "QXmppSocks.h"
#include "QXmppStreamInitiationIq_p.h"
#include "QXmppStun.h"
#include "QXmppTransferManager_p.h"
#include "QXmppUtils.h"
#include "QXmppUtils_p.h"

#include "StringLiterals.h"

#include <QCryptographicHash>
#include <QDomElement>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QHostAddress>
#include <QMetaMethod>
#include <QNetworkInterface>
#include <QTime>
#include <QTimer>
#include <QUrl>

using namespace QXmpp::Private;

// time to try to connect to a SOCKS host (7 seconds)
const int socksTimeout = 7000;

static QString streamHash(const QString &sid, const QString &initiatorJid, const QString &targetJid)
{
    QCryptographicHash hash(QCryptographicHash::Sha1);
    QString str = sid + initiatorJid + targetJid;
    hash.addData(str.toLatin1());
    return QString::fromUtf8(hash.result().toHex());
}

class QXmppTransferFileInfoPrivate : public QSharedData
{
public:
    QDateTime date;
    QByteArray hash;
    QString name;
    QString description;
    qint64 size = 0;
};

///
/// \class QXmppTransferFileInfo
///
/// Contains information about a file that is transferred using QXmppTransferJob.
///

QXmppTransferFileInfo::QXmppTransferFileInfo()
    : d(new QXmppTransferFileInfoPrivate)
{
}

/// Default copy-constructor
QXmppTransferFileInfo::QXmppTransferFileInfo(const QXmppTransferFileInfo &other) = default;

QXmppTransferFileInfo::~QXmppTransferFileInfo() = default;

///
/// Returns the last modification timestamp of the file.
///
QDateTime QXmppTransferFileInfo::date() const
{
    return d->date;
}

///
/// Sets the last modification timestamp of the file.
///
void QXmppTransferFileInfo::setDate(const QDateTime &date)
{
    d->date = date;
}

///
/// Returns the checksum of the file.
///
QByteArray QXmppTransferFileInfo::hash() const
{
    return d->hash;
}

///
/// Sets the checksum of the file.
///
void QXmppTransferFileInfo::setHash(const QByteArray &hash)
{
    d->hash = hash;
}

///
/// Returns the name of the file.
///
QString QXmppTransferFileInfo::name() const
{
    return d->name;
}

///
/// Sets the name of the file.
///
void QXmppTransferFileInfo::setName(const QString &name)
{
    d->name = name;
}

///
/// Returns a description of the file.
///
QString QXmppTransferFileInfo::description() const
{
    return d->description;
}

///
/// Sets a description of the file.
///
void QXmppTransferFileInfo::setDescription(const QString &description)
{
    d->description = description;
}

///
/// Returns the size of the file in bytes.
///
qint64 QXmppTransferFileInfo::size() const
{
    return d->size;
}

///
/// Sets the size of the file in bytes.
///
void QXmppTransferFileInfo::setSize(qint64 size)
{
    d->size = size;
}

///
/// Returns true if the file info has no valid data set.
///
bool QXmppTransferFileInfo::isNull() const
{
    return d->date.isNull() && d->description.isEmpty() && d->hash.isEmpty() && d->name.isEmpty() && d->size == 0;
}

/// Default assignment operator
QXmppTransferFileInfo &QXmppTransferFileInfo::operator=(const QXmppTransferFileInfo &other) = default;

///
/// Returns true if the content (size, hash and name) of the file info objects
/// equals.
///
bool QXmppTransferFileInfo::operator==(const QXmppTransferFileInfo &other) const
{
    return other.d->size == d->size &&
        other.d->hash == d->hash &&
        other.d->name == d->name;
}

/// \cond
void QXmppTransferFileInfo::parse(const QDomElement &element)
{
    d->date = QXmppUtils::datetimeFromString(element.attribute(u"date"_s));
    d->hash = QByteArray::fromHex(element.attribute(u"hash"_s).toLatin1());
    d->name = element.attribute(u"name"_s);
    d->size = element.attribute(u"size"_s).toLongLong();
    d->description = firstChildElement(element, u"desc").text();
}

void QXmppTransferFileInfo::toXml(QXmlStreamWriter *writer) const
{
    writer->writeStartElement(QSL65("file"));
    writer->writeDefaultNamespace(toString65(ns_stream_initiation_file_transfer));
    if (d->date.isValid()) {
        writer->writeAttribute(QSL65("date"), QXmppUtils::datetimeToString(d->date));
    }
    if (!d->hash.isEmpty()) {
        writer->writeAttribute(QSL65("hash"), QString::fromUtf8(d->hash.toHex()));
    }
    if (!d->name.isEmpty()) {
        writer->writeAttribute(QSL65("name"), d->name);
    }
    if (d->size > 0) {
        writer->writeAttribute(QSL65("size"), QString::number(d->size));
    }
    if (!d->description.isEmpty()) {
        writer->writeTextElement(QSL65("desc"), d->description);
    }
    writer->writeEndElement();
}
/// \endcond

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
    QElapsedTimer transferStart;
    bool deviceIsOwn;

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
      client(nullptr),
      direction(QXmppTransferJob::IncomingDirection),
      done(0),
      error(QXmppTransferJob::NoError),
      hash(QCryptographicHash::Md5),
      iodevice(nullptr),
      method(QXmppTransferJob::NoMethod),
      state(QXmppTransferJob::OfferState),
      deviceIsOwn(false),
      ibbSequence(0),
      socksSocket(nullptr)
{
}

///
/// \class QXmppTransferJob
///
/// The QXmppTransferJob class represents a single file transfer job.
///
/// \sa QXmppTransferManager
///

QXmppTransferJob::QXmppTransferJob(const QString &jid, QXmppTransferJob::Direction direction, QXmppClient *client, QObject *parent)
    : QXmppLoggable(parent),
      d(std::make_unique<QXmppTransferJobPrivate>())
{
    d->client = client;
    d->direction = direction;
    d->jid = jid;
}

QXmppTransferJob::~QXmppTransferJob() = default;

///
/// Call this method if you wish to abort on ongoing transfer job.
///
void QXmppTransferJob::abort()
{
    terminate(AbortError);
}

///
/// Call this method if you wish to accept an incoming transfer job.
///
void QXmppTransferJob::accept(const QString &filePath)
{
    if (d->direction == IncomingDirection && d->state == OfferState && !d->iodevice) {
        auto *file = new QFile(filePath, this);
        if (!file->open(QIODevice::WriteOnly)) {
            warning(u"Could not write to %1"_s.arg(filePath));
            abort();
            return;
        }

        d->iodevice = file;
        setLocalFileUrl(QUrl::fromLocalFile(filePath));
        setState(QXmppTransferJob::StartState);
    }
}

///
/// Call this method if you wish to accept an incoming transfer job.
///
void QXmppTransferJob::accept(QIODevice *iodevice)
{
    if (d->direction == IncomingDirection && d->state == OfferState && !d->iodevice) {
        d->iodevice = iodevice;
        setState(QXmppTransferJob::StartState);
    }
}

QXmppTransferJob::Direction QXmppTransferJob::direction() const
{
    return d->direction;
}

///
/// Returns the last error that was encountered.
///
QXmppTransferJob::Error QXmppTransferJob::error() const
{
    return d->error;
}

QString QXmppTransferJob::jid() const
{
    return d->jid;
}

QUrl QXmppTransferJob::localFileUrl() const
{
    return d->localFileUrl;
}

///
/// Sets the local file URL.
///
/// \note You do not need to call this method if you called accept()
///  with a file path.
///
void QXmppTransferJob::setLocalFileUrl(const QUrl &localFileUrl)
{
    if (localFileUrl != d->localFileUrl) {
        d->localFileUrl = localFileUrl;
        Q_EMIT localFileUrlChanged(localFileUrl);
    }
}

///
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

QXmppTransferJob::Method QXmppTransferJob::method() const
{
    return d->method;
}

///
/// Returns the job's session identifier.
///
QString QXmppTransferJob::sid() const
{
    return d->sid;
}

///
/// Returns the job's transfer speed in bytes per second.
///
/// If the transfer has not started yet or is already finished, returns 0.
///
qint64 QXmppTransferJob::speed() const
{
    qint64 elapsed = d->transferStart.elapsed();
    if (d->state != QXmppTransferJob::TransferState || !elapsed) {
        return 0;
    }
    return (d->done * 1000.0) / elapsed;
}

QXmppTransferJob::State QXmppTransferJob::state() const
{
    return d->state;
}

void QXmppTransferJob::setState(QXmppTransferJob::State state)
{
    if (d->state != state) {
        d->state = state;
        if (d->state == QXmppTransferJob::TransferState) {
            d->transferStart.start();
        }
        Q_EMIT stateChanged(d->state);
    }
}

void QXmppTransferJob::_q_terminated()
{
    Q_EMIT stateChanged(d->state);
    if (d->error != NoError) {
        Q_EMIT error(d->error);
    }
    Q_EMIT finished();
}

void QXmppTransferJob::terminate(QXmppTransferJob::Error cause)
{
    if (d->state == FinishedState) {
        return;
    }

    // change state
    d->error = cause;
    d->state = FinishedState;

    // close IO device
    if (d->iodevice && d->deviceIsOwn) {
        d->iodevice->close();
    }

    // close socket
    if (d->socksSocket) {
        d->socksSocket->flush();
        d->socksSocket->close();
    }

    // emit signals later
    QMetaObject::invokeMethod(this, "_q_terminated", Qt::QueuedConnection);
}

/// \cond
QXmppTransferIncomingJob::QXmppTransferIncomingJob(const QString &jid, QXmppClient *client, QObject *parent)
    : QXmppTransferJob(jid, IncomingDirection, client, parent),
      m_candidateClient(nullptr),
      m_candidateTimer(nullptr)
{
}

void QXmppTransferIncomingJob::checkData()
{
    if ((d->fileInfo.size() && d->done != d->fileInfo.size()) ||
        (!d->fileInfo.hash().isEmpty() && d->hash.result() != d->fileInfo.hash())) {
        terminate(QXmppTransferJob::FileCorruptError);
    } else {
        terminate(QXmppTransferJob::NoError);
    }
}

void QXmppTransferIncomingJob::connectToNextHost()
{
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
    info(u"Connecting to streamhost: %1 (%2 %3)"_s.arg(m_candidateHost.jid(), m_candidateHost.host(), QString::number(m_candidateHost.port())));

    const QString hostName = streamHash(d->sid,
                                        d->jid,
                                        d->client->configuration().jid());

    // try to connect to stream host
    m_candidateClient = new QXmppSocksClient(m_candidateHost.host(), m_candidateHost.port(), this);
    m_candidateTimer = new QTimer(this);

    connect(m_candidateClient, &QAbstractSocket::disconnected,
            this, &QXmppTransferIncomingJob::_q_candidateDisconnected);
    connect(m_candidateClient, &QXmppSocksClient::ready,
            this, &QXmppTransferIncomingJob::_q_candidateReady);
    connect(m_candidateTimer, &QTimer::timeout,
            this, &QXmppTransferIncomingJob::_q_candidateDisconnected);

    m_candidateTimer->setSingleShot(true);
    m_candidateTimer->start(socksTimeout);
    m_candidateClient->connectToHost(hostName, 0);
}

void QXmppTransferIncomingJob::connectToHosts(const QXmppByteStreamIq &iq)
{
    m_streamCandidates = iq.streamHosts();
    m_streamOfferId = iq.id();
    m_streamOfferFrom = iq.from();

    connectToNextHost();
}

bool QXmppTransferIncomingJob::writeData(const QByteArray &data)
{
    const qint64 written = d->iodevice->write(data);
    if (written < 0) {
        return false;
    }
    d->done += written;
    if (!d->fileInfo.hash().isEmpty()) {
        d->hash.addData(data);
    }
    Q_EMIT progress(d->done, d->fileInfo.size());
    return true;
}

void QXmppTransferIncomingJob::_q_candidateReady()
{
    if (!m_candidateClient) {
        return;
    }

    info(u"Connected to streamhost: %1 (%2 %3)"_s.arg(m_candidateHost.jid(), m_candidateHost.host(), QString::number(m_candidateHost.port())));

    setState(QXmppTransferJob::TransferState);
    d->socksSocket = m_candidateClient;
    m_candidateClient = nullptr;
    m_candidateTimer->deleteLater();
    m_candidateTimer = nullptr;

    connect(d->socksSocket, &QIODevice::readyRead, this, &QXmppTransferIncomingJob::_q_receiveData);
    connect(d->socksSocket, &QAbstractSocket::disconnected, this, &QXmppTransferIncomingJob::_q_disconnected);

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
    if (!m_candidateClient) {
        return;
    }

    warning(u"Failed to connect to streamhost: %1 (%2 %3)"_s.arg(m_candidateHost.jid(), m_candidateHost.host(), QString::number(m_candidateHost.port())));

    m_candidateClient->deleteLater();
    m_candidateClient = nullptr;
    m_candidateTimer->deleteLater();
    m_candidateTimer = nullptr;

    // try next host
    connectToNextHost();
}

void QXmppTransferIncomingJob::_q_disconnected()
{
    if (d->state == QXmppTransferJob::FinishedState) {
        return;
    }

    checkData();
}

void QXmppTransferIncomingJob::_q_receiveData()
{
    if (d->state != QXmppTransferJob::TransferState) {
        return;
    }

    // receive data block
    if (d->direction == QXmppTransferJob::IncomingDirection) {
        writeData(d->socksSocket->readAll());

        // if we have received all the data, stop here
        if (fileSize() && d->done >= fileSize()) {
            checkData();
        }
    }
}

QXmppTransferOutgoingJob::QXmppTransferOutgoingJob(const QString &jid, QXmppClient *client, QObject *parent)
    : QXmppTransferJob(jid, OutgoingDirection, client, parent)
{
}

void QXmppTransferOutgoingJob::connectToProxy()
{
    info(u"Connecting to proxy: %1 (%2 %3)"_s.arg(d->socksProxy.jid(), d->socksProxy.host(), QString::number(d->socksProxy.port())));

    const QString hostName = streamHash(d->sid,
                                        d->client->configuration().jid(),
                                        d->jid);

    QXmppSocksClient *socksClient = new QXmppSocksClient(d->socksProxy.host(), d->socksProxy.port(), this);

    connect(socksClient, &QAbstractSocket::disconnected, this, &QXmppTransferOutgoingJob::_q_disconnected);
    connect(socksClient, &QXmppSocksClient::ready, this, &QXmppTransferOutgoingJob::_q_proxyReady);

    d->socksSocket = socksClient;
    socksClient->connectToHost(hostName, 0);
}

void QXmppTransferOutgoingJob::startSending()
{
    setState(QXmppTransferJob::TransferState);

    connect(d->socksSocket, &QIODevice::bytesWritten, this, &QXmppTransferOutgoingJob::_q_sendData);
    connect(d->iodevice, &QIODevice::readyRead, this, &QXmppTransferOutgoingJob::_q_sendData);

    _q_sendData();
}

void QXmppTransferOutgoingJob::_q_disconnected()
{
    if (d->state == QXmppTransferJob::FinishedState) {
        return;
    }

    if (fileSize() && d->done != fileSize()) {
        terminate(QXmppTransferJob::ProtocolError);
    } else {
        terminate(QXmppTransferJob::NoError);
    }
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
    if (d->state != QXmppTransferJob::TransferState) {
        return;
    }

    // don't saturate the outgoing socket
    if (d->socksSocket->bytesToWrite() > 2 * d->blockSize) {
        return;
    }

    // check whether we have written the whole file
    if (d->fileInfo.size() && d->done >= d->fileInfo.size()) {
        if (!d->socksSocket->bytesToWrite()) {
            terminate(QXmppTransferJob::NoError);
        }
        return;
    }

    char *buffer = new char[d->blockSize];
    qint64 length = d->iodevice->read(buffer, d->blockSize);
    if (length < 0) {
        delete[] buffer;
        terminate(QXmppTransferJob::FileAccessError);
        return;
    } else {
        d->socksSocket->write(buffer, length);
        delete[] buffer;
        d->done += length;
        Q_EMIT progress(d->done, fileSize());
    }
}
/// \endcond

class QXmppTransferManagerPrivate
{
public:
    QXmppTransferManagerPrivate();

    QXmppTransferIncomingJob *getIncomingJobByRequestId(const QString &jid, const QString &id);
    QXmppTransferIncomingJob *getIncomingJobBySid(const QString &jid, const QString &sid);
    QXmppTransferOutgoingJob *getOutgoingJobByRequestId(const QString &jid, const QString &id);

    int ibbBlockSize;
    QList<QXmppTransferJob *> jobs;
    QString proxy;
    bool proxyOnly;
    QXmppSocksServer *socksServer;
    QXmppTransferJob::Methods supportedMethods;

private:
    QXmppTransferJob *getJobByRequestId(QXmppTransferJob::Direction direction, const QString &jid, const QString &id);
};

QXmppTransferManagerPrivate::QXmppTransferManagerPrivate()
    : ibbBlockSize(4096),
      proxyOnly(false),
      socksServer(nullptr),
      supportedMethods(QXmppTransferJob::AnyMethod)
{
}

QXmppTransferJob *QXmppTransferManagerPrivate::getJobByRequestId(QXmppTransferJob::Direction direction, const QString &jid, const QString &id)
{
    for (auto *job : std::as_const(jobs)) {
        if (job->d->direction == direction &&
            job->d->jid == jid &&
            job->d->requestId == id) {
            return job;
        }
    }
    return nullptr;
}

QXmppTransferIncomingJob *QXmppTransferManagerPrivate::getIncomingJobByRequestId(const QString &jid, const QString &id)
{
    return static_cast<QXmppTransferIncomingJob *>(getJobByRequestId(QXmppTransferJob::IncomingDirection, jid, id));
}

QXmppTransferIncomingJob *QXmppTransferManagerPrivate::getIncomingJobBySid(const QString &jid, const QString &sid)
{
    for (auto *job : std::as_const(jobs)) {
        if (job->d->direction == QXmppTransferJob::IncomingDirection &&
            job->d->jid == jid &&
            job->d->sid == sid) {
            return static_cast<QXmppTransferIncomingJob *>(job);
        }
    }
    return nullptr;
}

QXmppTransferOutgoingJob *QXmppTransferManagerPrivate::getOutgoingJobByRequestId(const QString &jid, const QString &id)
{
    return static_cast<QXmppTransferOutgoingJob *>(getJobByRequestId(QXmppTransferJob::OutgoingDirection, jid, id));
}

///
/// \class QXmppTransferManager
///
/// The QXmppTransferManager class provides support for sending and receiving
/// files.
///
/// Stream initiation is performed as described in \xep{0095, Stream Initiation}
/// and \xep{0096, SI File Transfer}. The actual file transfer is then performed
/// using either \xep{0065, SOCKS5 Bytestreams} or \xep{0047, In-Band
/// Bytestreams}.
///
/// To make use of this manager, you need to instantiate it and load it into the
/// QXmppClient instance as follows:
///
/// \code
/// auto *manager = new QXmppTransferManager;
/// client->addExtension(manager);
/// \endcode
///
/// \ingroup Managers
///

///
/// Constructs a QXmppTransferManager to handle incoming and outgoing
/// file transfers.
///
QXmppTransferManager::QXmppTransferManager()
    : d(std::make_unique<QXmppTransferManagerPrivate>())
{
    // start SOCKS server
    d->socksServer = new QXmppSocksServer(this);
    connect(d->socksServer, &QXmppSocksServer::newConnection, this, &QXmppTransferManager::_q_socksServerConnected);

    if (!d->socksServer->listen()) {
        qWarning("QXmppSocksServer could not start listening");
    }
}

QXmppTransferManager::~QXmppTransferManager() = default;

void QXmppTransferManager::byteStreamIqReceived(const QXmppByteStreamIq &iq)
{
    // handle IQ from proxy
    for (auto *job : std::as_const(d->jobs)) {
        if (job->d->socksProxy.jid() == iq.from() && job->d->requestId == iq.id()) {
            if (iq.type() == QXmppIq::Result && iq.streamHosts().size() > 0) {
                job->d->socksProxy = iq.streamHosts().constFirst();
                socksServerSendOffer(job);
                return;
            }
        }
    }

    if (iq.type() == QXmppIq::Result) {
        byteStreamResultReceived(iq);
    } else if (iq.type() == QXmppIq::Set) {
        byteStreamSetReceived(iq);
    }
}

///
/// Handle a response to a bystream set, i.e. after we informed the remote party
/// that we connected to a stream host.
///
void QXmppTransferManager::byteStreamResponseReceived(const QXmppIq &iq)
{
    QXmppTransferJob *job = d->getIncomingJobByRequestId(iq.from(), iq.id());
    if (!job ||
        job->method() != QXmppTransferJob::SocksMethod ||
        job->state() != QXmppTransferJob::StartState) {
        return;
    }

    if (iq.type() == QXmppIq::Error) {
        job->terminate(QXmppTransferJob::ProtocolError);
    }
}

///
/// Handle a bytestream result, i.e. after the remote party has connected to
/// a stream host.
///
void QXmppTransferManager::byteStreamResultReceived(const QXmppByteStreamIq &iq)
{
    auto *job = d->getOutgoingJobByRequestId(iq.from(), iq.id());
    if (!job ||
        job->method() != QXmppTransferJob::SocksMethod ||
        job->state() != QXmppTransferJob::StartState) {
        return;
    }

    // check the stream host
    if (iq.streamHostUsed() == job->d->socksProxy.jid()) {
        job->connectToProxy();
        return;
    }

    // direction connection, start sending data
    if (!job->d->socksSocket) {
        warning(u"Client says they connected to our SOCKS server, but they did not"_s);
        job->terminate(QXmppTransferJob::ProtocolError);
        return;
    }

    connect(job->d->socksSocket, &QTcpSocket::disconnected, job, &QXmppTransferOutgoingJob::_q_disconnected);
    job->startSending();
}

///
/// Handle a bytestream set, i.e. an invitation from the remote party to connect
/// to a stream host.
///
void QXmppTransferManager::byteStreamSetReceived(const QXmppByteStreamIq &iq)
{
    QXmppIq response;
    response.setId(iq.id());
    response.setTo(iq.from());

    auto *job = d->getIncomingJobBySid(iq.from(), iq.sid());
    if (!job ||
        job->method() != QXmppTransferJob::SocksMethod ||
        job->state() != QXmppTransferJob::StartState) {
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
    return {
        ns_ibb.toString(),                              // XEP-0047: In-Band Bytestreams
        ns_bytestreams.toString(),                      // XEP-0065: SOCKS5 Bytestreams
        ns_stream_initiation.toString(),                // XEP-0095: Stream Initiation
        ns_stream_initiation_file_transfer.toString(),  // XEP-0096: SI File Transfer
    };
}

bool QXmppTransferManager::handleStanza(const QDomElement &element)
{
    if (element.tagName() != u"iq") {
        return false;
    }

    // XEP-0047 In-Band Bytestreams
    if (QXmppIbbCloseIq::isIbbCloseIq(element)) {
        QXmppIbbCloseIq ibbCloseIq;
        ibbCloseIq.parse(element);
        ibbCloseIqReceived(ibbCloseIq);
        return true;
    } else if (QXmppIbbDataIq::isIbbDataIq(element)) {
        QXmppIbbDataIq ibbDataIq;
        ibbDataIq.parse(element);
        ibbDataIqReceived(ibbDataIq);
        return true;
    } else if (QXmppIbbOpenIq::isIbbOpenIq(element)) {
        QXmppIbbOpenIq ibbOpenIq;
        ibbOpenIq.parse(element);
        ibbOpenIqReceived(ibbOpenIq);
        return true;
    }
    // XEP-0065: SOCKS5 Bytestreams
    else if (QXmppByteStreamIq::isByteStreamIq(element)) {
        QXmppByteStreamIq byteStreamIq;
        byteStreamIq.parse(element);
        byteStreamIqReceived(byteStreamIq);
        return true;
    }
    // XEP-0095: Stream Initiation
    else if (QXmppStreamInitiationIq::isStreamInitiationIq(element)) {
        QXmppStreamInitiationIq siIq;
        siIq.parse(element);
        streamInitiationIqReceived(siIq);
        return true;
    }

    return false;
}

void QXmppTransferManager::onRegistered(QXmppClient *client)
{
    // XEP-0047: In-Band Bytestreams
    connect(client, &QXmppClient::iqReceived, this, &QXmppTransferManager::_q_iqReceived);
}

void QXmppTransferManager::onUnregistered(QXmppClient *client)
{
    disconnect(client, &QXmppClient::iqReceived, this, &QXmppTransferManager::_q_iqReceived);
}
/// \endcond

void QXmppTransferManager::ibbCloseIqReceived(const QXmppIbbCloseIq &iq)
{
    QXmppIq response;
    response.setTo(iq.from());
    response.setId(iq.id());

    auto *job = d->getIncomingJobBySid(iq.from(), iq.sid());
    if (!job ||
        job->method() != QXmppTransferJob::InBandMethod) {
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

    auto *job = d->getIncomingJobBySid(iq.from(), iq.sid());
    if (!job ||
        job->method() != QXmppTransferJob::InBandMethod ||
        job->state() != QXmppTransferJob::TransferState) {
        // the job is unknown, cancel it
        QXmppStanza::Error error(QXmppStanza::Error::Cancel, QXmppStanza::Error::ItemNotFound);
        response.setType(QXmppIq::Error);
        response.setError(error);
        client()->sendPacket(response);
        return;
    }

    if (iq.sequence() != job->d->ibbSequence) {
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

    auto *job = d->getIncomingJobBySid(iq.from(), iq.sid());
    if (!job ||
        job->method() != QXmppTransferJob::InBandMethod) {
        // the job is unknown, cancel it
        QXmppStanza::Error error(QXmppStanza::Error::Cancel, QXmppStanza::Error::ItemNotFound);
        response.setType(QXmppIq::Error);
        response.setError(error);
        client()->sendPacket(response);
        return;
    }

    if (iq.blockSize() > d->ibbBlockSize) {
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
    auto *job = d->getOutgoingJobByRequestId(iq.from(), iq.id());
    if (!job ||
        job->method() != QXmppTransferJob::InBandMethod ||
        job->state() == QXmppTransferJob::FinishedState) {
        return;
    }

    // if the IO device is closed, do nothing
    if (!job->d->iodevice->isOpen()) {
        return;
    }

    if (iq.type() == QXmppIq::Result) {
        const QByteArray buffer = job->d->iodevice->read(job->d->blockSize);
        job->setState(QXmppTransferJob::TransferState);
        if (buffer.size()) {
            // send next data block
            QXmppIbbDataIq dataIq;
            dataIq.setTo(job->d->jid);
            dataIq.setSid(job->d->sid);
            dataIq.setSequence(job->d->ibbSequence++);
            dataIq.setPayload(buffer);
            job->d->requestId = dataIq.id();
            client()->sendPacket(dataIq);

            job->d->done += buffer.size();
            Q_EMIT job->progress(job->d->done, job->fileSize());
        } else {
            // close the bytestream
            QXmppIbbCloseIq closeIq;
            closeIq.setTo(job->d->jid);
            closeIq.setSid(job->d->sid);
            job->d->requestId = closeIq.id();
            client()->sendPacket(closeIq);

            job->terminate(QXmppTransferJob::NoError);
        }
    } else if (iq.type() == QXmppIq::Error) {
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
    for (auto *ptr : std::as_const(d->jobs)) {
        // handle IQ from proxy
        if (ptr->direction() == QXmppTransferJob::OutgoingDirection && ptr->d->socksProxy.jid() == iq.from() && ptr->d->requestId == iq.id()) {
            auto *job = static_cast<QXmppTransferOutgoingJob *>(ptr);
            if (job->d->socksSocket) {
                // proxy connection activation result
                if (iq.type() == QXmppIq::Result) {
                    // proxy stream activated, start sending data
                    job->startSending();
                } else if (iq.type() == QXmppIq::Error) {
                    // proxy stream not activated, terminate
                    warning(u"Could not activate SOCKS5 proxy bytestream"_s);
                    job->terminate(QXmppTransferJob::ProtocolError);
                }
            } else {
                // we could not get host/port from proxy, proceed without a proxy
                if (iq.type() == QXmppIq::Error) {
                    socksServerSendOffer(job);
                }
            }
            return;
        }

        // handle IQ from peer
        else if (ptr->d->jid == iq.from() && ptr->d->requestId == iq.id()) {
            auto *job = ptr;
            if (job->direction() == QXmppTransferJob::OutgoingDirection &&
                job->method() == QXmppTransferJob::InBandMethod) {
                ibbResponseReceived(iq);
                return;
            } else if (job->direction() == QXmppTransferJob::IncomingDirection &&
                       job->method() == QXmppTransferJob::SocksMethod) {
                byteStreamResponseReceived(iq);
                return;
            } else if (job->direction() == QXmppTransferJob::OutgoingDirection &&
                       iq.type() == QXmppIq::Error) {
                // remote party cancelled stream initiation
                job->terminate(QXmppTransferJob::AbortError);
                return;
            }
        }
    }
}

void QXmppTransferManager::_q_jobDestroyed(QObject *object)
{
    d->jobs.removeAll(static_cast<QXmppTransferJob *>(object));
}

void QXmppTransferManager::_q_jobError(QXmppTransferJob::Error error)
{
    auto *job = qobject_cast<QXmppTransferJob *>(sender());
    if (!job || !d->jobs.contains(job)) {
        return;
    }

    if (job->direction() == QXmppTransferJob::OutgoingDirection &&
        job->method() == QXmppTransferJob::InBandMethod &&
        error == QXmppTransferJob::AbortError) {
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
    auto *job = qobject_cast<QXmppTransferJob *>(sender());
    if (!job || !d->jobs.contains(job)) {
        return;
    }

    Q_EMIT jobFinished(job);
}

void QXmppTransferManager::_q_jobStateChanged(QXmppTransferJob::State state)
{
    auto *job = qobject_cast<QXmppTransferJob *>(sender());
    if (!job || !d->jobs.contains(job)) {
        return;
    }

    if (job->direction() != QXmppTransferJob::IncomingDirection) {
        return;
    }

    // disconnect from the signal
    disconnect(job, &QXmppTransferJob::stateChanged,
               this, &QXmppTransferManager::_q_jobStateChanged);

    // the job was refused by the local party
    if (state != QXmppTransferJob::StartState || !job->d->iodevice || !job->d->iodevice->isWritable()) {
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
    connect(job, QOverload<QXmppTransferJob::Error>::of(&QXmppTransferJob::error), this, &QXmppTransferManager::_q_jobError);

    QXmppDataForm form;
    form.setType(QXmppDataForm::Submit);

    QXmppDataForm::Field methodField(QXmppDataForm::Field::ListSingleField);
    methodField.setKey(u"stream-method"_s);
    if (job->method() == QXmppTransferJob::InBandMethod) {
        methodField.setValue(ns_ibb.toString());
    } else if (job->method() == QXmppTransferJob::SocksMethod) {
        methodField.setValue(ns_bytestreams.toString());
    }
    form.setFields(QList<QXmppDataForm::Field>() << methodField);

    QXmppStreamInitiationIq response;
    response.setTo(job->jid());
    response.setId(job->d->offerId);
    response.setType(QXmppIq::Result);
    response.setProfile(QXmppStreamInitiationIq::FileTransfer);
    response.setFeatureForm(form);

    client()->sendPacket(response);

    // notify user
    Q_EMIT jobStarted(job);
}

///
/// Sends the file at \a filePath to a remote party.
///
/// The remote party will be given the choice to accept or refuse the transfer.
///
/// Returns 0 if the \a jid is not valid or if the file at \a filePath cannot be
/// read.
///
/// \note The recipient's \a jid must be a full JID with a resource, for
/// instance "user@host/resource".
///
QXmppTransferJob *QXmppTransferManager::sendFile(const QString &jid, const QString &filePath, const QString &description)
{
    if (QXmppUtils::jidToResource(jid).isEmpty()) {
        warning(u"The file recipient's JID must be a full JID"_s);
        return nullptr;
    }

    QFileInfo info(filePath);

    QXmppTransferFileInfo fileInfo;
    fileInfo.setDate(info.lastModified());
    fileInfo.setName(info.fileName());
    fileInfo.setSize(info.size());
    fileInfo.setDescription(description);

    // open file
    QIODevice *device = new QFile(filePath, this);
    if (!device->open(QIODevice::ReadOnly)) {
        warning(u"Could not read from %1"_s.arg(filePath));
        delete device;
        device = nullptr;
    }

    // hash file
    if (device && !device->isSequential()) {
        QCryptographicHash hash(QCryptographicHash::Md5);
        QByteArray buffer;
        while (device->bytesAvailable()) {
            buffer = device->read(16384);
            hash.addData(buffer);
        }
        device->reset();
        fileInfo.setHash(hash.result());
    }

    // create job
    QXmppTransferJob *job = sendFile(jid, device, fileInfo);
    job->setLocalFileUrl(QUrl::fromLocalFile(filePath));
    job->d->deviceIsOwn = true;
    return job;
}

///
/// Sends the file in \a device to a remote party.
///
/// The remote party will be given the choice to accept or refuse the transfer.
///
/// Returns 0 if the \a jid is not valid.
///
/// \note The recipient's \a jid must be a full JID with a resource, for
/// instance "user@host/resource".
/// \note The ownership of the \a device should be managed by the caller.
///
QXmppTransferJob *QXmppTransferManager::sendFile(const QString &jid, QIODevice *device, const QXmppTransferFileInfo &fileInfo, const QString &sid)
{
    if (QXmppUtils::jidToResource(jid).isEmpty()) {
        warning(u"The file recipient's JID must be a full JID"_s);
        return nullptr;
    }

    auto *job = new QXmppTransferOutgoingJob(jid, client(), this);
    job->d->sid = sid.isEmpty() ? QXmppUtils::generateStanzaHash() : sid;
    job->d->fileInfo = fileInfo;
    job->d->iodevice = device;

    // check file is open
    if (!device || !device->isReadable()) {
        job->terminate(QXmppTransferJob::FileAccessError);
        return job;
    }

    // check we support some methods
    if (!d->supportedMethods) {
        job->terminate(QXmppTransferJob::ProtocolError);
        return job;
    }

    // collect supported stream methods
    QXmppDataForm form;
    form.setType(QXmppDataForm::Form);

    QXmppDataForm::Field methodField(QXmppDataForm::Field::ListSingleField);
    methodField.setKey(u"stream-method"_s);
    if (d->supportedMethods & QXmppTransferJob::InBandMethod) {
        methodField.setOptions(methodField.options() << qMakePair(QString(), ns_ibb.toString()));
    }
    if (d->supportedMethods & QXmppTransferJob::SocksMethod) {
        methodField.setOptions(methodField.options() << qMakePair(QString(), ns_bytestreams.toString()));
    }
    form.setFields(QList<QXmppDataForm::Field>() << methodField);

    // start job
    d->jobs.append(job);

    connect(job, &QObject::destroyed, this, &QXmppTransferManager::_q_jobDestroyed);
    connect(job, QOverload<QXmppTransferJob::Error>::of(&QXmppTransferJob::error), this, &QXmppTransferManager::_q_jobError);
    connect(job, &QXmppTransferJob::finished, this, &QXmppTransferManager::_q_jobFinished);

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
    Q_EMIT jobStarted(job);

    return job;
}

void QXmppTransferManager::_q_socksServerConnected(QTcpSocket *socket, const QString &hostName, quint16 port)
{
    const QString ownJid = client()->configuration().jid();
    for (auto *job : std::as_const(d->jobs)) {
        if (hostName == streamHash(job->d->sid, ownJid, job->jid()) && port == 0) {
            job->d->socksSocket = socket;
            return;
        }
    }
    warning(u"QXmppSocksServer got a connection for a unknown stream"_s);
    socket->close();
}

void QXmppTransferManager::socksServerSendOffer(QXmppTransferJob *job)
{
    const auto ownJid = client()->configuration().jid();
    QList<QXmppByteStreamIq::StreamHost> streamHosts;

    // discover local IPs
    if (!d->proxyOnly) {
        const auto &addresses = QXmppIceComponent::discoverAddresses();
        for (const auto &address : addresses) {
            QXmppByteStreamIq::StreamHost streamHost;
            streamHost.setJid(ownJid);
            streamHost.setHost(address.toString());
            streamHost.setPort(d->socksServer->serverPort());
            streamHosts.append(streamHost);
        }
    }

    // add proxy
    if (!job->d->socksProxy.jid().isEmpty()) {
        streamHosts.append(job->d->socksProxy);
    }

    // check we have some stream hosts
    if (!streamHosts.size()) {
        warning(u"Could not determine local stream hosts"_s);
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
    if (iq.type() == QXmppIq::Result) {
        streamInitiationResultReceived(iq);
    } else if (iq.type() == QXmppIq::Set) {
        streamInitiationSetReceived(iq);
    }
}

// The remote party has accepted an outgoing transfer.
void QXmppTransferManager::streamInitiationResultReceived(const QXmppStreamInitiationIq &iq)
{
    auto *job = d->getOutgoingJobByRequestId(iq.from(), iq.id());
    if (!job ||
        job->state() != QXmppTransferJob::OfferState) {
        return;
    }

    const auto &form = iq.featureForm();
    const auto &fields = form.fields();

    for (const auto &field : fields) {
        if (field.key() == u"stream-method") {
            if ((field.value().toString() == ns_ibb) &&
                (d->supportedMethods & QXmppTransferJob::InBandMethod)) {
                job->d->method = QXmppTransferJob::InBandMethod;
            } else if ((field.value().toString() == ns_bytestreams) &&
                       (d->supportedMethods & QXmppTransferJob::SocksMethod)) {
                job->d->method = QXmppTransferJob::SocksMethod;
            }
        }
    }

    // remote party accepted stream initiation
    job->setState(QXmppTransferJob::StartState);
    if (job->method() == QXmppTransferJob::InBandMethod) {
        // lower block size for IBB
        job->d->blockSize = d->ibbBlockSize;

        QXmppIbbOpenIq openIq;
        openIq.setTo(job->d->jid);
        openIq.setSid(job->d->sid);
        openIq.setBlockSize(job->d->blockSize);
        job->d->requestId = openIq.id();
        client()->sendPacket(openIq);
    } else if (job->method() == QXmppTransferJob::SocksMethod) {
        if (!d->proxy.isEmpty()) {
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
        warning(u"QXmppTransferManager received an unsupported method"_s);
        job->terminate(QXmppTransferJob::ProtocolError);
    }
}

void QXmppTransferManager::streamInitiationSetReceived(const QXmppStreamInitiationIq &iq)
{
    QXmppIq response;
    response.setTo(iq.from());
    response.setId(iq.id());

    // check we support the profile
    if (iq.profile() != QXmppStreamInitiationIq::FileTransfer) {
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
    if (!isSignalConnected(QMetaMethod::fromSignal(&QXmppTransferManager::fileReceived))) {
        QXmppStanza::Error error(QXmppStanza::Error::Cancel, QXmppStanza::Error::Forbidden);
        error.setCode(403);

        response.setType(QXmppIq::Error);
        response.setError(error);
        client()->sendPacket(response);
        return;
    }

    // check the stream type
    auto *job = new QXmppTransferIncomingJob(iq.from(), client(), this);
    int offeredMethods = QXmppTransferJob::NoMethod;
    job->d->offerId = iq.id();
    job->d->sid = iq.siId();
    job->d->mimeType = iq.mimeType();
    job->d->fileInfo = iq.fileInfo();

    const auto &form = iq.featureForm();
    const auto &fields = form.fields();
    for (const auto &field : fields) {
        if (field.key() == u"stream-method") {
            QPair<QString, QString> option;
            const auto &options = field.options();
            for (const auto &option : options) {
                if (option.second == ns_ibb) {
                    offeredMethods = offeredMethods | QXmppTransferJob::InBandMethod;
                } else if (option.second == ns_bytestreams) {
                    offeredMethods = offeredMethods | QXmppTransferJob::SocksMethod;
                }
            }
        }
    }

    // select a method supported by both parties
    int sharedMethods = (offeredMethods & d->supportedMethods);
    if (sharedMethods & QXmppTransferJob::SocksMethod) {
        job->d->method = QXmppTransferJob::SocksMethod;
    } else if (sharedMethods & QXmppTransferJob::InBandMethod) {
        job->d->method = QXmppTransferJob::InBandMethod;
    } else {
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

    connect(job, &QObject::destroyed, this, &QXmppTransferManager::_q_jobDestroyed);
    connect(job, &QXmppTransferJob::finished, this, &QXmppTransferManager::_q_jobFinished);
    connect(job, &QXmppTransferJob::stateChanged, this, &QXmppTransferManager::_q_jobStateChanged);

    // allow user to accept or decline the job
    Q_EMIT fileReceived(job);
}

QString QXmppTransferManager::proxy() const
{
    return d->proxy;
}

///
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

bool QXmppTransferManager::proxyOnly() const
{
    return d->proxyOnly;
}

///
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

QXmppTransferJob::Methods QXmppTransferManager::supportedMethods() const
{
    return d->supportedMethods;
}

///
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
