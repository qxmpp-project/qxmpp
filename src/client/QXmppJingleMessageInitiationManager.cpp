// SPDX-FileCopyrightText: 2023 Tibor Csötönyi <work@taibsu.de>
// SPDX-FileCopyrightText: 2023 Melvin Keskin <melvo@olomono.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppJingleMessageInitiationManager.h"

#include "QXmppClient.h"
#include "QXmppConstants_p.h"
#include "QXmppMessage.h"
#include "QXmppPromise.h"
#include "QXmppUtils.h"

#include "StringLiterals.h"

#include <QUuid>

using namespace QXmpp;
using Jmi = QXmppJingleMessageInitiation;
using JmiManager = QXmppJingleMessageInitiationManager;
using JmiElement = QXmppJingleMessageInitiationElement;
using JmiType = JmiElement::Type;

class QXmppJingleMessageInitiationPrivate
{
public:
    QXmppJingleMessageInitiationPrivate(JmiManager *manager)
        : manager(manager)
    {
    }

    QXmppTask<SendResult> request(JmiElement &&jmiElement);

    QXmppJingleMessageInitiationManager *manager;
    QString id;
    QString callPartnerJid;
    bool isProceeded { false };
};

///
/// \brief Creates a Jingle Message Initiation request based on given type.
/// \param type The request type (proceed, accept, reject, retract, finish).
///
QXmppTask<SendResult> QXmppJingleMessageInitiationPrivate::request(JmiElement &&jmiElement)
{
    jmiElement.setId(id);
    return manager->sendMessage(jmiElement, callPartnerJid);
}

///
/// \class QXmppJingleMessageInitiation
///
/// \brief The QXmppJingleMessageInitiation class holds information about the JMI element in the
/// current context.
///
/// \since QXmpp 1.6
///

///
/// \brief Constructs a Jingle Message Initiation object.
///
QXmppJingleMessageInitiation::QXmppJingleMessageInitiation(QXmppJingleMessageInitiationManager *manager)
    : d(new QXmppJingleMessageInitiationPrivate(manager))
{
}

QXmppJingleMessageInitiation::~QXmppJingleMessageInitiation() = default;

///
/// Creates a JMI element of type "ringing" and sends a request containing the element.
///
QXmppTask<SendResult> QXmppJingleMessageInitiation::ring()
{
    QXmppJingleMessageInitiationElement jmiElement;
    jmiElement.setType(JmiType::Ringing);

    return d->request(std::move(jmiElement));
}

///
/// Creates a JMI element of type "proceed" and sends a request containing the element.
///
QXmppTask<SendResult> QXmppJingleMessageInitiation::proceed()
{
    QXmppJingleMessageInitiationElement jmiElement;
    jmiElement.setType(JmiType::Proceed);

    return d->request(std::move(jmiElement));
}

///
/// Creates a JMI element of type "reject" and sends a request containing the element.
/// The default reason tag/type will be "busy" with text "Busy".
///
/// \param reason Reason object for reject element
/// \param containsTieBreak Whether the reject element contains a tie-break tag or not
///
QXmppTask<SendResult> QXmppJingleMessageInitiation::reject(std::optional<QXmppJingleReason> reason, bool containsTieBreak)
{
    JmiElement jmiElement;
    jmiElement.setType(JmiType::Reject);

    if (!reason) {
        reason = QXmppJingleReason();
        reason->setType(QXmppJingleReason::Busy);
        reason->setText(u"Busy"_s);
    }

    jmiElement.setReason(reason);
    jmiElement.setContainsTieBreak(containsTieBreak);

    return d->request(std::move(jmiElement));
}

