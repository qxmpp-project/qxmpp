/*
 * Copyright (C) 2008-2014 The QXmpp developers
 *
 * Author:
 *  Manjeet Dahiya
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


#ifndef QXMPPGLOBAL_H
#define QXMPPGLOBAL_H

#include <QString>

#if defined(QXMPP_STATIC)
#  define QXMPP_EXPORT
#else
#  if defined(QXMPP_BUILD)
#    define QXMPP_EXPORT Q_DECL_EXPORT
#  else
#    define QXMPP_EXPORT Q_DECL_IMPORT
#  endif
#endif

#if defined(QXMPP_AUTOTEST_INTERNAL)
#    define QXMPP_AUTOTEST_EXPORT QXMPP_EXPORT
#else
#    define QXMPP_AUTOTEST_EXPORT
#endif

/// This macro expands a numeric value of the form 0xMMNNPP (MM =
/// major, NN = minor, PP = patch) that specifies QXmpp's version
/// number. For example, if you compile your application against
/// QXmpp 1.2.3, the QXMPP_VERSION macro will expand to 0x010203.
///
/// You can use QXMPP_VERSION to use the latest QXmpp features where
/// available.
///

#define QXMPP_VERSION 0x000803

QXMPP_EXPORT QString QXmppVersion();

#endif //QXMPPGLOBAL_H
