#include "QXmppEntityTimeManager.h"

#include <QDomElement>

#include "QXmppConstants.h"
#include "QXmppOutgoingClient.h"
#include "QXmppEntityTimeIq.h"

void QXmppEntityTimeManager::requestTime(const QString& jid)
{
    QXmppEntityTimeIq request;
    request.setType(QXmppIq::Get);
    request.setFrom(client()->configuration().jid());
    request.setTo(jid);
    client()->sendPacket(request);
}

QStringList QXmppEntityTimeManager::discoveryFeatures() const
{
    return QStringList() << ns_entity_time;
}

bool QXmppEntityTimeManager::handleStanza(QXmppStream *stream, const QDomElement &element)
{
    if(element.tagName() == "iq" && QXmppEntityTimeIq::isEntityTimeIq(element))
    {
        QXmppEntityTimeIq entityTime;
        entityTime.parse(element);

        if(entityTime.type() == QXmppIq::Get)
        {
            // respond to query
            QXmppEntityTimeIq responseIq;
            responseIq.setType(QXmppIq::Result);
            responseIq.setId(entityTime.id());
            responseIq.setTo(entityTime.from());

            // TODO: set valid values
            responseIq.setTzo("");
            responseIq.setUtc("");

            stream->sendPacket(responseIq);
        }

        emit timeReceived(entityTime);
        return true;
    }

    return false;
}
