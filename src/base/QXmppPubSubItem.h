/*
 * Copyright (C) 2008-2022 The QXmpp developers
 *
 * Author:
 *  Jeremy Lainé
 *  Linus Jahn
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

#ifndef QXMPPPUBSUBITEM_H
#define QXMPPPUBSUBITEM_H

#include "QXmppGlobal.h"

#include <QDomElement>
#include <QMetaType>
#include <QSharedDataPointer>

class QXmlStreamWriter;
class QXmppPubSubItemPrivate;

class QXMPP_EXPORT QXmppPubSubItem
{
public:
    QXmppPubSubItem(const QString &id = {}, const QString &publisher = {});
    QXmppPubSubItem(const QXmppPubSubItem &);
    virtual ~QXmppPubSubItem();

    QXmppPubSubItem &operator=(const QXmppPubSubItem &);

    QString id() const;
    void setId(const QString &id);

    QString publisher() const;
    void setPublisher(const QString &publisher);

    /// \cond
    void parse(const QDomElement &element);
    void toXml(QXmlStreamWriter *writer) const;
    /// \endcond

    static bool isItem(const QDomElement &element);

protected:
    virtual void parsePayload(const QDomElement &payloadElement);
    virtual void serializePayload(QXmlStreamWriter *writer) const;

    template<typename PayloadChecker>
    static bool isItem(const QDomElement &element, PayloadChecker isPayloadValid);

private:
    QSharedDataPointer<QXmppPubSubItemPrivate> d;
};

///
/// Returns true, if the element is a valid PubSub item and (if existant) the
/// payload is correct.
///
/// \param element The element to be checked to be an &lt;item/&gt; element.
/// \param isPayloadValid A function that validates the payload element (first
/// child element). The functions needs to return true, if the payload is valid.
/// In case there is no payload, the function is not called.
///
/// Here is an example covering how this could be used to check for the
/// \xep{0118, User Tune} payload:
/// \code
/// auto isPayloadValid = [](const QDomElement &payload) -> bool {
///     return payload.tagName() == "tune" && payload.namespaceURI() == ns_tune;
/// };
///
/// bool valid = QXmppPubSubItem::isItem(itemElement, isPayloadValid);
/// \endcode
///
template<typename PayloadChecker>
bool QXmppPubSubItem::isItem(const QDomElement &element, PayloadChecker isPayloadValid)
{
    if (!isItem(element)) {
        return false;
    }

    const QDomElement payload = element.firstChildElement();

    // we can only check the payload if it's existant
    if (!payload.isNull()) {
        return isPayloadValid(payload);
    }
    return true;
}

Q_DECLARE_METATYPE(QXmppPubSubItem)

#endif  // QXMPPPUBSUBITEM_H
