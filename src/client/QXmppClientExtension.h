// SPDX-FileCopyrightText: 2010 Jeremy Lainé <jeremy.laine@m4x.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPCLIENTEXTENSION_H
#define QXMPPCLIENTEXTENSION_H

#include "QXmppDiscoveryIq.h"
#include "QXmppExtension.h"
#include "QXmppLogger.h"

#include <memory>

class QDomElement;

class QXmppClient;
class QXmppClientExtensionPrivate;
class QXmppMessage;
class QXmppStream;

///
/// \brief The QXmppClientExtension class is the base class for QXmppClient
/// extensions.
///
/// If you want to extend QXmppClient, for instance to support an IQ type
/// which is not natively supported, you can subclass QXmppClientExtension
/// and implement handleStanza(). You can then add your extension to the
/// client instance using QXmppClient::addExtension().
///
/// \ingroup Core
///
class QXMPP_EXPORT QXmppClientExtension : public QXmppLoggable, public QXmppExtension
{
    Q_OBJECT

public:
    QXmppClientExtension();
    ~QXmppClientExtension() override;

    virtual QStringList discoveryFeatures() const;
    virtual QList<QXmppDiscoveryIq::Identity> discoveryIdentities() const;

    virtual bool handleStanza(const QDomElement &stanza);
    virtual bool handleStanza(const QDomElement &stanza, const std::optional<QXmppE2eeMetadata> &e2eeMetadata);

protected:
    QXmppClient *client();
    virtual void setClient(QXmppClient *client);

    void injectIq(const QDomElement &element, const std::optional<QXmppE2eeMetadata> &e2eeMetadata);
    bool injectMessage(QXmppMessage &&message);

private:
    // m_client can be replaced with a d-ptr if needed (same size)
    QXmppClient *m_client;

    friend class QXmppClient;
};

#endif
