// SPDX-FileCopyrightText: 2023 Tibor Csötönyi <work@taibsu.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppCallInviteManager.h"

#include "QXmppClient.h"
#include "QXmppConstants_p.h"
#include "QXmppMessage.h"
#include "QXmppPromise.h"
#include "QXmppUtils.h"

#include <QUuid>

using namespace QXmpp;
using CallInviteType = QXmppCallInviteElement::Type;

class QXmppCallInvitePrivate
{
public:
    QXmppCallInvitePrivate(QXmppCallInviteManager *manager)
        : manager(manager)
    {
    }

    QXmppTask<SendResult> request(QXmppCallInviteElement &&callInviteElement);

    QXmppCallInviteManager *manager;
    QString id;
    QString callPartnerJid;
    bool isAccepted { false };
};

///
/// \brief Creates a Call Invite request based on given type.
/// \param type The request type (accept, reject, retract, left).
///
QXmppTask<SendResult> QXmppCallInvitePrivate::request(QXmppCallInviteElement &&callInviteElement)
{
    callInviteElement.setId(id);
    return manager->sendMessage(callInviteElement, callPartnerJid);
}

///
/// \class QXmppCallInvite
///
/// \brief The QXmppCallInvite class holds information about the Call Invite element in the
/// current context.
///
/// \since QXmpp 1.6
///

///
/// \brief Constructs a Call Invite object.
///
QXmppCallInvite::QXmppCallInvite(QXmppCallInviteManager *manager)
    : d(new QXmppCallInvitePrivate(manager))
{
}

///
/// Creates a Call Invite element of type "accept" and sends a request containing the element.
///
QXmppTask<SendResult> QXmppCallInvite::accept()
{
    QXmppCallInviteElement callInviteElement;
    callInviteElement.setType(CallInviteType::Accept);

    return d->request(std::move(callInviteElement));
}

QXmppCallInvite::~QXmppCallInvite() = default;

///
/// Creates a Call Invite element of type "reject" and sends a request containing the element.
///
QXmppTask<SendResult> QXmppCallInvite::reject()
{
    QXmppCallInviteElement callInviteElement;
    callInviteElement.setType(CallInviteType::Reject);

    return d->request(std::move(callInviteElement));
}

///
/// Creates a Call Invite element of type "retract" and sends a request containing the element.
///
QXmppTask<SendResult> QXmppCallInvite::retract()
{
    QXmppCallInviteElement callInviteElement;
    callInviteElement.setType(CallInviteType::Retract);

    return d->request(std::move(callInviteElement));
}

///
/// Creates a Call Invite element of type "leave" and sends a request containing the element.
///
QXmppTask<SendResult> QXmppCallInvite::leave()
{
    QXmppCallInviteElement callInviteElement;
    callInviteElement.setType(CallInviteType::Left);

    return d->request(std::move(callInviteElement));
}

///
/// Creates a Call Invite element of type "invite" and sends a request containing the element.
/// \param audio True if Call Invite contains audio
/// \param video True if Call Invite contains video
/// \param jingle Possible jingle element
/// \param external Possible external elements
///
QXmppTask<SendResult> QXmppCallInvite::invite(
    bool audio,
    bool video,
    std::optional<QXmppCallInviteElement::Jingle> jingle,
    std::optional<QVector<QXmppCallInviteElement::External>> external)
{
    QXmppCallInviteElement callInviteElement;
    callInviteElement.setType(CallInviteType::Invite);
    callInviteElement.setAudio(audio);
    callInviteElement.setVideo(video);
    callInviteElement.setJingle(jingle);
    callInviteElement.setExternal(external);

    return d->request(std::move(callInviteElement));
}

///
/// Returns the Call Invite ID.
///
QString QXmppCallInvite::id() const
{
    return d->id;
}

///
/// Sets the Call Invite ID.
///
void QXmppCallInvite::setId(const QString &id)
{
    d->id = id;
}

///
/// Sets the call partner's bare JID.
///
/// Normally, the Call Invite ID would be sufficient in order to differentiate the Call Invites.
/// However, attackers pretending to be the call partner can be mitigated by caching the call
/// partner's JID.
///
/// \param callPartnerJid bare JID of the call partner
///
void QXmppCallInvite::setCallPartnerJid(const QString &callPartnerJid)
{
    d->callPartnerJid = callPartnerJid;
}

