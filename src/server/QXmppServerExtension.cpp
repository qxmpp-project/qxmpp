// SPDX-FileCopyrightText: 2010 Jeremy Lainé <jeremy.laine@m4x.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppServerExtension.h"

#include "QXmppLogger.h"
#include "QXmppServer.h"

#include <QDomElement>
#include <QMetaClassInfo>
#include <QStringList>

class QXmppServerExtensionPrivate
{
public:
    QXmppServer *server;
};

QXmppServerExtension::QXmppServerExtension()
    : d(new QXmppServerExtensionPrivate)
{
    d->server = nullptr;
}

QXmppServerExtension::~QXmppServerExtension()
{
    delete d;
}

/// Returns the discovery features to add to the server.
///

QStringList QXmppServerExtension::discoveryFeatures() const
{
    return QStringList();
}

/// Returns the discovery items to add to the server.
///

QStringList QXmppServerExtension::discoveryItems() const
{
    return QStringList();
}

/// Returns the extension's name.
///

QString QXmppServerExtension::extensionName() const
{
    int index = metaObject()->indexOfClassInfo("ExtensionName");
    if (index < 0)
        return QString();
    const char *name = metaObject()->classInfo(index).value();
    return QString::fromLatin1(name);
}

/// Returns the extension's priority.
///
/// Higher priority extensions are called first when handling
/// incoming stanzas.
///
/// The default implementation returns 0.

int QXmppServerExtension::extensionPriority() const
{
    return 0;
}

/// Handles an incoming XMPP stanza.
///
/// Return true if no further processing should occur, false otherwise.
///
/// \param stanza The received stanza.

bool QXmppServerExtension::handleStanza(const QDomElement &stanza)
{
    Q_UNUSED(stanza);
    return false;
}

/// Returns the list of subscribers for the given JID.
///
/// \param jid

QSet<QString> QXmppServerExtension::presenceSubscribers(const QString &jid)
{
    Q_UNUSED(jid);
    return QSet<QString>();
}

/// Returns the list of subscriptions for the given JID.
///
/// \param jid

QSet<QString> QXmppServerExtension::presenceSubscriptions(const QString &jid)
{
    Q_UNUSED(jid);
    return QSet<QString>();
}

/// Starts the extension.
///
/// Return true if the extension was started, false otherwise.

bool QXmppServerExtension::start()
{
    return true;
}

/// Stops the extension.

void QXmppServerExtension::stop()
{
}

/// Returns the server which loaded this extension.

QXmppServer *QXmppServerExtension::server() const
{
    return d->server;
}

/// Sets the server which loaded this extension.
///
/// \param server

void QXmppServerExtension::setServer(QXmppServer *server)
{
    d->server = server;
}
