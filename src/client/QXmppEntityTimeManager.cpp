// SPDX-FileCopyrightText: 2010 Manjeet Dahiya <manjeetdahiya@gmail.com>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppEntityTimeManager.h"

#include "QXmppClient.h"
#include "QXmppConstants_p.h"
#include "QXmppEntityTimeIq.h"
#include "QXmppFutureUtils_p.h"
#include "QXmppIqHandling.h"
#include "QXmppUtils.h"

#include "StringLiterals.h"

#include <QDateTime>
#include <QDomElement>

using namespace QXmpp::Private;

///
/// \typedef QXmppEntityTimeManager::EntityTimeResult
///
/// Contains the requested entity time or the returned error in case of a
/// failure.
///
/// \since QXmpp 1.5
///

///
/// Request the time from an XMPP entity.
///
/// The result is emitted on the timeReceived() signal.
///
/// \param jid
///
QString QXmppEntityTimeManager::requestTime(const QString &jid)
{
    QXmppEntityTimeIq request;
    request.setType(QXmppIq::Get);
    request.setTo(jid);
    if (client()->sendPacket(request)) {
        return request.id();
    } else {
        return QString();
    }
}

///
/// Requests the time from an XMPP entity and reports it via a QFuture.
///
/// The timeReceived() signal is not emitted.
///
/// \param jid
///
/// \warning THIS API IS NOT FINALIZED YET!
///
/// \since QXmpp 1.5
///
auto QXmppEntityTimeManager::requestEntityTime(const QString &jid) -> QXmppTask<EntityTimeResult>
{
    QXmppEntityTimeIq iq;
    iq.setType(QXmppIq::Get);
    iq.setTo(jid);

    return chainIq<EntityTimeResult>(client()->sendIq(std::move(iq)), this);
}

/// \cond
QStringList QXmppEntityTimeManager::discoveryFeatures() const
{
    return { ns_entity_time.toString() };
}

bool QXmppEntityTimeManager::handleStanza(const QDomElement &element)
{
    if (QXmpp::handleIqRequests<QXmppEntityTimeIq>(element, client(), this)) {
        return true;
    }

    if (element.tagName() == u"iq" && QXmppEntityTimeIq::isEntityTimeIq(element)) {
        QXmppEntityTimeIq entityTime;
        entityTime.parse(element);
        Q_EMIT timeReceived(entityTime);
        return true;
    }

    return false;
}

std::variant<QXmppEntityTimeIq, QXmppStanza::Error> QXmppEntityTimeManager::handleIq(QXmppEntityTimeIq iq)
{
    using Err = QXmppStanza::Error;
    if (iq.type() != QXmppIq::Get) {
        return Err(Err::Cancel, Err::BadRequest, u"Only IQ requests of type 'get' allowed."_s);
    }

    QXmppEntityTimeIq responseIq;
    QDateTime currentTime = QDateTime::currentDateTime();
    QDateTime utc = currentTime.toUTC();
    responseIq.setUtc(utc);

    currentTime.setTimeSpec(Qt::UTC);
    responseIq.setTzo(utc.secsTo(currentTime));
    return responseIq;
}
/// \endcond
