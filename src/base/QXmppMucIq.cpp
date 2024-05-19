// SPDX-FileCopyrightText: 2010 Jeremy Lainé <jeremy.laine@m4x.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppMucIq.h"

#include "QXmppConstants_p.h"
#include "QXmppUtils_p.h"

#include "StringLiterals.h"

#include <QDomElement>

using namespace QXmpp::Private;

QXmppMucItem::QXmppMucItem()
    : m_affiliation(QXmppMucItem::UnspecifiedAffiliation),
      m_role(QXmppMucItem::UnspecifiedRole)
{
}

/// Returns true if the current item is null.
bool QXmppMucItem::isNull() const
{
    return m_actor.isEmpty() &&
        m_affiliation == UnspecifiedAffiliation &&
        m_jid.isEmpty() &&
        m_nick.isEmpty() &&
        m_reason.isEmpty() &&
        m_role == UnspecifiedRole;
}

/// Returns the actor for this item, for instance the admin who kicked
/// a user out of a room.
QString QXmppMucItem::actor() const
{
    return m_actor;
}

/// Sets the \a actor for this item, for instance the admin who kicked
/// a user out of a room.
void QXmppMucItem::setActor(const QString &actor)
{
    m_actor = actor;
}

/// Returns the user's affiliation, i.e. long-lived permissions.
QXmppMucItem::Affiliation QXmppMucItem::affiliation() const
{
    return m_affiliation;
}

/// \cond
QXmppMucItem::Affiliation QXmppMucItem::affiliationFromString(const QString &affiliationStr)
{
    if (affiliationStr == u"owner") {
        return QXmppMucItem::OwnerAffiliation;
    } else if (affiliationStr == u"admin") {
        return QXmppMucItem::AdminAffiliation;
    } else if (affiliationStr == u"member") {
        return QXmppMucItem::MemberAffiliation;
    } else if (affiliationStr == u"outcast") {
        return QXmppMucItem::OutcastAffiliation;
    } else if (affiliationStr == u"none") {
        return QXmppMucItem::NoAffiliation;
    } else {
        return QXmppMucItem::UnspecifiedAffiliation;
    }
}

QString QXmppMucItem::affiliationToString(Affiliation affiliation)
{
    switch (affiliation) {
    case QXmppMucItem::OwnerAffiliation:
        return u"owner"_s;
    case QXmppMucItem::AdminAffiliation:
        return u"admin"_s;
    case QXmppMucItem::MemberAffiliation:
        return u"member"_s;
    case QXmppMucItem::OutcastAffiliation:
        return u"outcast"_s;
    case QXmppMucItem::NoAffiliation:
        return u"none"_s;
    default:
        return QString();
    }
}
/// \endcond

/// Sets the user's affiliation, i.e. long-lived permissions.
void QXmppMucItem::setAffiliation(Affiliation affiliation)
{
    m_affiliation = affiliation;
}

/// Returns the user's real JID.
QString QXmppMucItem::jid() const
{
    return m_jid;
}

/// Sets the user's real JID.
void QXmppMucItem::setJid(const QString &jid)
{
    m_jid = jid;
}

/// Returns the user's nickname.
QString QXmppMucItem::nick() const
{
    return m_nick;
}

/// Sets the user's nickname.
void QXmppMucItem::setNick(const QString &nick)
{
    m_nick = nick;
}

/// Returns the reason for this item, for example the reason for kicking
/// a user out of a room.
QString QXmppMucItem::reason() const
{
    return m_reason;
}

/// Sets the \a reason for this item, for example the reason for kicking
/// a user out of a room.
void QXmppMucItem::setReason(const QString &reason)
{
    m_reason = reason;
}

/// Returns the user's role, i.e. short-lived permissions.
QXmppMucItem::Role QXmppMucItem::role() const
{
    return m_role;
}

/// \cond
QXmppMucItem::Role QXmppMucItem::roleFromString(const QString &roleStr)
{
    if (roleStr == u"moderator") {
        return QXmppMucItem::ModeratorRole;
    } else if (roleStr == u"participant") {
        return QXmppMucItem::ParticipantRole;
    } else if (roleStr == u"visitor") {
        return QXmppMucItem::VisitorRole;
    } else if (roleStr == u"none") {
        return QXmppMucItem::NoRole;
    } else {
        return QXmppMucItem::UnspecifiedRole;
    }
}

