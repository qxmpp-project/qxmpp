// SPDX-FileCopyrightText: 2019 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPMIXITEM_H
#define QXMPPMIXITEM_H

#include "QXmppElement.h"

#include <QSharedDataPointer>

class QXmppMixInfoItemPrivate;
class QXmppMixParticipantItemPrivate;

class QXMPP_EXPORT QXmppMixInfoItem
{
public:
    QXmppMixInfoItem();
    QXmppMixInfoItem(const QXmppMixInfoItem &);
    ~QXmppMixInfoItem();

    QXmppMixInfoItem &operator=(const QXmppMixInfoItem &);

    QString name() const;
    void setName(const QString &);

    QString description() const;
    void setDescription(const QString &);

    QStringList contactJids() const;
    void setContactJids(const QStringList &);

    /// \cond
    void parse(const QXmppElement &itemContent);
    QXmppElement toElement() const;
    /// \endcond

    static bool isMixChannelInfo(const QDomElement &);

private:
    QSharedDataPointer<QXmppMixInfoItemPrivate> d;
};

class QXMPP_EXPORT QXmppMixParticipantItem
{
public:
    QXmppMixParticipantItem();
    QXmppMixParticipantItem(const QXmppMixParticipantItem &);
    ~QXmppMixParticipantItem();

    QXmppMixParticipantItem &operator=(const QXmppMixParticipantItem &);

    QString nick() const;
    void setNick(const QString &);

    QString jid() const;
    void setJid(const QString &);

    /// \cond
    void parse(const QXmppElement &itemContent);
    QXmppElement toElement() const;
    /// \endcond

    static bool isMixParticipantItem(const QDomElement &);

private:
    QSharedDataPointer<QXmppMixParticipantItemPrivate> d;
};

#endif  // QXMPPMIXITEM_H
