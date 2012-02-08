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

#ifndef QXMPPSERVERPLUGIN_H
#define QXMPPSERVERPLUGIN_H

#include <QtPlugin>

class QXmppServer;
class QXmppServerExtension;

class QXmppServerPluginInterface
{
public:
    virtual QXmppServerExtension *create(const QString &key) = 0;
    virtual QStringList keys() const = 0;
};

Q_DECLARE_INTERFACE(QXmppServerPluginInterface, "com.googlecode.qxmpp.ServerPlugin/1.0")

/// \brief The QXmppServerPlugin class is the base class for QXmppServer plugins.
///

class QXmppServerPlugin : public QObject, public QXmppServerPluginInterface
{
    Q_OBJECT
    Q_INTERFACES(QXmppServerPluginInterface)

public:
    /// Creates and returns the specified QXmppServerExtension.
    ///
    /// \param key The key for the QXmppServerExtension.
    virtual QXmppServerExtension *create(const QString &key) = 0;

    /// Returns the list of keys supported by this plugin.
    ///
    virtual QStringList keys() const = 0;
};

#endif