QString QXmppMucItem::roleToString(Role role)
{
    switch (role) {
    case QXmppMucItem::ModeratorRole:
        return u"moderator"_s;
    case QXmppMucItem::ParticipantRole:
        return u"participant"_s;
    case QXmppMucItem::VisitorRole:
        return u"visitor"_s;
    case QXmppMucItem::NoRole:
        return u"none"_s;
    default:
        return QString();
    }
}
/// \endcond

/// Sets the user's role, i.e. short-lived permissions.
void QXmppMucItem::setRole(Role role)
{
    m_role = role;
}

/// \cond
void QXmppMucItem::parse(const QDomElement &element)
{
    m_affiliation = QXmppMucItem::affiliationFromString(element.attribute(u"affiliation"_s).toLower());
    m_jid = element.attribute(u"jid"_s);
    m_nick = element.attribute(u"nick"_s);
    m_role = QXmppMucItem::roleFromString(element.attribute(u"role"_s).toLower());
    m_actor = element.firstChildElement(u"actor"_s).attribute(u"jid"_s);
    m_reason = element.firstChildElement(u"reason"_s).text();
}

void QXmppMucItem::toXml(QXmlStreamWriter *writer) const
{
    writer->writeStartElement(QSL65("item"));
    writeOptionalXmlAttribute(writer, u"affiliation", affiliationToString(m_affiliation));
    writeOptionalXmlAttribute(writer, u"jid", m_jid);
    writeOptionalXmlAttribute(writer, u"nick", m_nick);
    writeOptionalXmlAttribute(writer, u"role", roleToString(m_role));
    if (!m_actor.isEmpty()) {
        writer->writeStartElement(QSL65("actor"));
        writeOptionalXmlAttribute(writer, u"jid", m_actor);
        writer->writeEndElement();
    }
    if (!m_reason.isEmpty()) {
        writeXmlTextElement(writer, u"reason", m_reason);
    }
    writer->writeEndElement();
}
/// \endcond

/// Returns the IQ's items.
QList<QXmppMucItem> QXmppMucAdminIq::items() const
{
    return m_items;
}

/// Sets the IQ's items.
void QXmppMucAdminIq::setItems(const QList<QXmppMucItem> &items)
{
    m_items = items;
}

/// \cond
bool QXmppMucAdminIq::isMucAdminIq(const QDomElement &element)
{
    QDomElement queryElement = element.firstChildElement(u"query"_s);
    return (queryElement.namespaceURI() == ns_muc_admin);
}

void QXmppMucAdminIq::parseElementFromChild(const QDomElement &element)
{
    QDomElement queryElement = element.firstChildElement(u"query"_s);
    for (const auto &child : iterChildElements(queryElement, u"item")) {
        QXmppMucItem item;
        item.parse(child);
        m_items << item;
    }
}

void QXmppMucAdminIq::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    writer->writeStartElement(QSL65("query"));
    writer->writeDefaultNamespace(toString65(ns_muc_admin));
    for (const QXmppMucItem &item : m_items) {
        item.toXml(writer);
    }
    writer->writeEndElement();
}
/// \endcond

/// Returns the IQ's data form.
QXmppDataForm QXmppMucOwnerIq::form() const
{
    return m_form;
}

/// Sets the IQ's data form.
void QXmppMucOwnerIq::setForm(const QXmppDataForm &form)
{
    m_form = form;
}

/// \cond
bool QXmppMucOwnerIq::isMucOwnerIq(const QDomElement &element)
{
    QDomElement queryElement = element.firstChildElement(u"query"_s);
    return (queryElement.namespaceURI() == ns_muc_owner);
}

void QXmppMucOwnerIq::parseElementFromChild(const QDomElement &element)
{
    QDomElement queryElement = element.firstChildElement(u"query"_s);
    m_form.parse(queryElement.firstChildElement(u"x"_s));
}

void QXmppMucOwnerIq::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    writer->writeStartElement(QSL65("query"));
    writer->writeDefaultNamespace(toString65(ns_muc_owner));
    m_form.toXml(writer);
    writer->writeEndElement();
}
/// \endcond
