/*
 * Copyright (C) 2008-2011 The QXmpp developers
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
#include <QDomElement>
#include <QSettings>
#include <QTimer>

#include "QXmppConstants.h"
#include "QXmppDiscoveryIq.h"
#include "QXmppServer.h"
#include "QXmppServerPlugin.h"
#include "QXmppUtils.h"

#include "mod_stats.h"

static QXmppServerExtension *findExtension(QXmppServer *server, const QString &name)
{
    foreach (QXmppServerExtension *extension, server->extensions())
        if (extension->extensionName() == name)
            return extension;
    return 0;
}

class QXmppServerStatsPrivate
{
public:
    QString jid;

    int incomingClients;
    int incomingServers;
    int outgoingServers;

    QSettings *statistics;
    QString statisticsFile;
    QTimer *statisticsTimer;
};

/// Read statistics from file.
///
void QXmppServerStats::readStatistics()
{
    if (!d->statistics)
        return;

    foreach (QXmppServerExtension *extension, server()->extensions())
    {
        QVariantMap stats;

        // FIXME : remove hard-coded name
        const QString group = (extension == this) ? "xmpp-server" : extension->extensionName();
        d->statistics->beginGroup(group);
        foreach (const QString &key, d->statistics->childKeys())
            stats[key] = d->statistics->value(key);
        d->statistics->endGroup();
        extension->setStatistics(stats);
    }
}

/// Write statistics to file.
///
void QXmppServerStats::writeStatistics()
{
    if (!d->statistics)
        return;

    foreach (QXmppServerExtension *extension, server()->extensions())
    {
        const QVariantMap stats = extension->statistics();
        if (stats.isEmpty())
            continue;

        // FIXME : remove hard-coded name
        const QString group = (extension == this) ? "xmpp-server" : extension->extensionName();
        d->statistics->beginGroup(group);
        foreach (const QString &key, stats.keys())
            d->statistics->setValue(key, stats.value(key));
        d->statistics->endGroup();
    }
}

QXmppServerStats::QXmppServerStats()
    : d(new QXmppServerStatsPrivate)
{
    d->incomingClients = 0;
    d->incomingServers = 0;
    d->outgoingServers = 0;
    d->statistics = 0;
    d->statisticsTimer = new QTimer(this);
    d->statisticsTimer->setInterval(30 * 1000);

    bool check = connect(d->statisticsTimer, SIGNAL(timeout()),
        this, SLOT(writeStatistics()));
    Q_ASSERT(check);
    Q_UNUSED(check);
}

QXmppServerStats::~QXmppServerStats()
{
    delete d;
}

/// Returns the path of the file to which the statistics are written.
///

QString QXmppServerStats::file() const
{
    return d->statisticsFile;
}

/// Sets the path of the file to which the statistics are written.
///
/// \param file

void QXmppServerStats::setFile(const QString &file)
{
    if (d->statistics)
    {
        delete d->statistics;
        d->statistics = 0;
    }
    d->statisticsFile = file;
    if (!file.isEmpty())
    {
        d->statistics = new QSettings(file, QSettings::IniFormat, this);
        readStatistics();
        writeStatistics();
    }
}

/// Returns the JID from which statistics are served using Service Discovery.
///

QString QXmppServerStats::jid() const
{
    return d->jid;
}

/// Sets the JID from which statistics are served using Service Discovery.
///
/// \param jid

void QXmppServerStats::setJid(const QString &jid)
{
    d->jid = jid;
}

QStringList QXmppServerStats::discoveryItems() const
{
    return QStringList() << d->jid;
}

QVariantMap QXmppServerStats::statistics()
{
    return server()->statistics();
}

bool QXmppServerStats::handleStanza(const QDomElement &element)
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
            responseIq.setQueryNode(discoIq.queryNode());

            // check queried node
            const QString queryNode = discoIq.queryNode();
            QXmppServerExtension *extension = 0;
            QString key;
            if (!queryNode.isEmpty())
            {
                extension = findExtension(server(), queryNode.split("/").first());
                if (!extension || queryNode.count("/") > 1)
                {
                    responseIq.setType(QXmppIq::Error);
                    const QXmppStanza::Error error(QXmppStanza::Error::Cancel,
                        QXmppStanza::Error::ServiceUnavailable);
                    responseIq.setError(error);
                    server()->sendPacket(responseIq);
                    return true;
                }
                if (queryNode.count("/"))
                    key = queryNode.split("/").last();
            }

            if (discoIq.queryType() == QXmppDiscoveryIq::InfoQuery)
            {
                // features
                QStringList features = QStringList() << ns_disco_info << ns_disco_items;
                responseIq.setFeatures(features);

                // identity
                QList<QXmppDiscoveryIq::Identity> identities;
                QXmppDiscoveryIq::Identity identity;
                identity.setCategory("directory");
                identity.setType("statistics");
                if (!extension)
                    identity.setName("Server Statistics");
                else if (key.isEmpty())
                    identity.setName(QString("%1 module").arg(extension->extensionName()));
                else
                    identity.setName(QString("%1: %2").arg(key, extension->statistics().value(key).toString()));
                identities.append(identity);
                responseIq.setIdentities(identities);
            } else {
                QList<QXmppDiscoveryIq::Item> items;
                if (!extension)
                {
                    foreach (QXmppServerExtension *extension, server()->extensions())
                    {
                        if (extension->statistics().isEmpty())
                            continue;
                        QXmppDiscoveryIq::Item item;
                        item.setJid(d->jid);
                        item.setNode(extension->extensionName());
                        items.append(item);
                    }
                } else if (key.isEmpty()) {
                    QVariantMap stats = extension->statistics();
                    foreach (const QString &key, stats.keys())
                    {
                        QXmppDiscoveryIq::Item item;
                        item.setJid(d->jid);
                        item.setNode(extension->extensionName() + "/" + key);
                        items.append(item);
                    }
                }
                responseIq.setItems(items);
            }

            server()->sendPacket(responseIq);
            return true;
        }
    }

    return false;
}

bool QXmppServerStats::start()
{
    // determine jid
    if (d->jid.isEmpty())
        d->jid = "statistics." + server()->domain();

    d->statisticsTimer->start();

    return true;
}

void QXmppServerStats::stop()
{
    d->statisticsTimer->stop();
}

// PLUGIN

class QXmppServerStatsPlugin : public QXmppServerPlugin
{
public:
    QXmppServerExtension *create(const QString &key)
    {
        if (key == QLatin1String("stats"))
            return new QXmppServerStats;
        else
            return 0;
    };

    QStringList keys() const
    {
        return QStringList() << QLatin1String("stats");
    };
};

Q_EXPORT_STATIC_PLUGIN2(mod_stats, QXmppServerStatsPlugin)

