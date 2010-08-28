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

#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDebug>
#include <QDomElement>
#include <QHostInfo>
#include <QSettings>
#include <QTimer>

#include "QXmppByteStreamIq.h"
#include "QXmppConfiguration.h"
#include "QXmppConstants.h"
#include "QXmppDiscoveryIq.h"
#include "QXmppPingIq.h"
#include "QXmppServer.h"
#include "QXmppServerPlugin.h"
#include "QXmppSocks.h"
#include "QXmppStream.h"
#include "QXmppUtils.h"

#include "mod_proxy65.h"

const int blockSize = 16384;

static QString streamHash(const QString &sid, const QString &initiatorJid, const QString &targetJid)
{
    QCryptographicHash hash(QCryptographicHash::Sha1);
    QString str = sid + initiatorJid + targetJid;
    hash.addData(str.toAscii());
    return hash.result().toHex();
}

QTcpSocketPair::QTcpSocketPair(const QString &hash)
    : key(hash), transfer(0), target(0), source(0)
{
}

void QTcpSocketPair::activate()
{
    time.start();
    connect(target, SIGNAL(bytesWritten(qint64)), this, SLOT(sendData()));
    connect(source, SIGNAL(readyRead()), this, SLOT(sendData()));
}

void QTcpSocketPair::addSocket(QTcpSocket *socket)
{
    if (source)
    {
        qDebug() << "Unexpected connection for" << key;
        socket->deleteLater();
        return;
    }

    if (target)
    {
        qDebug() << "Opened source connection for" << key;
        source = socket;
        source->setReadBufferSize(4 * blockSize);
        connect(source, SIGNAL(disconnected()), this, SLOT(disconnected()));
    }
    else
    {
        qDebug() << "Opened target connection for" << key;
        target = socket;
        connect(target, SIGNAL(disconnected()), this, SLOT(disconnected()));
    }
    socket->setParent(this);
}

void QTcpSocketPair::disconnected()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket)
        return;

    if (target == socket)
    {
        qDebug() << "Closed target connection for" << key;
        emit finished();
    } else if (source == socket) {
        qDebug() << "Closed source connection for" << key;
        if (!target || !target->isOpen())
            emit finished();
    }
}

void QTcpSocketPair::sendData()
{
    // don't saturate the outgoing socket
    if (target->bytesToWrite() >= 2 * blockSize)
        return;

    // check for completion
    if (!source->isOpen())
    {
        if (!target->bytesToWrite())
            target->close();
        return;
    }

    char buffer[blockSize];
    qint64 length = source->read(buffer, blockSize);
    if (length < 0)
    {
        target->close();
        return;
    }
    if (length > 0)
    {
        target->write(buffer, length);
        transfer += length;
    }
}

struct TransferStats
{
    QDateTime date;
    qint64 size;
    int elapsed;
};

class QXmppServerProxy65Private
{
public:
    QString jid;
    QHostAddress host;
    quint16 port;

    QMap<QString, QTcpSocketPair*> pairs;
    QList<TransferStats> recent;
    QXmppSocksServer *server;
    QSettings *statistics;
    QString statisticsFile;
    QTimer *statisticsTimer;
};

QXmppServerProxy65::QXmppServerProxy65()
    : d(new QXmppServerProxy65Private)
{
    d->statistics = 0;
    d->statisticsTimer = new QTimer(this);
    d->statisticsTimer->setInterval(300 * 1000);
    d->statisticsTimer->start();

    d->server = new QXmppSocksServer(this);

    bool check = connect(d->server, SIGNAL(newConnection(QTcpSocket*, const QString&, quint16)),
        this, SLOT(slotSocketConnected(QTcpSocket*, const QString &, quint16)));
    Q_ASSERT(check);

    check = connect(d->statisticsTimer, SIGNAL(timeout()),
        this, SLOT(slotUpdateStatistics()));
    Q_ASSERT(check);
}

QXmppServerProxy65::~QXmppServerProxy65()
{
    delete d;
}

QStringList QXmppServerProxy65::discoveryItems() const
{
    return QStringList() << d->jid;
}

