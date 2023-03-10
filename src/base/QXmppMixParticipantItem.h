// SPDX-FileCopyrightText: 2019 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPMIXPARTICIPANTITEM_H
#define QXMPPMIXPARTICIPANTITEM_H

#include "QXmppPubSubBaseItem.h"

class QXmppMixParticipantItemPrivate;

class QXMPP_EXPORT QXmppMixParticipantItem : public QXmppPubSubBaseItem
{
public:
    QXmppMixParticipantItem();
    QXmppMixParticipantItem(const QXmppMixParticipantItem &);
    QXmppMixParticipantItem(QXmppMixParticipantItem &&);
    ~QXmppMixParticipantItem();

    QXmppMixParticipantItem &operator=(const QXmppMixParticipantItem &);
    QXmppMixParticipantItem &operator=(QXmppMixParticipantItem &&);

    const QString &nick() const;
    void setNick(QString);

    const QString &jid() const;
    void setJid(QString);

    static bool isItem(const QDomElement &);

protected:
    /// \cond
    void parsePayload(const QDomElement &payloadElement) override;
    void serializePayload(QXmlStreamWriter *writer) const override;
    /// \endcond

private:
    QSharedDataPointer<QXmppMixParticipantItemPrivate> d;
};

#endif  // QXMPPMIXPARTICIPANTITEM_H
