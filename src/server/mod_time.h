/*
 * Copyright (C) 2008-2011 The QXmpp developers
 *
 * Author:
 *  Jeremy Lainé
 *
 * Source:
 *  http://code.google.com/p/qxmpp
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

#ifndef QXMPP_SERVER_TIME_H
#define QXMPP_SERVER_TIME_H

#include "QXmppServerExtension.h"

/// \brief QXmppServer extension for XEP-0202: Entity Time.
///

class QXmppServerTime : public QXmppServerExtension
{
    Q_OBJECT
    Q_CLASSINFO("ExtensionName", "time");

public:
    QStringList discoveryFeatures() const;
    bool handleStanza(QXmppStream *stream, const QDomElement &element);
};

#endif
