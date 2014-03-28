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

#include <QStringList>

#include "QXmppClientExtension.h"

class QXmppClientExtensionPrivate
{
public:
    QXmppClient *client;
};

/// Constructs a QXmppClient extension.
///

QXmppClientExtension::QXmppClientExtension()
    : d(new QXmppClientExtensionPrivate)
{
    d->client = 0;
}

/// Destroys a QXmppClient extension.
///

QXmppClientExtension::~QXmppClientExtension()
{
    delete d;
}

/// Returns the discovery features to add to the client.
///

QStringList QXmppClientExtension::discoveryFeatures() const
{
    return QStringList();
}

/// Returns the discovery identities to add to the client.
///

QList<QXmppDiscoveryIq::Identity> QXmppClientExtension::discoveryIdentities() const
{
    return QList<QXmppDiscoveryIq::Identity>();
}

/// Returns the client which loaded this extension.
///

QXmppClient *QXmppClientExtension::client()
{
    return d->client;
}

/// Sets the client which loaded this extension.
///
/// \param client

void QXmppClientExtension::setClient(QXmppClient *client)
{
    d->client = client;
}

