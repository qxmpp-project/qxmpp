// SPDX-FileCopyrightText: 2009 Manjeet Dahiya <manjeetdahiya@gmail.com>
// SPDX-FileCopyrightText: 2010 Jeremy Lainé <jeremy.laine@m4x.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppRosterIq.h"

#include "QXmppConstants_p.h"
#include "QXmppUtils_p.h"

#include "StringLiterals.h"

#include <QDomElement>
#include <QSharedData>
#include <QXmlStreamWriter>

using namespace QXmpp::Private;

class QXmppRosterIqPrivate : public QSharedData
{
public:
    QList<QXmppRosterIq::Item> items;
    // XEP-0237 Roster Versioning
    QString version;
    // XEP-0405: Mediated Information eXchange (MIX): Participant Server Requirements
    bool mixAnnotate = false;
};

QXmppRosterIq::QXmppRosterIq()
    : d(new QXmppRosterIqPrivate)
{
}

/// Default copy-constructor
QXmppRosterIq::QXmppRosterIq(const QXmppRosterIq &) = default;
/// Default move-constructor
QXmppRosterIq::QXmppRosterIq(QXmppRosterIq &&) = default;
QXmppRosterIq::~QXmppRosterIq() = default;
/// Default assignment operator
QXmppRosterIq &QXmppRosterIq::operator=(const QXmppRosterIq &) = default;
/// Default move-assignment operator
QXmppRosterIq &QXmppRosterIq::operator=(QXmppRosterIq &&) = default;

///
/// Adds an item to the roster IQ.
///
/// \param item
///
void QXmppRosterIq::addItem(const Item &item)
{
    d->items.append(item);
}

///
/// Returns the roster IQ's items.
///
QList<QXmppRosterIq::Item> QXmppRosterIq::items() const
{
    return d->items;
}

///
/// Sets the roster IQ's items.
///
/// \since QXmpp 1.8
///
void QXmppRosterIq::setItems(const QList<Item> &items)
{
    d->items = items;
}

///
/// Returns the roster version of IQ.
///
/// \return version as a QString
///
/// \since QXmpp 1.0
///
QString QXmppRosterIq::version() const
{
    return d->version;
}

///
/// Sets the roster version of IQ.
///
/// \param version as a QString
///
/// \since QXmpp 1.0
///
void QXmppRosterIq::setVersion(const QString &version)
{
    d->version = version;
}

///
/// Whether to annotate which items are MIX channels.
///
/// \since QXmpp 1.3
///
bool QXmppRosterIq::mixAnnotate() const
{
    return d->mixAnnotate;
}

///
/// Sets whether to include which roster items are MIX channels. This MUST only
/// be enabled in get requests.
///
/// \since QXmpp 1.3
///
void QXmppRosterIq::setMixAnnotate(bool mixAnnotate)
{
    d->mixAnnotate = mixAnnotate;
}

/// \cond
bool QXmppRosterIq::isRosterIq(const QDomElement &element)
{
    return isIqType(element, u"query", ns_roster);
}

void QXmppRosterIq::parseElementFromChild(const QDomElement &element)
{
    QDomElement queryElement = element.firstChildElement(u"query"_s);
    setVersion(queryElement.attribute(u"ver"_s));

    for (const auto &itemElement : iterChildElements(queryElement, u"item")) {
        QXmppRosterIq::Item item;
        item.parse(itemElement);
        d->items.append(item);
    }

    setMixAnnotate(!firstChildElement(queryElement, u"annotate", ns_mix_roster).isNull());
}

void QXmppRosterIq::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    writer->writeStartElement(QSL65("query"));
    writer->writeDefaultNamespace(toString65(ns_roster));

    // XEP-0237 roster versioning - If the server does not advertise support for roster versioning, the client MUST NOT include the 'ver' attribute.
    if (!version().isEmpty()) {
        writer->writeAttribute(QSL65("ver"), version());
    }

    // XEP-0405: Mediated Information eXchange (MIX): Participant Server Requirements
    if (d->mixAnnotate) {
        writer->writeStartElement(QSL65("annotate"));
        writer->writeAttribute(QSL65("xmlns"), toString65(ns_mix_roster));
        writer->writeEndElement();
    }

    for (int i = 0; i < d->items.count(); ++i) {
        d->items.at(i).toXml(writer);
    }
    writer->writeEndElement();
}
/// \endcond

