/*
 * Copyright (C) 2008-2014 The QXmpp developers
 *
 * Author:
 *  Jeremy Lainé
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

#include "QXmppElement.h"
#include "QXmppUtils.h"

#include <QDomElement>
#include <QTextStream>

class QXmppElementPrivate
{
public:
    QXmppElementPrivate();
    QXmppElementPrivate(const QDomElement &element);
    ~QXmppElementPrivate();

    QAtomicInt counter;

    QXmppElementPrivate *parent;
    QMap<QString, QString> attributes;
    QList<QXmppElementPrivate*> children;
    QString name;
    QString value;

    QByteArray serializedSource;
};

QXmppElementPrivate::QXmppElementPrivate()
    : counter(1), parent(NULL)
{
}

QXmppElementPrivate::QXmppElementPrivate(const QDomElement &element)
    : counter(1), parent(NULL)
{
    if (element.isNull())
        return;

    name = element.tagName();
    QString xmlns = element.namespaceURI();
    QString parentns = element.parentNode().namespaceURI();
    if (!xmlns.isEmpty() && xmlns != parentns)
        attributes.insert("xmlns", xmlns);
    QDomNamedNodeMap attrs = element.attributes();
    for (int i = 0; i < attrs.size(); i++)
    {
        QDomAttr attr = attrs.item(i).toAttr();
        attributes.insert(attr.name(), attr.value());
    }

    QDomNode childNode = element.firstChild();
    while (!childNode.isNull())
    {
        if (childNode.isElement())
        {
            QXmppElementPrivate *child = new QXmppElementPrivate(childNode.toElement());
            child->parent = this;
            children.append(child);
        } else if (childNode.isText()) {
            value += childNode.toText().data();
        }
        childNode = childNode.nextSibling();
    }

    QTextStream stream(&serializedSource);
    element.save(stream, 0);
}

QXmppElementPrivate::~QXmppElementPrivate()
{
    foreach (QXmppElementPrivate *child, children)
        if (!child->counter.deref())
            delete child;
}

QXmppElement::QXmppElement()
{
    d = new QXmppElementPrivate();
}

QXmppElement::QXmppElement(const QXmppElement &other)
{
    other.d->counter.ref();
    d = other.d;
}

QXmppElement::QXmppElement(QXmppElementPrivate *other)
{
    other->counter.ref();
    d = other;
}

QXmppElement::QXmppElement(const QDomElement &element)
{
    d = new QXmppElementPrivate(element);
}

QXmppElement::~QXmppElement()
{
    if (!d->counter.deref())
        delete d;
}

QXmppElement &QXmppElement::operator=(const QXmppElement &other)
{
    if (this != &other) // self-assignment check
    {
        other.d->counter.ref();
        if (!d->counter.deref())
            delete d;
        d = other.d;
    }
    return *this;
}

QDomElement QXmppElement::sourceDomElement() const
{
    if (d->serializedSource.isEmpty())
        return QDomElement();

    QDomDocument doc;
    if (!doc.setContent(d->serializedSource, true))
    {
        qWarning("[QXmpp] QXmppElement::sourceDomElement(): cannot parse source element");
        return QDomElement();
    }

    return doc.documentElement();
}

QStringList QXmppElement::attributeNames() const
{
    return d->attributes.keys();
}

QString QXmppElement::attribute(const QString &name) const
{
    return d->attributes.value(name);
}

void QXmppElement::setAttribute(const QString &name, const QString &value)
{
    d->attributes.insert(name, value);
}

void QXmppElement::appendChild(const QXmppElement &child)
{
    if (child.d->parent == d)
        return;

    if (child.d->parent)
        child.d->parent->children.removeAll(child.d);
    else
        child.d->counter.ref();
    child.d->parent = d;
    d->children.append(child.d);
}

QXmppElement QXmppElement::firstChildElement(const QString &name) const
{
    foreach (QXmppElementPrivate *child_d, d->children)
        if (name.isEmpty() || child_d->name == name)
            return QXmppElement(child_d);
    return QXmppElement();
}

QXmppElement QXmppElement::nextSiblingElement(const QString &name) const
{
    if (!d->parent)
        return QXmppElement();
    const QList<QXmppElementPrivate*> &siblings_d = d->parent->children;
    for (int i = siblings_d.indexOf(d) + 1; i < siblings_d.size(); i++)
        if (name.isEmpty() || siblings_d[i]->name == name)
            return QXmppElement(siblings_d[i]);
    return QXmppElement();
}

bool QXmppElement::isNull() const
{
    return d->name.isEmpty();
}

void QXmppElement::removeChild(const QXmppElement &child)
{
    if (child.d->parent != d)
        return;

    d->children.removeAll(child.d);
    child.d->counter.deref();
    child.d->parent = NULL;
}

QString QXmppElement::tagName() const
{
    return d->name;
}

void QXmppElement::setTagName(const QString &tagName)
{
    d->name = tagName;
}

QString QXmppElement::value() const
{
    return d->value;
}

void QXmppElement::setValue(const QString &value)
{
    d->value = value;
}

void QXmppElement::toXml(QXmlStreamWriter *writer) const
{
    if (isNull())
        return;

    writer->writeStartElement(d->name);
    if (d->attributes.contains("xmlns"))
        writer->writeAttribute("xmlns", d->attributes.value("xmlns"));
    foreach (const QString &attr, d->attributes.keys())
        if (attr != "xmlns")
            helperToXmlAddAttribute(writer, attr, d->attributes.value(attr));
    if (!d->value.isEmpty())
        writer->writeCharacters(d->value);
    foreach (const QXmppElement &child, d->children)
        child.toXml(writer);
    writer->writeEndElement();
}
