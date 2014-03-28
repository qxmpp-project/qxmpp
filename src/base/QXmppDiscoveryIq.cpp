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

#include <QCryptographicHash>
#include <QDomElement>

#include "QXmppConstants.h"
#include "QXmppDiscoveryIq.h"
#include "QXmppUtils.h"

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

QString QXmppDiscoveryIq::Identity::category() const
{
    return m_category;
}

void QXmppDiscoveryIq::Identity::setCategory(const QString &category)
{
    m_category = category;
}

QString QXmppDiscoveryIq::Identity::language() const
{
    return m_language;
}

void QXmppDiscoveryIq::Identity::setLanguage(const QString &language)
{
    m_language = language;
}

QString QXmppDiscoveryIq::Identity::name() const
{
    return m_name;
}

void QXmppDiscoveryIq::Identity::setName(const QString &name)
{
    m_name = name;
}

QString QXmppDiscoveryIq::Identity::type() const
{
    return m_type;
}

void QXmppDiscoveryIq::Identity::setType(const QString &type)
{
    m_type = type;
}

QString QXmppDiscoveryIq::Item::jid() const
{
    return m_jid;
}

void QXmppDiscoveryIq::Item::setJid(const QString &jid)
{
    m_jid = jid;
}

QString QXmppDiscoveryIq::Item::name() const
{
    return m_name;
}

void QXmppDiscoveryIq::Item::setName(const QString &name)
{
    m_name = name;
}

QString QXmppDiscoveryIq::Item::node() const
{
    return m_node;
}

void QXmppDiscoveryIq::Item::setNode(const QString &node)
{
    m_node = node;
}

QStringList QXmppDiscoveryIq::features() const
{
    return m_features;
}

void QXmppDiscoveryIq::setFeatures(const QStringList &features)
{
    m_features = features;
}

QList<QXmppDiscoveryIq::Identity> QXmppDiscoveryIq::identities() const
{
    return m_identities;
}

void QXmppDiscoveryIq::setIdentities(const QList<QXmppDiscoveryIq::Identity> &identities)
{
    m_identities = identities;
}

QList<QXmppDiscoveryIq::Item> QXmppDiscoveryIq::items() const
{
    return m_items;
}

void QXmppDiscoveryIq::setItems(const QList<QXmppDiscoveryIq::Item> &items)
{
    m_items = items;
}

/// Returns the QXmppDataForm for this IQ, as defined by
/// XEP-0128: Service Discovery Extensions.
///

QXmppDataForm QXmppDiscoveryIq::form() const
{
    return m_form;
}

/// Sets the QXmppDataForm for this IQ, as define by
/// XEP-0128: Service Discovery Extensions.
///
/// \param form
///

void QXmppDiscoveryIq::setForm(const QXmppDataForm &form)
{
    m_form = form;
}

QString QXmppDiscoveryIq::queryNode() const
{
    return m_queryNode;
}

void QXmppDiscoveryIq::setQueryNode(const QString &node)
{
    m_queryNode = node;
}

enum QXmppDiscoveryIq::QueryType QXmppDiscoveryIq::queryType() const
{
    return m_queryType;
}

void QXmppDiscoveryIq::setQueryType(enum QXmppDiscoveryIq::QueryType type)
{
    m_queryType = type;
}

/// Calculate the verification string for XEP-0115 : Entity Capabilities

QByteArray QXmppDiscoveryIq::verificationString() const
{
    QString S;
    QList<QXmppDiscoveryIq::Identity> sortedIdentities = m_identities;
    qSort(sortedIdentities.begin(), sortedIdentities.end(), identityLessThan);
    QStringList sortedFeatures = m_features;
    qSort(sortedFeatures);
    sortedFeatures.removeDuplicates();
    foreach (const QXmppDiscoveryIq::Identity &identity, sortedIdentities)
        S += QString("%1/%2/%3/%4<").arg(identity.category(), identity.type(), identity.language(), identity.name());
    foreach (const QString &feature, sortedFeatures)
        S += feature + QLatin1String("<");

    if (!m_form.isNull()) {
        QMap<QString, QXmppDataForm::Field> fieldMap;
        foreach (const QXmppDataForm::Field &field, m_form.fields()) {
            fieldMap.insert(field.key(), field);
        }

        if (fieldMap.contains("FORM_TYPE")) {
            const QXmppDataForm::Field field = fieldMap.take("FORM_TYPE");
            S += field.value().toString() + QLatin1String("<");

            QStringList keys = fieldMap.keys();
            qSort(keys);
            foreach (const QString &key, keys) {
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
    m_queryNode = queryElement.attribute("node");
    if (queryElement.namespaceURI() == ns_disco_items)
        m_queryType = ItemsQuery;
    else
        m_queryType = InfoQuery;

    QDomElement itemElement = queryElement.firstChildElement();
    while (!itemElement.isNull())
    {
        if (itemElement.tagName() == "feature")
        {
            m_features.append(itemElement.attribute("var"));
        }
        else if (itemElement.tagName() == "identity")
        {
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

            m_identities.append(identity);
        }
        else if (itemElement.tagName() == "item")
        {
            QXmppDiscoveryIq::Item item;
            item.setJid(itemElement.attribute("jid"));
            item.setName(itemElement.attribute("name"));
            item.setNode(itemElement.attribute("node"));
            m_items.append(item);
        }
        else if (itemElement.tagName() == "x" &&
                 itemElement.namespaceURI() == ns_data)
        {
            m_form.parse(itemElement);
        }
        itemElement = itemElement.nextSiblingElement();
    }
}

void QXmppDiscoveryIq::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    writer->writeStartElement("query");
    writer->writeAttribute("xmlns",
        m_queryType == InfoQuery ? ns_disco_info : ns_disco_items);
    helperToXmlAddAttribute(writer, "node", m_queryNode);

    if (m_queryType == InfoQuery) {
        foreach (const QXmppDiscoveryIq::Identity& identity, m_identities) {
            writer->writeStartElement("identity");
            helperToXmlAddAttribute(writer, "xml:lang", identity.language());
            helperToXmlAddAttribute(writer, "category", identity.category());
            helperToXmlAddAttribute(writer, "name", identity.name());
            helperToXmlAddAttribute(writer, "type", identity.type());
            writer->writeEndElement();
        }

        foreach (const QString &feature, m_features) {
            writer->writeStartElement("feature");
            helperToXmlAddAttribute(writer, "var", feature);
            writer->writeEndElement();
        }
    } else {
        foreach (const QXmppDiscoveryIq::Item& item, m_items) {
            writer->writeStartElement("item");
            helperToXmlAddAttribute(writer, "jid", item.jid());
            helperToXmlAddAttribute(writer, "name", item.name());
            helperToXmlAddAttribute(writer, "node", item.node());
            writer->writeEndElement();
        }
    }

    m_form.toXml(writer);

    writer->writeEndElement();
}
/// \endcond
