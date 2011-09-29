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

#include "QXmppSrvInfo.h"
#include "QXmppSrvInfo_p.h"

#include <QCoreApplication>
#include <QLibrary>
#include <QMetaObject>
#include <QMetaType>

#if defined(Q_OS_WIN)
#include <windows.h>
#include <windns.h>
#elif defined(Q_OS_SYMBIAN)
#include <dns_qry.h>
#elif defined(Q_OS_UNIX)
#include <sys/types.h>
#include <netdb.h>
#if defined(Q_OS_MAC)
#include <arpa/nameser.h>
#include <arpa/nameser_compat.h>
#endif
#include <resolv.h>
#endif

Q_GLOBAL_STATIC(QXmppSrvInfoLookupManager, theSrvInfoLookupManager)

#if defined(Q_OS_WIN)
typedef DNS_STATUS (WINAPI *dns_query_utf8_proto)(PCSTR,WORD,DWORD,PIP4_ARRAY,PDNS_RECORD*,PVOID*);
static dns_query_utf8_proto local_dns_query_utf8 = 0;
typedef void (WINAPI *dns_record_list_free_proto)(PDNS_RECORD,DNS_FREE_TYPE);
static dns_record_list_free_proto local_dns_record_list_free = 0;

static void resolveLibrary()
{
    QLibrary lib(QLatin1String("dnsapi.dll"));
    if (!lib.load())
        return;

    local_dns_query_utf8 = dns_query_utf8_proto(lib.resolve("DnsQuery_UTF8"));
    local_dns_record_list_free = dns_record_list_free_proto(lib.resolve("DnsRecordListFree"));
}

#elif defined(Q_OS_UNIX) && !defined(Q_OS_ANDROID) && !defined(Q_OS_SYMBIAN)
typedef int (*dn_expand_proto)(const unsigned char *, const unsigned char *, const unsigned char *, char *, int);
static dn_expand_proto local_dn_expand = 0;
typedef int (*res_ninit_proto)(res_state);
static res_ninit_proto local_res_ninit = 0;
typedef int (*res_nquery_proto)(res_state, const char *, int, int, unsigned char *, int);
static res_nquery_proto local_res_nquery = 0;

static void resolveLibrary()
{
    QLibrary lib(QLatin1String("resolv"));
    if (!lib.load())
        return;

    local_dn_expand = dn_expand_proto(lib.resolve("__dn_expand"));
    if (!local_dn_expand)
        local_dn_expand = dn_expand_proto(lib.resolve("dn_expand"));

    local_res_ninit = res_ninit_proto(lib.resolve("__res_ninit"));
    if (!local_res_ninit)
        local_res_ninit = res_ninit_proto(lib.resolve("res_9_ninit"));
    if (!local_res_ninit)
        local_res_ninit = res_ninit_proto(lib.resolve("res_ninit"));

    local_res_nquery = res_nquery_proto(lib.resolve("__res_nquery"));
    if (!local_res_nquery)
        local_res_nquery = res_nquery_proto(lib.resolve("res_9_nquery"));
    if (!local_res_nquery)
        local_res_nquery = res_nquery_proto(lib.resolve("res_nquery"));
}
#endif

//#define QSRVINFO_DEBUG

static bool recordLessThan(const QXmppSrvRecord &r1, const QXmppSrvRecord &r2)
{
    // Order by priority.
    if (r1.priority() < r2.priority())
        return true;

    // If the priority is equal, put zero weight records first.
    if (r1.priority() == r2.priority() &&
        r1.weight() == 0 && r2.weight() > 0)
        return true;
    return false;
}

static void sortSrvRecords(QList<QXmppSrvRecord> &records)
{
    // If we have no more than one result, we are done.
    if (records.size() <= 1)
        return;

    // Order the records by priority, and for records with an equal
    // priority, put records with a zero weight first.
    qSort(records.begin(), records.end(), recordLessThan);

    int i = 0;
    while (i < records.size()) {

        // Determine the slice of records with the current priority.
        QList<QXmppSrvRecord> slice;
        const quint16 slicePriority = records[i].priority();
        int sliceWeight = 0;
        for (int j = i; j < records.size(); ++j) {
            if (records[j].priority() != slicePriority)
                break;
            sliceWeight += records[j].weight();
            slice << records[j];
        }
#ifdef QSRVINFO_DEBUG
        qDebug("sortSrvRecords() : priority %i (size: %i, total weight: %i)",
               slicePriority, slice.size(), sliceWeight);
#endif

        // Order the slice of records.
        while (!slice.isEmpty()) {
            int randVal = qrand() % (sliceWeight + 1);
            sliceWeight = 0;
            bool sliceDone = false;
            int j = 0;
            while (j < slice.size()) {
                if (!sliceDone && sliceWeight + slice[j].weight() >= randVal) {
#ifdef QSRVINFO_DEBUG
                    qDebug("sortSrvRecords() : adding %s %i (weight: %i)",
                           qPrintable(slice[j].target()), slice[j].port(),
                           slice[j].weight());
#endif
                    records[i++] = slice.takeAt(j);
                    sliceDone = true;
                } else {
                    sliceWeight += slice[j].weight();
                    j++;
                }
            }
        }
    }
}

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
/// \param target

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
/// \param weight

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

    // Load DnsQuery_UTF8 and DnsRecordListFree on demand.
    static volatile bool triedResolve = false;
    if (!triedResolve) {
        if (!triedResolve) {
            resolveLibrary();
            triedResolve = true;
        }
    }

    // If DnsQuery_UTF8 or DnsRecordListFree is missing, fail.
    if (!local_dns_query_utf8 || !local_dns_record_list_free) {
        result.d->error = QXmppSrvInfo::UnknownError;
        result.d->errorString = QLatin1String("DnsQuery_UTF8 or DnsRecordListFree functions not found");
        return result;
    }

    // Perform DNS query.
    if (local_dns_query_utf8(dname.toUtf8(), DNS_TYPE_SRV, DNS_QUERY_STANDARD, NULL, &records, NULL) != ERROR_SUCCESS)
    {
        result.d->error = QXmppSrvInfo::NotFoundError;
        result.d->errorString = QLatin1String("DnsQuery_UTF8 failed");
        return result;
    }

    // Extract results.
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

    local_dns_record_list_free(records, DnsFreeRecordList);

