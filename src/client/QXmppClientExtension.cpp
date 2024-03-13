// SPDX-FileCopyrightText: 2010 Jeremy Lainé <jeremy.laine@m4x.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppClientExtension.h"

#include "QXmppClient.h"

///
/// Constructs a QXmppClient extension.
///
QXmppClientExtension::QXmppClientExtension()
    : m_client(nullptr)
{
}

QXmppClientExtension::~QXmppClientExtension() = default;

///
/// Returns the discovery features to add to the client.
///
QStringList QXmppClientExtension::discoveryFeatures() const
{
    return QStringList();
}

///
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
/// \param stanza The DOM element to be handled.
/// \param e2eeMetadata If the element has been decrypted this contains metadata
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

///
/// Returns the client which loaded this extension.
///
QXmppClient *QXmppClientExtension::client()
{
    return m_client;
}

///
/// Sets the client which loaded this extension.
///
/// \param client
///
void QXmppClientExtension::setClient(QXmppClient *client)
{
    if (m_client != nullptr) {
        onUnregistered(m_client);
    }

    m_client = client;

    if (client != nullptr) {
        onRegistered(client);
    }
}

///
/// Called after the extension has been added to a QXmppClient.
///
/// \param client
///
void QXmppClientExtension::onRegistered(QXmppClient *client)
{
    Q_UNUSED(client);
}

///
/// Called after the extension has been removed from a QXmppClient.
///
/// \param client
///
void QXmppClientExtension::onUnregistered(QXmppClient *client)
{
    Q_UNUSED(client);
}

///
/// Injects an IQ element into the client.
///
/// The IQ is handled like any other stanza received via the XMPP stream.
///
/// \param element
/// \param e2eeMetadata End-to-end encryption metadata for the IQ. Should
/// be set if the stanza has been decrypted with an end-to-end encryption.
///
/// \since QXmpp 1.5
///
void QXmppClientExtension::injectIq(const QDomElement &element, const std::optional<QXmppE2eeMetadata> &e2eeMetadata)
{
    client()->injectIq(element, e2eeMetadata);
}

///
/// Injects a message stanza into the client.
///
/// The stanza is processed by the client with all extensions implementing
/// MessageHandler.
///
/// \since QXmpp 1.5
///
bool QXmppClientExtension::injectMessage(QXmppMessage &&message)
{
    return client()->injectMessage(std::move(message));
}
