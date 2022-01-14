// SPDX-FileCopyrightText: 2010 Jeremy Lainé <jeremy.laine@m4x.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppClientExtension.h"

#include <QStringList>

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
    d->client = nullptr;
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
