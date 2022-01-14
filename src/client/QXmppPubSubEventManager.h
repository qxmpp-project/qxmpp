// SPDX-FileCopyrightText: 2020 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPPUBSUBEVENTMANAGER_H
#define QXMPPPUBSUBEVENTMANAGER_H

#include "QXmppClient.h"
#include "QXmppClientExtension.h"

class QXmppPubSubManager;

class QXMPP_EXPORT QXmppPubSubEventManager : public QXmppClientExtension
{
    Q_OBJECT

public:
    inline bool handleStanza(const QDomElement &) override
    {
        return false;
    }

protected:
    virtual bool handlePubSubEvent(const QDomElement &element, const QString &pubSubService, const QString &nodeName) = 0;

    inline QXmppPubSubManager *pubSub()
    {
        return client()->findExtension<QXmppPubSubManager>();
    }

    friend class QXmppPubSubManager;
};

#endif
