/*
 * Copyright (C) 2010 Bolloré telecom
 *
 * Author:
 *	Jeremy Lainé
 *
 * Source:
 *	http://code.google.com/p/qxmpp
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

#include "QXmppElement.h"
#include "QXmppUtils.h"

#include <QDomElement>

QXmppElement::QXmppElement()
{
}

QXmppElement::QXmppElement(const QDomElement &element)
{
    if (element.isNull())
        return;

    m_tagName = element.tagName();
    QString xmlns = element.namespaceURI();
    QString parentns = element.parentNode().namespaceURI();
    if (!xmlns.isEmpty() && xmlns != parentns)
        m_attributes.insert("xmlns", xmlns);
    QDomNamedNodeMap attributes = element.attributes();
    for (int i = 0; i < attributes.size(); i++)
    {
        QDomAttr attr = attributes.item(i).toAttr();
        m_attributes.insert(attr.name(), attr.value());
    }

    QDomNode childNode = element.firstChild();
    while (!childNode.isNull())
    {
        if (childNode.isElement())
            m_children.append(QXmppElement(childNode.toElement()));
        else if (childNode.isText())
            m_value += childNode.toText().data();
        childNode = childNode.nextSibling();
    }
}

QStringList QXmppElement::attributeNames() const
{
    return m_attributes.keys();
}

QString QXmppElement::attribute(const QString &name) const
{
    return m_attributes.value(name);
}

void QXmppElement::setAttribute(const QString &name, const QString &value)
{
    m_attributes.insert(name, value);
}

QXmppElementList QXmppElement::children() const
{
    return m_children;
}

void QXmppElement::setChildren(const QXmppElementList &children)
{
    m_children = children;
}

QXmppElement QXmppElement::firstChild(const QString &name) const
{
    foreach (const QXmppElement &child, m_children)
        if (child.tagName() == name)
            return child;
    return QXmppElement();
}

bool QXmppElement::isNull() const
{
    return m_tagName.isEmpty();
}

QString QXmppElement::tagName() const
{
    return m_tagName;
}

void QXmppElement::setTagName(const QString &tagName)
{
    m_tagName = tagName;
}

QString QXmppElement::value() const
{
    return m_value;
}

void QXmppElement::setValue(const QString &value)
{
    m_value = value;
}

void QXmppElement::toXml(QXmlStreamWriter *writer) const
{
    if (isNull())
        return;

    writer->writeStartElement(m_tagName);
    foreach (const QString &attr, m_attributes.keys())
        helperToXmlAddAttribute(writer, attr, m_attributes.value(attr));
    writer->writeCharacters(m_value);
    foreach (const QXmppElement &child, m_children)
        child.toXml(writer);
    writer->writeEndElement();
}

QXmppElementList::QXmppElementList()
{
}

QXmppElementList::QXmppElementList(const QXmppElement &element)
{
    append(element);
}


QXmppElementList::QXmppElementList(const QList<QXmppElement> &other)
    : QList<QXmppElement>(other)
{
}

