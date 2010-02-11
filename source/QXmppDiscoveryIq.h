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

#ifndef QXMPPDISCOVERY_H
#define QXMPPDISCOVERY_H

#include "QXmppIq.h"

#include <QMap>
#include <QStringList>

class QDomElement;

class QXmppDiscoveryItem
{
public:
    QStringList attributes() const;

    QString attribute(const QString &name) const;
    void setAttribute(const QString &name, const QString &value);

    QString type() const;
    void setType(const QString &type);

private:
    QMap<QString, QString> m_attributes;
    QString m_type;
};

class QXmppDiscoveryIq : public QXmppIq
{
public:
    enum QueryType {
        InfoQuery,
        ItemsQuery,
    };

    void toXmlElementFromChild(QXmlStreamWriter *writer) const;
    void parse( QDomElement &element );
    static bool isDiscoveryIq( QDomElement &element );

    QList<QXmppDiscoveryItem> getItems() const;
    void setItems(const QList<QXmppDiscoveryItem> &items);

    enum QueryType getQueryType() const;
    void setQueryType(enum QueryType type);

private:
    QList<QXmppDiscoveryItem> m_items;
    enum QueryType m_queryType;
};

#endif