class QXmppRosterIq::ItemPrivate : public QSharedData
{
public:
    ItemPrivate();

    QString bareJid;
    Item::SubscriptionType type;
    QString name;
    // can be subscribe/unsubscribe (attribute "ask")
    QString subscriptionStatus;
    QSet<QString> groups;
    bool approved;
    // XEP-0405: Mediated Information eXchange (MIX): Participant Server Requirements
    bool isMixChannel = false;
    QString mixParticipantId;
};

QXmppRosterIq::ItemPrivate::ItemPrivate()
    : type(QXmppRosterIq::Item::NotSet),
      approved(false)
{
}

///
/// Constructs a new roster entry.
///
QXmppRosterIq::Item::Item()
    : d(new ItemPrivate)
{
}

/// Default copy-constructor
QXmppRosterIq::Item::Item(const QXmppRosterIq::Item &other) = default;
/// Default move-constructor
QXmppRosterIq::Item::Item(QXmppRosterIq::Item &&) = default;
QXmppRosterIq::Item::~Item() = default;
/// Default assignment operator
QXmppRosterIq::Item &QXmppRosterIq::Item::operator=(const Item &other) = default;
/// Default assignment operator
QXmppRosterIq::Item &QXmppRosterIq::Item::operator=(Item &&) = default;

///
/// Returns the bareJid of the roster entry.
///
/// \return bareJid as a QString
///
QString QXmppRosterIq::Item::bareJid() const
{
    return d->bareJid;
}

/// Sets the bareJid of the roster entry.
///
/// \param bareJid as a QString
///
void QXmppRosterIq::Item::setBareJid(const QString &bareJid)
{
    d->bareJid = bareJid;
}

///
/// Returns the groups of the roster entry.
///
/// \return QSet<QString> list of all the groups
///
QSet<QString> QXmppRosterIq::Item::groups() const
{
    return d->groups;
}

///
/// Sets the groups of the roster entry.
///
/// \param groups list of all the groups as a QSet<QString>
///
void QXmppRosterIq::Item::setGroups(const QSet<QString> &groups)
{
    d->groups = groups;
}

///
/// Returns the name of the roster entry.
///
/// \return name as a QString
///
QString QXmppRosterIq::Item::name() const
{
    return d->name;
}

///
/// Sets the name of the roster entry.
///
/// \param name as a QString
///
void QXmppRosterIq::Item::setName(const QString &name)
{
    d->name = name;
}

///
/// Returns the subscription status of the roster entry. It is the "ask"
/// attribute in the Roster IQ stanza. Its value can be "subscribe" or "unsubscribe"
/// or empty.
///
/// \return subscription status as a QString
///
QString QXmppRosterIq::Item::subscriptionStatus() const
{
    return d->subscriptionStatus;
}

///
/// Sets the subscription status of the roster entry. It is the "ask"
/// attribute in the Roster IQ stanza. Its value can be "subscribe" or "unsubscribe"
/// or empty.
///
/// \param status as a QString
///
void QXmppRosterIq::Item::setSubscriptionStatus(const QString &status)
{
    d->subscriptionStatus = status;
}

///
/// Returns the subscription type of the roster entry.
///
QXmppRosterIq::Item::SubscriptionType
QXmppRosterIq::Item::subscriptionType() const
{
    return d->type;
}

///
/// Sets the subscription type of the roster entry.
///
/// \param type
///
void QXmppRosterIq::Item::setSubscriptionType(SubscriptionType type)
{
    d->type = type;
}

///
/// Returns whether the item has a pre-approved presence subscription.
///
/// \since QXmpp 1.3
///
bool QXmppRosterIq::Item::isApproved() const
{
    return d->approved;
}

///
/// Sets whether the item has a pre-approved presence subscription.
///
/// This cannot be used to initiate a pre-approved subscription. For this
/// purpose the client must send a &lt;presence/&gt; stanza of type
/// \c subscribed to the user.
///
/// \since QXmpp 1.3
///
void QXmppRosterIq::Item::setIsApproved(bool approved)
{
    d->approved = approved;
}