#elif defined(Q_OS_ANDROID)
    // TODO
#elif defined(Q_OS_SYMBIAN)
    RHostResolver dnsResolver;
    RSocketServ dnsSocket;

    // Initialise resolver.
    TInt err = dnsSocket.Connect();
    err = dnsResolver.Open(dnsSocket, KAfInet, KProtocolInetUdp);
    if (err != KErrNone)
    {
        result.d->error = QXmppSrvInfo::UnknownError;
        result.d->errorString = QLatin1String("RHostResolver::Open failed");
        return result;
    }

    // Perform DNS query.
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

    // Extract results.
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

    // Load dn_expand, res_ninit and res_nquery on demand.
    static volatile bool triedResolve = false;
    if (!triedResolve) {
        if (!triedResolve) {
            resolveLibrary();
            triedResolve = true;
        }
    }

    // If dn_expand or res_nquery is missing, fail.
    if (!local_dn_expand || !local_res_ninit || !local_res_nquery) {
        result.d->error = QXmppSrvInfo::UnknownError;
        result.d->errorString = QLatin1String("dn_expand, res_ninit or res_nquery functions not found");
        return result;
    }

    // Initialise state.
    struct __res_state state;
    local_res_ninit(&state);

    // Perform DNS query.
    memset(response, 0, sizeof(response));
    responseLength = local_res_nquery(&state, dname.toAscii(), C_IN, T_SRV, response, sizeof(response));
    if (responseLength < int(sizeof(HEADER)))
    {
        result.d->error = QXmppSrvInfo::NotFoundError;
        result.d->errorString = QLatin1String("res_nquery failed");
        return result;
    }

    // Check the response header.
    HEADER *header = (HEADER*)response;
    if (header->rcode != NOERROR || !(answerCount = ntohs(header->ancount)))
    {
        result.d->error = QXmppSrvInfo::UnknownError;
        result.d->errorString = QLatin1String("res_nquery returned an error");
        return result;
    }

    // Skip the query.
    char host[PACKETSZ], answer[PACKETSZ];
    unsigned char *p = response + sizeof(HEADER);
    int status = local_dn_expand(response, response + responseLength, p, host, sizeof(host));
    if (status < 0)
    {
        result.d->error = QXmppSrvInfo::UnknownError;
        result.d->errorString = QLatin1String("dn_expand failed");
        return result;
    }
    p += status + 4;

    // Extract results.
    answerIndex = 0;
    while ((p < response + responseLength) && (answerIndex < answerCount))
    {
        status = local_dn_expand(response, response + responseLength, p, host, sizeof(host));
        if (status < 0)
        {
            result.d->error = QXmppSrvInfo::UnknownError;
            result.d->errorString = QLatin1String("dn_expand failed");
            return result;
        }

        p += status;
        const int type = (p[0] << 8) | p[1];
        p += 2;
        //const int klass = (p[0] << 8) | p[1];
        p += 2;
        //const int ttl = (p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3];
        p += 4;
        const int size = (p[0] << 8) | p[1];
        p += 2;

        if (type == T_SRV)
        {
            quint16 priority = (p[0] << 8) | p[1];
            quint16 weight = (p[2] << 8) | p[3];
            quint16 port = (p[4] << 8) | p[5];
            status = local_dn_expand(response, response + responseLength, p + 6, answer, sizeof(answer));
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
    else
        sortSrvRecords(result.d->records);
    return result;
}

/// Performs a DNS lookup for an SRV entry. When the result of the lookup is
/// ready, the slot or signal \a member in \a receiver is called with a
/// QXmppSrvInfo argument.

void QXmppSrvInfo::lookupService(const QString &name, QObject *receiver, const char *member)
{
    qRegisterMetaType<QXmppSrvInfo>("QXmppSrvInfo");

    QXmppSrvInfoLookupManager *manager = theSrvInfoLookupManager();
    if (manager)
    {
        // the application is still alive
        QXmppSrvInfoLookupRunnable *runnable = new QXmppSrvInfoLookupRunnable(name);
        QObject::connect(runnable, SIGNAL(foundInfo(QXmppSrvInfo)), receiver, member);
        manager->start(runnable);
    }
}

QXmppSrvInfoLookupManager::QXmppSrvInfoLookupManager()
{
    moveToThread(QCoreApplication::instance()->thread());
    connect(QCoreApplication::instance(), SIGNAL(destroyed()),
            SLOT(waitForThreadPoolDone()), Qt::DirectConnection);
    setMaxThreadCount(5); // up to 5 parallel SRV lookups
}

void QXmppSrvInfoLookupRunnable::run()
{
    const QXmppSrvInfo result = QXmppSrvInfo::fromName(lookupName);
    emit foundInfo(result);
}