///
/// Creates a JMI element of type "retract" and sends a request containing the element.
/// The default reason tag/type will be "cancel" with text "Retracted".
///
/// \param reason Reason object for retract element
/// \param containsTieBreak Whether the retract element contains a tie-break tag or not
///
QXmppTask<SendResult> QXmppJingleMessageInitiation::retract(std::optional<QXmppJingleReason> reason, bool containsTieBreak)
{
    JmiElement jmiElement;
    jmiElement.setType(JmiType::Retract);

    if (!reason) {
        reason = QXmppJingleReason();
        reason->setType(QXmppJingleReason::Cancel);
        reason->setText(u"Retracted"_s);
    }

    jmiElement.setReason(reason);
    jmiElement.setContainsTieBreak(containsTieBreak);

    return d->request(std::move(jmiElement));
}

///
/// Creates a JMI element of type "finish" and sends a request containing the element.
/// The default reason type/tag will be "success" with text "Success".
///
/// \param reason Reason object for finish element
/// \param migratedTo JMI id the session has been migrated to
///
QXmppTask<SendResult> QXmppJingleMessageInitiation::finish(std::optional<QXmppJingleReason> reason, const QString &migratedTo)
{
    JmiElement jmiElement;
    jmiElement.setType(JmiType::Finish);

    if (!reason) {
        reason = QXmppJingleReason();
        reason->setType(QXmppJingleReason::Success);
        reason->setText(u"Success"_s);
    }

    jmiElement.setReason(reason);
    jmiElement.setMigratedTo(migratedTo);

    return d->request(std::move(jmiElement));
}

///
/// Returns the JMI ID.
///
QString QXmppJingleMessageInitiation::id() const
{
    return d->id;
}

///
/// Sets the JMI ID.
///
void QXmppJingleMessageInitiation::setId(const QString &id)
{
    d->id = id;
}

///
/// Sets the call partner's bare JID.
///
/// Normally, the JMI ID would be sufficient in order to differentiate the JMIs.
/// However, attackers pretending to be the call partner can be mitigated by caching the call
/// partner's JID.
///
/// \param callPartnerJid bare JID of the call partner
///
void QXmppJingleMessageInitiation::setCallPartnerJid(const QString &callPartnerJid)
{
    d->callPartnerJid = callPartnerJid;
}

///
/// Returns the call partner's bare JID.
///
/// \return the call partner's bare JID.
///
QString QXmppJingleMessageInitiation::callPartnerJid() const
{
    return d->callPartnerJid;
}

///
/// Returns the "isProceeded" flag, e.g., if the Jingle Message Initiation has already been
/// proceeded.
///
bool QXmppJingleMessageInitiation::isProceeded() const
{
    return d->isProceeded;
}

///
/// Sets the "isProceeded" flag, e.g., if the Jingle Message Initiation has already been
/// proceeded.
///
void QXmppJingleMessageInitiation::setIsProceeded(bool isProceeded)
{
    d->isProceeded = isProceeded;
}

///
/// \fn QXmppJingleMessageInitiation::ringing()
///
/// Emitted when a propose request was accepted and the device starts ringing.
///

///
/// \fn QXmppJingleMessageInitiation::proceeded(const QString &, const QString &)
///
/// Emitted when a propose request was successfully processed and accepted.
///
/// \param id belonging JMI id
/// \param callPartnerResource resource of the call partner about to be called
///

///
/// \fn QXmppJingleMessageInitiation::closed(const Result &)
///
/// Emitted when a call was ended either through rejection, retraction, finish or an error.
///
/// \param result close reason
///

class QXmppJingleMessageInitiationManagerPrivate
{
public:
    QVector<std::shared_ptr<Jmi>> jmis;
};

///
/// \typedef QXmppJingleMessageInitiationManager::ProposeResult
///
/// Contains JMI object or an error if sending the propose message failed.
///

///
/// \class QXmppJingleMessageInitiationManager
///
/// \brief The QXmppJingleMessageInitiationManager class makes it possible to retrieve
/// Jingle Message Initiation elements as defined by \xep{0353, Jingle Message Initiation}.
///
/// \since QXmpp 1.6
///
QXmppJingleMessageInitiationManager::QXmppJingleMessageInitiationManager()
    : d(std::make_unique<QXmppJingleMessageInitiationManagerPrivate>())
{
}

QXmppJingleMessageInitiationManager::~QXmppJingleMessageInitiationManager() = default;

