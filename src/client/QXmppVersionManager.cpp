// SPDX-FileCopyrightText: 2010 Manjeet Dahiya <manjeetdahiya@gmail.com>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppVersionManager.h"

#include "QXmppClient.h"
#include "QXmppConstants_p.h"
#include "QXmppIqHandling.h"
#include "QXmppVersionIq.h"

#include "StringLiterals.h"

#include <QCoreApplication>
#include <QDomElement>
#include <QSysInfo>

using namespace QXmpp;

class QXmppVersionManagerPrivate
{
public:
    QString clientName;
    QString clientVersion;
    QString clientOs;
};

QXmppVersionManager::QXmppVersionManager()
    : d(std::make_unique<QXmppVersionManagerPrivate>())
{
    d->clientName = qApp->applicationName();
    if (d->clientName.isEmpty()) {
        d->clientName = u"Based on QXmpp"_s;
    }

    d->clientOs = QSysInfo::prettyProductName();
    d->clientVersion = qApp->applicationVersion();
    if (d->clientVersion.isEmpty()) {
        d->clientVersion = QXmppVersion();
    }
}

QXmppVersionManager::~QXmppVersionManager() = default;

/// Request version information from the specified XMPP entity.
QString QXmppVersionManager::requestVersion(const QString &jid)
{
    QXmppVersionIq request;
    request.setType(QXmppIq::Get);
    request.setTo(jid);
    if (client()->sendPacket(request)) {
        return request.id();
    } else {
        return QString();
    }
}

/// Sets the local XMPP client's name.
void QXmppVersionManager::setClientName(const QString &name)
{
    d->clientName = name;
}

/// Sets the local XMPP client's version.
void QXmppVersionManager::setClientVersion(const QString &version)
{
    d->clientVersion = version;
}

/// Sets the local XMPP client's operating system.
void QXmppVersionManager::setClientOs(const QString &os)
{
    d->clientOs = os;
}

///
/// Returns the local XMPP client's name.
///
/// By default this is set to the QApplication::applicationName(), or
/// "Based on QXmpp" if not specified.
///
QString QXmppVersionManager::clientName() const
{
    return d->clientName;
}

///
/// Returns the local XMPP client's version.
///
/// By default this is set to QApplication::applicationVersion(), or
/// QXmpp's version if not specified.
///
QString QXmppVersionManager::clientVersion() const
{
    return d->clientVersion;
}

///
/// Returns the local XMPP client's operating system.
///
/// By default this equals to QSysInfo::prettyProductName() which contains the
/// OS name and version (e.g. "Windows 8.1" or "Debian GNU/Linux buster").
///
QString QXmppVersionManager::clientOs() const
{
    return d->clientOs;
}

/// \cond
QStringList QXmppVersionManager::discoveryFeatures() const
{
    return {
        // XEP-0092: Software Version
        ns_version.toString(),
    };
}

bool QXmppVersionManager::handleStanza(const QDomElement &element)
{
    if (QXmpp::handleIqRequests<QXmppVersionIq>(element, client(), this)) {
        return true;
    }

    if (element.tagName() == u"iq" && QXmppVersionIq::isVersionIq(element)) {
        QXmppVersionIq versionIq;
        versionIq.parse(element);

        if (versionIq.type() == QXmppIq::Result) {
            // emit response
            Q_EMIT versionReceived(versionIq);
        }

        return true;
    }

    return false;
}

QXmppVersionIq QXmppVersionManager::handleIq(QXmppVersionIq &&iq)
{
    QXmppVersionIq response;
    response.setType(QXmppIq::Result);
    response.setName(clientName());
    response.setVersion(clientVersion());
    response.setOs(clientOs());
    return response;
}
/// \endcond
