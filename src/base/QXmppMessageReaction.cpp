// SPDX-FileCopyrightText: 2022 Melvin Keskin <melvo@olomono.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppMessageReaction.h"

#include "QXmppConstants_p.h"
#include "QXmppUtils.h"

#include <QDomElement>

class QXmppMessageReactionPrivate : public QSharedData
{
public:
    QString messageId;
    QVector<QString> emojis;
};

///
/// \class QXmppMessageReaction
///
/// \brief The QXmppMessageReaction class represents a reaction to a message in the form of emojis
/// as specified by \xep{0444, Message Reactions}.
///
/// \since QXmpp 1.5
///

///
/// Constructs a message reaction.
///
QXmppMessageReaction::QXmppMessageReaction()
    : d(new QXmppMessageReactionPrivate)
{
}

QXMPP_PRIVATE_DEFINE_RULE_OF_SIX(QXmppMessageReaction)

///
/// Returns the ID of the message for that the reaction is sent.
///
/// For a group chat message, \code QXmppMessage::stanzaId() \endcode is used.
///
/// For other message types, \code QXmppMessage::originId() \endcode is used.
/// If that is not available, \code QXmppMessage::id() \endcode is used.
///
/// \return the message's ID
///
QString QXmppMessageReaction::messageId() const
{
    return d->messageId;
}

///
/// Sets the ID of the message for that the reaction is sent.
///
/// For a group chat message, \code QXmppMessage::stanzaId() \endcode must be used.
/// If there is no such ID, a message reaction must not be sent.
///
/// For other message types, \code QXmppMessage::originId() \endcode should be used.
/// If that is not available, \code QXmppMessage::id() \endcode should be used.
///
/// \param messageId message's ID
///
void QXmppMessageReaction::setMessageId(const QString &messageId)
{
    d->messageId = messageId;
}

///
/// Returns the emojis in reaction to a message.
///
/// \return the emoji reactions
///
QVector<QString> QXmppMessageReaction::emojis() const
{
    return d->emojis;
}

///
/// Sets the emojis in reaction to a message.
///
/// Each reaction should only consist of unicode codepoints that can be displayed as a single emoji.
/// Duplicates are not allowed.
///
/// \param emojis emoji reactions
///
void QXmppMessageReaction::setEmojis(const QVector<QString> &emojis)
{
    d->emojis = emojis;
}

/// \cond
void QXmppMessageReaction::parse(const QDomElement &element)
{
    d->messageId = element.attribute(QStringLiteral("id"));

    for (auto childElement = element.firstChildElement();
         !childElement.isNull();
         childElement = childElement.nextSiblingElement()) {
        d->emojis.append(childElement.text());
    }

    // Remove duplicate emojis.
    std::sort(d->emojis.begin(), d->emojis.end());
    d->emojis.erase(std::unique(d->emojis.begin(), d->emojis.end()), d->emojis.end());
}

void QXmppMessageReaction::toXml(QXmlStreamWriter *writer) const
{
    writer->writeStartElement(QStringLiteral("reactions"));
    writer->writeDefaultNamespace(ns_reactions);
    writer->writeAttribute(QStringLiteral("id"), d->messageId);

    for (const auto &reaction : d->emojis) {
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