///
/// Returns the call partner's bare JID.
///
/// \return the call partner's bare JID.
///
QString QXmppCallInvite::callPartnerJid() const
{
    return d->callPartnerJid;
}

bool QXmppCallInvite::isAccepted() const
{
    return d->isAccepted;
}

void QXmppCallInvite::setIsAccepted(bool isAccepted)
{
    d->isAccepted = isAccepted;
}

///
/// \fn QXmppCallInvite::invited()
///
/// Emitted when a call invitation was sent.
///

///
/// \fn QXmppCallInvite::accepted()
///
/// Emitted when a call was accepted.
///

///
/// \fn QXmppCallInvite::closed(const Result &)
///
/// Emitted when a call was closed.
///
/// \param result close reason
///

class QXmppCallInviteManagerPrivate
{
public:
    QVector<std::shared_ptr<QXmppCallInvite>> callInvites;
};

///
/// \typedef QXmppCallInviteManager::ProposeResult
///
/// Contains Call Invite object or an error if sending the propose message failed.
///

///
/// \class QXmppCallInviteManager
///
/// \brief The QXmppCallInviteManager class makes it possible to retrieve
/// Call Invite elements as defined by \xep{0482, Call Invites}.
///
/// \ingroup Managers
///
/// \since QXmpp 1.6
///
QXmppCallInviteManager::QXmppCallInviteManager()
    : d(std::make_unique<QXmppCallInviteManagerPrivate>())
{
}

QXmppCallInviteManager::~QXmppCallInviteManager() = default;

/// \cond
QStringList QXmppCallInviteManager::discoveryFeatures() const
{
    return { ns_call_invites.toString() };
}
/// \endcond

///
/// Creates a proposal Call Invite element and passes it as a message.
///
QXmppTask<QXmppCallInviteManager::ProposeResult> QXmppCallInviteManager::invite(
    const QString &callPartnerJid,
    bool audio,
    bool video,
    std::optional<QXmppCallInviteElement::Jingle> jingle,
    std::optional<QVector<QXmppCallInviteElement::External>> external)
{
    QXmppPromise<ProposeResult> promise;

    QXmppCallInviteElement callInviteElement;
    callInviteElement.setType(CallInviteType::Invite);
    callInviteElement.setId(QXmppUtils::generateStanzaUuid());
    callInviteElement.setAudio(audio);
    callInviteElement.setVideo(video);

    if (jingle) {
        callInviteElement.setJingle(jingle);
    }

    if (external) {
        callInviteElement.setExternal(external);
    }

    sendMessage(callInviteElement, callPartnerJid).then(this, [this, promise, callPartnerJid](SendResult result) mutable {
        if (auto error = std::get_if<QXmppError>(&result)) {
            warning(u"Error sending Call Invite proposal: " + error->description);
            promise.finish(*error);
        } else {
            promise.finish(addCallInvite(callPartnerJid));
        }
    });

    return promise.task();
}

/// \cond
bool QXmppCallInviteManager::handleMessage(const QXmppMessage &message)
{
    // Call Invite messages must be of type "chat".
    if (message.type() != QXmppMessage::Chat) {
        return false;
    }

    // Only continue if the message contains a Call Invite element.
    if (auto callInviteElement = message.callInviteElement()) {
        QString senderJid {
            callInviteElement->jingle() && callInviteElement->jingle()->jid ? callInviteElement->jingle()->jid.value() : client()->configuration().jid()
        };

        if (senderJid.isEmpty()) {
            return false;
        }

        return handleCallInviteElement(std::move(*callInviteElement), senderJid);
    }

    return false;
}
/// \endcond

///
/// Lets the client send a message to user with given callPartnerJid containing the Call Invite element.
///
/// \param callInviteElement the Call Invite element to be passed
/// \param callPartnerJid bare JID of the call partner
///
QXmppTask<SendResult> QXmppCallInviteManager::sendMessage(const QXmppCallInviteElement &callInviteElement, const QString &callPartnerJid)
{
    QXmppMessage message;
    message.setTo(callPartnerJid);
    message.setCallInviteElement(callInviteElement);

    return client()->send(std::move(message));
}

