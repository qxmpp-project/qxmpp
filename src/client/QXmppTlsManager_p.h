/*
 * Copyright (C) 2008-2020 The QXmpp developers
 *
 * Authors:
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

#ifndef QXMPPTLSMANAGER_H
#define QXMPPTLSMANAGER_H

#include "QXmppInternalClientExtension_p.h"

///
/// \brief The QXmppTlsManager enables the QXmppClient to use STARTTLS. It is
/// added to the client by default and can be configured using the
/// \c QXmppConfiguration class.
///
/// \ingroup Managers
///
class QXmppTlsManager : public QXmppInternalClientExtension
{
    Q_OBJECT

public:
    QXmppTlsManager();

    bool handleStanza(const QDomElement &stanza) override;
};

#endif  // QXMPPTLSMANAGER_H
