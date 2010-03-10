#include "QXmppInformationRequestResult.h"
#include "QXmppConstants.h"
#include <QXmlStreamWriter>

QXmppInformationRequestResult::QXmppInformationRequestResult()
{
    setType(QXmppIq::Result);
    setQueryType(QXmppDiscoveryIq::InfoQuery);

    QStringList features;
    features
        << ns_rpc               // XEP-0009: Jabber-RPC
        << ns_disco_info        // XEP-0030: Service Discovery
        << ns_ibb               // XEP-0047: In-Band Bytestreams
        << ns_vcard             // XEP-0054: vcard-temp
        << ns_bytestreams       // XEP-0065: SOCKS5 Bytestreams
        << ns_chat_states       // XEP-0085: Chat State Notifications
        << ns_version           // XEP-0092: Software Version
        << ns_stream_initiation // XEP-0095: Stream Initiation
        << ns_stream_initiation_file_transfer // XEP-0096: SI File Transfer
        << ns_ping;             // XEP-0199: XMPP Ping

    // build query items
    QList<QXmppElement> queryItems;
    foreach (const QString &var, features)
    {
        QXmppElement feature;
        feature.setTagName("feature");
        feature.setAttribute("var", var);
        queryItems.append(feature);
    }

    QXmppElement identity;
    identity.setTagName("identity");
    identity.setAttribute("category", "automation");
    identity.setAttribute("type", "rpc");
    queryItems.append(identity);

    setQueryItems(queryItems);
}
