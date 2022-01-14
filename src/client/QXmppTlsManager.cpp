// SPDX-FileCopyrightText: 2019 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppClient.h"
#include "QXmppClient_p.h"
#include "QXmppConstants_p.h"
#include "QXmppOutgoingClient.h"
#include "QXmppStartTlsPacket.h"
#include "QXmppStreamFeatures.h"
#include "QXmppTlsManager_p.h"

#include <QDomElement>
#include <QSslSocket>

/// \cond
QXmppTlsManager::QXmppTlsManager() = default;

bool QXmppTlsManager::handleStanza(const QDomElement &stanza)
{
    if (QXmppStreamFeatures::isStreamFeatures(stanza) && !clientStream()->socket()->isEncrypted()) {
        QXmppStreamFeatures features;
        features.parse(stanza);

        // determine TLS mode to use
        const QXmppConfiguration::StreamSecurityMode localSecurity = client()->configuration().streamSecurityMode();
        const QXmppStreamFeatures::Mode remoteSecurity = features.tlsMode();
        if (!clientStream()->socket()->supportsSsl() &&
            (localSecurity == QXmppConfiguration::TLSRequired ||
             remoteSecurity == QXmppStreamFeatures::Required)) {
            warning("Disconnecting since TLS is required, but SSL support is not available");
            client()->disconnectFromServer();
            return true;
        }
        if (localSecurity == QXmppConfiguration::TLSRequired &&
            remoteSecurity == QXmppStreamFeatures::Disabled) {
            warning("Disconnecting since TLS is required, but not supported by the server");
            client()->disconnectFromServer();
            return true;
        }

        if (clientStream()->socket()->supportsSsl() &&
            localSecurity != QXmppConfiguration::TLSDisabled &&
            remoteSecurity != QXmppStreamFeatures::Disabled) {
            // enable TLS since it is supported by both parties
            client()->sendPacket(QXmppStartTlsPacket());
            return true;
        }
    }

    if (QXmppStartTlsPacket::isStartTlsPacket(stanza, QXmppStartTlsPacket::Proceed)) {
        debug("Starting encryption");
        clientStream()->socket()->startClientEncryption();
        return true;
    }

    return false;
}
/// \endcond
