/*
 * Copyright (C) 2008-2020 The QXmpp developers
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

#include "QXmppDiscoveryIq.h"

#include "QXmppConstants_p.h"
#include "QXmppUtils.h"

#include <QCryptographicHash>
#include <QDomElement>
#include <QSharedData>

static bool identityLessThan(const QXmppDiscoveryIq::Identity &i1, const QXmppDiscoveryIq::Identity &i2)
{
    if (i1.category() < i2.category())
        return true;
    else if (i1.category() > i2.category())
        return false;

    if (i1.type() < i2.type())
        return true;
    else if (i1.type() > i2.type())
        return false;

    if (i1.language() < i2.language())
        return true;
    else if (i1.language() > i2.language())
        return false;

    if (i1.name() < i2.name())
        return true;
    else if (i1.name() > i2.name())
        return false;

    return false;
}

class QXmppDiscoveryIdentityPrivate : public QSharedData
{
public:
    QString category;
    QString language;
    QString name;
    QString type;
};

QXmppDiscoveryIq::Identity::Identity()
    : d(new QXmppDiscoveryIdentityPrivate)
{
}

QXmppDiscoveryIq::Identity::Identity(const QXmppDiscoveryIq::Identity &other) = default;

QXmppDiscoveryIq::Identity::~Identity() = default;

QXmppDiscoveryIq::Identity &QXmppDiscoveryIq::Identity::operator=(const QXmppDiscoveryIq::Identity &) = default;

QString QXmppDiscoveryIq::Identity::category() const
{
    return d->category;
}

void QXmppDiscoveryIq::Identity::setCategory(const QString &category)
{
    d->category = category;
}

QString QXmppDiscoveryIq::Identity::language() const
{
    return d->language;
}

void QXmppDiscoveryIq::Identity::setLanguage(const QString &language)
{
    d->language = language;
}

QString QXmppDiscoveryIq::Identity::name() const
{
    return d->name;
}

void QXmppDiscoveryIq::Identity::setName(const QString &name)
{
    d->name = name;
}

QString QXmppDiscoveryIq::Identity::type() const
{
    return d->type;
}

void QXmppDiscoveryIq::Identity::setType(const QString &type)
{
    d->type = type;
}

class QXmppDiscoveryItemPrivate : public QSharedData
{
public:
    QString jid;
    QString name;
    QString node;
};

QXmppDiscoveryIq::Item::Item()
    : d(new QXmppDiscoveryItemPrivate)
{
}

QXmppDiscoveryIq::Item::Item(const QXmppDiscoveryIq::Item &) = default;

QXmppDiscoveryIq::Item::~Item() = default;

QXmppDiscoveryIq::Item &QXmppDiscoveryIq::Item::operator=(const QXmppDiscoveryIq::Item &) = default;

QString QXmppDiscoveryIq::Item::jid() const
{
    return d->jid;
}

void QXmppDiscoveryIq::Item::setJid(const QString &jid)
{
    d->jid = jid;
}

QString QXmppDiscoveryIq::Item::name() const
{
    return d->name;
}

void QXmppDiscoveryIq::Item::setName(const QString &name)
{
    d->name = name;
}

QString QXmppDiscoveryIq::Item::node() const
{
    return d->node;
}

void QXmppDiscoveryIq::Item::setNode(const QString &node)
{
    d->node = node;
}

class QXmppDiscoveryIqPrivate : public QSharedData
{
public:
    QStringList features;
    QList<QXmppDiscoveryIq::Identity> identities;
    QList<QXmppDiscoveryIq::Item> items;
    QXmppDataForm form;
    QString queryNode;
    enum QXmppDiscoveryIq::QueryType queryType;
};

QXmppDiscoveryIq::QXmppDiscoveryIq()
    : d(new QXmppDiscoveryIqPrivate)
{
}

QXmppDiscoveryIq::QXmppDiscoveryIq(const QXmppDiscoveryIq &) = default;

QXmppDiscoveryIq::~QXmppDiscoveryIq() = default;

QXmppDiscoveryIq &QXmppDiscoveryIq::operator=(const QXmppDiscoveryIq &) = default;

QStringList QXmppDiscoveryIq::features() const
{
    return d->features;
}

void QXmppDiscoveryIq::setFeatures(const QStringList &features)
{
    d->features = features;
}

QList<QXmppDiscoveryIq::Identity> QXmppDiscoveryIq::identities() const
{
    return d->identities;
}

void QXmppDiscoveryIq::setIdentities(const QList<QXmppDiscoveryIq::Identity> &identities)
{
    d->identities = identities;
}

QList<QXmppDiscoveryIq::Item> QXmppDiscoveryIq::items() const
{
    return d->items;
}

void QXmppDiscoveryIq::setItems(const QList<QXmppDiscoveryIq::Item> &items)
{
    d->items = items;
}

/// Returns the QXmppDataForm for this IQ, as defined by
/// \xep{0128}: Service Discovery Extensions.
///

QXmppDataForm QXmppDiscoveryIq::form() const
{
    return d->form;
}

/// Sets the QXmppDataForm for this IQ, as define by
/// \xep{0128}: Service Discovery Extensions.
///
/// \param form
///

void QXmppDiscoveryIq::setForm(const QXmppDataForm &form)
{
    d->form = form;
}

QString QXmppDiscoveryIq::queryNode() const
{
    return d->queryNode;
}

void QXmppDiscoveryIq::setQueryNode(const QString &node)
{
    d->queryNode = node;
}

enum QXmppDiscoveryIq::QueryType QXmppDiscoveryIq::queryType() const
{
    return d->queryType;
}

void QXmppDiscoveryIq::setQueryType(enum QXmppDiscoveryIq::QueryType type)
{
    d->queryType = type;
}

/// Calculate the verification string for \xep{0115}: Entity Capabilities

QByteArray QXmppDiscoveryIq::verificationString() const
{
    QString S;
    QList<QXmppDiscoveryIq::Identity> sortedIdentities = d->identities;
    std::sort(sortedIdentities.begin(), sortedIdentities.end(), identityLessThan);
    QStringList sortedFeatures = d->features;
    std::sort(sortedFeatures.begin(), sortedFeatures.end());
    sortedFeatures.removeDuplicates();
    for (const auto &identity : sortedIdentities)
        S += QString("%1/%2/%3/%4<").arg(identity.category(), identity.type(), identity.language(), identity.name());
    for (const auto &feature : sortedFeatures)
        S += feature + QLatin1String("<");

    if (!d->form.isNull()) {
        QMap<QString, QXmppDataForm::Field> fieldMap;
        for (const auto &field : d->form.fields()) {
            fieldMap.insert(field.key(), field);
        }

        if (fieldMap.contains("FORM_TYPE")) {
            const QXmppDataForm::Field field = fieldMap.take("FORM_TYPE");
            S += field.value().toString() + QLatin1String("<");

            QStringList keys = fieldMap.keys();
            std::sort(keys.begin(), keys.end());
            for (const auto &key : keys) {
                const QXmppDataForm::Field field = fieldMap.value(key);
                S += key + QLatin1String("<");
                if (field.value().canConvert<QStringList>()) {
                    QStringList list = field.value().toStringList();
                    list.sort();
                    S += list.join(QLatin1String("<"));
                } else {
                    S += field.value().toString();
                }
                S += QLatin1String("<");
            }
        } else {
            qWarning("QXmppDiscoveryIq form does not contain FORM_TYPE");
        }
    }

    QCryptographicHash hasher(QCryptographicHash::Sha1);
    hasher.addData(S.toUtf8());
    return hasher.result();
}

/// \cond
bool QXmppDiscoveryIq::isDiscoveryIq(const QDomElement &element)
{
    QDomElement queryElement = element.firstChildElement("query");
    return (queryElement.namespaceURI() == ns_disco_info ||
            queryElement.namespaceURI() == ns_disco_items);
}

void QXmppDiscoveryIq::parseElementFromChild(const QDomElement &element)
{
    QDomElement queryElement = element.firstChildElement("query");
    d->queryNode = queryElement.attribute("node");
    if (queryElement.namespaceURI() == ns_disco_items)
        d->queryType = ItemsQuery;
    else
        d->queryType = InfoQuery;

    QDomElement itemElement = queryElement.firstChildElement();
    while (!itemElement.isNull()) {
        if (itemElement.tagName() == "feature") {
            d->features.append(itemElement.attribute("var"));
        } else if (itemElement.tagName() == "identity") {
            QXmppDiscoveryIq::Identity identity;
            identity.setLanguage(itemElement.attribute("xml:lang"));
            identity.setCategory(itemElement.attribute("category"));
            identity.setName(itemElement.attribute("name"));
            identity.setType(itemElement.attribute("type"));

            // FIXME: for some reason the language does not found,
            // so we are forced to use QDomNamedNodeMap
            QDomNamedNodeMap m(itemElement.attributes());
            for (int i = 0; i < m.size(); ++i) {
                if (m.item(i).nodeName() == "xml:lang") {
                    identity.setLanguage(m.item(i).nodeValue());
                    break;
                }
            }

            d->identities.append(identity);
        } else if (itemElement.tagName() == "item") {
            QXmppDiscoveryIq::Item item;
            item.setJid(itemElement.attribute("jid"));
            item.setName(itemElement.attribute("name"));
            item.setNode(itemElement.attribute("node"));
            d->items.append(item);
        } else if (itemElement.tagName() == "x" &&
                   itemElement.namespaceURI() == ns_data) {
            d->form.parse(itemElement);
        }
        itemElement = itemElement.nextSiblingElement();
    }
}

void QXmppDiscoveryIq::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    writer->writeStartElement("query");
    writer->writeDefaultNamespace(
        d->queryType == InfoQuery ? ns_disco_info : ns_disco_items);
    helperToXmlAddAttribute(writer, "node", d->queryNode);

    if (d->queryType == InfoQuery) {
        for (const auto &identity : d->identities) {
            writer->writeStartElement("identity");
            helperToXmlAddAttribute(writer, "xml:lang", identity.language());
            helperToXmlAddAttribute(writer, "category", identity.category());
            helperToXmlAddAttribute(writer, "name", identity.name());
            helperToXmlAddAttribute(writer, "type", identity.type());
            writer->writeEndElement();
        }

        for (const auto &feature : d->features) {
            writer->writeStartElement("feature");
            helperToXmlAddAttribute(writer, "var", feature);
            writer->writeEndElement();
        }
    } else {
        for (const auto &item : d->items) {
            writer->writeStartElement("item");
            helperToXmlAddAttribute(writer, "jid", item.jid());
            helperToXmlAddAttribute(writer, "name", item.name());
            helperToXmlAddAttribute(writer, "node", item.node());
            writer->writeEndElement();
        }
    }

    d->form.toXml(writer);

    writer->writeEndElement();
}
/// \endcond
