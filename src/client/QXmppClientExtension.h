// SPDX-FileCopyrightText: 2010 Jeremy Lainé <jeremy.laine@m4x.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPCLIENTEXTENSION_H
#define QXMPPCLIENTEXTENSION_H

#include "QXmppDiscoveryIq.h"
#include "QXmppLogger.h"

class QDomElement;

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
    ~QXmppClientExtension() override;

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
    QXmppClientExtensionPrivate *const d;

    friend class QXmppClient;
};

#endif
