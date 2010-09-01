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

#ifndef QXMPPSRVLOOKUP_H
#define QXMPPSRVLOOKUP_H

#include <QList>
#include <QString>

/// \brief The QXmppSrvLookup class provides static methods for DNS SRV lookups.
///

class QXmppSrvLookup
{
public:
    static const QString c2sPrefix;
    static const QString s2sPrefix;

    /// \brief Represents a DNS SRV record
    ///
    class SrvRecord
    {
    public:
        SrvRecord();

        QString hostName() const;
        void setHostName(const QString &hostName);

        quint16 port() const;
        void setPort(quint16 port);

    private:
        QString host_name;
        quint16 host_port;
    };

    QString errorString() const;
    QList<QXmppSrvLookup::SrvRecord> records() const;

    bool fromName(const QString &name);
    bool fromNameC2S(const QString &domain);
    bool fromNameS2S(const QString &domain);

private:
    QString m_errorString;
    QList<QXmppSrvLookup::SrvRecord> m_records;
};

#endif
