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
#include "QXmppConstants.h"
#include "QXmppUtils.h"

/// Constructs a QXmppArchiveListIq.

QXmppSimpleArchiveQueryIq::QXmppSimpleArchiveQueryIq()
    : QXmppIq(QXmppIq::Get)
{
}

/// Returns the JID which archived messages must match.
///

QString QXmppSimpleArchiveQueryIq::with() const
{
    return m_with;
}

/// Sets the JID which archived messages must match.
///
/// \param with

void QXmppSimpleArchiveQueryIq::setWith(const QString &with)
{
    m_with = with;
}

/// Returns the query ID
///

QString QXmppSimpleArchiveQueryIq::queryId() const
{
    return m_queryId;
}

/// Sets the query ID 
///
/// \param queryId

void QXmppSimpleArchiveQueryIq::setQueryId(const QString &queryId)
{
    m_queryId = queryId;
}

/// Returns the start date/time for the archived messages.
///

QDateTime QXmppSimpleArchiveQueryIq::start() const
{
    return m_start;
}

/// Sets the start date/time for the archived messages.
///
/// \param start

void QXmppSimpleArchiveQueryIq::setStart(const QDateTime &start)
{
    m_start = start;
}

/// Returns the end date/time for the archived messages.
///

QDateTime QXmppSimpleArchiveQueryIq::end() const
{
    return m_end;
}

/// Sets the end date/time for the archived messages.
///
/// \param end

void QXmppSimpleArchiveQueryIq::setEnd(const QDateTime &end)
{
    m_end = end;
}

/// Returns the result set management query.
///
/// This is used for paging through messsages.

QXmppResultSetQuery QXmppSimpleArchiveQueryIq::resultSetQuery() const
{
    return m_rsmQuery;
}

/// Sets the result set management query.
///
/// This is used for paging through messages.

void QXmppSimpleArchiveQueryIq::setResultSetQuery(const QXmppResultSetQuery& rsm)
{
    m_rsmQuery = rsm;
}

/// Returns the result set management reply.
///
/// This is used for paging through messages.

QXmppResultSetReply QXmppSimpleArchiveQueryIq::resultSetReply() const
{
    return m_rsmReply;
}

/// Sets the result set management reply.
///
/// This is used for paging through messages.

void QXmppSimpleArchiveQueryIq::setResultSetReply(const QXmppResultSetReply& rsm)
{
    m_rsmReply = rsm;
}

/// \cond
bool QXmppSimpleArchiveQueryIq::isSimpleArchiveQueryIq(const QDomElement &element)
{
    QDomElement listElement = element.firstChildElement("query");
    return (listElement.namespaceURI() == ns_simple_archive);
}

void QXmppSimpleArchiveQueryIq::parseElementFromChild(const QDomElement &element)
{
    QDomElement queryElement = element.firstChildElement("query");    
    m_with = queryElement.firstChildElement("with").text();
    m_queryId = queryElement.attribute("queryid");
    m_start = QXmppUtils::datetimeFromString(queryElement.firstChildElement("start").text());
    m_end = QXmppUtils::datetimeFromString(queryElement.firstChildElement("end").text());
    m_rsmQuery.parse(queryElement);
    m_rsmReply.parse(queryElement);
}

void QXmppSimpleArchiveQueryIq::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    writer->writeStartElement("query");
    writer->writeAttribute("xmlns", ns_simple_archive);
    if (!m_queryId.isEmpty())
        helperToXmlAddAttribute(writer, "queryid", m_queryId);
    if (!m_with.isEmpty())
        helperToXmlAddTextElement(writer, "with", m_with);
    if (m_start.isValid())
        helperToXmlAddTextElement(writer, "start", QXmppUtils::datetimeToString(m_start));
    if (m_end.isValid())
        helperToXmlAddTextElement(writer, "end", QXmppUtils::datetimeToString(m_end));
    if (!m_rsmQuery.isNull())
        m_rsmQuery.toXml(writer);
    else if (!m_rsmReply.isNull())
        m_rsmReply.toXml(writer);
    writer->writeEndElement();
}

/// \endcond
