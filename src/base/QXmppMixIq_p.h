// SPDX-FileCopyrightText: 2023 Melvin Keskin <melvo@olomono.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPMIXIQ_P_H
#define QXMPPMIXIQ_P_H

#include "QXmppIq.h"
#include "QXmppMixConfigItem.h"
#include "QXmppMixInvitation.h"

class QXmppMixInvitationRequestIqPrivate;
class QXmppMixInvitationResponseIqPrivate;

class QXMPP_EXPORT QXmppMixSubscriptionUpdateIq : public QXmppIq
{
public:
    QXmppMixSubscriptionUpdateIq();

    QXMPP_PRIVATE_DECLARE_RULE_OF_SIX(QXmppMixSubscriptionUpdateIq)

    QXmppMixConfigItem::Nodes additions() const;
    void setAdditions(QXmppMixConfigItem::Nodes);

    QXmppMixConfigItem::Nodes removals() const;
    void setRemovals(QXmppMixConfigItem::Nodes);

    /// \cond
    static bool isMixSubscriptionUpdateIq(const QDomElement &);

protected:
    void parseElementFromChild(const QDomElement &) override;
    void toXmlElementFromChild(QXmlStreamWriter *) const override;
    /// \endcond

private:
    QXmppMixConfigItem::Nodes m_additions;
    QXmppMixConfigItem::Nodes m_removals;
};

namespace QXmpp::Private {

QXMPP_EXPORT QVector<QString> mixNodesToList(QXmppMixConfigItem::Nodes nodes);
QXMPP_EXPORT QXmppMixConfigItem::Nodes listToMixNodes(const QVector<QString> &nodeList);

}  // namespace QXmpp::Private

#endif  // QXMPPMIXIQ_P_H
