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

#ifndef QXMPPELEMENT_H
#define QXMPPELEMENT_H

#include <QMap>
#include <QStringList>
#include <QXmlStreamWriter>

class QDomElement;

class QXmppElement
{
public:
    QXmppElement();
    QXmppElement(const QDomElement &element);

    QStringList attributeNames() const;

    QString attribute(const QString &name) const;
    void setAttribute(const QString &name, const QString &value);

    QList<QXmppElement> children() const;
    void setChildren(QList<QXmppElement> &children);

    QString tagName() const;
    void setTagName(const QString &type);

    QString value() const;
    void setValue(const QString &text);

    bool isNull() const;
    void toXml(QXmlStreamWriter *writer) const;

private:
    QMap<QString, QString> m_attributes;
    QList<QXmppElement> m_children;
    QString m_tagName;
    QString m_value;
};

#endif
