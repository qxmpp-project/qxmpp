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

#include "QXmppClient.h"
#include "QXmppConstants.h"
#include "QXmppIbbIqs.h"
#include "QXmppStreamInitiationIq.h"
#include "QXmppTransferManager.h"
#include "QXmppUtils.h"

QXmppTransferJob::QXmppTransferJob(const QString &jid, QXmppTransferManager *manager)
    : QObject(manager),
    m_done(0),
    m_error(NoError),
    m_iodevice(0),
    m_jid(jid),
    m_fileSize(0),
    m_ibbSequence(0)
{
}

void QXmppTransferJob::accept(QIODevice *iodevice)
{
    if (!m_iodevice)
        m_iodevice = iodevice;
}

QXmppTransferJob::Error QXmppTransferJob::error() const
{
    return m_error;
}

QString QXmppTransferJob::jid() const
{
    return m_jid;
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

void QXmppTransferJob::terminate(QXmppTransferJob::Error cause)
{
    // close IO device
    m_iodevice->close();

    // emit signal
    m_error = cause;
    if (cause == NoError)
        emit finished();
    else
        emit error(m_error);
}

QXmppTransferManager::QXmppTransferManager(QXmppClient *client)
    : m_client(client), m_ibbBlockSize(4096)
{
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

void QXmppTransferManager::ibbCloseIqReceived(const QXmppIbbCloseIq &iq)
{
    QXmppIq response;
    response.setTo(iq.getFrom());
    response.setId(iq.getId());

    QXmppTransferJob *job = getJobBySid(iq.getFrom(), iq.getSid());
    if (!job)
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
    if (!job)
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
    if (!job)
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

    // accept transfer
    response.setType(QXmppIq::Result);
    m_client->sendPacket(response);
}

void QXmppTransferManager::iqReceived(const QXmppIq &iq)
{
    QXmppTransferJob *job = getJobByRequestId(iq.getFrom(), iq.getId());
    if (!job)
        return;

    // if the IO device is closed, do nothing
    if (!job->m_iodevice->isOpen())
        return;

    if (iq.getType() == QXmppIq::Result)
    {
        const QByteArray buffer = job->m_iodevice->read(m_ibbBlockSize);

        if (buffer.size())
        {
            // send next data block
            QXmppIbbDataIq dataIq;
            dataIq.setTo(job->m_jid);
            dataIq.setSid(job->m_sid);
            dataIq.setSequence(job->m_ibbSequence);
            dataIq.setPayload(buffer);
            m_client->sendPacket(dataIq);

            job->m_done += buffer.size();
            job->m_requestId = dataIq.getId();
            job->m_ibbSequence++;
            job->progress(job->m_done, job->fileSize());
        } else {
            // close the bytestream
            QXmppIbbCloseIq closeIq;
            closeIq.setTo(job->m_jid);
            closeIq.setSid(job->m_sid);
            m_client->sendPacket(closeIq);

            job->m_requestId = closeIq.getId();
            job->terminate(QXmppTransferJob::NoError);
        }
    }
    else if (iq.getType() == QXmppIq::Error)
    {
        // close the bytestream
        QXmppIbbCloseIq closeIq;
        closeIq.setTo(job->m_jid);
        closeIq.setSid(job->m_sid);
        m_client->sendPacket(closeIq);

        job->m_requestId = closeIq.getId();
        job->terminate(QXmppTransferJob::ProtocolError);

    }
}

QXmppTransferJob *QXmppTransferManager::sendFile(const QString &jid, const QString &fileName)
{
    // open file
    QFile *fileIo = new QFile(fileName, this);
    fileIo->open(QIODevice::ReadOnly);
    QFileInfo info(*fileIo);

    // create job
    QXmppTransferJob *job = new QXmppTransferJob(jid, this);
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

    QXmppElement option;
    option.setTagName("option");
    field.appendChild(option);

    QXmppElement value;
    value.setTagName("value");
    value.setValue(ns_ibb);
    option.appendChild(value);
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

    int method = 0;
    foreach (const QXmppElement &item, iq.getSiItems())
    {
        if (item.tagName() == "feature" && item.attribute("xmlns") == ns_feature_negotiation)
        {
            QXmppElement field = item.firstChildElement("x").firstChildElement("field");
            while (!field.isNull())
            {
                if (field.attribute("var") == "stream-method")
                {
                    if (field.firstChildElement("value").value() == ns_ibb)
                        method = QXmppTransferJob::InBandByteStream;
/*
                    else if (field.firstChildElement("value").value() == ns_bytestreams)
                        method = QXmppTransferJob::SocksByteStream;
*/
                }
                field = field.nextSiblingElement("field");
            }
        }
    }

    if (method == QXmppTransferJob::InBandByteStream)
    {
        QXmppIbbOpenIq openIq;
        openIq.setTo(job->m_jid);
        openIq.setSid(job->m_sid);
        openIq.setBlockSize(m_ibbBlockSize);
        m_client->sendPacket(openIq);
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
    QXmppTransferJob *job = new QXmppTransferJob(iq.getFrom(), this);
    job->m_methods = 0;
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
                            job->m_methods = job->m_methods | QXmppTransferJob::InBandByteStream;
/*
                        else if (option.firstChildElement("value").value() == ns_bytestreams)
                            job->m_methods = job->m_methods | QXmppTransferJob::SocksByteStream;
*/
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

    if (!job->m_methods)
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
    value.setValue(ns_ibb);

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

