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

#include <QDebug>

#include "QXmppServiceInfo.h"

#ifdef Q_OS_WIN
#include <windows.h>
#include <windns.h>
#else
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/nameser.h>
#include <arpa/nameser_compat.h>
#include <resolv.h>
#endif

/// Constructs an empty service record object.
///

QXmppServiceRecord::QXmppServiceRecord()
    : host_port(0)
{
}

/// Returns host name for this service record.
///

QString QXmppServiceRecord::hostName() const
{
    return host_name;
}

/// Sets the host name for this service record.
///
/// \param hostName

void QXmppServiceRecord::setHostName(const QString &hostName)
{
    host_name = hostName;
}

/// Returns the port for this service record.
///

quint16 QXmppServiceRecord::port() const
{
    return host_port;
}

/// Sets the port for this service record.
///
/// \param port

void QXmppServiceRecord::setPort(quint16 port)
{
    host_port = port;
}

/// If the lookup failed, this function returns a human readable description of the error.
///

QString QXmppServiceInfo::errorString() const
{
    return m_errorString;
}

/// Returns the list of records associated with this service.
///
QList<QXmppServiceRecord> QXmppServiceInfo::records() const
{
    return m_records;
}

/// Perform a DNS lookup for an SRV entry.
///
/// Returns QXmppServiceInfo object with records
///
/// \param dname

QXmppServiceInfo QXmppServiceInfo::fromName(const QString &dname)
{
    QXmppServiceInfo result;

#ifdef Q_OS_WIN
    PDNS_RECORD records, ptr;

    /* perform DNS query */
    if (DnsQuery_UTF8(dname.toUtf8(), DNS_TYPE_SRV, DNS_QUERY_STANDARD, NULL, &records, NULL) != ERROR_SUCCESS)
    {
        result.m_errorString = QLatin1String("DnsQuery_UTF8 failed");
        return result;
    }

    /* extract results */
    for (ptr = records; ptr != NULL; ptr = ptr->pNext)
    {
        if ((ptr->wType == DNS_TYPE_SRV) && !strcmp((char*)ptr->pName, dname.toUtf8()))
        {
            QXmppServiceRecord record;
            record.setHostName(QString::fromUtf8((char*)ptr->Data.Srv.pNameTarget));
            record.setPort(ptr->Data.Srv.wPort);
            result.m_records.append(record);
        }
    }

    DnsRecordListFree(records, DnsFreeRecordList);
#else
    unsigned char response[PACKETSZ];
    int responseLength, answerCount, answerIndex;

    /* explicitly call res_init in case config changed */
    res_init();

    /* perform DNS query */
    memset(response, 0, sizeof(response));
    responseLength = res_query(dname.toAscii(), C_IN, T_SRV, response, sizeof(response));
    if (responseLength < int(sizeof(HEADER)))
    {
        result.m_errorString = QString("res_query failed: %1").arg(hstrerror(h_errno));
        return result;
    }

    /* check the response header */
    HEADER *header = (HEADER*)response;
    if (header->rcode != NOERROR || !(answerCount = ntohs(header->ancount)))
    {
        result.m_errorString = QLatin1String("res_query returned an error");
        return result;
    }

    /* skip the query */
    char host[PACKETSZ], answer[PACKETSZ];
    unsigned char *p = response + sizeof(HEADER);
    int status = dn_expand(response, response + responseLength, p, host, sizeof(host));
    if (status < 0)
    {
        result.m_errorString = QLatin1String("dn_expand failed");
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
            result.m_errorString = QLatin1String("dn_expand failed");
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
            quint16 port = (p[4] << 8) | p[5];
            status = dn_expand(response, response + responseLength, p + 6, answer, sizeof(answer));
            if (status < 0)
            {
                result.m_errorString = QLatin1String("dn_expand failed");
                return result;
            }
            QXmppServiceRecord record;
            record.setHostName(answer);
            record.setPort(port);
            result.m_records.append(record);
        } else {
            qWarning("Unexpected DNS answer type");
        }
        p += size;
        answerIndex++;
    }
#endif
    return result;
}
