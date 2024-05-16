// SPDX-FileCopyrightText: 2020 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QXmpp API.
//
// This header file may change from version to version without notice,
// or even be removed.
//
// We mean it.
//

#ifndef QXMPPINTERNALCLIENTEXTENSION_H
#define QXMPPINTERNALCLIENTEXTENSION_H

#include "QXmppClientExtension.h"

class QXmppOutgoingClient;

//
// \brief The QXmppInternalClientExtension class is used to access private
// components of the QXmppClient.
//
// It is not exposed to the public API and is only used to split up internal
// parts of the QXmppClient, like TLS negotiation.
//
class QXmppInternalClientExtension : public QXmppClientExtension
{
    Q_OBJECT

public:
    QXmppInternalClientExtension();

protected:
    QXmppOutgoingClient *clientStream();
};

#endif  // QXMPPINTERNALCLIENTEXTENSION_H
