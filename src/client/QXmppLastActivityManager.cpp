#include <QDomElement>

#include "QXmppClient.h"
#include "QXmppConstants.h"
#include "QXmppLastActivityIq.h"
#include "QXmppLastActivityManager.h"

QXmppLastActivityManager::QXmppLastActivityManager()
{
}

QXmppLastActivityManager::~QXmppLastActivityManager()
{
}

QString QXmppLastActivityManager::requestLastActivity(const QString& to)
{
    QXmppLastActivityIq request(to);
    if(client()->sendPacket(request))
        return request.id();
    else
        return QString();
}

QStringList QXmppLastActivityManager::requestLastActivityList(const QStringList& list)
{
    QStringList ids;
    foreach(const QString& to, list)
        ids << requestLastActivity(to);

    return ids;
}

QStringList QXmppLastActivityManager::discoveryFeatures() const
{
    // XEP-0012: Last Activity
    return QStringList() << ns_last_activity;
}

bool QXmppLastActivityManager::handleStanza(const QDomElement& element)
{
    if (element.tagName() == "iq" && QXmppLastActivityIq::isLastActivityIq(element))
    {
        QXmppLastActivityIq lastActivityIq;
        lastActivityIq.parse(element);

        if (lastActivityIq.type() == QXmppIq::Get) {
            // respond to query
            client()->sendPacket(lastActivityIq);
        } else if (lastActivityIq.type() == QXmppIq::Result) {
            // emit response
            emit lastActivityReceived(lastActivityIq);
        }

        return true;
    }

    return false;
}


