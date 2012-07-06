/*
 * Copyright (C) 2008-2011 The QXmpp developers
 *
 * Author:
 *  Olivier Goffart <ogoffart@woboq.com>
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


#include "QXmppResultSet.h"
#include "QXmppUtils.h"

#include <QDomElement>
#include <QDebug>

static const char *ns_rsm = "http://jabber.org/protocol/rsm";

QXmppResultSetQuery::QXmppResultSetQuery()
    : m_index(-1), m_max(-1)
{}

int QXmppResultSetQuery::max() const
{
    return m_max;
}

void QXmppResultSetQuery::setMax(int max)
{
    m_max = max;
}

int QXmppResultSetQuery::index() const
{
    return m_index;
}

void QXmppResultSetQuery::setIndex(int index)
{
    m_index=index;
}

QString QXmppResultSetQuery::before() const
{
    return m_before;
}

void QXmppResultSetQuery::setBefore(const QString& before)
{
    m_before=before;
}

QString QXmppResultSetQuery::after() const
{
    return m_after;
}

void QXmppResultSetQuery::setAfter(const QString& after)
{
    m_after=after;
}

bool QXmppResultSetQuery::isNull() const
{
    return m_max == -1 && m_index == -1 && m_after.isNull() && m_before.isNull();
}

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



QXmppResultSetReply::QXmppResultSetReply()
    : m_count(-1), m_index(-1)
{}

QString QXmppResultSetReply::first() const
{
    return m_first;
}

void QXmppResultSetReply::setFirst(const QString& first)
{
    m_first=first;
}

QString QXmppResultSetReply::last() const
{
    return m_last;
}

void QXmppResultSetReply::setLast(const QString& last)
{
    m_last=last;
}

int QXmppResultSetReply::count() const
{
    return m_count;
}

void QXmppResultSetReply::setCount(int count)
{
    m_count = count;
}

int QXmppResultSetReply::index() const
{
    return m_index;
}

void QXmppResultSetReply::setIndex(int index)
{
    m_index=index;
}

bool QXmppResultSetReply::isNull() const
{
    return m_count == -1 && m_index == -1 && m_first.isNull() && m_last.isNull();
}


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





