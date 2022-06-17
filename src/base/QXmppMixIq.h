// SPDX-FileCopyrightText: 2019 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPMIXIQ_H
#define QXMPPMIXIQ_H

#include "QXmppIq.h"

#include <QSharedDataPointer>

class QXmppMixIqPrivate;

///
/// \brief The QXmppMixIq class represents an IQ used to do actions on a MIX
/// channel as defined by \xep{0369}: Mediated Information eXchange (MIX) and
/// \xep{0405}: Mediated Information eXchange (MIX): Participant Server
/// Requirements.
///
/// \since QXmpp 1.1
///
/// \ingroup Stanzas
///
class QXMPP_EXPORT QXmppMixIq : public QXmppIq
{
public:
    /// The action type of the MIX query IQ.
    enum Type {
        None,
        ClientJoin,
        ClientLeave,
        Join,
        Leave,
        UpdateSubscription,
        SetNick,
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

#endif  // QXMPPMIXIQ_H
