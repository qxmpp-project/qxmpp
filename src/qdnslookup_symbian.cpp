/****************************************************************************
**
** Copyright (C) 2012 Jeremy Lainé <jeremy.laine@m4x.org>
** Contact: http://www.qt-project.org/
**
** This file is part of the QtNetwork module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qdnslookup_p.h"

#include <QUrl>
#include <QMutex>
#include <QLibrary>

#include <dns_qry.h>

QT_BEGIN_NAMESPACE

static void resolveLibrary()
{
    QLibrary lib(QLatin1String("dnsapi"));
    if (!lib.load())
        return;

    local_dns_query_utf8 = (dns_query_utf8_proto) lib.resolve(QLatin1String("dnsapi"), "DnsQuery_UTF8");
    local_dns_record_list_free = (dns_record_list_free_proto) lib.resolve(QLatin1String("dnsapi"), "DnsRecordListFree");
}

void QDnsLookupRunnable::query(const int requestType, const QByteArray &requestName, QDnsLookupReply *reply)
{
    RHostResolver dnsResolver;
    RSocketServ dnsSocket;

    // Initialise resolver.
    TInt err = dnsSocket.Connect();
    err = dnsResolver.Open(dnsSocket, KAfInet, KProtocolInetUdp);
    if (err != KErrNone) {
        reply->error = QXmppSrvInfo::UnknownError;
        reply->errorString = QLatin1String("RHostResolver::Open failed");
        return;
    }

    // Perform DNS query.
    TDnsQueryBuf dnsQuery;
    TDnsRespSRVBuf dnsResponse;
    dnsQuery().SetClass(KDnsRRClassIN);
    TPtrC8 queryPtr(reinterpret_cast<const TUint8*>(requestName.constData()), requestNname.size());
    dnsQuery().SetData(queryPtr);
    dnsQuery().SetType(KDnsRRTypeSRV);
    err = dnsResolver.Query(dnsQuery, dnsResponse);
    if (err != KErrNone) {
        error = QXmppSrvInfo::NotFoundError;
        errorString = QLatin1String("RHostResolver::Query failed");
        return;
    }

    // Extract results.
    while (err == KErrNone) {
        const QByteArray aceName((const char*)dnsResponse().Target().Ptr(),
                                 dnsResponse().Target().Length());

        QDnsServiceRecord record;
        record.d->name = name;
        record.d->target = QUrl::fromAce(aceName);
        record.d->port = dnsResponse().Port();
        record.d->priority = dnsResponse().Priority();
        //record.d->timeToLive = 0;
        record.d->weight = dnsResponse().Weight();
        reply->serviceRecords.append(record);

        err = dnsResolver.QueryGetNext(dnsResponse);
    }
}

QT_END_NAMESPACE
