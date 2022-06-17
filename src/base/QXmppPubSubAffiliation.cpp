// SPDX-FileCopyrightText: 2020 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppPubSubAffiliation.h"

#include "QXmppConstants_p.h"
#include "QXmppUtils.h"

#include <QDomElement>
#include <QXmlStreamWriter>

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

static const QStringList PUBSUB_AFFILIATIONS = {
    QStringLiteral("none"),
    QStringLiteral("member"),
    QStringLiteral("outcast"),
    QStringLiteral("owner"),
    QStringLiteral("publisher"),
    QStringLiteral("publish-only"),
};

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
    if (element.tagName() != QStringLiteral("affiliation") ||
        !PUBSUB_AFFILIATIONS.contains(element.attribute(QStringLiteral("affiliation")))) {
        return false;
    }

    if (element.namespaceURI() == ns_pubsub) {
        return element.hasAttribute(QStringLiteral("node"));
    }
    if (element.namespaceURI() == ns_pubsub_owner) {
        return element.hasAttribute(QStringLiteral("jid"));
    }
    return false;
}

/// \cond
void QXmppPubSubAffiliation::parse(const QDomElement &element)
{
    if (const auto typeIndex = PUBSUB_AFFILIATIONS.indexOf(element.attribute(QStringLiteral("affiliation")));
        typeIndex != -1) {
        d->type = Affiliation(typeIndex);
    } else {
        // this can only happen, when isAffiliation() returns false
        d->type = None;
    }

    d->node = element.attribute(QStringLiteral("node"));
    d->jid = element.attribute(QStringLiteral("jid"));
}

void QXmppPubSubAffiliation::toXml(QXmlStreamWriter *writer) const
{
    writer->writeStartElement(QStringLiteral("affiliation"));
    writer->writeAttribute(QStringLiteral("affiliation"), PUBSUB_AFFILIATIONS.at(int(d->type)));
    helperToXmlAddAttribute(writer, QStringLiteral("node"), d->node);
    helperToXmlAddAttribute(writer, QStringLiteral("jid"), d->jid);
    writer->writeEndElement();
}
/// \endcond
