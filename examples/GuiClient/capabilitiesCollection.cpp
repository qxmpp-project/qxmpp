#include "capabilitiesCollection.h"

#include "QXmppClient.h"
#include "QXmppDiscoveryManager.h"
#include <QXmlStreamWriter>

capabilitiesCollection::capabilitiesCollection(QXmppClient* client) :
    QObject(client), m_client(client)
{
    QXmppDiscoveryManager* ext = m_client->findExtension<QXmppDiscoveryManager>();
    if(ext)
    {
        bool check = connect(ext, SIGNAL(infoReceived(const QXmppDiscoveryIq&)),
                             SLOT(infoReceived(const QXmppDiscoveryIq&)));
        Q_ASSERT(check);
    }
}

bool capabilitiesCollection::isCapabilityAvailable(const QString& nodeVer)
{
    return m_mapCapabilities.contains(nodeVer);
}

void capabilitiesCollection::requestInfo(const QString& jid, const QString& node)
{
    QXmppDiscoveryManager* ext = m_client->findExtension<QXmppDiscoveryManager>();
    if(ext)
    {
        ext->requestInfo(jid, node);
    }
}

void capabilitiesCollection::infoReceived(const QXmppDiscoveryIq& discoIqRcv)
{
    QXmppDiscoveryIq discoIq = discoIqRcv;
    if(discoIq.queryType() == QXmppDiscoveryIq::InfoQuery &&
       discoIq.type() == QXmppIq::Result)
    {
        if(!discoIq.queryNode().isEmpty())
        {
            discoIq.setTo("");
            discoIq.setFrom("");
            discoIq.setId("");
            m_mapCapabilities[discoIq.queryNode()] = discoIq;
        }
    }
}
