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

#ifndef QXMPP_SERVER_STATS_H
#define QXMPP_SERVER_STATS_H

#include "QXmppServerExtension.h"

class QSettings;
class QXmppServerStatsPrivate;

/// \brief QXmppServer extension for statistics.
///

class QXmppServerStats : public QXmppServerExtension
{
    Q_OBJECT
    Q_CLASSINFO("ExtensionName", "stats");
    Q_PROPERTY(QString file READ file WRITE setFile);
    Q_PROPERTY(QString jid READ jid WRITE setJid);

public:
    QXmppServerStats();
    ~QXmppServerStats();

    QString file() const;
    void setFile(const QString &file);

    QString jid() const;
    void setJid(const QString &jid);

    /// cond
    QStringList discoveryItems() const;
    bool handleStanza(QXmppStream *stream, const QDomElement &element);
    QVariantMap statistics() const;
    bool start();
    void stop();
    /// \endcond

private slots:
    void streamAdded(QXmppStream *stream);
    void streamRemoved(QXmppStream *stream);
    void writeStatistics();

private:
    void readStatistics();
    QXmppServerStatsPrivate * const d;
};

#endif