bool QXmppServerProxy65::handleStanza(QXmppStream *stream, const QDomElement &element)
{
    if (element.attribute("to") != d->jid)
        return false;

    if (element.tagName() == "iq" && QXmppDiscoveryIq::isDiscoveryIq(element))
    {
        QXmppDiscoveryIq discoIq;
        discoIq.parse(element);

        if (discoIq.type() == QXmppIq::Get)
        {
            QXmppDiscoveryIq responseIq;
            responseIq.setTo(discoIq.from());
            responseIq.setFrom(discoIq.to());
            responseIq.setId(discoIq.id());
            responseIq.setType(QXmppIq::Result);
            responseIq.setQueryType(discoIq.queryType());

            if (discoIq.queryType() == QXmppDiscoveryIq::InfoQuery)
            {
                QStringList features = QStringList() << ns_disco_info << ns_disco_items << ns_bytestreams;

                QList<QXmppDiscoveryIq::Identity> identities;
                QXmppDiscoveryIq::Identity identity;
                identity.setCategory("proxy");
                identity.setType("bytestreams");
                identity.setName("SOCKS5 Bytestreams");
                identities.append(identity);

                responseIq.setFeatures(features);
                responseIq.setIdentities(identities);
            }

            stream->sendPacket(responseIq);
            return true;
        }
    }
    else if (element.tagName() == "iq" && QXmppByteStreamIq::isByteStreamIq(element))
    {
        QXmppByteStreamIq bsIq;
        bsIq.parse(element);

        if (bsIq.type() == QXmppIq::Get)
        {
            QXmppByteStreamIq responseIq;
            responseIq.setType(QXmppIq::Result);
            responseIq.setTo(bsIq.from());
            responseIq.setFrom(bsIq.to());
            responseIq.setId(bsIq.id());

            QList<QXmppByteStreamIq::StreamHost> streamHosts;

            QXmppByteStreamIq::StreamHost streamHost;
            streamHost.setJid(d->jid);
            streamHost.setHost(d->host);
            streamHost.setPort(d->port);
            streamHosts.append(streamHost);

            responseIq.setStreamHosts(streamHosts);
            stream->sendPacket(responseIq);
        }
        else if (bsIq.type() == QXmppIq::Set)
        {
            QString hash = streamHash(bsIq.sid(), bsIq.from(), bsIq.activate());
            QTcpSocketPair *pair = d->pairs.value(hash);

            QXmppIq responseIq;
            responseIq.setTo(bsIq.from());
            responseIq.setFrom(bsIq.to());
            responseIq.setId(bsIq.id());

            // FIXME : make it possible to specify permissions
            if (pair &&
                jidToDomain(bsIq.from()) == server()->domain())
            {
                qDebug() << "Activating connection" << hash << "by" << bsIq.from();
                pair->activate();
                responseIq.setType(QXmppIq::Result);
            } else {
                qWarning() << "Not activating connection" << hash << "by" << bsIq.from();
                responseIq.setType(QXmppIq::Error);
            }
            stream->sendPacket(responseIq);
        }
        return true;
    }
    return false;
}

QString QXmppServerProxy65::jid() const
{
    return d->jid;
}

void QXmppServerProxy65::setJid(const QString &jid)
{
    d->jid = jid;
}

QString QXmppServerProxy65::statisticsFile() const
{
    return d->statisticsFile;
}

void QXmppServerProxy65::setStatisticsFile(const QString &statisticsFile)
{
    if (d->statistics)
    {
        delete d->statistics;
        d->statistics = 0;
    }
    d->statisticsFile = statisticsFile;
    if (!statisticsFile.isEmpty())
    {
        d->statistics = new QSettings(statisticsFile, QSettings::IniFormat, this);
        slotUpdateStatistics();
    }
}

