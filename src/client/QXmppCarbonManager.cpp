// SPDX-FileCopyrightText: 2016 Manjeet Dahiya <manjeetdahiya@gmail.com>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppCarbonManager.h"

#include "QXmppClient.h"
#include "QXmppConstants_p.h"
#include "QXmppDiscoveryManager.h"
#include "QXmppMessage.h"
#include "QXmppUtils.h"

#include <QDomElement>

QXmppCarbonManager::QXmppCarbonManager()
    : m_carbonsEnabled(false)
{
}

QXmppCarbonManager::~QXmppCarbonManager()
{
}

///
/// Returns whether message carbons are currently enabled
///
bool QXmppCarbonManager::carbonsEnabled() const
{
    return m_carbonsEnabled;
}

///
/// Enables or disables message carbons for this connection.
///
/// This function does not check whether the server supports
/// message carbons, but just sends the corresponding stanza
/// to the server, so one must check in advance by using the
/// discovery manager.
///
/// By default, carbon copies are disabled.
///
void QXmppCarbonManager::setCarbonsEnabled(bool enabled)
{
    if (m_carbonsEnabled == enabled) {
        return;
    }

    m_carbonsEnabled = enabled;

    if (client()) {
        QXmppIq iq(QXmppIq::Set);
        QXmppElement carbonselement;
        carbonselement.setTagName(m_carbonsEnabled ? "enable" : "disable");
        carbonselement.setAttribute("xmlns", ns_carbons);

        iq.setExtensions(QXmppElementList() << carbonselement);
        client()->sendPacket(iq);
    }
}

/// \cond
QStringList QXmppCarbonManager::discoveryFeatures() const
{
    return QStringList() << ns_carbons;
}

bool QXmppCarbonManager::handleStanza(const QDomElement &element)
{
    if (element.tagName() != "message") {
        return false;
    }

    bool sent = true;
    QDomElement carbon = element.firstChildElement("sent");
    if (carbon.isNull()) {
        carbon = element.firstChildElement("received");
        sent = false;
    }

    if (carbon.isNull() || carbon.namespaceURI() != ns_carbons) {
        return false;
    }

    // carbon copies must always come from our bare JID
    if (element.attribute("from") != client()->configuration().jidBare()) {
        info("Received carbon copy from possible attacker trying to use CVE-2017-5603.");
        return false;
    }

    auto forwarded = carbon.firstChildElement("forwarded");
    auto messageElement = forwarded.firstChildElement("message");
    if (messageElement.isNull()) {
        return false;
    }

    QXmppMessage message;
    message.parse(messageElement);
    message.setCarbonForwarded(true);

    if (sent) {
        Q_EMIT messageSent(message);
    } else {
        Q_EMIT messageReceived(message);
    }

    return true;
}
/// \endcond

///
/// \fn QXmppCarbonManager::messageReceived()
///
/// Emitted when a message was received from someone else and directed to
/// another resource.
///
/// If you connect this signal to the QXmppClient::messageReceived signal, they
/// will appear as normal messages.
///

///
/// \fn QXmppCarbonManager::messageSent()
///
/// Emitted when another resource sent a message to someone else.
///