/// \cond
QStringList QXmppJingleMessageInitiationManager::discoveryFeatures() const
{
    return { ns_jingle_message_initiation.toString() };
}
/// \endcond

///
/// Creates a proposal JMI element and passes it as a message.
///
QXmppTask<QXmppJingleMessageInitiationManager::ProposeResult> QXmppJingleMessageInitiationManager::propose(const QString &callPartnerJid, const QXmppJingleDescription &description)
{
    QXmppPromise<ProposeResult> promise;

    QXmppJingleMessageInitiationElement jmiElement;
    jmiElement.setType(JmiType::Propose);
    jmiElement.setId(QXmppUtils::generateStanzaUuid());
    jmiElement.setDescription(description);

    sendMessage(jmiElement, callPartnerJid).then(this, [this, promise, callPartnerJid](SendResult result) mutable {
        if (auto error = std::get_if<QXmppError>(&result)) {
            warning(u"Error sending Jingle Message Initiation proposal: " + error->description);
            promise.finish(*error);
        } else {
            promise.finish(addJmi(callPartnerJid));
        }
    });

    return promise.task();
}

/// \cond
bool QXmppJingleMessageInitiationManager::handleMessage(const QXmppMessage &message)
{
    // JMI messages must be of type "chat" and contain a <store/> hint.
    if (message.type() != QXmppMessage::Chat || !message.hasHint(QXmppMessage::Store)) {
        return false;
    }

    // Only continue if the message contains a JMI element.
    if (auto jmiElement = message.jingleMessageInitiationElement()) {
        return handleJmiElement(std::move(*jmiElement), message.from());
    }

    return false;
}
/// \endcond

///
/// Lets the client send a message to user with given callPartnerJid containing the JMI element.
///
/// \param jmiElement the JMI element to be passed
/// \param callPartnerJid bare JID of the call partner
///
QXmppTask<SendResult> QXmppJingleMessageInitiationManager::sendMessage(const QXmppJingleMessageInitiationElement &jmiElement, const QString &callPartnerJid)
{
    QXmppMessage message;
    message.setTo(callPartnerJid);
    message.addHint(QXmppMessage::Store);
    message.setJingleMessageInitiationElement(jmiElement);

    return client()->send(std::move(message));
}

///
/// Removes a JMI object from the JMIs vector.
///
/// \param jmi object to be removed
///
void QXmppJingleMessageInitiationManager::clear(const std::shared_ptr<QXmppJingleMessageInitiation> &jmi)
{
    d->jmis.erase(
        std::remove_if(
            d->jmis.begin(),
            d->jmis.end(),
            [&jmi](const auto &storedJmi) {
                return jmi->id() == storedJmi->id() && jmi->callPartnerJid() == storedJmi->callPartnerJid();
            }),
        d->jmis.end());
}

///
/// Removes all JMI objects from the JMI vector.
///
void QXmppJingleMessageInitiationManager::clearAll()
{
    d->jmis.clear();
}

bool QXmppJingleMessageInitiationManager::handleJmiElement(QXmppJingleMessageInitiationElement &&jmiElement, const QString &senderJid)
{
    auto jmiElementId = jmiElement.id();
    auto callPartnerJid = QXmppUtils::jidToBareJid(senderJid);

    // Check if there's already a JMI object with jmiElementId and callPartnerJid in JMIs vector.
    // That means that a JMI has already been created with given (J)IDs.
    auto itr = std::find_if(d->jmis.begin(), d->jmis.end(), [&jmiElementId, &callPartnerJid](const auto &jmi) {
        return jmi->id() == jmiElementId && jmi->callPartnerJid() == callPartnerJid;
    });

    if (itr != d->jmis.end()) {
        return handleExistingJmi(*itr, jmiElement, QXmppUtils::jidToResource(senderJid));
    }

    if (jmiElement.type() == JmiType::Propose) {
        return handleProposeJmiElement(jmiElement, callPartnerJid);
    }

    return false;
}

