// SPDX-FileCopyrightText: 2020 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppClient.h"
#include "QXmppClient_p.h"
#include "QXmppInternalClientExtension_p.h"

/// \cond
QXmppInternalClientExtension::QXmppInternalClientExtension()
    : QXmppClientExtension()
{
}

QXmppOutgoingClient *QXmppInternalClientExtension::clientStream()
{
    return client()->d->stream;
}
/// \endcond
