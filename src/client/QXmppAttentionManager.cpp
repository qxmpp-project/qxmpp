// SPDX-FileCopyrightText: 2020 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppAttentionManager.h"

// Qt
#include <QTimer>
// QXmpp
#include "QXmppClient.h"
#include "QXmppConstants_p.h"
#include "QXmppMessage.h"
#include "QXmppRosterManager.h"
#include "QXmppUtils.h"

///
/// \class QXmppAttentionManager
///
/// \brief The QXmppAttentionManager class manages attention requests as defined
/// by \xep{0224}: Attention.
///
/// The manager also does some checks, including rate limiting and checking
/// whether the senders are trusted (aka. in the roster).
///
/// Rate limited messages are not emitted on the normal attentionRequested()
/// signal and are sent on the attentionRequestRateLimited() signal instead.
///
/// To use this manager you still need to instantiate it and register it with
/// the QXmppClient:
///
/// \code
/// auto *attentionManager = new QXmppAttentionManager();
/// client->addExtension(attentionManager);
/// \endcode
///
/// \since QXmpp 1.4
///

///
/// \fn QXmppAttentionManager::attentionRequested
///
/// This signal is emitted when an attention request was received and it passed
/// the rate limiter.
///
/// \param message The message with the attention request that was received.
/// \param isTrusted Whether the sender of the message exists in the user's
/// roster.
///

///
/// \fn QXmppAttentionManager::attentionRequestRateLimited
///
/// This signal is emitted when an attention request did not pass the rate
/// limiter.
///
/// \param message The message with the attention request that did not pass the
/// rate limiter.
///

struct PastRequest {
    QString bareJid;
    QDateTime timestamp;
};

class QXmppAttentionManagerPrivate : public QObject
{
public:
    QXmppAttentionManagerPrivate(QXmppAttentionManager *parent, quint8 allowedAttempts, QTime timeFrame);

    bool checkRateLimit(const QString &bareJid);
    void cleanUp();

    quint8 allowedAttempts;
    QTime allowedAttemptsTimeInterval;

    // map of bare JIDs and time of previous requests
    QVector<PastRequest> previousRequests;
    QTimer *cleanUpTimer;
};

///
/// \brief QXmppAttentionManager::QXmppAttentionManager
/// \param allowedAttempts
/// \param timeFrame
///
QXmppAttentionManager::QXmppAttentionManager(quint8 allowedAttempts, QTime timeFrame)
    : d(new QXmppAttentionManagerPrivate(this, allowedAttempts, timeFrame))
{
}

QXmppAttentionManager::~QXmppAttentionManager() = default;

///
/// Returns the \xep{0224}: Attention feature.
///
QStringList QXmppAttentionManager::discoveryFeatures() const
{
    return {
        ns_attention.toString(),
    };
}

///
/// Returns the number of allowed attempts of attentions from a bare JID in the
/// set time frame.
///
/// \sa setAllowedAttempts()
/// \sa allowedAttemptsTimeInterval()
/// \sa setAllowedAttemptsTimeInterval()
///
quint8 QXmppAttentionManager::allowedAttempts() const
{
    return d->allowedAttempts;
}

///
/// Sets the number of allowed attempts of attentions from a bare JID in the set
/// time frame.
///
/// \sa allowedAttempts()
/// \sa allowedAttemptsTimeInterval()
/// \sa setAllowedAttemptsTimeInterval()
///
void QXmppAttentionManager::setAllowedAttempts(quint8 allowedAttempts)
{
    d->allowedAttempts = allowedAttempts;
}

///
/// Returns the time interval for the allowed attempts for rate limiting.
///
/// \sa setAllowedAttemptsTimeInterval()
/// \sa allowedAttempts()
/// \sa setAllowedAttemptsTimeInterval()
///
QTime QXmppAttentionManager::allowedAttemptsTimeInterval() const
{
    return d->allowedAttemptsTimeInterval;
}

///
/// Returns the time interval for the allowed attempts for rate limiting.
///
/// \sa allowedAttemptsTimeInterval()
/// \sa allowedAttempts()
/// \sa setAllowedAttempts()
///
void QXmppAttentionManager::setAllowedAttemptsTimeInterval(QTime interval)
{
    d->allowedAttemptsTimeInterval = interval;
}

