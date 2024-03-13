// SPDX-FileCopyrightText: 2009 Manjeet Dahiya <manjeetdahiya@gmail.com>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppClient.h"
#include "QXmppRosterManager.h"
#include "QXmppVCardManager.h"
#include "QXmppVersionManager.h"

///
/// \brief You need to implement this method to process incoming XMPP
/// stanzas.
///
/// You should return true if the stanza was handled and no further
/// processing should occur, or false to let other extensions process
/// the stanza.
///
/// End-to-end encrypted stanzas are not passed to this overload, for that
/// purpose use the new overload instead.
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
/// Returns the reference to QXmppRosterManager object of the client.
///
/// \return Reference to the roster object of the connected client. Use this to
/// get the list of friends in the roster and their presence information.
///
/// \deprecated This method is deprecated since QXmpp 1.1. Use
/// \c QXmppClient::findExtension<QXmppRosterManager>() instead.
///
QXmppRosterManager &QXmppClient::rosterManager()
{
    return *findExtension<QXmppRosterManager>();
}

///
/// Returns the reference to QXmppVCardManager, implementation of \xep{0054}.
///
/// \deprecated This method is deprecated since QXmpp 1.1. Use
/// \c QXmppClient::findExtension<QXmppVCardManager>() instead.
///
QXmppVCardManager &QXmppClient::vCardManager()
{
    return *findExtension<QXmppVCardManager>();
}

///
/// Returns the reference to QXmppVersionManager, implementation of \xep{0092}.
///
/// \deprecated This method is deprecated since QXmpp 1.1. Use
/// \c QXmppClient::findExtension<QXmppVersionManager>() instead.
///
QXmppVersionManager &QXmppClient::versionManager()
{
    return *findExtension<QXmppVersionManager>();
}
