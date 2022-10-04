// SPDX-FileCopyrightText: 2022 Melvin Keskin <melvo@olomono.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppMessageReaction.h"

#include "QXmppConstants_p.h"
#include "QXmppUtils.h"

#include "QDomElement"

///
/// \class QXmppMessageReaction
///
/// \brief The QXmppMessageReaction class represents a reaction to a message as specified by
/// \xep{0444: Message Reactions} containing emojis.
///
/// Each element should only consist of unicode codepoints that can be displayed as a single emoji.
/// Duplicates are not allowed.
///
/// \since QXmpp 1.5
///

///
/// Constructs a message reaction.
///
QXmppMessageReaction::QXmppMessageReaction() = default;

QXMPP_PRIVATE_DEFINE_RULE_OF_SIX(QXmppMessageReaction)

///
/// Returns the ID of the message for that the reaction is sent.
///
/// \return the message's ID
///
QString QXmppMessageReaction::id() const
{
    return m_id;
}

///
/// Sets the ID of the message for that the reaction is sent.
///
/// \param id message's ID
///
void QXmppMessageReaction::setId(const QString &id)
{
    m_id = id;
}

/// \cond
void QXmppMessageReaction::parse(const QDomElement &element)
{
    m_id = element.attribute(QStringLiteral("id"));

    for (auto childElement = element.firstChildElement();
        !childElement.isNull();
        childElement = childElement.nextSiblingElement()) {
        append(childElement.text());
    }

    // Remove duplicate emojis.
    std::sort(this->begin(), this->end());
    this->erase(std::unique(this->begin(), this->end()), this->end());
}

void QXmppMessageReaction::toXml(QXmlStreamWriter *writer) const
{
    writer->writeStartElement(QStringLiteral("reactions"));
    writer->writeDefaultNamespace(ns_reactions);
    writer->writeAttribute(QStringLiteral("id"), m_id);

    for (const auto &reaction : *this) {
        helperToXmlAddTextElement(writer, QStringLiteral("reaction"), reaction);
    }
    writer->writeEndElement();
}
/// \endcond

///
/// Determines whether the given DOM element is a message reaction element.
///
/// \param element DOM element being checked
///
/// \return true if element is a message reaction element, otherwise false
///
bool QXmppMessageReaction::isMessageReaction(const QDomElement &element)
{
    return element.tagName() == QStringLiteral("reactions") &&
        element.namespaceURI() == ns_reactions;
}
