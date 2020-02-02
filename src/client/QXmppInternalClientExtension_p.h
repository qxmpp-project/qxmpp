/*
 * Copyright (C) 2008-2020 The QXmpp developers
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

///
/// \brief The QXmppInternalClientExtension class is used to access private
/// components of the QXmppClient.
///
/// It is not exposed to the public API and is only used to split up internal
/// parts of the QXmppClient, like TLS negotiation.
///
class QXmppInternalClientExtension : public QXmppClientExtension
{
    Q_OBJECT

public:
    QXmppInternalClientExtension();

protected:
    QXmppOutgoingClient *clientStream();
};

#endif  // QXMPPINTERNALCLIENTEXTENSION_H
