// SPDX-FileCopyrightText: 2010 Jeremy Lainé <jeremy.laine@m4x.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppDiscoveryIq.h"

#include "QXmppConstants_p.h"
#include "QXmppUtils.h"

#include <QCryptographicHash>
#include <QDomElement>
#include <QSharedData>

static bool identityLessThan(const QXmppDiscoveryIq::Identity &i1, const QXmppDiscoveryIq::Identity &i2)
{
    if (i1.category() < i2.category()) {
        return true;
    } else if (i1.category() > i2.category()) {
        return false;
    }

    if (i1.type() < i2.type()) {
        return true;
    } else if (i1.type() > i2.type()) {
        return false;
    }

    if (i1.language() < i2.language()) {
        return true;
    } else if (i1.language() > i2.language()) {
        return false;
    }

    if (i1.name() < i2.name()) {
        return true;
    } else if (i1.name() > i2.name()) {
        return false;
    }

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

///
/// \class QXmppDiscoveryIq::Identity
///
/// \brief Identity represents one of possibly multiple identities of an
/// XMPP entity obtained from a service discovery request as defined in
/// \xep{0030}: Service Discovery.
///

QXmppDiscoveryIq::Identity::Identity()
    : d(new QXmppDiscoveryIdentityPrivate)
{
}

/// Default copy-constructor
QXmppDiscoveryIq::Identity::Identity(const QXmppDiscoveryIq::Identity &other) = default;
/// Default move-constructor
QXmppDiscoveryIq::Identity::Identity(QXmppDiscoveryIq::Identity &&) = default;
QXmppDiscoveryIq::Identity::~Identity() = default;
/// Default assignment operator
QXmppDiscoveryIq::Identity &QXmppDiscoveryIq::Identity::operator=(const QXmppDiscoveryIq::Identity &) = default;
/// Default move-assignment operator
QXmppDiscoveryIq::Identity &QXmppDiscoveryIq::Identity::operator=(QXmppDiscoveryIq::Identity &&) = default;

///
/// Returns the category (e.g. "account", "client", "conference", etc.) of the
/// identity.
///
/// See https://xmpp.org/registrar/disco-categories.html for more details.
///
QString QXmppDiscoveryIq::Identity::category() const
{
    return d->category;
}

///
/// Sets the category (e.g. "account", "client", "conference", etc.) of the
/// identity.
///
/// See https://xmpp.org/registrar/disco-categories.html for more details.
///
void QXmppDiscoveryIq::Identity::setCategory(const QString &category)
{
    d->category = category;
}

///
/// Returns the language code of the identity.
///
/// It is possible that the same identity (same type and same category) is
/// included multiple times with different languages and localized names.
///
QString QXmppDiscoveryIq::Identity::language() const
{
    return d->language;
}

///
/// Sets the language code of the identity.
///
/// It is possible that the same identity (same type and same category) is
/// included multiple times with different languages and localized names.
///
void QXmppDiscoveryIq::Identity::setLanguage(const QString &language)
{
    d->language = language;
}

///
/// Returns the human-readable name of the service.
///
QString QXmppDiscoveryIq::Identity::name() const
{
    return d->name;
}

///
/// Sets the human-readable name of the service.
///
void QXmppDiscoveryIq::Identity::setName(const QString &name)
{
    d->name = name;
}

///
/// Returns the service type in this category.
///
/// See https://xmpp.org/registrar/disco-categories.html for details.
///
QString QXmppDiscoveryIq::Identity::type() const
{
    return d->type;
}

///
/// Sets the service type in this category.
///
/// See https://xmpp.org/registrar/disco-categories.html for details.
///
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

///
/// \class QXmppDiscoveryIq::Item
///
/// Item represents a related XMPP entity that can be queried using \xep{0030,
/// Service Discovery}.
///

QXmppDiscoveryIq::Item::Item()
    : d(new QXmppDiscoveryItemPrivate)
{
}

/// Default copy-constructor
QXmppDiscoveryIq::Item::Item(const QXmppDiscoveryIq::Item &) = default;
/// Default move-constructor
QXmppDiscoveryIq::Item::Item(QXmppDiscoveryIq::Item &&) = default;
QXmppDiscoveryIq::Item::~Item() = default;
/// Default assignment operator
QXmppDiscoveryIq::Item &QXmppDiscoveryIq::Item::operator=(const QXmppDiscoveryIq::Item &) = default;
/// Default move-assignment operator
QXmppDiscoveryIq::Item &QXmppDiscoveryIq::Item::operator=(QXmppDiscoveryIq::Item &&) = default;

///
/// Returns the jid of the item.
///
QString QXmppDiscoveryIq::Item::jid() const
{
    return d->jid;
}

///
/// Sets the jid of the item.
///
void QXmppDiscoveryIq::Item::setJid(const QString &jid)
{
    d->jid = jid;
}

///
/// Returns the items human-readable name.
///
QString QXmppDiscoveryIq::Item::name() const
{
    return d->name;
}

///
/// Sets the items human-readable name.
///
void QXmppDiscoveryIq::Item::setName(const QString &name)
{
    d->name = name;
}

///
/// Returns a special service discovery node.
///
QString QXmppDiscoveryIq::Item::node() const
{
    return d->node;
}

///
/// Sets a special service discovery node.
///
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
    QXmppDiscoveryIq::QueryType queryType;
};

