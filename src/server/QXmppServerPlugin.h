// SPDX-FileCopyrightText: 2010 Jeremy Lainé <jeremy.laine@m4x.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "QXmppGlobal.h"

#include <QtPlugin>

class QXmppServer;
class QXmppServerExtension;

class QXMPP_EXPORT QXmppServerPluginInterface
{
public:
    /// Creates the server extension identified by \a key.
    virtual QXmppServerExtension *create(const QString &key) = 0;

    /// Returns a list of valid extension keys.
    virtual QStringList keys() const = 0;
};

Q_DECLARE_INTERFACE(QXmppServerPluginInterface, "com.googlecode.qxmpp.ServerPlugin/1.0")

/// \brief The QXmppServerPlugin class is the base class for QXmppServer plugins.
///

class QXMPP_EXPORT QXmppServerPlugin : public QObject, public QXmppServerPluginInterface
{
    Q_OBJECT
    Q_INTERFACES(QXmppServerPluginInterface)

public:
    /// Creates and returns the specified QXmppServerExtension.
    ///
    /// \param key The key for the QXmppServerExtension.
    QXmppServerExtension *create(const QString &key) override = 0;

    /// Returns the list of keys supported by this plugin.
    ///
    QStringList keys() const override = 0;
};
