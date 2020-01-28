/*
 * Copyright (C) 2008-2020 The QXmpp developers
 *
 * Authors:
 *  Linus Jahn
 *
 * Source:
 *  https://github.com/qxmpp-project/qxmpp
 *
 * This file is a part of QXmpp library.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 */

#include "QXmppTlsManager.h"

#include "QXmppConstants_p.h"
#include "QXmppClient.h"
#include "QXmppStreamFeatures.h"
#include "QXmppStartTlsPacket.h"

#include <QDomElement>

QXmppTlsManager::QXmppTlsManager() = default;

bool QXmppTlsManager::handleStanza(const QDomElement &stanza)
{
    if (QXmppStreamFeatures::isStreamFeatures(stanza) && !client()->isSocketEncrypted()) {
        QXmppStreamFeatures features;
        features.parse(stanza);

        // determine TLS mode to use
        const QXmppConfiguration::StreamSecurityMode localSecurity = client()->configuration().streamSecurityMode();
        const QXmppStreamFeatures::Mode remoteSecurity = features.tlsMode();
        if (!client()->socketSupportsSsl() &&
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

        if (client()->socketSupportsSsl() &&
            localSecurity != QXmppConfiguration::TLSDisabled &&
            remoteSecurity != QXmppStreamFeatures::Disabled) {
            // enable TLS since it is supported by both parties
            client()->sendPacket(QXmppStartTlsPacket());
            return true;
        }
    }

    if (QXmppStartTlsPacket::isStartTlsPacket(stanza, QXmppStartTlsPacket::Proceed)) {
        debug("Starting encryption");
        client()->startSocketEncryption();
        return true;
    }

    return false;
}