///
/// Sends a message of type chat with an attention request to the specified JID.
///
/// \xep{0224} allows to include other elements with an attention request, but
/// the QXmppAttentionManager has no method for this purpose. However, such a
/// request can still be made manually.
///
/// \param jid The address to which the request should be sent.
/// \param message The message body to include in the attention request.
///
/// \return The ID of the sent message, if sent successfully, a null string
/// otherwise. In case an ID is returned, it also corresponds to the origin ID
/// of the message as defined by \xep{0359}: Unique and Stable Stanza IDs.
///
QString QXmppAttentionManager::requestAttention(const QString &jid, const QString &message)
{
    QXmppMessage msg;
    // The XEP recommends to use type headline, but the message body might still
    // be of interest later, so we use type chat to allow caching.
    msg.setType(QXmppMessage::Chat);
    msg.setId(QXmppUtils::generateStanzaUuid());
    msg.setOriginId(msg.id());
    msg.setTo(jid);
    msg.setBody(message);
    msg.setAttentionRequested(true);

    if (client()->sendPacket(msg)) {
        return msg.id();
    }
    return {};
}

void QXmppAttentionManager::onRegistered(QXmppClient *client)
{
    connect(client, &QXmppClient::messageReceived,
            this, &QXmppAttentionManager::handleMessageReceived);
}

void QXmppAttentionManager::onUnregistered(QXmppClient *client)
{
    disconnect(client, &QXmppClient::messageReceived,
               this, &QXmppAttentionManager::handleMessageReceived);
}

void QXmppAttentionManager::handleMessageReceived(const QXmppMessage &message)
{
    if (!message.isAttentionRequested() || !message.stamp().isNull()) {
        return;
    }

    const QString bareJid = QXmppUtils::jidToBareJid(message.from());

    // ignore messages from our own bare JID (e.g. carbon or IM-NG message)
    if (bareJid == client()->configuration().jidBare()) {
        return;
    }

    // check rate limit
    if (!d->checkRateLimit(bareJid)) {
        Q_EMIT attentionRequestRateLimited(message);
        return;
    }

    bool isTrusted = false;
    if (auto *rosterManager = client()->findExtension<QXmppRosterManager>()) {
        isTrusted = rosterManager->getRosterBareJids().contains(bareJid);
    }

    Q_EMIT attentionRequested(message, isTrusted);
}

QXmppAttentionManagerPrivate::QXmppAttentionManagerPrivate(QXmppAttentionManager *parent, quint8 allowedAttempts, QTime timeFrame)
    : allowedAttempts(allowedAttempts),
      allowedAttemptsTimeInterval(timeFrame),
      cleanUpTimer(new QTimer(parent))
{
    QObject::connect(cleanUpTimer, &QTimer::timeout, [this]() {
        cleanUp();
    });
}

///
/// Returns true if the request passes the rate limit.
///
bool QXmppAttentionManagerPrivate::checkRateLimit(const QString &bareJid)
{
    // add to request to cache
    previousRequests << PastRequest { bareJid, QDateTime::currentDateTimeUtc() };

    // start timer to remove request again
    if (!cleanUpTimer->isActive()) {
        cleanUpTimer->start(allowedAttemptsTimeInterval.msecsSinceStartOfDay());
    }

    // check whether there are too many requests
    int count = std::count_if(previousRequests.cbegin(), previousRequests.cend(), [=](const PastRequest &request) {
        return request.bareJid == bareJid;
    });
    return count <= allowedAttempts;
}

///
/// Removes the first entry and reschedules the timer to remove the next.
///
void QXmppAttentionManagerPrivate::cleanUp()
{
    previousRequests.removeFirst();

    if (!previousRequests.isEmpty()) {
        // reschedule timer for next removal
        int next = allowedAttemptsTimeInterval.msecsSinceStartOfDay() -
            previousRequests.first().timestamp.msecsTo(QDateTime::currentDateTimeUtc());

        if (next < 1) {
            cleanUp();
        } else {
            cleanUpTimer->start(next);
        }
    }
}
