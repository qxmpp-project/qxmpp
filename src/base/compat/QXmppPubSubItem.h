// SPDX-FileCopyrightText: 2010 Jeremy Lainé <jeremy.laine@m4x.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPPUBSUBITEM_H
#define QXMPPPUBSUBITEM_H

#include "QXmppGlobal.h"

#include <QSharedDataPointer>

class QDomElement;
class QXmlStreamWriter;

class QXmppElement;
class QXmppPubSubItemPrivate;

#if QXMPP_DEPRECATED_SINCE(1, 5)
class QXMPP_EXPORT QXmppPubSubItem
{
public:
    [[deprecated]] QXmppPubSubItem();
    QXmppPubSubItem(const QXmppPubSubItem &iq);
    ~QXmppPubSubItem();

    QXmppPubSubItem &operator=(const QXmppPubSubItem &iq);

    [[deprecated]] QString id() const;
    [[deprecated]] void setId(const QString &id);

    [[deprecated]] QXmppElement contents() const;
    [[deprecated]] void setContents(const QXmppElement &contents);

    [[deprecated]] void parse(const QDomElement &element);
    [[deprecated]] void toXml(QXmlStreamWriter *writer) const;

private:
    QSharedDataPointer<QXmppPubSubItemPrivate> d;
};
#endif

#endif  // QXMPPPUBSUBITEM_H
