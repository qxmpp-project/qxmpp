// SPDX-FileCopyrightText: 2024 Filipe Azevedo <pasnox@gmail.com>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppMovedItem.h"

#include "QXmppConstants_p.h"

#include <QXmlStreamWriter>

constexpr QStringView MOVED = u"moved";
constexpr QStringView NEW_JID = u"new-jid";

class QXmppMovedItemPrivate : public QSharedData
{
public:
    QString newJid;
};

///
/// \class QXmppMovedItem
///
/// \brief The QXmppMovedItem class represents a PubSub item for \xep{0283, Moved}.
///
/// \since QXmpp 1.7
///
/// \ingroup Stanzas
///

///
/// Helper constructor taking the newJid
///
QXmppMovedItem::QXmppMovedItem(const QString &newJid)
    : d(new QXmppMovedItemPrivate)
{
    setId(QStringLiteral("current"));
    d->newJid = newJid;
}

/// Default copy-constructor
QXmppMovedItem::QXmppMovedItem(const QXmppMovedItem &) = default;
/// Default move-constructor
QXmppMovedItem::QXmppMovedItem(QXmppMovedItem &&) = default;

/// Default assignment operator
QXmppMovedItem &QXmppMovedItem::operator=(const QXmppMovedItem &) = default;
/// Default move-assignment operator
QXmppMovedItem &QXmppMovedItem::operator=(QXmppMovedItem &&) = default;
QXmppMovedItem::~QXmppMovedItem() = default;

///
/// Returns the bare new JID the user moved to.
///
/// \return the new JID
///
QString QXmppMovedItem::newJid() const
{
    return d->newJid;
}

///
/// Sets the bare new JID the user moved to.
///
/// \see newJid()
///
/// \param newJid the new JID
///
void QXmppMovedItem::setNewJid(const QString &newJid)
{
    d->newJid = newJid;
}

///
/// Returns true if the given DOM element is a valid \xep{0283, Moved} item.
///
bool QXmppMovedItem::isItem(const QDomElement &itemElement)
{
    return QXmppPubSubBaseItem::isItem(itemElement, [](const QDomElement &payload) {
        if (payload.tagName() != MOVED || payload.namespaceURI() != ns_moved) {
            return false;
        }
        return payload.firstChildElement().tagName() == NEW_JID;
    });
}

/// \cond
void QXmppMovedItem::parsePayload(const QDomElement &payloadElement)
{
    d->newJid = payloadElement.firstChildElement(NEW_JID.toString()).text();
}

void QXmppMovedItem::serializePayload(QXmlStreamWriter *writer) const
{
    if (d->newJid.isEmpty()) {
        return;
    }

    writer->writeStartElement(MOVED.toString());
    writer->writeDefaultNamespace(ns_moved.toString());
    writer->writeTextElement(NEW_JID.toString(), d->newJid);
    writer->writeEndElement();
}
/// \endcond
