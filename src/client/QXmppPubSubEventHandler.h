// SPDX-FileCopyrightText: 2022 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPPUBSUBEVENTHANDLER_H
#define QXMPPPUBSUBEVENTHANDLER_H

#include "QXmppExtension.h"

class QDomElement;
class QXmppPubSubManager;

// Documented in QXmppPubSubManager.cpp.
class QXMPP_EXPORT QXmppPubSubEventHandler : public QXmppExtension
{
public:
    virtual bool handlePubSubEvent(const QDomElement &element, const QString &pubSubService, const QString &nodeName) = 0;
};

#endif  // QXMPPPUBSUBEVENTHANDLER_H
