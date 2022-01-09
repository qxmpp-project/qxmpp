/*
 * Copyright (C) 2008-2022 The QXmpp developers
 *
 * Author:
 *  Linus Jahn
 *
 * Source:
 *  https://github.com/qxmpp-project/qxmpp
 *
 * This file is a part of QXmpp library.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 */

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
