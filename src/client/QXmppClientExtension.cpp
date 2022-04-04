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

///
/// \brief You need to implement this method to process incoming XMPP
/// stanzas.
///
/// You should return true if the stanza was handled and no further
/// processing should occur, or false to let other extensions process
/// the stanza.
///
/// \deprecated This is deprecated since QXmpp 1.5. Please use
/// QXmppClientExtension::handleStanza(const QDomElement &stanza,
/// const std::optional<QXmppE2eeMetadata> &e2eeMetadata).
/// Currently both methods are called by the client, so only implement one!
///
bool QXmppClientExtension::handleStanza(const QDomElement &)
{
    return false;
}

///
/// \brief You need to implement this method to process incoming XMPP
/// stanzas.
///
/// \param stanza The DOM element to be handled.
/// \param e2eeMetadata If the element has been decrypted this contains metdata
/// about the encryption.
///
/// \return You should return true if the stanza was handled and no further
/// processing should occur, or false to let other extensions process the
/// stanza.
///
/// \since QXmpp 1.5
///
bool QXmppClientExtension::handleStanza(const QDomElement &, const std::optional<QXmppE2eeMetadata> &)
{
    return false;
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
