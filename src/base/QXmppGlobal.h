/*
 * Copyright (C) 2008-2011 The QXmpp developers
 *
 * Author:
 *  Manjeet Dahiya
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


#ifndef QXMPPGLOBAL_H
#define QXMPPGLOBAL_H

#include <QString>

/// This macro expands a numeric value of the form 0xMMNNPP (MM =
/// major, NN = minor, PP = patch) that specifies QXmpp's version
/// number. For example, if you compile your application against
/// QXmpp 1.2.3, the QXMPP_VERSION macro will expand to 0x010203.
///
/// You can use QXMPP_VERSION to use the latest QXmpp features where
/// available.
///

#define QXMPP_VERSION 0x00035b

QString QXmppVersion();

#endif //QXMPPGLOBAL_H