///
/// Handles a JMI object which already exists in the JMIs vector.
///
/// \param existingJmi JMI object to be handled
/// \param jmiElement JMI element to be processed with the JMI object
/// \param callPartnerResource resource of the call partner (i.e., phone, tablet etc.)
/// \return success (true) or failure
///
bool QXmppJingleMessageInitiationManager::handleExistingJmi(const std::shared_ptr<Jmi> &existingJmi, const QXmppJingleMessageInitiationElement &jmiElement, const QString &callPartnerResource)
{
    switch (jmiElement.type()) {
    case JmiType::Ringing:
        Q_EMIT existingJmi->ringing();
        return true;
    case JmiType::Proceed:
        Q_EMIT existingJmi->proceeded(jmiElement.id(), callPartnerResource);
        existingJmi->setIsProceeded(true);
        return true;
    case JmiType::Reject:
        Q_EMIT existingJmi->closed(
            Jmi::Rejected { jmiElement.reason(), jmiElement.containsTieBreak() });
        return true;
    case JmiType::Retract:
        Q_EMIT existingJmi->closed(
            Jmi::Retracted { jmiElement.reason(), jmiElement.containsTieBreak() });
        return true;
    case JmiType::Finish:
        existingJmi->finish(jmiElement.reason(), jmiElement.migratedTo());
        Q_EMIT existingJmi->closed(
            Jmi::Finished { jmiElement.reason(), jmiElement.migratedTo() });
        return true;
    default:
        return false;
    }
}

///
/// Handles a propose JMI element.
///
/// \param jmiElement to be handled
/// \param callPartnerJid bare JID of the call partner
/// \return success (true) or failure
///
bool QXmppJingleMessageInitiationManager::handleProposeJmiElement(const QXmppJingleMessageInitiationElement &jmiElement, const QString &callPartnerJid)
{
    // Check if there's already a JMI object with provided callPartnerJid in JMIs vector.
    // That means that a propose has already been sent.
    auto itr = std::find_if(
        d->jmis.cbegin(),
        d->jmis.cend(),
        [&callPartnerJid](const auto &jmi) {
            return jmi->callPartnerJid() == callPartnerJid;
        });

    // Tie break case or usual JMI proposal?
    if (itr != d->jmis.end()) {
        return handleTieBreak(*itr, jmiElement, callPartnerJid);
    }

    Q_EMIT proposed(addJmi(callPartnerJid), jmiElement.id(), jmiElement.description());
    return true;
}

///
/// Handles a tie break case as defined in https://xmpp.org/extensions/xep-0353.html#tie-breaking.
/// \param existingJmi existing JMI object to be handled
/// \param jmiElement JMI element to be processed with existing JMI object
/// \param callPartnerResource resource of the call partner (i.e., phone, tablet etc.)
/// \return success (true) or failure
///
bool QXmppJingleMessageInitiationManager::handleTieBreak(const std::shared_ptr<Jmi> &existingJmi, const QXmppJingleMessageInitiationElement &jmiElement, const QString &callPartnerResource)
{
    // Tie break -> session is set to be expired
    QXmppJingleReason reason;
    reason.setType(QXmppJingleReason::Expired);

    // Existing (proceeded) or non-existing session?
    if (existingJmi->isProceeded()) {
        return handleExistingSession(existingJmi, jmiElement.id());
    }

    // Tie break in propose state (no existing session) - two parties try calling each other
    // at the same time, the proposal with the lower ID overrules the other one.
    return handleNonExistingSession(existingJmi, jmiElement.id(), callPartnerResource);
}