QString QXmppRosterIq::Item::getSubscriptionTypeStr() const
{
    switch (d->type) {
    case NotSet:
        return u""_s;
    case None:
        return u"none"_s;
    case Both:
        return u"both"_s;
    case From:
        return u"from"_s;
    case To:
        return u"to"_s;
    case Remove:
        return u"remove"_s;
    default: {
        qWarning("QXmppRosterIq::Item::getTypeStr(): invalid type");
        return QString();
    }
    }
}

void QXmppRosterIq::Item::setSubscriptionTypeFromStr(const QString &type)
{
    if (type.isEmpty()) {
        setSubscriptionType(NotSet);
    } else if (type == u"none") {
        setSubscriptionType(None);
    } else if (type == u"both") {
        setSubscriptionType(Both);
    } else if (type == u"from") {
        setSubscriptionType(From);
    } else if (type == u"to") {
        setSubscriptionType(To);
    } else if (type == u"remove") {
        setSubscriptionType(Remove);
    } else {
        qWarning("QXmppRosterIq::Item::setTypeFromStr(): invalid type");
    }
}

///
/// Returns whether this is a MIX channel.
///
/// \since QXmpp 1.3
///
bool QXmppRosterIq::Item::isMixChannel() const
{
    return d->isMixChannel;
}

///
/// Sets whether this is a MIX channel.
///
/// \since QXmpp 1.3
///
void QXmppRosterIq::Item::setIsMixChannel(bool isMixChannel)
{
    d->isMixChannel = isMixChannel;
}

///
/// Returns the participant id for this MIX channel.
///
/// \since QXmpp 1.3
///
QString QXmppRosterIq::Item::mixParticipantId() const
{
    return d->mixParticipantId;
}

///
/// Sets the participant id for this MIX channel.
///
/// \since QXmpp 1.3
///
void QXmppRosterIq::Item::setMixParticipantId(const QString &participantId)
{
    d->mixParticipantId = participantId;
}

/// \cond
void QXmppRosterIq::Item::parse(const QDomElement &element)
{
    d->name = element.attribute(u"name"_s);
    d->bareJid = element.attribute(u"jid"_s);
    setSubscriptionTypeFromStr(element.attribute(u"subscription"_s));
    setSubscriptionStatus(element.attribute(u"ask"_s));

    // pre-approved
    const QString approved = element.attribute(u"approved"_s);
    d->approved = (approved == u"1" || approved == u"true");

    // groups
    for (const auto &groupElement : iterChildElements(element, u"group")) {
        d->groups << groupElement.text();
    }

    // XEP-0405: Mediated Information eXchange (MIX): Participant Server Requirements
    auto channelElement = firstChildElement(element, u"channel", ns_mix_roster);
    if (!channelElement.isNull()) {
        d->isMixChannel = true;
        d->mixParticipantId = channelElement.attribute(u"participant-id"_s);
    }
}

void QXmppRosterIq::Item::toXml(QXmlStreamWriter *writer) const
{
    toXml(writer, false);
}

void QXmppRosterIq::Item::toXml(QXmlStreamWriter *writer, bool external) const
{
    writer->writeStartElement(QSL65("item"));
    if (external) {
        writer->writeDefaultNamespace(toString65(ns_roster));
    }
    writeOptionalXmlAttribute(writer, u"jid", d->bareJid);
    writeOptionalXmlAttribute(writer, u"name", d->name);
    writeOptionalXmlAttribute(writer, u"subscription", getSubscriptionTypeStr());
    writeOptionalXmlAttribute(writer, u"ask", subscriptionStatus());
    if (d->approved) {
        writer->writeAttribute(QSL65("approved"), u"true"_s);
    }

    QSet<QString>::const_iterator i = d->groups.constBegin();
    while (i != d->groups.constEnd()) {
        writeXmlTextElement(writer, u"group", *i);
        ++i;
    }

    // XEP-0405: Mediated Information eXchange (MIX): Participant Server Requirements
    if (d->isMixChannel) {
        writer->writeStartElement(QSL65("channel"));
        writer->writeAttribute(QSL65("xmlns"), toString65(ns_mix_roster));
        writeOptionalXmlAttribute(writer, u"participant-id", d->mixParticipantId);
        writer->writeEndElement();
    }

    writer->writeEndElement();
}
/// \endcond
