// SPDX-FileCopyrightText: 2019 Linus Jahn <lnj@kaidan.im>
// SPDX-FileCopyrightText: 2023 Melvin Keskin <melvo@olomono.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPMIXIQ_H
#define QXMPPMIXIQ_H

#include "QXmppIq.h"

#include <QSharedDataPointer>

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

    QString jid() const;
    void setJid(const QString &);

    QString channelName() const;
    void setChannelName(const QString &);

    QStringList nodes() const;
    void setNodes(const QStringList &);

    QString nick() const;
    void setNick(const QString &);

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
