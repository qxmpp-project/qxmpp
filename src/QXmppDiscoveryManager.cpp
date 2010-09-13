/*
 * Copyright (C) 2008-2010 The QXmpp developers
 *
 * Author:
 *  Manjeet Dahiya
 *
 * Source:
 *  http://code.google.com/p/qxmpp
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

#include "QXmppDiscoveryManager.h"

#include <QDomElement>
#include <QCoreApplication>

#include "QXmppClient.h"
#include "QXmppConstants.h"
#include "QXmppDiscoveryIq.h"
#include "QXmppStream.h"

static const QString capabilitiesNode = "http://code.google.com/p/qxmpp";

bool QXmppDiscoveryManager::handleStanza(QXmppStream *stream, const QDomElement &element)
{
    if (element.tagName() == "iq" && QXmppDiscoveryIq::isDiscoveryIq(element))
    {
        QXmppDiscoveryIq infoIq;
        infoIq.parse(element);

        if(infoIq.type() == QXmppIq::Get &&
           infoIq.queryType() == QXmppDiscoveryIq::InfoQuery &&
           (infoIq.queryNode().isEmpty() || infoIq.queryNode().startsWith(capabilitiesNode)))
        {
            // respond to query
            QXmppDiscoveryIq qxmppFeatures = capabilities();
            qxmppFeatures.setId(infoIq.id());
            qxmppFeatures.setTo(infoIq.from());
            qxmppFeatures.setQueryNode(infoIq.queryNode());
            stream->sendPacket(qxmppFeatures);
        }

        emit informationReceived(infoIq);
        return true;
    }
    return false;
}

void QXmppDiscoveryManager::requestInformation(const QString& jid)
{
}

QXmppDiscoveryIq QXmppDiscoveryManager::capabilities()
{
    QXmppDiscoveryIq iq;
    iq.setType(QXmppIq::Result);
    iq.setQueryType(QXmppDiscoveryIq::InfoQuery);

    // features
    QStringList features;
    features
        << ns_rpc               // XEP-0009: Jabber-RPC
        << ns_disco_info        // XEP-0030: Service Discovery
        << ns_ibb               // XEP-0047: In-Band Bytestreams
        << ns_vcard             // XEP-0054: vcard-temp
        << ns_bytestreams       // XEP-0065: SOCKS5 Bytestreams
        << ns_chat_states       // XEP-0085: Chat State Notifications
        << ns_stream_initiation // XEP-0095: Stream Initiation
        << ns_stream_initiation_file_transfer // XEP-0096: SI File Transfer
        << ns_capabilities      // XEP-0115 : Entity Capabilities
        << ns_jingle            // XEP-0166 : Jingle
        << ns_jingle_rtp        // XEP-0167 : Jingle RTP Sessions
        << ns_jingle_rtp_audio
        << ns_jingle_ice_udp    // XEP-0176 : Jingle ICE-UDP Transport Method
        << ns_ping;             // XEP-0199: XMPP Ping

    foreach(QXmppClientExtension* extension, client()->extensions())
    {
        if(extension)
            features << extension->discoveryFeatures();
    }

    iq.setFeatures(features);

    // identities
    QList<QXmppDiscoveryIq::Identity> identities;
    QXmppDiscoveryIq::Identity identity;

    identity.setCategory("automation");
    identity.setType("rpc");
    identities.append(identity);

    identity.setCategory("client");
    identity.setType("pc");
    identity.setName(QString("%1 %2").arg(qApp->applicationName(), qApp->applicationVersion()));
    identities.append(identity);

    iq.setIdentities(identities);
    return iq;
}
