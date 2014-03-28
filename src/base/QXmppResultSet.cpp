/*
 * Copyright (C) 2008-2014 The QXmpp developers
 *
 * Author:
 *  Olivier Goffart <ogoffart@woboq.com>
 *
 * Source:
 *  https://github.com/qxmpp-project/qxmpp
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


#include "QXmppConstants.h"
#include "QXmppResultSet.h"
#include "QXmppUtils.h"

#include <QDomElement>
#include <QDebug>

QXmppResultSetQuery::QXmppResultSetQuery()
    : m_index(-1)
    , m_max(-1)
{}

/// Returns the maximum number of results.
///
/// \note -1 means no limit, 0 means no results are wanted.
///

int QXmppResultSetQuery::max() const
{
    return m_max;
}

/// Sets the maximum number of results.
///
/// \note -1 means no limit, 0 means no results are wanted.

void QXmppResultSetQuery::setMax(int max)
{
    m_max = max;
}

/// Returns the index for the first element in the page.
///
/// This is used for retrieving pages out of order.

int QXmppResultSetQuery::index() const
{
    return m_index;
}

/// Sets the index for the first element in the page.
///
/// This is used for retrieving pages out of order.

void QXmppResultSetQuery::setIndex(int index)
{
    m_index=index;
}

/// Returns the UID of the first result in the next page.
///
/// This is used for for paging backwards through results.

QString QXmppResultSetQuery::before() const
{
    return m_before;
}

/// Sets the UID of the first result in the next page.
///
/// This is used for for paging backwards through results.

void QXmppResultSetQuery::setBefore(const QString& before)
{
    m_before=before;
}

/// Returns the UID of the last result in the previous page.
///
/// This is used for for paging forwards through results.

QString QXmppResultSetQuery::after() const
{
    return m_after;
}

/// Sets the UID of the last result in the previous page.
///
/// This is used for for paging forwards through results.

void QXmppResultSetQuery::setAfter(const QString& after)
{
    m_after=after;
}

/// Returns true if no result set information is present.

bool QXmppResultSetQuery::isNull() const
{
    return m_max == -1 && m_index == -1 && m_after.isNull() && m_before.isNull();
}

/// \cond
void QXmppResultSetQuery::parse(const QDomElement& element)
{
    QDomElement setElement = (element.tagName() == "set") ? element : element.firstChildElement("set");
    if (setElement.namespaceURI() == ns_rsm) {
        bool ok = false;
        m_max = setElement.firstChildElement("max").text().toInt(&ok);
        if (!ok) m_max = -1;
        m_after = setElement.firstChildElement("after").text();
        m_before = setElement.firstChildElement("before").text();
        m_index = setElement.firstChildElement("index").text().toInt(&ok);
        if (!ok) m_index = -1;
    }
}

void QXmppResultSetQuery::toXml(QXmlStreamWriter* writer) const
{
    if (isNull())
        return;
    writer->writeStartElement("set");
    writer->writeAttribute("xmlns", ns_rsm);
    if (m_max >= 0)
        helperToXmlAddTextElement(writer, "max", QString::number(m_max));
    if (!m_after.isNull())
        helperToXmlAddTextElement(writer, "after", m_after);
    if (!m_before.isNull())
        helperToXmlAddTextElement(writer, "before", m_before);
    if (m_index >= 0)
        helperToXmlAddTextElement(writer, "index", QString::number(m_index));
    writer->writeEndElement();
}
/// \endcond

QXmppResultSetReply::QXmppResultSetReply()
    : m_count(-1)
    , m_index(-1)
{}

/// Returns the UID of the first result in the page.

QString QXmppResultSetReply::first() const
{
    return m_first;
}

/// Sets the UID of the first result in the page.

void QXmppResultSetReply::setFirst(const QString& first)
{
    m_first=first;
}

/// Returns the UID of the last result in the page.

QString QXmppResultSetReply::last() const
{
    return m_last;
}

/// Sets the UID of the last result in the page.

void QXmppResultSetReply::setLast(const QString& last)
{
    m_last=last;
}

/// Returns the total number of items in the set.
///
/// \note This may be an approximate count.

int QXmppResultSetReply::count() const
{
    return m_count;
}

/// Sets the total number of items in the set.
///
/// \note This may be an approximate count.

void QXmppResultSetReply::setCount(int count)
{
    m_count = count;
}

/// Returns the index for the first result in the page.
///
/// This is used for retrieving pages out of order.
///
/// \note This may be an approximate index.

int QXmppResultSetReply::index() const
{
    return m_index;
}

/// Sets the index for the first result in the page.
///
/// This is used for retrieving pages out of order.
///
/// \note This may be an approximate index.

void QXmppResultSetReply::setIndex(int index)
{
    m_index = index;
}

/// Returns true if no result set information is present.

bool QXmppResultSetReply::isNull() const
{
    return m_count == -1 && m_index == -1 && m_first.isNull() && m_last.isNull();
}

/// \cond
void QXmppResultSetReply::parse(const QDomElement& element)
{
    QDomElement setElement = (element.tagName() == "set") ? element : element.firstChildElement("set");
    if (setElement.namespaceURI() == ns_rsm) {
        m_count = setElement.firstChildElement("count").text().toInt();
        QDomElement firstElem = setElement.firstChildElement("first");
        m_first = firstElem.text();
        bool ok = false;
        m_index = firstElem.attribute("index").toInt(&ok);
        if(!ok) m_index = -1;
        m_last = setElement.firstChildElement("last").text();
    }
}

void QXmppResultSetReply::toXml(QXmlStreamWriter* writer) const
{
    if (isNull())
        return;
    writer->writeStartElement("set");
    writer->writeAttribute("xmlns", ns_rsm);
    if (!m_first.isNull() || m_index >= 0) {
        writer->writeStartElement("first");
        if (m_index >= 0)
            writer->writeAttribute("index", QString::number(m_index));
        writer->writeCharacters(m_first);
        writer->writeEndElement();
    }
    if (!m_last.isNull())
        helperToXmlAddTextElement(writer, "last", m_last);

    if (m_count >= 0)
        helperToXmlAddTextElement(writer, "count", QString::number(m_count));
    writer->writeEndElement();
}
/// \endcond
