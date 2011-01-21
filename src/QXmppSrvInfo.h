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

#ifndef QXMPPSRVINFO_H
#define QXMPPSRVINFO_H

#include <QList>
#include <QString>

class QObject;
class QXmppSrvInfoPrivate;
class QXmppSrvRecordPrivate;

/// \brief The QXmppSrvRecord class represents a DNS SRV record.
///

class QXmppSrvRecord
{
public:
    QXmppSrvRecord();
    QXmppSrvRecord(const QXmppSrvRecord &other);
    ~QXmppSrvRecord();

    QString target() const;
    void setTarget(const QString &target);

    quint16 port() const;
    void setPort(quint16 port);

    quint16 priority() const;
    void setPriority(quint16 priority);

    quint16 weight() const;
    void setWeight(quint16 weight);

    QXmppSrvRecord &operator=(const QXmppSrvRecord &other);

private:
    QXmppSrvRecordPrivate *d;
};

/// \brief The QXmppSrvInfo class provides static methods for DNS SRV lookups.
///

class QXmppSrvInfo
{
public:
    /// This enum is used to describe the error encountered during lookup.
    enum Error
    {
        NoError = 0,
        NotFoundError = 1,
        UnknownError = 2,
    };

    QXmppSrvInfo();
    QXmppSrvInfo(const QXmppSrvInfo &other);
    ~QXmppSrvInfo();

    Error error() const;
    QString errorString() const;
    QList<QXmppSrvRecord> records() const;

    static QXmppSrvInfo fromName(const QString &dname);
    static void lookupService(const QString &name, QObject *receiver, const char *member);

private:
    QXmppSrvInfoPrivate *d;
};

#endif
