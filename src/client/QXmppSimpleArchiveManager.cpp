/*
 * Copyright (C) 2008-2012 The QXmpp developers
 *
 * Author:
 *  James Turner (james.turner@kdab.com)
 *  Truphone Labs (labs@truphone.com)
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

#include <QDomElement>

#include "QXmppSimpleArchiveIq.h"
#include "QXmppSimpleArchiveManager.h"
#include "QXmppClient.h"
#include "QXmppConstants.h"

/// \cond
QStringList QXmppSimpleArchiveManager::discoveryFeatures() const
{
    // XEP-0313: Message Archive Management
    return QStringList() << ns_simple_archive;
}

bool QXmppSimpleArchiveManager::handleStanza(const QDomElement &element)
{
    bool isIq = (element.tagName() == "iq");
    if (!isIq && (element.tagName() != "message"))
        return false;

    // XEP-0313: Message Archiving
    if(isIq && !QXmppSimpleArchiveQueryIq::isSimpleArchiveQueryIq(element))
        return false;

    if (isIq && (element.attribute("type") == "result")) {
        QString id = element.attribute("id");
        if (m_pendingQueries.contains(id)) {
            PendingQuery pendingQuery = m_pendingQueries.value(id);
            m_pendingQueries.remove(id);
            emit archiveMessagesReceived(pendingQuery.jid, pendingQuery.messages, QXmppResultSetReply());
            return true;
        }
    } else if (isIq && (element.attribute("type") == "error")) {
        QDomElement queryElement = element.firstChildElement("query");
        if (!queryElement.isNull()) {
            QString queryId = queryElement.attribute("queryid");
            if (m_pendingQueries.contains(queryId)) {
              PendingQuery pendingQuery = m_pendingQueries.value(queryId);
              m_pendingQueries.remove(queryId);
              emit archiveMessagesReceived(pendingQuery.jid, pendingQuery.messages, QXmppResultSetReply());
              return true;
            }
        }
    } else {
        // message, check for result UUID + query ID
        QDomElement resultElement = element.firstChildElement("result");
        if (!resultElement.isNull()) {
            QString queryId = resultElement.attribute("queryid");
            if (m_pendingQueries.contains(queryId)) {
                QXmppMessage msg;
                msg.parse(element);
                m_pendingQueries[queryId].messages.append(msg.forwarded());
                
                return true;
            } else {
                qWarning() << "SimpleArchiveManager: unknown query ID:" << queryId;
            }
        }
    }

    return false;
}
/// \endcond

/// Retrieves the specified collection. Once the results are received,
/// the archiveMessagesReceived() will be emitted.
///
/// \param jid The JID of the collection
/// \param start The start time of messages to retrieve.
/// \param end The end time of messages to retrieve.
/// \param rsm Optional Result Set Management query
///
void QXmppSimpleArchiveManager::retrieveMessages(const QString &jid, const QDateTime &start, const QDateTime &end, const QXmppResultSetQuery &rsm)
{
    QXmppSimpleArchiveQueryIq packet;
    packet.setResultSetQuery(rsm);
    packet.setStart(start);
    packet.setEnd(end);
    packet.setWith(jid);
    
    // set query ID
    QString queryId = "query_" + jid;
    while (m_pendingQueries.contains(queryId)) {
        queryId.append("_x");
    }
    
    PendingQuery result; // empty results for now
    result.jid = jid;
    m_pendingQueries[queryId] = result;
    packet.setQueryId(queryId);
    packet.setId(queryId);
    
    client()->sendPacket(packet);
}
