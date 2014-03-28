/*
 * Copyright (C) 2008-2014 The QXmpp developers
 *
 * Author:
 *  Jeremy Lainé
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

#ifndef QXMPPCLIENTEXTENSION_H
#define QXMPPCLIENTEXTENSION_H

#include "QXmppDiscoveryIq.h"
#include "QXmppLogger.h"

class QDomElement;
class QStringList;

class QXmppClient;
class QXmppClientExtensionPrivate;
class QXmppStream;

/// \brief The QXmppClientExtension class is the base class for QXmppClient
/// extensions.
///
/// If you want to extend QXmppClient, for instance to support an IQ type
/// which is not natively supported, you can subclass QXmppClientExtension
/// and implement handleStanza(). You can then add your extension to the
/// client instance using QXmppClient::addExtension().
///
/// \ingroup Core

class QXMPP_EXPORT QXmppClientExtension : public QXmppLoggable
{
    Q_OBJECT

public:
    QXmppClientExtension();
    virtual ~QXmppClientExtension();

    virtual QStringList discoveryFeatures() const;
    virtual QList<QXmppDiscoveryIq::Identity> discoveryIdentities() const;

    /// \brief You need to implement this method to process incoming XMPP
    /// stanzas.
    ///
    /// You should return true if the stanza was handled and no further
    /// processing should occur, or false to let other extensions process
    /// the stanza.
    virtual bool handleStanza(const QDomElement &stanza) = 0;

protected:
    QXmppClient *client();
    virtual void setClient(QXmppClient *client);

private:
    QXmppClientExtensionPrivate * const d;

    friend class QXmppClient;
};

#endif
