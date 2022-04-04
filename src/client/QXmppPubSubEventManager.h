// SPDX-FileCopyrightText: 2020 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPPUBSUBEVENTMANAGER_H
#define QXMPPPUBSUBEVENTMANAGER_H

#include "QXmppClient.h"
#include "QXmppClientExtension.h"
#include "QXmppPubSubEventHandler.h"

class QXmppPubSubManager;

class QXMPP_EXPORT QXmppPubSubEventManager : public QXmppClientExtension, public QXmppPubSubEventHandler
{
    Q_OBJECT

protected:
    inline QXmppPubSubManager *pubSub()
    {
        return client()->findExtension<QXmppPubSubManager>();
    }
};

#endif
