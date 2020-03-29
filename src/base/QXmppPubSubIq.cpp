/*
 * Copyright (C) 2008-2020 The QXmpp developers
 *
 * Author:
 *  Jeremy Lainé
 *  Germán Márquez Mejía
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

#include "QXmppPubSubIq.h"

#include "QXmppConstants_p.h"
#include "QXmppUtils.h"

#include <QDomElement>
#include <QSharedData>

static const QStringList PUBSUB_QUERIES = {
    QStringLiteral("affiliations"),
    QStringLiteral("default"),
    QStringLiteral("items"),
    QStringLiteral("publish"),
    QStringLiteral("retract"),
    QStringLiteral("subscribe"),
    QStringLiteral("subscription"),
    QStringLiteral("subscriptions"),
    QStringLiteral("unsubscribe"),
    QStringLiteral("create"),
    QStringLiteral("delete"),
};

class QXmppPubSubIqPrivate : public QSharedData
{
public:
    QXmppPubSubIqPrivate();

    QXmppPubSubIq::QueryType queryType;
    QString queryJid;
    QString queryNodeName;
    QList<QXmppPubSubItem> items;
    QString subscriptionId;
    QString subscriptionType;
    QXmppDataForm publishOptions;
};

QXmppPubSubIqPrivate::QXmppPubSubIqPrivate()
    : queryType(QXmppPubSubIq::ItemsQuery)
{
}

///
/// Constructs a PubSub IQ.
///
QXmppPubSubIq::QXmppPubSubIq()
    : d(new QXmppPubSubIqPrivate)
{
}

///
/// Constructs a copy of other.
///
/// \param other
///
QXmppPubSubIq::QXmppPubSubIq(const QXmppPubSubIq &other) = default;

QXmppPubSubIq::~QXmppPubSubIq() = default;

///
/// Assigns \a other to this IQ.
///
/// \param other
///
QXmppPubSubIq &QXmppPubSubIq::operator=(const QXmppPubSubIq &other) = default;

///
/// Returns the PubSub query type for this IQ.
///
QXmppPubSubIq::QueryType QXmppPubSubIq::queryType() const
{
    return d->queryType;
}

///
/// Sets the PubSub query type for this IQ.
///
/// \param queryType
///
void QXmppPubSubIq::setQueryType(QXmppPubSubIq::QueryType queryType)
{
    d->queryType = queryType;
}

///
/// Returns the JID being queried.
///
QString QXmppPubSubIq::queryJid() const
{
    return d->queryJid;
}

///
/// Sets the JID being queried.
///
/// \param queryJid
///
void QXmppPubSubIq::setQueryJid(const QString &queryJid)
{
    d->queryJid = queryJid;
}

///
/// Returns the name of the node being queried.
///
QString QXmppPubSubIq::queryNodeName() const
{
    return d->queryNodeName;
}

///
/// Sets the name of the node being queried.
///
/// \param queryNodeName
///
void QXmppPubSubIq::setQueryNodeName(const QString &queryNodeName)
{
    d->queryNodeName = queryNodeName;
}

///
/// Returns the subscription ID.
///
QString QXmppPubSubIq::subscriptionId() const
{
    return d->subscriptionId;
}

///
/// Sets the subscription ID.
///
/// \param subscriptionId
///
void QXmppPubSubIq::setSubscriptionId(const QString &subscriptionId)
{
    d->subscriptionId = subscriptionId;
}

///
/// Returns the publish options for the IQ's items.
///
QXmppDataForm QXmppPubSubIq::publishOptions() const
{
    return d->publishOptions;
}

///
/// Sets the publish options for the IQ's items.
///
/// \param publishOptions
///
void QXmppPubSubIq::setPublishOptions(const QXmppDataForm &publishOptions)
{
    d->publishOptions = publishOptions;
}

///
/// Returns the IQ's items.
///
QList<QXmppPubSubItem> QXmppPubSubIq::items() const
{
    return d->items;
}

///
/// Sets the IQ's items.
///
/// \param items
///
void QXmppPubSubIq::setItems(const QList<QXmppPubSubItem> &items)
{
    d->items = items;
}

/// \cond
bool QXmppPubSubIq::isPubSubIq(const QDomElement &element)
{
    QString ns = element.firstChildElement(QStringLiteral("pubsub")).namespaceURI();
    return ns == ns_pubsub || ns == ns_pubsub_owner;
}

void QXmppPubSubIq::parseElementFromChild(const QDomElement &element)
{
    const QDomElement pubSubElement = element.firstChildElement(QStringLiteral("pubsub"));

    const QDomElement queryElement = pubSubElement.firstChildElement();

    // determine query type
    const QString tagName = queryElement.tagName();

    int queryType = PUBSUB_QUERIES.indexOf(queryElement.tagName());
    if (queryType > -1)
        d->queryType = QueryType(queryType);

    d->queryJid = queryElement.attribute(QStringLiteral("jid"));
    d->queryNodeName = queryElement.attribute(QStringLiteral("node"));

    // parse contents
    QDomElement childElement;

    switch (d->queryType) {
    case QXmppPubSubIq::ItemsQuery:
    case QXmppPubSubIq::PublishQuery:
    case QXmppPubSubIq::RetractQuery:
        childElement = queryElement.firstChildElement(QStringLiteral("item"));
        while (!childElement.isNull()) {
            QXmppPubSubItem item;
            item.parse(childElement);
            d->items << item;
            childElement = childElement.nextSiblingElement(QStringLiteral("item"));
        }
        break;
    case QXmppPubSubIq::SubscriptionQuery:
        d->subscriptionId = queryElement.attribute(QStringLiteral("subid"));
        d->subscriptionType = queryElement.attribute(QStringLiteral("subscription"));
        break;
    default:
        break;
    }

    // parse publish options
    if (d->queryType == QXmppPubSubIq::PublishQuery) {
        QDomElement optionsElement = pubSubElement.firstChildElement("publish-options");
        QXmppDataForm form;
        form.parse(optionsElement.firstChildElement());
        d->publishOptions = form;
    }
}

void QXmppPubSubIq::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    writer->writeStartElement(QStringLiteral("pubsub"));

    QString defaultNamespace = ns_pubsub;
    if (d->queryType == QXmppPubSubIq::DeleteQuery)
        defaultNamespace = ns_pubsub_owner;
    writer->writeDefaultNamespace(defaultNamespace);

    // write query type
    writer->writeStartElement(PUBSUB_QUERIES.at(d->queryType));
    helperToXmlAddAttribute(writer, QStringLiteral("jid"), d->queryJid);
    helperToXmlAddAttribute(writer, QStringLiteral("node"), d->queryNodeName);

    // write contents
    switch (d->queryType) {
    case QXmppPubSubIq::ItemsQuery:
    case QXmppPubSubIq::PublishQuery:
    case QXmppPubSubIq::RetractQuery:
        for (const auto &item : d->items)
            item.toXml(writer);
        break;
    case QXmppPubSubIq::SubscriptionQuery:
        helperToXmlAddAttribute(writer, QStringLiteral("subid"), d->subscriptionId);
        helperToXmlAddAttribute(writer, QStringLiteral("subscription"), d->subscriptionType);
        break;
    default:
        break;
    }
    writer->writeEndElement();

    // write publish options
    if (d->queryType == QXmppPubSubIq::PublishQuery && !d->publishOptions.isNull()) {
        writer->writeStartElement("publish-options");
        d->publishOptions.toXml(writer);
        writer->writeEndElement();
    }

    writer->writeEndElement();
}
/// \endcond