bool QXmppServerProxy65::start()
{
    // determine jid
    if (d->jid.isEmpty())
        d->jid = "proxy." + server()->domain();

    // determine address
    const QString hostName = server()->domain();
    QHostAddress hostAddress;
    if (!hostAddress.setAddress(hostName))
    {
        QHostInfo hostInfo = QHostInfo::fromName(hostName);
        if (hostInfo.addresses().isEmpty())
        {
            qWarning() << "Could not lookup host" << hostName;
            return false;
        }
        hostAddress = hostInfo.addresses().first();
    }
    quint16 port = 7777;

    // start listening
    if (!d->server->listen(hostAddress, port))
        return false;
    d->host = hostAddress;
    d->port = port;
    return true;
}

void QXmppServerProxy65::stop()
{
    d->server->close();
    foreach (QTcpSocketPair *pair, d->pairs)
        delete pair;
    d->pairs.clear();
}

void QXmppServerProxy65::slotSocketConnected(QTcpSocket *socket, const QString &hostName, quint16 port)
{
    bool check;
    QTcpSocketPair *pair = d->pairs.value(hostName);
    if (!pair)
    {
        pair = new QTcpSocketPair(hostName);
        check = connect(pair, SIGNAL(finished()), this, SLOT(slotPairFinished()));
        Q_ASSERT(check);
        d->pairs.insert(hostName, pair);
    }
    pair->addSocket(socket);
}

void QXmppServerProxy65::slotPairFinished()
{
    QTcpSocketPair *pair = qobject_cast<QTcpSocketPair*>(sender());
    if (!pair)
        return;

    qDebug() << "Data transfered for" << pair->key << pair->transfer;

    // update speed statistics
    TransferStats stats;
    stats.date = QDateTime::currentDateTime();
    stats.size = pair->transfer;
    stats.elapsed = pair->time.elapsed();
    d->recent.prepend(stats);
    slotUpdateStatistics();

    // store total statistics
    if (d->statistics)
    {
        d->statistics->beginGroup("socks-proxy");
        d->statistics->setValue("total-bytes",
            d->statistics->value("total-bytes").toULongLong() + pair->transfer);
        d->statistics->setValue("total-transfers",
            d->statistics->value("total-transfers").toULongLong() + 1);
        d->statistics->endGroup();
    }

    // remove socket pair
    d->pairs.remove(pair->key);
    pair->deleteLater();
}

void QXmppServerProxy65::slotUpdateStatistics()
{
    qint64 minimumSpeed = -1;
    qint64 maximumSpeed = 0;
    qint64 totalSize = 0;
    qint64 totalElapsed = 0;
    QDateTime cutoff = QDateTime::currentDateTime().addDays(-1);
    for (int i = d->recent.size() - 1; i >= 0; --i)
    {
        // only keep 100 points, less than one day old
        if (i >= 100 || d->recent[i].date < cutoff)
        {
            d->recent.removeAt(i);
        } else if (d->recent[i].elapsed > 0) {
            qint64 speed = (1000 * d->recent[i].size) / d->recent[i].elapsed;
            if (speed > maximumSpeed)
                maximumSpeed = speed;
            if (minimumSpeed < 0 || speed < minimumSpeed)
                minimumSpeed = speed;
            totalSize += d->recent[i].size;
            totalElapsed += d->recent[i].elapsed;
        }
    }
    if (minimumSpeed < 0)
        minimumSpeed = 0;
    qint64 averageSpeed = totalElapsed > 0 ? (1000 * totalSize) / totalElapsed : 0;

    // store statistics
    if (d->statistics)
    {
        d->statistics->beginGroup("socks-proxy");
        d->statistics->setValue("version", qApp->applicationVersion());
        d->statistics->setValue("average-speed", averageSpeed);
        d->statistics->setValue("minimum-speed", minimumSpeed);
        d->statistics->setValue("maximum-speed", maximumSpeed);
        d->statistics->endGroup();
    }
}

// PLUGIN

class QXmppServerProxy65Plugin : public QXmppServerPlugin
{
public:
    QXmppServerExtension *create(const QString &key)
    {
        if (key == QLatin1String("proxy65"))
            return new QXmppServerProxy65;
        else
            return 0;
    };

    QStringList keys() const
    {
        return QStringList() << QLatin1String("proxy65");
    };
};

Q_EXPORT_STATIC_PLUGIN2(mod_proxy65, QXmppServerProxy65Plugin)

