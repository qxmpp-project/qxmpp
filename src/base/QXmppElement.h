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

#ifndef QXMPPELEMENT_H
#define QXMPPELEMENT_H

#include <QMap>
#include <QStringList>
#include <QXmlStreamWriter>

#include "QXmppGlobal.h"

class QDomElement;
class QXmppElement;
class QXmppElementPrivate;

typedef QList<QXmppElement> QXmppElementList;
class QXMPP_EXPORT QXmppElement
{
public:
    QXmppElement();
    QXmppElement(const QXmppElement &other);
    QXmppElement(const QDomElement &element);
    ~QXmppElement();

    QDomElement sourceDomElement() const;

    QStringList attributeNames() const;

    QString attribute(const QString &name) const;
    void setAttribute(const QString &name, const QString &value);

    void appendChild(const QXmppElement &child);
    QXmppElement firstChildElement(const QString &name = QString()) const;
    QXmppElement nextSiblingElement(const QString &name = QString()) const;
    void removeChild(const QXmppElement &child);

    QString tagName() const;
    void setTagName(const QString &type);

    QString value() const;
    void setValue(const QString &text);

    bool isNull() const;
    void toXml(QXmlStreamWriter *writer) const;

    QXmppElement &operator=(const QXmppElement &other);

private:
    QXmppElement(QXmppElementPrivate *other);
    QXmppElementPrivate *d;
};

#endif
