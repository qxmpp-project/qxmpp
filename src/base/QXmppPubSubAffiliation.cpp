// SPDX-FileCopyrightText: 2020 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppPubSubAffiliation.h"

#include "QXmppConstants_p.h"
#include "QXmppUtils_p.h"

#include "StringLiterals.h"

#include <QDomElement>
#include <QXmlStreamWriter>

using namespace QXmpp::Private;

///
/// \class QXmppPubSubAffiliation
///
/// This class represents an affiliation of a user with a PubSub node as defined
/// in \xep{0060, Publish-Subscribe}.
///
/// \sa QXmppPubSubIq
/// \sa QXmppPubSubEvent
///
/// \since QXmpp 1.5
///

constexpr auto PUBSUB_AFFILIATIONS = to_array<QStringView>({
    u"none",
    u"member",
    u"outcast",
    u"owner",
    u"publisher",
    u"publish-only",
});

class QXmppPubSubAffiliationPrivate : public QSharedData
{
public:
    QXmppPubSubAffiliationPrivate(QXmppPubSubAffiliation::Affiliation type,
                                  const QString &node,
                                  const QString &jid);

    QXmppPubSubAffiliation::Affiliation type;
    QString node;
    QString jid;
};

QXmppPubSubAffiliationPrivate::QXmppPubSubAffiliationPrivate(QXmppPubSubAffiliation::Affiliation type,
                                                             const QString &node,
                                                             const QString &jid)
    : type(type),
      node(node),
      jid(jid)
{
}

///
/// Default constructor.
///
QXmppPubSubAffiliation::QXmppPubSubAffiliation(Affiliation type,
                                               const QString &node,
                                               const QString &jid)
    : d(new QXmppPubSubAffiliationPrivate(type, node, jid))
{
}

/// Copy constructor.
QXmppPubSubAffiliation::QXmppPubSubAffiliation(const QXmppPubSubAffiliation &) = default;
/// Move-constructor.
QXmppPubSubAffiliation::QXmppPubSubAffiliation(QXmppPubSubAffiliation &&) = default;
QXmppPubSubAffiliation::~QXmppPubSubAffiliation() = default;
/// Assignment operator.
QXmppPubSubAffiliation &QXmppPubSubAffiliation::operator=(const QXmppPubSubAffiliation &) = default;
/// Move-assignment operator.
QXmppPubSubAffiliation &QXmppPubSubAffiliation::operator=(QXmppPubSubAffiliation &&) = default;

///
/// Returns the type of the affiliation.
///
QXmppPubSubAffiliation::Affiliation QXmppPubSubAffiliation::type() const
{
    return d->type;
}

///
/// Sets the type of the affiliation.
///
void QXmppPubSubAffiliation::setType(Affiliation type)
{
    d->type = type;
}

///
/// Returns the node name of the node the affiliation belongs to.
///
QString QXmppPubSubAffiliation::node() const
{
    return d->node;
}

///
/// Sets the node name.
///
void QXmppPubSubAffiliation::setNode(const QString &node)
{
    d->node = node;
}

///
/// Returns the JID of the user.
///
QString QXmppPubSubAffiliation::jid() const
{
    return d->jid;
}

///
/// Sets the JID of the user.
///
void QXmppPubSubAffiliation::setJid(const QString &jid)
{
    d->jid = jid;
}

///
/// Returns true if the DOM element is a PubSub affiliation.
///
bool QXmppPubSubAffiliation::isAffiliation(const QDomElement &element)
{
    if (element.tagName() != u"affiliation" ||
        !enumFromString<Affiliation>(PUBSUB_AFFILIATIONS, element.attribute(u"affiliation"_s))) {
        return false;
    }

    if (element.namespaceURI() == ns_pubsub) {
        return element.hasAttribute(u"node"_s);
    }
    if (element.namespaceURI() == ns_pubsub_owner) {
        return element.hasAttribute(u"jid"_s);
    }
    return false;
}

/// \cond
void QXmppPubSubAffiliation::parse(const QDomElement &element)
{
    d->type = enumFromString<Affiliation>(PUBSUB_AFFILIATIONS, element.attribute(u"affiliation"_s)).value_or(None);

    d->node = element.attribute(u"node"_s);
    d->jid = element.attribute(u"jid"_s);
}

void QXmppPubSubAffiliation::toXml(QXmlStreamWriter *writer) const
{
    writer->writeStartElement(QSL65("affiliation"));
    writer->writeAttribute(QSL65("affiliation"), toString65(PUBSUB_AFFILIATIONS.at(size_t(d->type))));
    writeOptionalXmlAttribute(writer, u"node", d->node);
    writeOptionalXmlAttribute(writer, u"jid", d->jid);
    writer->writeEndElement();
}
/// \endcond