///
/// \class QXmppDiscoveryIq
///
/// QXmppDiscoveryIq represents a discovery IQ request or result containing a
/// list of features and other information about an entity as defined by
/// \xep{0030, Service Discovery}.
///
/// \ingroup Stanzas
///

///
/// \enum QXmppDiscoveryIq::QueryType
///
/// Specifies the type of a service discovery query. An InfoQuery queries
/// identities and features, an ItemsQuery queries subservices in the form of
/// items.
///

QXmppDiscoveryIq::QXmppDiscoveryIq()
    : d(new QXmppDiscoveryIqPrivate)
{
}

/// Default copy-constructor
QXmppDiscoveryIq::QXmppDiscoveryIq(const QXmppDiscoveryIq &) = default;
/// Default move-constructor
QXmppDiscoveryIq::QXmppDiscoveryIq(QXmppDiscoveryIq &&) = default;
QXmppDiscoveryIq::~QXmppDiscoveryIq() = default;
/// Default assignment operator
QXmppDiscoveryIq &QXmppDiscoveryIq::operator=(const QXmppDiscoveryIq &) = default;
/// Default move-assignment operator
QXmppDiscoveryIq &QXmppDiscoveryIq::operator=(QXmppDiscoveryIq &&) = default;

///
/// Returns the features of the service.
///
QStringList QXmppDiscoveryIq::features() const
{
    return d->features;
}

///
/// Sets the features of the service.
///
void QXmppDiscoveryIq::setFeatures(const QStringList &features)
{
    d->features = features;
}

///
/// Returns the list of identities for this service.
///
QList<QXmppDiscoveryIq::Identity> QXmppDiscoveryIq::identities() const
{
    return d->identities;
}

///
/// Sets the list of identities for this service.
///
void QXmppDiscoveryIq::setIdentities(const QList<QXmppDiscoveryIq::Identity> &identities)
{
    d->identities = identities;
}

///
/// Returns the list of service discovery items.
///
QList<QXmppDiscoveryIq::Item> QXmppDiscoveryIq::items() const
{
    return d->items;
}

///
/// Sets the list of service discovery items.
///
void QXmppDiscoveryIq::setItems(const QList<QXmppDiscoveryIq::Item> &items)
{
    d->items = items;
}

///
/// Returns the QXmppDataForm for this IQ, as defined by \xep{0128, Service
/// Discovery Extensions}.
///
QXmppDataForm QXmppDiscoveryIq::form() const
{
    return d->form;
}

///
/// Sets the QXmppDataForm for this IQ, as define by \xep{0128, Service
/// Discovery Extensions}.
///
/// \param form
///
void QXmppDiscoveryIq::setForm(const QXmppDataForm &form)
{
    d->form = form;
}

///
/// Returns the special node to query.
///
QString QXmppDiscoveryIq::queryNode() const
{
    return d->queryNode;
}

///
/// Sets the special node to query.
///
void QXmppDiscoveryIq::setQueryNode(const QString &node)
{
    d->queryNode = node;
}

///
/// Returns the query type (info query or items query).
///
QXmppDiscoveryIq::QueryType QXmppDiscoveryIq::queryType() const
{
    return d->queryType;
}

///
/// Sets the query type (info query or items query).
///
void QXmppDiscoveryIq::setQueryType(enum QXmppDiscoveryIq::QueryType type)
{
    d->queryType = type;
}

///
/// Calculate the verification string for \xep{0115, Entity Capabilities}.
///
QByteArray QXmppDiscoveryIq::verificationString() const
{
    QString S;
    QList<QXmppDiscoveryIq::Identity> sortedIdentities = d->identities;
    std::sort(sortedIdentities.begin(), sortedIdentities.end(), identityLessThan);
    QStringList sortedFeatures = d->features;
    std::sort(sortedFeatures.begin(), sortedFeatures.end());
    sortedFeatures.removeDuplicates();
    for (const auto &identity : sortedIdentities) {
        S += QString("%1/%2/%3/%4<").arg(identity.category(), identity.type(), identity.language(), identity.name());
    }
    for (const auto &feature : sortedFeatures) {
        S += feature + QLatin1String("<");
    }

    if (!d->form.isNull()) {
        QMap<QString, QXmppDataForm::Field> fieldMap;
        const auto fields = d->form.fields();
        for (const auto &field : fields) {
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

///
/// Returns true, if the element is a valid service discovery IQ and can be
/// parsed.
///
bool QXmppDiscoveryIq::isDiscoveryIq(const QDomElement &element)
{
    QDomElement queryElement = element.firstChildElement();
    return checkIqType(queryElement.tagName(), queryElement.namespaceURI());
}

/// \cond
bool QXmppDiscoveryIq::checkIqType(const QString &tagName, const QString &xmlNamespace)
{
    return tagName == QStringLiteral("query") &&
        (xmlNamespace == ns_disco_info || xmlNamespace == ns_disco_items);
}

void QXmppDiscoveryIq::parseElementFromChild(const QDomElement &element)
{
    QDomElement queryElement = element.firstChildElement("query");
    d->queryNode = queryElement.attribute("node");
    if (queryElement.namespaceURI() == ns_disco_items) {
        d->queryType = ItemsQuery;
    } else {
        d->queryType = InfoQuery;
    }

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
