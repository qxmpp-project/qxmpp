/*
 * Copyright (C) 2008-2012 The QXmpp developers
 *
 * Author:
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

#include "QXmppSimpleArchivePreferenceIq.h"
#include "QXmppConstants.h"
#include "QXmppUtils.h"

static const char * PREFERENCE_NAMES[] =
{
    "always",
    "never",
    "roster"
};

QXmppSimpleArchivePreferenceIq::QXmppSimpleArchivePreferenceIq(const QXmppArchivePreference& def)
    : m_default(def)
{
}

bool QXmppSimpleArchivePreferenceIq::isSimpleArchivePreferenceIq(const QDomElement &element)
{
    QDomElement listElement = element.firstChildElement("prefs");
    return (listElement.namespaceURI() == ns_simple_archive);
}

QXmppSimpleArchivePreferenceIq::QXmppArchivePreference QXmppSimpleArchivePreferenceIq::archiveDefault() const
{
    return m_default;
}
void QXmppSimpleArchivePreferenceIq::setArchiveDefault(const QXmppArchivePreference& def)
{
    m_default = def;
}

QList<QString> QXmppSimpleArchivePreferenceIq::alwaysArchive() const
{
    return m_always;
}
void QXmppSimpleArchivePreferenceIq::addAlwaysArchive(const QString& jid)
{
    m_always.push_back(jid);
}

QList<QString> QXmppSimpleArchivePreferenceIq::neverArchive() const
{
    return m_never;
}
void QXmppSimpleArchivePreferenceIq::addNeverArchive(const QString& jid)
{
    m_never.push_back(jid);
}

void QXmppSimpleArchivePreferenceIq::parseElementFromChild(const QDomElement &element)
{
    QDomElement queryElement = element.firstChildElement("prefs");
    const QString def = queryElement.attribute(QString("default"), QString("always"));
    for (int i = Always; i <= Roster; i++)
    {
        if (def == QString(PREFERENCE_NAMES[i])) {
            m_default = (QXmppArchivePreference)i;
            break;
        }
    }

    QDomElement listElement = queryElement.firstChildElement("always");
    QDomElement child = listElement.firstChildElement();
    while (!child.isNull())
    {
        if (child.tagName() == "jid")
        {
            m_always << child.text();
        }
        child = child.nextSiblingElement();
    }

    listElement = queryElement.firstChildElement("never");
    child = listElement.firstChildElement();
    while (!child.isNull())
    {
        if (child.tagName() == "jid")
        {
            m_never << child.text();
        }
        child = child.nextSiblingElement();
    }
}

void QXmppSimpleArchivePreferenceIq::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    writer->writeStartElement("prefs");
    writer->writeAttribute("xmlns", ns_simple_archive);
    writer->writeAttribute("default", PREFERENCE_NAMES[m_default]);
    writer->writeStartElement("always");
    Q_FOREACH(const QString jid, m_always)
    {
        helperToXmlAddTextElement(writer, "jid", jid);
    }
    writer->writeEndElement(); // always
    writer->writeStartElement("never");
    Q_FOREACH(const QString jid, m_never)
    {
        helperToXmlAddTextElement(writer, "jid", jid);
    }
    writer->writeEndElement(); // enever
    writer->writeEndElement();
}
