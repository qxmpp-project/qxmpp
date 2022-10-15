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
    QString id;
    QVector<QString> emojis;
};

///
/// \class QXmppMessageReaction
///
/// \brief The QXmppMessageReaction class represents a reaction to a message in the form of emojis
/// as specified by \xep{0444: Message Reactions}.
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
/// \return the message's ID
///
QString QXmppMessageReaction::id() const
{
    return d->id;
}

///
/// Sets the ID of the message for that the reaction is sent.
///
/// \param id message's ID
///
void QXmppMessageReaction::setId(const QString &id)
{
    d->id = id;
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
    d->id = element.attribute(QStringLiteral("id"));

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
    writer->writeAttribute(QStringLiteral("id"), d->id);

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