///
/// Removes a Call Invite object from the Call Invites vector.
///
/// \param callInvite Call Invite object to be removed
///
void QXmppCallInviteManager::clear(const std::shared_ptr<QXmppCallInvite> &callInvite)
{
    d->callInvites.erase(
        std::remove_if(
            d->callInvites.begin(),
            d->callInvites.end(),
            [&callInvite](const auto &storedCallInvite) {
                return callInvite->id() == storedCallInvite->id() &&
                    callInvite->callPartnerJid() == storedCallInvite->callPartnerJid();
            }),
        d->callInvites.end());
}

///
/// Removes all Call Invite objects from the Call Invites vector.
///
void QXmppCallInviteManager::clearAll()
{
    d->callInvites.clear();
}

bool QXmppCallInviteManager::handleCallInviteElement(QXmppCallInviteElement &&callInviteElement, const QString &senderJid)
{
    auto callInviteElementId = callInviteElement.id();
    auto callPartnerJid = QXmppUtils::jidToBareJid(senderJid);

    // Check if there's already a Call Invite object with callInviteElementId and callPartnerJid in Call Invites vector.
    // That means that a Call Invite has already been created with given (J)IDs.
    auto itr = std::find_if(
        d->callInvites.begin(),
        d->callInvites.end(),
        [&callInviteElementId, &callPartnerJid](const auto &callInvite) {
            return callInvite->id() == callInviteElementId && callInvite->callPartnerJid() == callPartnerJid;
        });

    if (itr != d->callInvites.end()) {
        return handleExistingCallInvite(*itr, callInviteElement, QXmppUtils::jidToResource(senderJid));
    }

    if (callInviteElement.type() == CallInviteType::Invite) {
        return handleInviteCallInviteElement(callInviteElement, callPartnerJid);
    }

    return false;
}

bool QXmppCallInviteManager::handleExistingCallInvite(const std::shared_ptr<QXmppCallInvite> &existingCallInvite, const QXmppCallInviteElement &callInviteElement, const QString &callPartnerResource)
{
    switch (callInviteElement.type()) {
    case CallInviteType::Invite:
        Q_EMIT existingCallInvite->invited();
        return true;
    case CallInviteType::Accept:
        Q_EMIT existingCallInvite->accepted(callInviteElement.id(), callPartnerResource);
        existingCallInvite->setIsAccepted(true);
        return true;
    case CallInviteType::Retract:
        Q_EMIT existingCallInvite->closed(QXmppCallInvite::Retracted {});
        return true;
    case CallInviteType::Reject:
        Q_EMIT existingCallInvite->closed(QXmppCallInvite::Rejected {});
        return true;
    case CallInviteType::Left:
        existingCallInvite->leave();
        Q_EMIT existingCallInvite->closed(QXmppCallInvite::Left {});
        return true;
    default:
        return false;
    }
}

///
/// Handles a propose Call Invite element.
///
/// \param Call Invite element to be handled
/// \param callPartnerJid bare JID of the call partner
/// \return success (true) or failure
///
bool QXmppCallInviteManager::handleInviteCallInviteElement(
    const QXmppCallInviteElement &callInviteElement,
    const QString &callPartnerJid)
{
    Q_EMIT invited(addCallInvite(callPartnerJid), callInviteElement.id());
    return true;
}

///
/// Adds a Call Invite object to the Call Invites vector and sets the bare JID of the call partner in the Call Invite object.
/// \param callPartnerJid bare JID of the call partner
/// \return The newly created Call Invite
///
std::shared_ptr<QXmppCallInvite> QXmppCallInviteManager::addCallInvite(const QString &callPartnerJid)
{
    auto callInvite { std::make_shared<QXmppCallInvite>(this) };
    callInvite->setCallPartnerJid(callPartnerJid);
    d->callInvites.append(callInvite);
    return callInvite;
}

///
/// Returns the Call Invites vector.
///
const QVector<std::shared_ptr<QXmppCallInvite>> &QXmppCallInviteManager::callInvites() const
{
    return d->callInvites;
}

///
/// \fn QXmppCallInviteManager::invited(const std::shared_ptr<QXmppCallInvite> &, const QString &)
///
/// Emitted when a call invitation has been received.
///
/// \param callInvite Call Invite object of proposed session
/// \param id Call Invite element id
///

///
/// \typedef QXmppCallInvite::Result
///
/// Contains one of the result types Rejected, Retracted, Left or QXmppError used for
/// Call Invite states.
///
/// \since QXmpp 1.6
///
