// SPDX-FileCopyrightText: 2016 Manjeet Dahiya <manjeetdahiya@gmail.com>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppCarbonManager.h"

#include "QXmppClient.h"
#include "QXmppConstants_p.h"
#include "QXmppMessage.h"
#include "QXmppUtils.h"
#include "QXmppUtils_p.h"

#include "StringLiterals.h"

#include <QDomElement>

using namespace QXmpp::Private;

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
        carbonselement.setTagName(m_carbonsEnabled ? u"enable"_s : u"disable"_s);
        carbonselement.setAttribute(u"xmlns"_s, ns_carbons.toString());

        iq.setExtensions(QXmppElementList() << carbonselement);
        client()->sendPacket(iq);
    }
}

/// \cond
QStringList QXmppCarbonManager::discoveryFeatures() const
{
    return { ns_carbons.toString() };
}

bool QXmppCarbonManager::handleStanza(const QDomElement &element)
{
    if (element.tagName() != u"message") {
        return false;
    }

    bool sent = true;
    QDomElement carbon = firstChildElement(element, u"sent", ns_carbons);
    if (carbon.isNull()) {
        carbon = firstChildElement(element, u"received", ns_carbons);
        sent = false;
    }

    if (carbon.isNull()) {
        return false;
    }

    // carbon copies must always come from our bare JID
    if (element.attribute(u"from"_s) != client()->configuration().jidBare()) {
        info(u"Received carbon copy from possible attacker trying to use CVE-2017-5603."_s);
        return false;
    }

    auto forwarded = firstChildElement(carbon, u"forwarded", ns_forwarding);
    auto messageElement = firstChildElement(forwarded, u"message", ns_client);
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
