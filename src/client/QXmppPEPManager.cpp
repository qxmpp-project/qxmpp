#include "QXmppPEPManager.h"



#include <QDomElement>

#include "QXmppClient.h"
#include "QXmppConstants.h"
#include "QXmppReachAddress.h"
#include "QXmppMessage.h"
#include "QXmppPubSubIq.h"

QXmppPEPManager::QXmppPEPManager()
    : QXmppClientExtension()
    , m_reachActive(false)

{

}

QXmppPEPManager::QXmppPEPManager(const bool reachActive)
    : QXmppClientExtension()
    , m_reachActive(reachActive)
{

}

/// \cond
QStringList QXmppPEPManager::discoveryFeatures() const
{
    QStringList features;

    // XEP-0152: Reachability Addresses
    if(m_reachActive)
        features << ns_reach;

    return features;
}

bool QXmppPEPManager::handleStanza(const QDomElement &stanza)
{
    bool isIq = (stanza.tagName() == "iq");
    if (!isIq && (stanza.tagName() != "message")) {
        return false;
    }

    // XEP-0163: Personal Eventing Protocol
    QDomElement pepElement = stanza.firstChildElement("event");

    if(!pepElement.isNull() && pepElement.namespaceURI() == ns_personal_eventing_protocol)
    {
        QXmppMessage message;
        message.parse(stanza);

        QDomElement itemsElement = pepElement.firstChildElement("items");
        QString nodeType = itemsElement.attribute("node");

        // XEP-0152: Reachability Addresses
        if(nodeType == ns_reach)
        {
            QDomElement itemElement = itemsElement.firstChildElement("item");
            if(!itemElement.isNull())
            {
                QString itemId = itemElement.attribute("id");
                QDomElement reachElement = itemElement.firstChildElement("reach");

                QXmppReachAddress reachAddress;
                reachAddress.parse(reachElement);

                if(!reachAddress.isNull())
                    emit reachabilityAddressReceived(message.from(), itemId, reachAddress);

                return true;
            }
        }
    }
    return false;
}
/// \endcond

