// SPDX-FileCopyrightText: 2024 Filipe Azevedo <pasnox@gmail.com>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPMOVEDITEM_P_H
#define QXMPPMOVEDITEM_P_H

#include <QXmppPubSubBaseItem.h>

class QXmppMovedItem : public QXmppPubSubBaseItem
{
public:
    QXmppMovedItem(const QString &newJid = {});

    QString newJid() const { return m_newJid; }
    void setNewJid(const QString &newJid) { m_newJid = newJid; }

    static bool isItem(const QDomElement &itemElement);

protected:
    /// \cond
    void parsePayload(const QDomElement &payloadElement) override;
    void serializePayload(QXmlStreamWriter *writer) const override;
    /// \endcond

private:
    QString m_newJid;
};

#endif  // QXMPPMOVEDITEM_P_H
