// SPDX-FileCopyrightText: 2019 Linus Jahn <lnj@kaidan.im>
// SPDX-FileCopyrightText: 2023 Melvin Keskin <melvo@olomono.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPMIXIQ_H
#define QXMPPMIXIQ_H

#include "QXmppIq.h"
#include "QXmppMixConfigItem.h"

#include <QSharedDataPointer>

class QXmppMixInvitation;
class QXmppMixIqPrivate;

class QXMPP_EXPORT QXmppMixIq : public QXmppIq
{
public:
    enum Type {
        None,
        ClientJoin,
        ClientLeave,
        Join,
        Leave,
#if QXMPP_DEPRECATED_SINCE(1, 7)
        UpdateSubscription [[deprecated("Use QXmppMixManager")]],
#endif
        SetNick = 6,
        Create,
        Destroy
    };

    QXmppMixIq();
    QXmppMixIq(const QXmppMixIq &);
    QXmppMixIq(QXmppMixIq &&);
    ~QXmppMixIq() override;

    QXmppMixIq &operator=(const QXmppMixIq &);
    QXmppMixIq &operator=(QXmppMixIq &&);

    QXmppMixIq::Type actionType() const;
    void setActionType(QXmppMixIq::Type);

#if QXMPP_DEPRECATED_SINCE(1, 7)
    [[deprecated("Use participantId() and channelJid()")]] QString jid() const;
    [[deprecated("Use setParticipantId() and setChannelJid()")]] void setJid(const QString &);
#endif

    QString participantId() const;
    void setParticipantId(const QString &);

#if QXMPP_DEPRECATED_SINCE(1, 7)
    [[deprecated("Use channelId()")]] QString channelName() const;
    [[deprecated("Use setChannelId()")]] void setChannelName(const QString &);
#endif

    QString channelId() const;
    void setChannelId(const QString &);

    QString channelJid() const;
    void setChannelJid(const QString &);

#if QXMPP_DEPRECATED_SINCE(1, 7)
    [[deprecated("Use subscriptions()")]] QStringList nodes() const;
    [[deprecated("Use setSubscriptions()")]] void setNodes(const QStringList &);
#endif

    QXmppMixConfigItem::Nodes subscriptions() const;
    void setSubscriptions(QXmppMixConfigItem::Nodes);

    QString nick() const;
    void setNick(const QString &);

    std::optional<QXmppMixInvitation> invitation() const;
    void setInvitation(const std::optional<QXmppMixInvitation> &);

    /// \cond
    static bool isMixIq(const QDomElement &);
    /// \endcond

protected:
    /// \cond
    void parseElementFromChild(const QDomElement &) override;
    void toXmlElementFromChild(QXmlStreamWriter *) const override;
    /// \endcond

private:
    QSharedDataPointer<QXmppMixIqPrivate> d;
};

Q_DECLARE_METATYPE(QXmppMixIq::Type)

#endif  // QXMPPMIXIQ_H
