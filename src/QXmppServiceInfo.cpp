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

/// Constructs an empty service info object.
///

QXmppServiceInfo::QXmppServiceInfo()
    : host_port(0)
{
}

/// Returns host name for this service.
///

QString QXmppServiceInfo::hostName() const
{
    return host_name;
}

/// Sets the host name for this service.
///
/// \param hostName

void QXmppServiceInfo::setHostName(const QString &hostName)
{
    host_name = hostName;
}

/// Returns the port for this service.
///

quint16 QXmppServiceInfo::port() const
{
    return host_port;
}

/// Sets the port for this service.
///
/// \param port

void QXmppServiceInfo::setPort(quint16 port)
{
    host_port = port;
}

/// Perform a DNS lookup for an SRV entry.
///
/// Returns true if the lookup was succesful, false if it failed.
///
/// \param dname
/// \param results

bool QXmppServiceInfo::lookupService(const QString &dname, QList<QXmppServiceInfo> &results)
{
#ifdef Q_OS_WIN
    PDNS_RECORD records, ptr;

    /* perform DNS query */
    if (DnsQuery_UTF8(dname.toAscii(), DNS_TYPE_SRV, DNS_QUERY_STANDARD, NULL, &records, NULL) != ERROR_SUCCESS)
    {
        qWarning() << "SRV lookup for" << dname << "failed";
        return false;
    }

    /* extract results */
    for (ptr = records; ptr != NULL; ptr = ptr->pNext)
    {
        if ((ptr->wType == DNS_TYPE_SRV) && !strcmp(ptr->pName, dname.toAscii()))
        {
            QXmppServiceInfo info;
            info.setHostName(ptr->Data.Srv.pNameTarget);
            info.setPort(ptr->Data.Srv.wPort);
            results.append(info);
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
        qWarning() << "SRV lookup for" << dname << "failed";
        herror("SRV lookup error");
        return false;
    }

    /* check the response header */
    HEADER *header = (HEADER*)response;
    if (header->rcode != NOERROR || !(answerCount = ntohs(header->ancount)))
    {
        qWarning() << "SRV lookup for" << dname << "returned an error";
        return false;
    }

    /* skip the query */
    char host[PACKETSZ], answer[PACKETSZ];
    unsigned char *p = response + sizeof(HEADER);
    int status = dn_expand(response, response + responseLength, p, host, sizeof(host));
    if (status < 0)
    {
        qWarning("dn_expand failed");
        return false;
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
            qWarning("dn_expand failed");
            return false;
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
                qWarning("dn_expand failed");
                return false;
            }
            QXmppServiceInfo info;
            info.setHostName(answer);
            info.setPort(port);
            results.append(info);
        } else {
            qWarning("Unexpected DNS answer type");
        }
        p += size;
        answerIndex++;
    }
#endif
    return !results.isEmpty();
}
