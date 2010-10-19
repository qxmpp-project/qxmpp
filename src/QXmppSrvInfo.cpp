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

#include "QXmppSrvInfo.h"
#include <QMetaObject>

#if defined(Q_OS_WIN)
#include <windows.h>
#include <windns.h>
#elif defined(Q_OS_SYMBIAN)
#include <dns_qry.h>
#elif defined(Q_OS_UNIX)
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/nameser.h>
#include <arpa/nameser_compat.h>
#include <resolv.h>
#endif

class QXmppSrvRecordPrivate
{
public:
    QXmppSrvRecordPrivate()
        : port(0),
          priority(0),
          weight(0)
    { }

    QString target;
    quint16 port;
    quint16 priority;
    quint16 weight;
};

/// Constructs an empty service record object.
///

QXmppSrvRecord::QXmppSrvRecord()
    : d(new QXmppSrvRecordPrivate)
{
}

/// Constructs a copy of \a other.

QXmppSrvRecord::QXmppSrvRecord(const QXmppSrvRecord &other)
    : d(new QXmppSrvRecordPrivate)
{
    *d = *other.d;
}

/// Destroys a service record.

QXmppSrvRecord::~QXmppSrvRecord()
{
    delete d;
}

/// Returns host name for this service record.
///

QString QXmppSrvRecord::target() const
{
    return d->target;
}

/// Sets the host name for this service record.
///
/// \param hostName

void QXmppSrvRecord::setTarget(const QString &target)
{
    d->target = target;
}

/// Returns the port for this service record.
///

quint16 QXmppSrvRecord::port() const
{
    return d->port;
}

/// Sets the port for this service record.
///
/// \param port

void QXmppSrvRecord::setPort(quint16 port)
{
    d->port = port;
}

/// Returns the priority for this service record.
///

quint16 QXmppSrvRecord::priority() const
{
    return d->priority;
}

/// Sets the priority for this service record.
///
/// \param priority

void QXmppSrvRecord::setPriority(quint16 priority)
{
    d->priority = priority;
}

/// Returns the weight for this service record.
///

quint16 QXmppSrvRecord::weight() const
{
    return d->weight;
}

/// Sets the weight for this service record.
///
/// \param priority

void QXmppSrvRecord::setWeight(quint16 weight)
{
    d->weight = weight;
}

/// Assigns the data of the \a other object to this service record object,
/// and returns a reference to it.

QXmppSrvRecord &QXmppSrvRecord::operator=(const QXmppSrvRecord &other)
{
    *d = *other.d;
    return *this;
}

class QXmppSrvInfoPrivate
{
public:
    QXmppSrvInfoPrivate()
        : error(QXmppSrvInfo::NoError)
    { }

    QXmppSrvInfo::Error error;
    QString errorString;
    QList<QXmppSrvRecord> records;
};

/// Constructs an empty service info.

QXmppSrvInfo::QXmppSrvInfo()
    : d(new QXmppSrvInfoPrivate)
{
}

/// Constructs a copy of \a other.

QXmppSrvInfo::QXmppSrvInfo(const QXmppSrvInfo &other)
    : d(new QXmppSrvInfoPrivate)
{
    *d = *other.d;
}

/// Destroys a service info.

QXmppSrvInfo::~QXmppSrvInfo()
{
    delete d;
}

/// Returns the type of error that occurred if the service lookup
/// failed; otherwise returns NoError.

QXmppSrvInfo::Error QXmppSrvInfo::error() const
{
    return d->error;
}

/// If the lookup failed, this function returns a human readable description
/// of the error.

QString QXmppSrvInfo::errorString() const
{
    return d->errorString;
}

/// Returns the list of records associated with this service.
///
QList<QXmppSrvRecord> QXmppSrvInfo::records() const
{
    return d->records;
}

/// Perform a DNS lookup for an SRV entry.
///
/// Returns a QXmppSrvInfo object containing the found records.
///
/// \param dname