///
/// Device switch: session already exists and will be migrated to new device with id jmiElementId.
///
/// \param existingJmi Current JMI object
/// \param jmiElementId New (counterpart's) session JMI element ID
///
bool QXmppJingleMessageInitiationManager::handleExistingSession(const std::shared_ptr<Jmi> &existingJmi, const QString &jmiElementId)
{
    // Old session will be finished with reason "Expired".
    QXmppJingleReason reason;
    reason.setType(QXmppJingleReason::Expired);
    reason.setText(u"Session migrated"_s);

    // Tell the old session to be finished.
    Q_EMIT existingJmi->closed(Jmi::Finished { reason, jmiElementId });

    existingJmi->finish(reason, jmiElementId).then(this, [this, existingJmi, jmiElementId](SendResult result) {
        if (auto *error = std::get_if<QXmppError>(&result)) {
            Q_EMIT existingJmi->closed(*error);
        } else {
            // Then, proceed (accept) the new proposal and set the JMI ID
            // to the ID of the received JMI element.
            existingJmi->setId(jmiElementId);
            existingJmi->proceed().then(this, [existingJmi](SendResult result) {
                if (auto *error = std::get_if<QXmppError>(&result)) {
                    Q_EMIT existingJmi->closed(*error);
                } else {
                    // The session is now closed as it is finished.
                    existingJmi->setIsProceeded(true);
                }
            });
        }
    });

    return true;
}

///
/// \brief Tie break in propose state (no existing session) - two parties try calling each other
/// at the same time, the proposal with the lower ID overrules the other one.
///
/// \param existingJmi Current JMI object
/// \param jmiElementId Counterpart's JMI element ID
///
bool QXmppJingleMessageInitiationManager::handleNonExistingSession(const std::shared_ptr<Jmi> &existingJmi, const QString &jmiElementId, const QString &callPartnerResource)
{
    QXmppJingleReason reason;
    reason.setType(QXmppJingleReason::Expired);
    reason.setText(u"Tie-Break"_s);

    if (QUuid::fromString(existingJmi->id()) < QUuid::fromString(jmiElementId)) {
        // Jingle message initiator with lower ID rejects the other proposal.
        existingJmi->setId(jmiElementId);
        existingJmi->reject(std::move(reason), true).then(this, [existingJmi](auto result) {
            if (auto *error = std::get_if<QXmppError>(&result)) {
                Q_EMIT existingJmi->closed(*error);
            }
        });
    } else {
        // Jingle message initiator with higher ID retracts its proposal.
        existingJmi->retract(std::move(reason), true).then(this, [this, existingJmi, jmiElementId, callPartnerResource](auto result) {
            if (auto error = std::get_if<QXmppError>(&result)) {
                Q_EMIT existingJmi->closed(*error);
            } else {
                // Afterwards, JMI ID is changed to lower ID.
                existingJmi->setId(jmiElementId);

                // Finally, the call is being accepted.
                existingJmi->proceed().then(this, [existingJmi, jmiElementId, callPartnerResource](SendResult result) {
                    if (auto *error = std::get_if<QXmppError>(&result)) {
                        Q_EMIT existingJmi->closed(*error);
                    } else {
                        existingJmi->setIsProceeded(true);
                        Q_EMIT existingJmi->proceeded(jmiElementId, callPartnerResource);
                    }
                });
            }
        });
    }

    return true;
}

///
/// Adds a JMI object to the JMIs vector and sets the bare JID of the call partner in the JMI object.
/// \param callPartnerJid bare JID of the call partner
/// \return The newly created JMI
///
std::shared_ptr<QXmppJingleMessageInitiation> QXmppJingleMessageInitiationManager::addJmi(const QString &callPartnerJid)
{
    auto jmi { std::make_shared<QXmppJingleMessageInitiation>(this) };
    jmi->setCallPartnerJid(callPartnerJid);
    d->jmis.append(jmi);
    return jmi;
}

///
/// Returns the JMIs vector.
///
const QVector<std::shared_ptr<QXmppJingleMessageInitiation>> &QXmppJingleMessageInitiationManager::jmis() const
{
    return d->jmis;
}

///
/// \fn QXmppJingleMessageInitiationManager::proposed(const std::shared_ptr<QXmppJingleMessageInitiation> &, const QString &, const QXmppJingleDescription &)
///
/// Emitted when a call has been proposed.
///
/// \param jmi Jingle Message Initiation object of proposed session
/// \param id JMI element id
/// \param description JMI element's description containing media type (i.e., audio, video)
///
