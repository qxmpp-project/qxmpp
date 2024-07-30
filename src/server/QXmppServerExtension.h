// SPDX-FileCopyrightText: 2010 Jeremy Lainé <jeremy.laine@m4x.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPSERVEREXTENSION_H
#define QXMPPSERVEREXTENSION_H

#include "QXmppLogger.h"

#include <QVariant>

class QDomElement;

class QXmppServer;
class QXmppServerExtensionPrivate;

///
/// \brief The QXmppServerExtension class is the base class for QXmppServer
/// extensions.
///
/// If you want to extend QXmppServer, for instance to support an IQ type
/// which is not natively supported, you can subclass QXmppServerExtension
/// and implement handleStanza(). You can then add your extension to the
/// client instance using QXmppServer::addExtension().
///
/// \ingroup Core
///
class QXMPP_EXPORT QXmppServerExtension : public QXmppLoggable
{
    Q_OBJECT

public:
    QXmppServerExtension();
    ~QXmppServerExtension() override;
    virtual QString extensionName() const;
    virtual int extensionPriority() const;

    virtual QStringList discoveryFeatures() const;
    virtual QStringList discoveryItems() const;
    virtual bool handleStanza(const QDomElement &stanza);
    virtual QSet<QString> presenceSubscribers(const QString &jid);
    virtual QSet<QString> presenceSubscriptions(const QString &jid);

    virtual bool start();
    virtual void stop();

protected:
    QXmppServer *server() const;

private:
    void setServer(QXmppServer *server);
    const std::unique_ptr<QXmppServerExtensionPrivate> d;

    friend class QXmppServer;
};

#endif
