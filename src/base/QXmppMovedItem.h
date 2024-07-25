// SPDX-FileCopyrightText: 2024 Filipe Azevedo <pasnox@gmail.com>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPMOVEDITEM_H
#define QXMPPMOVEDITEM_H

#include "QXmppPubSubBaseItem.h"

class QXmppMovedItemPrivate;

class QXMPP_EXPORT QXmppMovedItem : public QXmppPubSubBaseItem
{
public:
    QXmppMovedItem(const QString &newJid = {});
    QXmppMovedItem(const QXmppMovedItem &);
    QXmppMovedItem(QXmppMovedItem &&);
    ~QXmppMovedItem();

    QXmppMovedItem &operator=(const QXmppMovedItem &);
    QXmppMovedItem &operator=(QXmppMovedItem &&);

    QString newJid() const;
    void setNewJid(const QString &newJid);

    static bool isItem(const QDomElement &itemElement);

protected:
    /// \cond
    void parsePayload(const QDomElement &payloadElement) override;
    void serializePayload(QXmlStreamWriter *writer) const override;
    /// \endcond

private:
    QSharedDataPointer<QXmppMovedItemPrivate> d;
};

Q_DECLARE_METATYPE(QXmppMovedItem)

#endif  // QXMPPMOVEDITEM_H