QXmppSrvInfo QXmppSrvInfo::fromName(const QString &dname)
{
    QXmppSrvInfo result;

#if defined(Q_OS_WIN)
    PDNS_RECORD records, ptr;

    /* perform DNS query */
    if (DnsQuery_UTF8(dname.toUtf8(), DNS_TYPE_SRV, DNS_QUERY_STANDARD, NULL, &records, NULL) != ERROR_SUCCESS)
    {
        result.d->error = QXmppSrvInfo::NotFoundError;
        result.d->errorString = QLatin1String("DnsQuery_UTF8 failed");
        return result;
    }

    /* extract results */
    for (ptr = records; ptr != NULL; ptr = ptr->pNext)
    {
        if ((ptr->wType == DNS_TYPE_SRV) && !strcmp((char*)ptr->pName, dname.toUtf8()))
        {
            QXmppSrvRecord record;
            record.setTarget(QString::fromUtf8((char*)ptr->Data.Srv.pNameTarget));
            record.setPort(ptr->Data.Srv.wPort);
            record.setPriority(ptr->Data.Srv.wPriority);
            record.setWeight(ptr->Data.Srv.wWeight);
            result.d->records.append(record);
        }
    }

    DnsRecordListFree(records, DnsFreeRecordList);

#elif defined(Q_OS_SYMBIAN)
    RHostResolver dnsResolver;
    RSocketServ dnsSocket;

    /* initialise resolver */
    TInt err = dnsSocket.Connect();
    err = dnsResolver.Open(dnsSocket, KAfInet, KProtocolInetUdp);
    if (err != KErrNone)
    {
        result.d->error = QXmppSrvInfo::UnknownError;
        result.d->errorString = QLatin1String("RHostResolver::Open failed");
        return result;
    }

    /* perform DNS query */
    TDnsQueryBuf dnsQuery;
    TDnsRespSRVBuf dnsResponse;
    dnsQuery().SetClass(KDnsRRClassIN);
    QByteArray utf8name = dname.toUtf8();
    TPtrC8 queryPtr(reinterpret_cast<const TUint8*>(utf8name.constData()),utf8name.size());
    dnsQuery().SetData(queryPtr);
    dnsQuery().SetType(KDnsRRTypeSRV);
    err = dnsResolver.Query(dnsQuery, dnsResponse);
    if (err != KErrNone)
    {
        result.d->error = QXmppSrvInfo::NotFoundError;
        result.d->errorString = QLatin1String("RHostResolver::Query failed");
        return result;
    }

    /* extract results */
    while (err == KErrNone)
    {
        QXmppSrvRecord record;
        record.setTarget(QString::fromUtf8((const char*)dnsResponse().Target().Ptr(),
                                             dnsResponse().Target().Length()));
        record.setPort(dnsResponse().Port());
        record.setPriority(dnsResponse().Priority());
        record.setWeight(dnsResponse().Weight());
        result.d->records.append(record);

        err = dnsResolver.QueryGetNext(dnsResponse);
    }

#elif defined(Q_OS_UNIX)
    unsigned char response[PACKETSZ];
    int responseLength, answerCount, answerIndex;

    /* explicitly call res_init in case config changed */
    res_init();

    /* perform DNS query */
    memset(response, 0, sizeof(response));
    responseLength = res_query(dname.toAscii(), C_IN, T_SRV, response, sizeof(response));
    if (responseLength < int(sizeof(HEADER)))
    {
        result.d->error = QXmppSrvInfo::NotFoundError;
        result.d->errorString = QString::fromLatin1("res_query failed: %1").arg(QLatin1String(hstrerror(h_errno)));
        return result;
    }

    /* check the response header */
    HEADER *header = (HEADER*)response;
    if (header->rcode != NOERROR || !(answerCount = ntohs(header->ancount)))
    {
        result.d->error = QXmppSrvInfo::UnknownError;
        result.d->errorString = QLatin1String("res_query returned an error");
        return result;
    }

    /* skip the query */
    char host[PACKETSZ], answer[PACKETSZ];
    unsigned char *p = response + sizeof(HEADER);
    int status = dn_expand(response, response + responseLength, p, host, sizeof(host));
    if (status < 0)
    {
        result.d->error = QXmppSrvInfo::UnknownError;
        result.d->errorString = QLatin1String("dn_expand failed");
        return result;
    }
    p += status + 4;

    /* parse answers */
    answerIndex = 0;
    while ((p < response + responseLength) && (answerIndex < answerCount))
    {
        int type, klass, ttl, size;
        status = dn_expand(response, response + responseLength, p, host, sizeof(host));
        if (status < 0)
        {
            result.d->error = QXmppSrvInfo::UnknownError;
            result.d->errorString = QLatin1String("dn_expand failed");
            return result;
        }

        p += status;
        type = (p[0] << 8) | p[1];
        p += 2;
        klass = (p[0] << 8) | p[1];
        p += 2;
        ttl = (p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3];
        p += 4;
        size = (p[0] << 8) | p[1];
        p += 2;

        if (type == T_SRV)
        {
            quint16 priority = (p[0] << 8) | p[1];
            quint16 weight = (p[2] << 8) | p[3];
            quint16 port = (p[4] << 8) | p[5];
            status = dn_expand(response, response + responseLength, p + 6, answer, sizeof(answer));
            if (status < 0)
            {
                result.d->error = QXmppSrvInfo::UnknownError;
                result.d->errorString = QLatin1String("dn_expand failed");
                return result;
            }
            QXmppSrvRecord record;
            record.setTarget(QString::fromUtf8(answer));
            record.setPort(port);
            record.setPriority(priority);
            record.setWeight(weight);
            result.d->records.append(record);
        }
        p += size;
        answerIndex++;
    }
#endif
    if (result.d->records.isEmpty())
        result.d->error = QXmppSrvInfo::NotFoundError;
    return result;
}

/// Performs a DNS lookup for an SRV entry. When the result of the lookup is
/// ready, the slot or signal \a member in \a receiver is called with a
/// QXmppSrvInfo argument.

void QXmppSrvInfo::lookupService(const QString &name, QObject *receiver, const char *member)
{
    QXmppSrvInfo result = QXmppSrvInfo::fromName(name);
    QMetaObject::invokeMethod(receiver, member, Qt::QueuedConnection, Q_ARG(QXmppSrvInfo, result));
}

