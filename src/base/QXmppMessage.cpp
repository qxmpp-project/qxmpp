// SPDX-FileCopyrightText: 2009 Manjeet Dahiya <manjeetdahiya@gmail.com>
// SPDX-FileCopyrightText: 2010 Jeremy Lainé <jeremy.laine@m4x.org>
// SPDX-FileCopyrightText: 2018 Linus Jahn <lnj@kaidan.im>
// SPDX-FileCopyrightText: 2021 Melvin Keskin <melvo@olomono.de>
// SPDX-FileCopyrightText: 2023 Tibor Csötönyi <work@taibsu.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppMessage.h"

#include "QXmppBitsOfBinaryDataList.h"
#include "QXmppConstants_p.h"
#include "QXmppFallback.h"
#include "QXmppFileShare.h"
#include "QXmppGlobal_p.h"
#include "QXmppJingleData.h"
#include "QXmppMessageReaction.h"
#include "QXmppMixInvitation.h"

#include "Algorithms.h"
#include "StringLiterals.h"
#ifdef BUILD_OMEMO
#include "QXmppOmemoElement_p.h"
#include "QXmppOmemoEnvelope_p.h"
#endif
#include "QXmppOutOfBandUrl.h"
#include "QXmppTrustMessageElement.h"
#include "QXmppUtils.h"
#include "QXmppUtils_p.h"

#include <ranges>

#include <QDateTime>
#include <QDomElement>
#include <QTextStream>
#include <QXmlStreamWriter>

using namespace QXmpp;
using namespace QXmpp::Private;
namespace views = std::views;

constexpr auto CHAT_STATES = to_array<QStringView>({
    {},
    u"active",
    u"inactive",
    u"gone",
    u"composing",
    u"paused",
});

constexpr auto MESSAGE_TYPES = to_array<QStringView>({
    u"error",
    u"normal",
    u"chat",
    u"groupchat",
    u"headline",
});

constexpr auto MARKER_TYPES = to_array<QStringView>({
    {},
    u"received",
    u"displayed",
    u"acknowledged",
});

static const QVector<QStringView> HINT_TYPES = {
    u"no-permanent-store",
    u"no-store",
    u"no-copy",
    u"store",
};

static bool checkElement(const QDomElement &element, QStringView tagName, QStringView xmlns)
{
    return element.tagName() == tagName && element.namespaceURI() == xmlns;
}

///
/// \struct QXmppStanzaId
///
/// \brief Stanza ID element as defined in \xep{0359, Unique and Stable Stanza IDs}.
///
/// \since QXmpp 1.8
///

enum StampType {
    LegacyDelayedDelivery,  // XEP-0091: Legacy Delayed Delivery
    DelayedDelivery         // XEP-0203: Delayed Delivery
};

class QXmppMessagePrivate : public QSharedData
{
public:
    QString body;
    QString e2eeFallbackBody;
    QString subject;
    QString thread;
    QString parentThread;
    QXmppMessage::Type type = QXmppMessage::Chat;

    // XEP-0066: Out of Band Data
    QVector<QXmppOutOfBandUrl> outOfBandUrls;

    // XEP-0071: XHTML-IM
    QString xhtml;

    // XEP-0085: Chat State Notifications
    QXmppMessage::State state = QXmppMessage::None;

    // XEP-0091: Legacy Delayed Delivery | XEP-0203: Delayed Delivery
    QDateTime stamp;
    StampType stampType = DelayedDelivery;

    // XEP-0184: Message Delivery Receipts
    QString receiptId;
    bool receiptRequested = false;

    // XEP-0224: Attention
    bool attentionRequested = false;

    // XEP-0231: Bits of Binary
    QXmppBitsOfBinaryDataList bitsOfBinaryData;

    // XEP-0249: Direct MUC Invitations
    QString mucInvitationJid;
    QString mucInvitationPassword;
    QString mucInvitationReason;

    // XEP-0280: Message Carbons
    bool privatemsg = false;
    bool isCarbonForwarded = false;

    // XEP-0308: Last Message Correction
    QString replaceId;

    // XEP-0333: Chat Markers
    bool markable = false;
    QXmppMessage::Marker marker = QXmppMessage::NoMarker;
    QString markedId;
    QString markedThread;

    // XEP-0334: Message Processing Hints
    quint8 hints = 0;

    // XEP-0353: Jingle Message Initiation
    std::optional<QXmppJingleMessageInitiationElement> jingleMessageInitiationElement;

    // XEP-0359: Unique and Stable Stanza IDs
    QVector<QXmppStanzaId> stanzaIds;
    QString originId;

    // XEP-0367: Message Attaching
    QString attachId;

    // XEP-0369: Mediated Information eXchange (MIX)
    QString mixUserJid;
    QString mixUserNick;

    // XEP-0380: Explicit Message Encryption
    QString encryptionMethod;
    QString encryptionName;

    // XEP-0382: Spoiler messages
    bool isSpoiler = false;
    QString spoilerHint;
#ifdef BUILD_OMEMO
    // XEP-0384: OMEMO Encryption
    std::optional<QXmppOmemoElement> omemoElement;
#endif
    // XEP-0407: Mediated Information eXchange (MIX): Miscellaneous Capabilities
    std::optional<QXmppMixInvitation> mixInvitation;

    // XEP-0428: Fallback Indication
    QVector<QXmppFallback> fallbackMarkers;

    // XEP-0434: Trust Messages (TM)
    std::optional<QXmppTrustMessageElement> trustMessageElement;

    // XEP-0444: Message Reactions
    std::optional<QXmppMessageReaction> reaction;

    // XEP-0447: Stateless file sharing
    QVector<QXmppFileShare> sharedFiles;
    QVector<QXmppFileSourcesAttachment> fileSourcesAttachments;

    // XEP-0461: Message Replies
    std::optional<Reply> reply;

    // XEP-0482: Call Invites
    std::optional<QXmppCallInviteElement> callInviteElement;
};

///
/// Constructs a QXmppMessage.
///
/// \param from
/// \param to
/// \param body
/// \param thread
///
QXmppMessage::QXmppMessage(const QString &from, const QString &to, const QString &body, const QString &thread)
    : QXmppStanza(from, to), d(new QXmppMessagePrivate)
{
    d->body = body;
    d->thread = thread;
}

/// Constructs a copy of \a other.
QXmppMessage::QXmppMessage(const QXmppMessage &other) = default;
/// Move-constructor.
QXmppMessage::QXmppMessage(QXmppMessage &&) = default;
QXmppMessage::~QXmppMessage() = default;
/// Assignment operator.
QXmppMessage &QXmppMessage::operator=(const QXmppMessage &other) = default;
/// Move-assignment operator.
QXmppMessage &QXmppMessage::operator=(QXmppMessage &&) = default;

///
/// Indicates if the QXmppStanza is a stanza in the XMPP sense (i. e. a message,
/// iq or presence)
///
/// \since QXmpp 1.0
///
bool QXmppMessage::isXmppStanza() const
{
    return true;
}

/// Returns the message's body.
QString QXmppMessage::body() const
{
    return d->body;
}

/// Sets the message's body.
void QXmppMessage::setBody(const QString &body)
{
    d->body = body;
}

///
/// Returns a body that is unlike the normal body not encrypted.
///
/// It can be presented to users if the message could not be decrypted (e.g.,
/// because their clients do not support the used end-to-end encryption).
///
/// \return the end-to-end encryption fallback body
///
/// \since QXmpp 1.5
///
QString QXmppMessage::e2eeFallbackBody() const
{
    return d->e2eeFallbackBody;
}

///
/// Sets a body that is unlike the normal body not encrypted.
///
/// It can be presented to users if the message could not be decrypted (e.g.,
/// because their clients do not support the used end-to-end encryption).
///
/// \param fallbackBody end-to-end encryption fallback body
///
/// \since QXmpp 1.5
///
void QXmppMessage::setE2eeFallbackBody(const QString &fallbackBody)
{
    d->e2eeFallbackBody = fallbackBody;
}

/// Returns the message's type.
QXmppMessage::Type QXmppMessage::type() const
{
    return d->type;
}

/// Sets the message's type.
void QXmppMessage::setType(QXmppMessage::Type type)
{
    d->type = type;
}

/// Returns the message's subject.
QString QXmppMessage::subject() const
{
    return d->subject;
}

/// Sets the message's subject.
void QXmppMessage::setSubject(const QString &subject)
{
    d->subject = subject;
}

/// Returns the message's thread.
QString QXmppMessage::thread() const
{
    return d->thread;
}

/// Sets the message's thread.
void QXmppMessage::setThread(const QString &thread)
{
    d->thread = thread;
}

///
/// Returns the optional parent thread of this message.
///
/// The possibility to create child threads was added in RFC6121.
///
/// \since QXmpp 1.3
///
QString QXmppMessage::parentThread() const
{
    return d->parentThread;
}

///
/// Sets the optional parent thread of this message.
///
/// The possibility to create child threads was added in RFC6121.
///
/// \since QXmpp 1.3
///
void QXmppMessage::setParentThread(const QString &parent)
{
    d->parentThread = parent;
}

///
/// Returns a possibly attached URL from \xep{0066}: Out of Band Data
///
/// \since QXmpp 1.0
///
QString QXmppMessage::outOfBandUrl() const
{
    if (d->outOfBandUrls.empty()) {
        return {};
    }

    return d->outOfBandUrls.front().url();
}

///
/// Sets the attached URL for \xep{0066}: Out of Band Data
///
/// This overrides all other urls that may be contained in the list of out of band urls.
///
/// \since QXmpp 1.0
///
void QXmppMessage::setOutOfBandUrl(const QString &url)
{
    QXmppOutOfBandUrl data;
    data.setUrl(url);
    d->outOfBandUrls = { std::move(data) };
}

///
/// Returns possibly attached URLs from \xep{0066}: Out of Band Data
///
/// \since QXmpp 1.5
///
QVector<QXmppOutOfBandUrl> QXmppMessage::outOfBandUrls() const
{
    return d->outOfBandUrls;
}

///
/// Sets the attached URLs for \xep{0066}: Out of Band Data
///
/// \since QXmpp 1.5
///
void QXmppMessage::setOutOfBandUrls(const QVector<QXmppOutOfBandUrl> &urls)
{
    d->outOfBandUrls = urls;
}

///
/// Returns the message's XHTML body as defined by \xep{0071}: XHTML-IM.
///
/// \since QXmpp 0.6.2
///
QString QXmppMessage::xhtml() const
{
    return d->xhtml;
}

///
/// Sets the message's XHTML body as defined by \xep{0071}: XHTML-IM.
///
/// \since QXmpp 0.6.2
///
void QXmppMessage::setXhtml(const QString &xhtml)
{
    d->xhtml = xhtml;
}

///
/// Returns the the chat state notification according to \xep{0085}: Chat State
/// Notifications.
///
/// \since QXmpp 0.2
///
QXmppMessage::State QXmppMessage::state() const
{
    return d->state;
}

///
/// Sets the the chat state notification according to \xep{0085}: Chat State
/// Notifications.
///
/// \since QXmpp 0.2
///
void QXmppMessage::setState(QXmppMessage::State state)
{
    d->state = state;
}

///
/// Returns the optional timestamp of the message specified using \xep{0093}:
/// Legacy Delayed Delivery or using \xep{0203}: Delayed Delivery (preferred).
///
/// \since QXmpp 0.2
///
QDateTime QXmppMessage::stamp() const
{
    return d->stamp;
}

///
/// Sets the message's timestamp without modifying the type of the stamp
/// (\xep{0093}: Legacy Delayed Delivery or \xep{0203}: Delayed Delivery).
///
/// By default messages are constructed with the new delayed delivery XEP, but
/// parsed messages keep their type.
///
/// \since QXmpp 0.2
///
void QXmppMessage::setStamp(const QDateTime &stamp)
{
    d->stamp = stamp;
}

///
/// Returns true if a delivery receipt is requested, as defined by \xep{0184}:
/// Message Delivery Receipts.
///
/// \since QXmpp 0.4
///
bool QXmppMessage::isReceiptRequested() const
{
    return d->receiptRequested;
}

///
/// Sets whether a delivery receipt is requested, as defined by \xep{0184}:
/// Message Delivery Receipts.
///
/// \since QXmpp 0.4
///
void QXmppMessage::setReceiptRequested(bool requested)
{
    d->receiptRequested = requested;
    if (requested && id().isEmpty()) {
        generateAndSetNextId();
    }
}

///
/// If this message is a delivery receipt, returns the ID of the original
/// message.
///
/// \since QXmpp 0.4
///
QString QXmppMessage::receiptId() const
{
    return d->receiptId;
}

///
/// Make this message a delivery receipt for the message with the given \a id.
///
/// \since QXmpp 0.4
///
void QXmppMessage::setReceiptId(const QString &id)
{
    d->receiptId = id;
}

///
/// Returns true if the user's attention is requested, as defined by \xep{0224}:
/// Attention.
///
/// \since QXmpp 0.4
///
bool QXmppMessage::isAttentionRequested() const
{
    return d->attentionRequested;
}

///
/// Sets whether the user's attention is requested, as defined by \xep{0224}:
/// Attention.
///
/// \param requested Whether to request attention (true) or not (false)
///
/// \since QXmpp 0.4
///
void QXmppMessage::setAttentionRequested(bool requested)
{
    d->attentionRequested = requested;
}

///
/// Returns a list of data packages attached using \xep{0231}: Bits of Binary.
///
/// This could be used to resolve \c cid: URIs found in the X-HTML body.
///
/// \since QXmpp 1.2
///
QXmppBitsOfBinaryDataList QXmppMessage::bitsOfBinaryData() const
{
    return d->bitsOfBinaryData;
}

///
/// Returns a list of data attached using \xep{0231}: Bits of Binary.
///
/// This could be used to resolve \c cid: URIs found in the X-HTML body.
///
/// \since QXmpp 1.2
///
QXmppBitsOfBinaryDataList &QXmppMessage::bitsOfBinaryData()
{
    return d->bitsOfBinaryData;
}

///
/// Sets a list of \xep{0231}: Bits of Binary attachments to be included.
///
/// \since QXmpp 1.2
///
void QXmppMessage::setBitsOfBinaryData(const QXmppBitsOfBinaryDataList &bitsOfBinaryData)
{
    d->bitsOfBinaryData = bitsOfBinaryData;
}

///
/// Returns whether the given text is a '/me command' as defined in \xep{0245}:
/// The /me Command.
///
/// \since QXmpp 1.3
///
bool QXmppMessage::isSlashMeCommand(const QString &body)
{
    return body.startsWith(u"/me "_s);
}

///
/// Returns whether the body of the message is a '/me command' as defined in
/// \xep{0245}: The /me Command.
///
/// \note If you want to check a custom string for the /me command, you can use
/// the static version of this method. This can be helpful when checking user
/// input before a message was sent.
///
/// \since QXmpp 1.3
///
bool QXmppMessage::isSlashMeCommand() const
{
    return isSlashMeCommand(d->body);
}

///
/// Returns the part of the body after the /me command.
///
/// This cuts off '/me ' (with the space) from the body, in case the body
/// starts with that. In case the body does not contain a /me command as
/// defined in \xep{0245}: The /me Command, a null string is returned.
///
/// This is useful when displaying the /me command correctly to the user.
///
/// \since QXmpp 1.3
///
QString QXmppMessage::slashMeCommandText(const QString &body)
{
    if (isSlashMeCommand(body)) {
        return body.mid(4);
    }
    return {};
}

///
/// Returns the part of the body after the /me command.
///
/// This cuts off '/me ' (with the space) from the body, in case the body
/// starts with that. In case the body does not contain a /me command as
/// defined in \xep{0245}: The /me Command, a null string is returned.
///
/// This is useful when displaying the /me command correctly to the user.
///
/// \since QXmpp 1.3
///
QString QXmppMessage::slashMeCommandText() const
{
    return slashMeCommandText(d->body);
}

///
/// Returns the JID for a multi-user chat direct invitation as defined by
/// \xep{0249}: Direct MUC Invitations.
///
/// \since QXmpp 0.7.4
///
QString QXmppMessage::mucInvitationJid() const
{
    return d->mucInvitationJid;
}

///
/// Sets the JID for a multi-user chat direct invitation as defined by
/// \xep{0249}: Direct MUC Invitations.
///
/// \since QXmpp 0.7.4
///
void QXmppMessage::setMucInvitationJid(const QString &jid)
{
    d->mucInvitationJid = jid;
}

///
/// Returns the password for a multi-user chat direct invitation as defined by
/// \xep{0249}: Direct MUC Invitations.
///
/// \since QXmpp 0.7.4
///
QString QXmppMessage::mucInvitationPassword() const
{
    return d->mucInvitationPassword;
}

///
/// Sets the \a password for a multi-user chat direct invitation as defined by
/// \xep{0249}: Direct MUC Invitations.
///
/// \since QXmpp 0.7.4
///
void QXmppMessage::setMucInvitationPassword(const QString &password)
{
    d->mucInvitationPassword = password;
}

///
/// Returns the reason for a multi-user chat direct invitation as defined by
/// \xep{0249}: Direct MUC Invitations.
///
/// \since QXmpp 0.7.4
///
QString QXmppMessage::mucInvitationReason() const
{
    return d->mucInvitationReason;
}

///
/// Sets the \a reason for a multi-user chat direct invitation as defined by
/// \xep{0249}: Direct MUC Invitations.
///
/// \since QXmpp 0.7.4
///
void QXmppMessage::setMucInvitationReason(const QString &reason)
{
    d->mucInvitationReason = reason;
}

///
/// Returns if the message is marked with a &lt;private/&gt; tag, in which case
/// it will not be forwarded to other resources according to \xep{0280}: Message
/// Carbons.
///
/// \since QXmpp 1.0
///
bool QXmppMessage::isPrivate() const
{
    return d->privatemsg;
}

///
/// If true is passed, the message is marked with a &lt;private/&gt; tag, in
/// which case it will not be forwarded to other resources according to
/// \xep{0280}: Message Carbons.
///
/// \since QXmpp 1.0
///
void QXmppMessage::setPrivate(const bool priv)
{
    d->privatemsg = priv;
}

///
/// Returns whether this message has been forwarded using carbons.
///
/// \since QXmpp 1.5
///
bool QXmppMessage::isCarbonForwarded() const
{
    return d->isCarbonForwarded;
}

///
/// Sets whether this message has been forwarded using carbons.
///
/// Setting this to true has no effect, this is purely informational.
///
/// \since QXmpp 1.5
///
void QXmppMessage::setCarbonForwarded(bool forwarded)
{
    d->isCarbonForwarded = forwarded;
}

///
/// Returns the message id to replace with this message as used in \xep{0308}:
/// Last Message Correction. If the returned string is empty, this message is
/// not replacing another.
///
/// \since QXmpp 1.0
///
QString QXmppMessage::replaceId() const
{
    return d->replaceId;
}

///
/// Sets the message id to replace with this message as in \xep{0308}: Last
/// Message Correction.
///
/// \since QXmpp 1.0
///
void QXmppMessage::setReplaceId(const QString &replaceId)
{
    d->replaceId = replaceId;
}

///
/// Returns true if a message is markable, as defined by \xep{0333}: Chat
/// Markers.
///
/// \since QXmpp 0.8.1
///
bool QXmppMessage::isMarkable() const
{
    return d->markable;
}

///
/// Sets if the message is markable, as defined by \xep{0333}: Chat Markers.
///
/// \since QXmpp 0.8.1
///
void QXmppMessage::setMarkable(const bool markable)
{
    d->markable = markable;
}

///
/// Returns the message's marker id, as defined by \xep{0333}: Chat Markers.
///
/// \since QXmpp 0.8.1
///
QString QXmppMessage::markedId() const
{
    return d->markedId;
}

///
/// Sets the message's marker id, as defined by \xep{0333}: Chat Markers.
///
/// \since QXmpp 0.8.1
///
void QXmppMessage::setMarkerId(const QString &markerId)
{
    d->markedId = markerId;
}

///
/// Returns the message's marker thread, as defined by \xep{0333}: Chat Markers.
///
/// \since QXmpp 0.8.1
///
QString QXmppMessage::markedThread() const
{
    return d->markedThread;
}

///
/// Sets the message's marked thread, as defined by \xep{0333}: Chat Markers.
///
/// \since QXmpp 0.8.1
///
void QXmppMessage::setMarkedThread(const QString &markedThread)
{
    d->markedThread = markedThread;
}

///
/// Returns the message's marker, as defined by \xep{0333}: Chat Markers.
///
/// \since QXmpp 0.8.1
///
QXmppMessage::Marker QXmppMessage::marker() const
{
    return d->marker;
}

///
/// Sets the message's marker, as defined by \xep{0333}: Chat Markers
///
/// \since QXmpp 0.8.1
///
void QXmppMessage::setMarker(const Marker marker)
{
    d->marker = marker;
}

///
/// Returns true if the message contains the hint passed, as defined in
/// \xep{0334}: Message Processing Hints
///
/// \since QXmpp 1.1
///
bool QXmppMessage::hasHint(const Hint hint) const
{
    return d->hints & hint;
}

///
/// Adds a hint to the message, as defined in \xep{0334}: Message Processing
/// Hints
///
/// \since QXmpp 1.1
///
void QXmppMessage::addHint(const Hint hint)
{
    d->hints |= hint;
}

///
/// Removes a hint from the message, as defined in \xep{0334}: Message
/// Processing Hints
///
/// \since QXmpp 1.1
///
void QXmppMessage::removeHint(const Hint hint)
{
    d->hints &= ~hint;
}

///
/// Removes all hints from the message, as defined in \xep{0334}: Message
/// Processing Hints
///
/// \since QXmpp 1.1
///
void QXmppMessage::removeAllHints()
{
    d->hints = 0;
}

///
/// Returns a Jingle Message Initiation element as defined in \xep{0353}: Jingle Message
/// Initiation.
///
std::optional<QXmppJingleMessageInitiationElement> QXmppMessage::jingleMessageInitiationElement() const
{
    return d->jingleMessageInitiationElement;
}

///
/// Sets a Jingle Message Initiation element as defined in \xep{0353}: Jingle Message
/// Initiation.
///
void QXmppMessage::setJingleMessageInitiationElement(const std::optional<QXmppJingleMessageInitiationElement> &jingleMessageInitiationElement)
{
    d->jingleMessageInitiationElement = jingleMessageInitiationElement;
}

///
/// Returns the stanza ID of the message according to \xep{0359}: Unique and
/// Stable Stanza IDs.
///
/// \deprecated Use stanzaIds() instead.
///
/// \since QXmpp 1.3
///
QString QXmppMessage::stanzaId() const
{
    return d->stanzaIds.empty() ? QString() : d->stanzaIds.last().id;
}

///
/// Sets the stanza ID of the message according to \xep{0359}: Unique and
/// Stable Stanza IDs.
///
/// \deprecated Use setStanzaIds() instead.
///
/// \since QXmpp 1.3
///
void QXmppMessage::setStanzaId(const QString &id)
{
    if (d->stanzaIds.size() == 1) {
        d->stanzaIds.first().id = id;
    } else {
        d->stanzaIds = { QXmppStanzaId { id, {} } };
    }
}

///
/// Returns the creator of the stanza ID according to \xep{0359}: Unique and
/// Stable Stanza IDs.
///
/// \deprecated Use stanzaIds() instead.
///
/// \since QXmpp 1.3
///
QString QXmppMessage::stanzaIdBy() const
{
    return d->stanzaIds.empty() ? QString() : d->stanzaIds.last().by;
}

///
/// Sets the creator of the stanza ID according to \xep{0359}: Unique and
/// Stable Stanza IDs.
///
/// \deprecated Use setStanzaIds() instead.
///
/// \since QXmpp 1.3
///
void QXmppMessage::setStanzaIdBy(const QString &by)
{
    if (d->stanzaIds.size() == 1) {
        d->stanzaIds.first().by = by;
    } else {
        d->stanzaIds = { QXmppStanzaId { {}, by } };
    }
}

///
/// Returns the stanza IDs of the message as defined in \xep{0359, Unique and Stable Stanza IDs}.
///
/// \since QXmpp 1.8
///
QVector<QXmppStanzaId> QXmppMessage::stanzaIds() const
{
    return d->stanzaIds;
}

///
/// Sets the stanza IDs of the message as defined in \xep{0359, Unique and Stable Stanza IDs}.
///
/// \since QXmpp 1.8
///
void QXmppMessage::setStanzaIds(const QVector<QXmppStanzaId> &ids)
{
    d->stanzaIds = ids;
}

///
/// Returns the origin ID of the message according to \xep{0359}: Unique and
/// Stable Stanza IDs.
///
/// \since QXmpp 1.3
///
QString QXmppMessage::originId() const
{
    return d->originId;
}

///
/// Sets the origin ID of the message according to \xep{0359}: Unique and
/// Stable Stanza IDs.
///
/// \since QXmpp 1.3
///
void QXmppMessage::setOriginId(const QString &id)
{
    d->originId = id;
}

///
/// Returns the message id this message is linked/attached to. See \xep{0367}:
/// Message Attaching for details.
///
/// \since QXmpp 1.1
///
QString QXmppMessage::attachId() const
{
    return d->attachId;
}

///
/// Sets the id of the attached message as in \xep{0367}: Message Attaching.
/// This can be used for a "reply to" or "reaction" function.
///
/// The used message id depends on the message context, see the Business rules
/// section of the XEP for details about when to use which id.
///
/// \since QXmpp 1.1
///
void QXmppMessage::setAttachId(const QString &attachId)
{
    d->attachId = attachId;
}

///
/// Returns the participant ID of the sender if the message is received from a MIX channel as
/// specified in \xep{0369, Mediated Information eXchange (MIX)}.
///
/// \since QXmpp 1.7
///
QString QXmppMessage::mixParticipantId() const
{
    return mixUserJid().isEmpty() && mixUserNick().isEmpty() ? QString() : QXmppUtils::jidToResource(from());
}

///
/// Returns the actual JID of a MIX channel participant.
///
/// \since QXmpp 1.1
///
QString QXmppMessage::mixUserJid() const
{
    return d->mixUserJid;
}

///
/// Sets the actual JID of a MIX channel participant.
///
/// \since QXmpp 1.1
///
void QXmppMessage::setMixUserJid(const QString &mixUserJid)
{
    d->mixUserJid = mixUserJid;
}

///
/// Returns the MIX participant's nickname.
///
/// \since QXmpp 1.1
///
QString QXmppMessage::mixUserNick() const
{
    return d->mixUserNick;
}

///
/// Sets the MIX participant's nickname.
///
/// \since QXmpp 1.1
///
void QXmppMessage::setMixUserNick(const QString &mixUserNick)
{
    d->mixUserNick = mixUserNick;
}

///
/// Returns the encryption method this message is advertised to be encrypted
/// with.
///
/// \note QXmppMessage::NoEncryption does not necesserily mean that the message
/// is not encrypted; it may also be that the author of the message does not
/// support \xep{0380}: Explicit Message Encryption.
///
/// \note If this returns QXmppMessage::UnknownEncryption, you can still get
/// the namespace of the encryption with \c encryptionMethodNs() and possibly
/// also a name with \c encryptionName().
///
/// \since QXmpp 1.1
///
QXmpp::EncryptionMethod QXmppMessage::encryptionMethod() const
{
    if (d->encryptionMethod.isEmpty()) {
        return QXmpp::NoEncryption;
    }
    return QXmpp::Private::encryptionFromString(d->encryptionMethod).value_or(QXmpp::UnknownEncryption);
}

///
/// Advertises that this message is encrypted with the given encryption method.
/// See \xep{0380}: Explicit Message Encryption for details.
///
/// \since QXmpp 1.1
///
void QXmppMessage::setEncryptionMethod(QXmpp::EncryptionMethod method)
{
    d->encryptionMethod = QXmpp::Private::encryptionToString(method).toString();
}

///
/// Returns the namespace of the advertised encryption method via. \xep{0380}:
/// Explicit Message Encryption.
///
/// \since QXmpp 1.1
///
QString QXmppMessage::encryptionMethodNs() const
{
    return d->encryptionMethod;
}

///
/// Sets the namespace of the encryption method this message advertises to be
/// encrypted with. See \xep{0380}: Explicit Message Encryption for details.
///
/// \since QXmpp 1.1
///
void QXmppMessage::setEncryptionMethodNs(const QString &encryptionMethod)
{
    d->encryptionMethod = encryptionMethod;
}

///
/// Returns the associated name of the encryption method this message
/// advertises to be encrypted with. See \xep{0380}: Explicit Message Encryption
/// for details.
///
/// \since QXmpp 1.1
///
QString QXmppMessage::encryptionName() const
{
    if (!d->encryptionName.isEmpty()) {
        return d->encryptionName;
    }
    return QXmpp::Private::encryptionToName(encryptionMethod()).toString();
}

///
/// Sets the name of the encryption method for \xep{0380}: Explicit Message
/// Encryption.
///
/// \note This should only be used, if the encryption method is custom and is
/// not one of the methods listed in the XEP.
///
/// \since QXmpp 1.1
///
void QXmppMessage::setEncryptionName(const QString &encryptionName)
{
    d->encryptionName = encryptionName;
}

///
/// Returns true, if this is a spoiler message according to \xep{0382}: Spoiler
/// messages. The spoiler hint however can still be empty.
///
/// A spoiler message's content should not be visible to the user by default.
///
/// \since QXmpp 1.1
///
bool QXmppMessage::isSpoiler() const
{
    return d->isSpoiler;
}

///
/// Sets whether this is a spoiler message as specified in \xep{0382}: Spoiler
/// messages.
///
/// The content of spoiler messages will not be displayed by default to the
/// user. However, clients not supporting spoiler messages will still display
/// the content as usual.
///
/// \since QXmpp 1.1
///
void QXmppMessage::setIsSpoiler(bool isSpoiler)
{
    d->isSpoiler = isSpoiler;
}

///
/// Returns the spoiler hint as specified in \xep{0382}: Spoiler messages.
///
/// The hint may be empty, even if isSpoiler is true.
///
/// \since QXmpp 1.1
///
QString QXmppMessage::spoilerHint() const
{
    return d->spoilerHint;
}

///
/// Sets a spoiler hint for \xep{0382}: Spoiler messages. If the spoiler hint
/// is not empty, isSpoiler will be set to true.
///
/// A spoiler hint is optional for spoiler messages.
///
/// Keep in mind that the spoiler hint is not displayed at all by clients not
/// supporting spoiler messages.
///
/// \since QXmpp 1.1
///
void QXmppMessage::setSpoilerHint(const QString &spoilerHint)
{
    d->spoilerHint = spoilerHint;
    if (!spoilerHint.isEmpty()) {
        d->isSpoiler = true;
    }
}

#ifdef BUILD_OMEMO
/// \cond
///
/// Returns an included OMEMO element as defined by \xep{0384, OMEMO Encryption}.
///
/// \since QXmpp 1.5
///
std::optional<QXmppOmemoElement> QXmppMessage::omemoElement() const
{
    return d->omemoElement;
}

///
/// Sets an OMEMO element as defined by \xep{0384, OMEMO Encryption}.
///
/// \since QXmpp 1.5
///
void QXmppMessage::setOmemoElement(const std::optional<QXmppOmemoElement> &omemoElement)
{
    d->omemoElement = omemoElement;
}
/// \endcond
#endif

///
/// Returns an included \xep{0369, Mediated Information eXchange (MIX)}
/// invitation as defined by
/// \xep{0407, Mediated Information eXchange (MIX): Miscellaneous Capabilities}.
///
/// \since QXmpp 1.4
///
std::optional<QXmppMixInvitation> QXmppMessage::mixInvitation() const
{
    return d->mixInvitation;
}

///
/// Sets a \xep{0369, Mediated Information eXchange (MIX)} invitation as defined
/// by \xep{0407, Mediated Information eXchange (MIX): Miscellaneous Capabilities}.
///
/// \since QXmpp 1.4
///
void QXmppMessage::setMixInvitation(const std::optional<QXmppMixInvitation> &mixInvitation)
{
    d->mixInvitation = mixInvitation;
}

///
/// Sets whether this message is only a fallback according to \xep{0428, Fallback Indication}.
///
/// This is useful for clients not supporting end-to-end encryption to indicate
/// that the message body does not contain the intended text of the author.
///
/// \deprecated Use fallbackMarkers() instead, it provides full access to the fallback elements.
///
/// \since QXmpp 1.3
///
bool QXmppMessage::isFallback() const
{
    return !d->fallbackMarkers.empty();
}

///
/// Sets whether this message is only a fallback according to \xep{0428, Fallback Indication}.
///
/// This is useful for clients not supporting end-to-end encryption to indicate
/// that the message body does not contain the intended text of the author.
///
/// \deprecated Use setFallbackMarkers() instead, it provides full access to the fallback elements.
///
/// \since QXmpp 1.3
///
void QXmppMessage::setIsFallback(bool isFallback)
{
    if (isFallback) {
        d->fallbackMarkers = { QXmppFallback { {}, {} } };
    } else {
        d->fallbackMarkers.clear();
    }
}

///
/// Returns the fallback elements defined in \xep{0428, Fallback Indication}.
///
/// \since QXmpp 1.7
///
const QVector<QXmppFallback> &QXmppMessage::fallbackMarkers() const
{
    return d->fallbackMarkers;
}

///
/// Sets the fallback elements defined in \xep{0428, Fallback Indication}.
///
/// \since QXmpp 1.7
///
void QXmppMessage::setFallbackMarkers(const QVector<QXmppFallback> &fallbackMarkers)
{
    d->fallbackMarkers = fallbackMarkers;
}

///
/// Returns the body or subject text of the message without parts that have a fallback marker
/// with a supported namespace.
///
/// \param element whether to use the body or the subject text
/// \param supportedNamespaces
///
/// \since QXmpp 1.9
///
QString QXmppMessage::readFallbackRemovedText(QXmppFallback::Element element, const QVector<QString> &supportedNamespaces) const
{
    // filter out all QXmppFallback::Reference s
    auto markers = d->fallbackMarkers |
        views::filter([&](const auto &marker) { return contains(supportedNamespaces, marker.forNamespace()); }) |
        views::transform([](auto &&marker) { return marker.references(); });

    // collect references with correct namespace and element type (body or subject)
    QVector<QXmppFallback::Range> references;
    for (const auto &markerReferences : markers) {
        for (const auto &ref : markerReferences) {
            if (ref.element == element) {
                // early exit: element without range means whole text is fallback
                if (!ref.range) {
                    return {};
                }
                references.push_back(ref.range.value());
            }
        }
    }

    // sort by begin of fallback
    std::ranges::sort(references, {}, &QXmppFallback::Range::start);

    const auto &fullText = element == QXmppFallback::Subject ? d->subject : d->body;
    QString output;
    qsizetype index = 0;
    for (const auto &range : std::as_const(references)) {
        if (!(range.start < fullText.size()) || !(0 < range.end <= fullText.size())) {
            // skip markers with invalid start/begin
            continue;
        }

        // fallback marker marks [start, end)
        // we want to copy the section before the marker: [index, start)

        if (index < range.start) {
            output.append(QStringView(fullText.data() + index, fullText.data() + range.start));
        }
        index = range.end;
    }
    // append rest of the string (after the last fallback marker)
    if (index < fullText.size()) {
        output.append(QStringView(fullText.data() + index, fullText.data() + fullText.size()));
    }

    return output;
}

///
/// Returns the parts of the body or subject that are marked as fallback for this namespace.
///
/// \param element whether to use the body or the subject text
/// \param forNamespace
///
/// \since QXmpp 1.9
///
QString QXmppMessage::readFallbackText(QXmppFallback::Element element, QStringView forNamespace) const
{
    const auto &fullText = element == QXmppFallback::Subject ? d->subject : d->body;

    // filter out all QXmppFallback::Reference s
    auto markers = d->fallbackMarkers |
        views::filter([&](const auto &marker) { return marker.forNamespace() == forNamespace; }) |
        views::transform([](auto &&marker) { return marker.references(); });

    // collect references with correct namespace and element type (body or subject)
    QVector<QXmppFallback::Range> references;
    for (const auto &markerReferences : markers) {
        for (const auto &ref : markerReferences) {
            if (ref.element == element) {
                // early exit: element without range means whole text is fallback
                if (!ref.range) {
                    return fullText;
                }
                references.push_back(ref.range.value());
            }
        }
    }

    // sort by begin of fallback
    std::ranges::sort(references, {}, &QXmppFallback::Range::start);

    QString output;
    for (const auto &range : references) {
        output.append(QStringView(fullText.data() + range.start, fullText.data() + range.end));
    }
    return output;
}

///
/// Returns an included trust message element as defined by
/// \xep{0434, Trust Messages (TM)}.
///
/// \since QXmpp 1.5
///
std::optional<QXmppTrustMessageElement> QXmppMessage::trustMessageElement() const
{
    return d->trustMessageElement;
}

///
/// Sets a trust message element as defined by \xep{0434, Trust Messages (TM)}.
///
/// \since QXmpp 1.5
///
void QXmppMessage::setTrustMessageElement(const std::optional<QXmppTrustMessageElement> &trustMessageElement)
{
    d->trustMessageElement = trustMessageElement;
}

///
/// Returns a reaction to a message as defined by \xep{0444, Message Reactions}.
///
/// \since QXmpp 1.5
///
std::optional<QXmppMessageReaction> QXmppMessage::reaction() const
{
    return d->reaction;
}

///
/// Sets a reaction to a message as defined by \xep{0444, Message Reactions}.
///
/// The corresponding hint must be set manually:
/// \code
/// QXmppMessage message;
/// message.addHint(QXmppMessage::Store);
/// \endcode
///
/// \since QXmpp 1.5
///
void QXmppMessage::setReaction(const std::optional<QXmppMessageReaction> &reaction)
{
    d->reaction = reaction;
}

///
/// Returns the via \xep{0447, Stateless file sharing} shared files attached to this message.
///
/// \since QXmpp 1.5
///
const QVector<QXmppFileShare> &QXmppMessage::sharedFiles() const
{
    return d->sharedFiles;
}

///
/// Sets the via \xep{0447, Stateless file sharing} shared files attached to this message.
///
/// \since QXmpp 1.5
///
void QXmppMessage::setSharedFiles(const QVector<QXmppFileShare> &sharedFiles)
{
    d->sharedFiles = sharedFiles;
}

///
/// Returns additional sources to be attached to a file share as defined by \xep{0447, Stateless
/// file sharing}.
///
/// \since QXmpp 1.7
///
QVector<QXmppFileSourcesAttachment> QXmppMessage::fileSourcesAttachments() const
{
    return d->fileSourcesAttachments;
}

///
/// Sets additional sources to be attached to a file share as defined by \xep{0447, Stateless
/// file sharing}.
///
/// \since QXmpp 1.7
///
void QXmppMessage::setFileSourcesAttachments(const QVector<QXmppFileSourcesAttachment> &fileSourcesAttachments)
{
    d->fileSourcesAttachments = fileSourcesAttachments;
}

///
/// Returns the message reply extension as defined in \xep{0461, Message Replies}.
///
/// \since QXmpp 1.9
///
std::optional<QXmpp::Reply> QXmppMessage::reply() const
{
    return d->reply;
}

///
/// Sets the message reply extension as defined in \xep{0461, Message Replies}.
///
/// \since QXmpp 1.9
///
void QXmppMessage::setReply(const std::optional<QXmpp::Reply> &reply)
{
    d->reply = reply;
}

///
/// Returns the part of the body that is marked as fallback for \xep{0461, Message Replies},
/// without any quotation marks ('> ').
///
/// \since QXmpp 1.9
///
QString QXmppMessage::readReplyQuoteFromBody() const
{
    auto replyFallbackBody = readFallbackText(QXmppFallback::Body, ns_reply.toString());
    auto lines = replyFallbackBody.split(u'\n');
    // remove '> ' quotation
    for (auto &line : lines) {
        if (line == u'>') {
            line = QString();
        } else if (line.startsWith(u"> ")) {
            line = line.mid(QStringView(u"> ").size());
        }
    }
    return lines.join(u'\n');
}

///
/// Returns a Call Invite element as defined in \xep{0482, Call Invites}.
///
std::optional<QXmppCallInviteElement> QXmppMessage::callInviteElement() const
{
    return d->callInviteElement;
}

///
/// Sets a Call Invite element as defined in \xep{0482, Call Invites}.
///
void QXmppMessage::setCallInviteElement(std::optional<QXmppCallInviteElement> callInviteElement)
{
    d->callInviteElement = callInviteElement;
}

/// \cond
void QXmppMessage::parse(const QDomElement &element)
{
    parse(element, QXmpp::SceAll);
}

void QXmppMessage::parse(const QDomElement &element, QXmpp::SceMode sceMode)
{
    QXmppStanza::parse(element);

    d->type = enumFromString<Type>(MESSAGE_TYPES, element.attribute(u"type"_s))
                  .value_or(Normal);

    parseExtensions(element, sceMode);
}

void QXmppMessage::toXml(QXmlStreamWriter *writer) const
{
    toXml(writer, QXmpp::SceAll);
}

void QXmppMessage::toXml(QXmlStreamWriter *writer, QXmpp::SceMode sceMode) const
{
    writer->writeStartElement(QSL65("message"));
    writeOptionalXmlAttribute(writer, u"xml:lang", lang());
    writeOptionalXmlAttribute(writer, u"id", id());
    writeOptionalXmlAttribute(writer, u"to", to());
    writeOptionalXmlAttribute(writer, u"from", from());
    writeOptionalXmlAttribute(writer, u"type", MESSAGE_TYPES.at(size_t(d->type)));
    error().toXml(writer);

    // extensions
    serializeExtensions(writer, sceMode);

    // other, unknown extensions
    QXmppStanza::extensionsToXml(writer);

    writer->writeEndElement();
}
/// \endcond

///
/// Parses all child elements of a message stanza.
///
/// \param element message element or SCE content element
/// \param sceMode mode to decide which child elements of the message to parse
///
void QXmppMessage::parseExtensions(const QDomElement &element, const QXmpp::SceMode sceMode)
{
    QXmppElementList unknownExtensions;
    for (const auto &childElement : iterChildElements(element)) {
        if (!checkElement(childElement, u"addresses", ns_extended_addressing) &&
            childElement.tagName() != u"error") {
            // Try to parse the element and add it as an unknown extension if it
            // fails.
            if (!parseExtension(childElement, sceMode)) {
                unknownExtensions << QXmppElement(childElement);
            }
        }
    }
    setExtensions(unknownExtensions);
}

///
/// Parses a child element of the message stanza.
///
/// Allows inherited classes to parse additional extension elements. This
/// function may be executed multiple times with different elements.
///
/// \param element child element of the message to be parsed
/// \param sceMode Which elements to be parsed from the DOM (all known / only
/// public / only sensitive)
///
/// \return True, if the element was successfully parsed.
///
/// \since QXmpp 1.5
///
bool QXmppMessage::parseExtension(const QDomElement &element, QXmpp::SceMode sceMode)
{
    if (sceMode & QXmpp::ScePublic) {
        if (sceMode == QXmpp::ScePublic && element.tagName() == u"body") {
            d->e2eeFallbackBody = element.text();
            return true;
        }

        // XEP-0280: Message Carbons
        if (checkElement(element, u"private", ns_carbons)) {
            d->privatemsg = true;
            return true;
        }
        // XEP-0334: Message Processing Hints
        if (element.namespaceURI() == ns_message_processing_hints &&
            HINT_TYPES.contains(element.tagName())) {
            if (const auto index = HINT_TYPES.indexOf(element.tagName()); index >= 0) {
                addHint(Hint(1 << index));
            }
            return true;
        }
        // XEP-0353: Jingle Message Initiation
        if (QXmppJingleMessageInitiationElement::isJingleMessageInitiationElement(element)) {
            QXmppJingleMessageInitiationElement jingleMessageInitiationElement;
            jingleMessageInitiationElement.parse(element);
            d->jingleMessageInitiationElement = jingleMessageInitiationElement;
            return true;
        }
        // XEP-0359: Unique and Stable Stanza IDs
        if (checkElement(element, u"stanza-id", ns_sid)) {
            d->stanzaIds.push_back(QXmppStanzaId {
                element.attribute(u"id"_s),
                element.attribute(u"by"_s),
            });
            return true;
        }
        if (checkElement(element, u"origin-id", ns_sid)) {
            d->originId = element.attribute(u"id"_s);
            return true;
        }
        // XEP-0369: Mediated Information eXchange (MIX)
        if (checkElement(element, u"mix", ns_mix)) {
            d->mixUserJid = element.firstChildElement(u"jid"_s).text();
            d->mixUserNick = element.firstChildElement(u"nick"_s).text();
            return true;
        }
        // XEP-0380: Explicit Message Encryption
        if (checkElement(element, u"encryption", ns_eme)) {
            d->encryptionMethod = element.attribute(u"namespace"_s);
            d->encryptionName = element.attribute(u"name"_s);
            return true;
        }
#ifdef BUILD_OMEMO
        // XEP-0384: OMEMO Encryption
        if (QXmppOmemoElement::isOmemoElement(element)) {
            QXmppOmemoElement omemoElement;
            omemoElement.parse(element);
            d->omemoElement = omemoElement;
            return true;
        }
#endif
        // XEP-0428: Fallback Indication
        if (checkElement(element, u"fallback", ns_fallback_indication)) {
            if (auto fallback = QXmppFallback::fromDom(element)) {
                d->fallbackMarkers.push_back(std::move(*fallback));
            }
            return true;
        }
        // XEP-0482: Call Invites
        if (QXmppCallInviteElement::isCallInviteElement(element)) {
            QXmppCallInviteElement callInviteElement;
            callInviteElement.parse(element);
            d->callInviteElement = callInviteElement;
            return true;
        }
    }
    if (sceMode & QXmpp::SceSensitive) {
        if (element.tagName() == u"body") {
            d->body = element.text();
            return true;
        }
        if (element.tagName() == u"subject") {
            d->subject = element.text();
            return true;
        }
        if (element.tagName() == u"thread") {
            d->thread = element.text();
            d->parentThread = element.attribute(u"parent"_s);
            return true;
        }
        if (element.tagName() == u"x") {
            if (element.namespaceURI() == ns_legacy_delayed_delivery) {
                // if XEP-0203 exists, XEP-0091 has no need to parse because XEP-0091
                // is no more standard protocol)
                if (d->stamp.isNull()) {
                    // XEP-0091: Legacy Delayed Delivery
                    d->stamp = QDateTime::fromString(
                        element.attribute(u"stamp"_s),
                        u"yyyyMMddThh:mm:ss"_s);
                    d->stamp.setTimeSpec(Qt::UTC);
                    d->stampType = LegacyDelayedDelivery;
                }
                return true;
            }
            // XEP-0249: Direct MUC Invitations
            if (element.namespaceURI() == ns_conference) {
                d->mucInvitationJid = element.attribute(u"jid"_s);
                d->mucInvitationPassword = element.attribute(u"password"_s);
                d->mucInvitationReason = element.attribute(u"reason"_s);
                return true;
            }
            // XEP-0066: Out of Band Data
            if (element.namespaceURI() == ns_oob) {
                QXmppOutOfBandUrl data;
                data.parse(element);
                d->outOfBandUrls.push_back(std::move(data));
                return true;
            }
        }
        // XEP-0071: XHTML-IM
        if (checkElement(element, u"html", ns_xhtml_im)) {
            QDomElement bodyElement = element.firstChildElement(u"body"_s);
            if (!bodyElement.isNull() && bodyElement.namespaceURI() == ns_xhtml) {
                QTextStream stream(&d->xhtml, QIODevice::WriteOnly);
                bodyElement.save(stream, 0);

                d->xhtml = d->xhtml.mid(d->xhtml.indexOf(u'>') + 1);
                d->xhtml.replace(
                    u" xmlns=\"http://www.w3.org/1999/xhtml\""_s,
                    QString());
                d->xhtml.replace(u"</body>"_s, QString());
                d->xhtml = d->xhtml.trimmed();
            }
            return true;
        }
        // XEP-0085: Chat State Notifications
        if (element.namespaceURI() == ns_chat_states) {
            d->state = enumFromString<State>(CHAT_STATES, element.tagName()).value_or(None);
            return true;
        }
        // XEP-0184: Message Delivery Receipts
        if (checkElement(element, u"received", ns_message_receipts)) {
            d->receiptId = element.attribute(u"id"_s);

            // compatibility with old-style XEP
            if (d->receiptId.isEmpty()) {
                d->receiptId = id();
            }
            return true;
        }
        if (checkElement(element, u"request", ns_message_receipts)) {
            d->receiptRequested = true;
            return true;
        }
        // XEP-0203: Delayed Delivery
        if (checkElement(element, u"delay", ns_delayed_delivery)) {
            d->stamp = QXmppUtils::datetimeFromString(
                element.attribute(u"stamp"_s));
            d->stampType = DelayedDelivery;
            return true;
        }
        // XEP-0224: Attention
        if (checkElement(element, u"attention", ns_attention)) {
            d->attentionRequested = true;
            return true;
        }
        // XEP-0231: Bits of Binary
        if (QXmppBitsOfBinaryData::isBitsOfBinaryData(element)) {
            QXmppBitsOfBinaryData data;
            data.parseElementFromChild(element);
            d->bitsOfBinaryData << data;
            return true;
        }
        // XEP-0308: Last Message Correction
        if (checkElement(element, u"replace", ns_message_correct)) {
            d->replaceId = element.attribute(u"id"_s);
            return true;
        }
        // XEP-0333: Chat Markers
        if (element.namespaceURI() == ns_chat_markers) {
            if (element.tagName() == u"markable") {
                d->markable = true;
            } else {
                if (auto marker = enumFromString<Marker>(MARKER_TYPES, element.tagName())) {
                    d->marker = *marker;
                    d->markedId = element.attribute(u"id"_s);
                    d->markedThread = element.attribute(u"thread"_s);
                }
            }
            return true;
        }
        // XEP-0367: Message Attaching
        if (checkElement(element, u"attach-to", ns_message_attaching)) {
            d->attachId = element.attribute(u"id"_s);
            return true;
        }
        // XEP-0382: Spoiler messages
        if (checkElement(element, u"spoiler", ns_spoiler)) {
            d->isSpoiler = true;
            d->spoilerHint = element.text();
            return true;
        }
        // XEP-0407: Mediated Information eXchange (MIX): Miscellaneous Capabilities
        if (checkElement(element, u"invitation", ns_mix_misc)) {
            QXmppMixInvitation mixInvitation;
            mixInvitation.parse(element);
            d->mixInvitation = mixInvitation;
            return true;
        }
        // XEP-0434: Trust Messages (TM)
        if (QXmppTrustMessageElement::isTrustMessageElement(element)) {
            QXmppTrustMessageElement trustMessageElement;
            trustMessageElement.parse(element);
            d->trustMessageElement = trustMessageElement;
            return true;
        }
        // XEP-0444: Message Reactions
        if (QXmppMessageReaction::isMessageReaction(element)) {
            QXmppMessageReaction reaction;
            reaction.parse(element);
            d->reaction = std::move(reaction);
            return true;
        }
        // XEP-0447: Stateless file sharing
        if (checkElement(element, u"file-sharing", ns_sfs)) {
            QXmppFileShare share;
            if (share.parse(element)) {
                d->sharedFiles.push_back(std::move(share));
            }
            return true;
        }
        // XEP-0461: Message Replies
        if (checkElement(element, u"reply", ns_reply)) {
            d->reply = Reply {
                element.attribute(u"to"_s),
                element.attribute(u"id"_s),
            };
            return true;
        }
        if (checkElement(element, u"sources", ns_sfs)) {
            if (auto fileSources = QXmppFileSourcesAttachment::fromDom(element)) {
                d->fileSourcesAttachments.push_back(std::move(*fileSources));
            }
            return true;
        }
    }
    return false;
}

///
/// Serializes all additional child elements.
///
/// \param writer The XML stream writer to output the XML
/// \param sceMode The mode which decides which elements to output (only useful
/// for encryption)
/// \param baseNamespace Custom namespace added to basic XMPP-Core elements like
/// &lt;body/&gt; (needed when encrypting elements outside of the stream).
///
/// \since QXmpp 1.5
///
void QXmppMessage::serializeExtensions(QXmlStreamWriter *writer, QXmpp::SceMode sceMode, const QString &baseNamespace) const
{
    if (sceMode & QXmpp::ScePublic) {
        if (sceMode == QXmpp::ScePublic && !d->e2eeFallbackBody.isEmpty()) {
            writer->writeTextElement(QSL65("body"), d->e2eeFallbackBody);
        }

        // XEP-0280: Message Carbons
        if (d->privatemsg) {
            writer->writeStartElement(QSL65("private"));
            writer->writeDefaultNamespace(toString65(ns_carbons));
            writer->writeEndElement();
        }

        // XEP-0334: Message Processing Hints
        for (quint8 i = 0; i < HINT_TYPES.size(); i++) {
            if (hasHint(Hint(1 << i))) {
                writer->writeStartElement(toString65(HINT_TYPES.at(i)));
                writer->writeDefaultNamespace(toString65(ns_message_processing_hints));
                writer->writeEndElement();
            }
        }

        // XEP-0359: Unique and Stable Stanza IDs
        for (const auto &stanzaId : d->stanzaIds) {
            writer->writeStartElement(QSL65("stanza-id"));
            writer->writeDefaultNamespace(toString65(ns_sid));
            writer->writeAttribute(QSL65("id"), stanzaId.id);
            writeOptionalXmlAttribute(writer, u"by", stanzaId.by);
            writer->writeEndElement();
        }

        if (!d->originId.isNull()) {
            writer->writeStartElement(QSL65("origin-id"));
            writer->writeDefaultNamespace(toString65(ns_sid));
            writer->writeAttribute(QSL65("id"), d->originId);
            writer->writeEndElement();
        }

        // XEP-0369: Mediated Information eXchange (MIX)
        if (!d->mixUserJid.isEmpty() || !d->mixUserNick.isEmpty()) {
            writer->writeStartElement(QSL65("mix"));
            writer->writeDefaultNamespace(toString65(ns_mix));
            writeXmlTextElement(writer, u"jid", d->mixUserJid);
            writeXmlTextElement(writer, u"nick", d->mixUserNick);
            writer->writeEndElement();
        }

        // XEP-0380: Explicit Message Encryption
        if (!d->encryptionMethod.isEmpty()) {
            writer->writeStartElement(QSL65("encryption"));
            writer->writeDefaultNamespace(toString65(ns_eme));
            writer->writeAttribute(QSL65("namespace"), d->encryptionMethod);
            writeOptionalXmlAttribute(writer, u"name", encryptionName());
            writer->writeEndElement();
        }

#ifdef BUILD_OMEMO
        // XEP-0384: OMEMO Encryption
        if (d->omemoElement) {
            d->omemoElement->toXml(writer);
        }
#endif

        // XEP-0428: Fallback Indication
        for (const auto &fallback : d->fallbackMarkers) {
            fallback.toXml(writer);
        }
    }

    if (sceMode & QXmpp::SceSensitive) {
        const auto writeTextElement = [writer, &baseNamespace](const auto &tagName, const auto &text) {
            if (!text.isEmpty()) {
                writer->writeStartElement(tagName);
                if (!baseNamespace.isNull()) {
                    writer->writeDefaultNamespace(baseNamespace);
                }
                writer->writeCharacters(text);
                writer->writeEndElement();
            }
        };

        // XMPP-Core
        writeTextElement(QSL65("subject"), d->subject);
        writeTextElement(QSL65("body"), d->body);

        if (!d->thread.isEmpty()) {
            writer->writeStartElement(QSL65("thread"));
            if (!baseNamespace.isNull()) {
                writer->writeDefaultNamespace(baseNamespace);
            }
            writeOptionalXmlAttribute(writer, u"parent", d->parentThread);
            writer->writeCharacters(d->thread);
            writer->writeEndElement();
        }

        // XEP-0066: Out of Band Data
        for (const auto &url : d->outOfBandUrls) {
            url.toXml(writer);
        }

        // XEP-0071: XHTML-IM
        if (!d->xhtml.isEmpty()) {
            writer->writeStartElement(QSL65("html"));
            writer->writeDefaultNamespace(toString65(ns_xhtml_im));
            writer->writeStartElement(QSL65("body"));
            writer->writeDefaultNamespace(toString65(ns_xhtml));
            writer->writeCharacters(QString());
            writer->device()->write(d->xhtml.toUtf8());
            writer->writeEndElement();
            writer->writeEndElement();
        }

        // XEP-0085: Chat State Notifications
        if (d->state > None && d->state <= Paused) {
            writer->writeStartElement(toString65(CHAT_STATES.at(d->state)));
            writer->writeDefaultNamespace(toString65(ns_chat_states));
            writer->writeEndElement();
        }

        // XEP-0091: Legacy Delayed Delivery | XEP-0203: Delayed Delivery
        if (d->stamp.isValid()) {
            QDateTime utcStamp = d->stamp.toUTC();
            if (d->stampType == DelayedDelivery) {
                // XEP-0203: Delayed Delivery
                writer->writeStartElement(QSL65("delay"));
                writer->writeDefaultNamespace(toString65(ns_delayed_delivery));
                writeOptionalXmlAttribute(writer, u"stamp", QXmppUtils::datetimeToString(utcStamp));
                writer->writeEndElement();
            } else {
                // XEP-0091: Legacy Delayed Delivery
                writer->writeStartElement(QSL65("x"));
                writer->writeDefaultNamespace(toString65(ns_legacy_delayed_delivery));
                writeOptionalXmlAttribute(writer, u"stamp", utcStamp.toString(u"yyyyMMddThh:mm:ss"));
                writer->writeEndElement();
            }
        }

        // XEP-0184: Message Delivery Receipts
        // An ack message (message containing a "received" element) must not
        // include a receipt request ("request" element) in order to prevent
        // looping.
        if (!d->receiptId.isEmpty()) {
            writer->writeStartElement(QSL65("received"));
            writer->writeDefaultNamespace(toString65(ns_message_receipts));
            writer->writeAttribute(QSL65("id"), d->receiptId);
            writer->writeEndElement();
        } else if (d->receiptRequested) {
            writer->writeStartElement(QSL65("request"));
            writer->writeDefaultNamespace(toString65(ns_message_receipts));
            writer->writeEndElement();
        }

        // XEP-0224: Attention
        if (d->attentionRequested) {
            writer->writeStartElement(QSL65("attention"));
            writer->writeDefaultNamespace(toString65(ns_attention));
            writer->writeEndElement();
        }

        // XEP-0249: Direct MUC Invitations
        if (!d->mucInvitationJid.isEmpty()) {
            writer->writeStartElement(QSL65("x"));
            writer->writeDefaultNamespace(toString65(ns_conference));
            writer->writeAttribute(QSL65("jid"), d->mucInvitationJid);
            if (!d->mucInvitationPassword.isEmpty()) {
                writer->writeAttribute(QSL65("password"), d->mucInvitationPassword);
            }
            if (!d->mucInvitationReason.isEmpty()) {
                writer->writeAttribute(QSL65("reason"), d->mucInvitationReason);
            }
            writer->writeEndElement();
        }

        // XEP-0231: Bits of Binary
        for (const auto &data : std::as_const(d->bitsOfBinaryData)) {
            data.toXmlElementFromChild(writer);
        }

        // XEP-0308: Last Message Correction
        if (!d->replaceId.isEmpty()) {
            writer->writeStartElement(QSL65("replace"));
            writer->writeDefaultNamespace(toString65(ns_message_correct));
            writer->writeAttribute(QSL65("id"), d->replaceId);
            writer->writeEndElement();
        }

        // XEP-0333: Chat Markers
        if (d->markable) {
            writer->writeStartElement(QSL65("markable"));
            writer->writeDefaultNamespace(toString65(ns_chat_markers));
            writer->writeEndElement();
        }
        if (d->marker != NoMarker) {
            writer->writeStartElement(toString65(MARKER_TYPES.at(d->marker)));
            writer->writeDefaultNamespace(toString65(ns_chat_markers));
            writer->writeAttribute(QSL65("id"), d->markedId);
            if (!d->markedThread.isNull() && !d->markedThread.isEmpty()) {
                writer->writeAttribute(QSL65("thread"), d->markedThread);
            }
            writer->writeEndElement();
        }

        // XEP-0353: Jingle Message Initiation
        if (d->jingleMessageInitiationElement) {
            d->jingleMessageInitiationElement->toXml(writer);
        }

        // XEP-0367: Message Attaching
        if (!d->attachId.isEmpty()) {
            writer->writeStartElement(QSL65("attach-to"));
            writer->writeDefaultNamespace(toString65(ns_message_attaching));
            writer->writeAttribute(QSL65("id"), d->attachId);
            writer->writeEndElement();
        }

        // XEP-0382: Spoiler messages
        if (d->isSpoiler) {
            writer->writeStartElement(QSL65("spoiler"));
            writer->writeDefaultNamespace(toString65(ns_spoiler));
            writer->writeCharacters(d->spoilerHint);
            writer->writeEndElement();
        }

        // XEP-0407: Mediated Information eXchange (MIX): Miscellaneous Capabilities
        if (d->mixInvitation) {
            d->mixInvitation->toXml(writer);
        }

        // XEP-0434: Trust Messages (TM)
        if (d->trustMessageElement) {
            d->trustMessageElement->toXml(writer);
        }

        // XEP-0444: Message Reactions
        if (d->reaction) {
            d->reaction->toXml(writer);
        }

        // XEP-0447: Stateless file sharing
        for (const auto &fileShare : d->sharedFiles) {
            fileShare.toXml(writer);
        }
        for (const auto &fileSources : d->fileSourcesAttachments) {
            fileSources.toXml(writer);
        }

        // XEP-0461: Message Replies
        if (d->reply) {
            writer->writeStartElement(QSL65("reply"));
            writer->writeDefaultNamespace(toString65(ns_reply));
            writeOptionalXmlAttribute(writer, u"to", d->reply->to);
            writer->writeAttribute(QSL65("id"), d->reply->id);
            writer->writeEndElement();
        }

        // XEP-0482: Call Invites
        if (d->callInviteElement) {
            d->callInviteElement->toXml(writer);
        }
    }
}

struct QXmppFallbackPrivate : QSharedData {
    QString forNamespace;
    QVector<QXmppFallback::Reference> references;
};

///
/// \class QXmppFallback
///
/// Fallback marker for message stanzas as defined in \xep{0428, Fallback Indication}.
///
/// \sa QXmppFallback
/// \since QXmpp 1.7
///

///
/// \enum QXmppFallback::Element
///
/// Describes the element of the message stanza this refers to.
///

///
/// \struct QXmppFallback::Range
///
/// A character range of a string, see \xep{0426, Character counting in message bodies} for details.
///

///
/// \struct QXmppFallback::Reference
///
/// A reference to a text in the message stanza.
///

QXMPP_PRIVATE_DEFINE_RULE_OF_SIX(QXmppFallback)

/// Creates a fallback marker.
QXmppFallback::QXmppFallback(const QString &forNamespace, const QVector<Reference> &references)
    : d(new QXmppFallbackPrivate { {}, forNamespace, references })
{
}

///
/// Returns the namespace of the XEP that this fallback marker is referring to.
///
const QString &QXmppFallback::forNamespace() const
{
    return d->forNamespace;
}

///
/// Sets the namespace of the XEP that this fallback marker is referring to.
///
void QXmppFallback::setForNamespace(const QString &ns)
{
    d->forNamespace = ns;
}

///
/// Returns the text references of this fallback marker.
///
const QVector<QXmppFallback::Reference> &QXmppFallback::references() const
{
    return d->references;
}

///
/// Sets the text references of this fallback marker.
///
void QXmppFallback::setReferences(const QVector<Reference> &references)
{
    d->references = references;
}

///
/// Tries to parse \a el into a QXmppFallback object.
///
/// \return Empty optional on failure, parsed object otherwise.
///
std::optional<QXmppFallback> QXmppFallback::fromDom(const QDomElement &el)
{
    if (el.tagName() != u"fallback" || el.namespaceURI() != ns_fallback_indication) {
        return {};
    }

    QVector<Reference> references;
    for (const auto &subEl : iterChildElements(el, {}, ns_fallback_indication)) {
        auto start = parseInt<uint32_t>(subEl.attribute(u"start"_s));
        auto end = parseInt<uint32_t>(subEl.attribute(u"end"_s));
        std::optional<Range> range;
        if (start && end) {
            range = Range { *start, *end };
        }

        if (subEl.tagName() == u"body") {
            references.push_back(Reference { Body, range });
        } else if (subEl.tagName() == u"subject") {
            references.push_back(Reference { Subject, range });
        }
    }

    return QXmppFallback {
        el.attribute(u"for"_s),
        references,
    };
}

///
/// Serializes the object to XML.
///
void QXmppFallback::toXml(QXmlStreamWriter *writer) const
{
    writer->writeStartElement(QSL65("fallback"));
    writer->writeDefaultNamespace(toString65(ns_fallback_indication));
    writeOptionalXmlAttribute(writer, u"for", d->forNamespace);
    for (const auto &reference : d->references) {
        writer->writeStartElement(reference.element == Body ? QSL65("body") : QSL65("subject"));
        if (reference.range) {
            writer->writeAttribute(QSL65("start"), serializeInt(reference.range->start));
            writer->writeAttribute(QSL65("end"), serializeInt(reference.range->end));
        }
        writer->writeEndElement();
    }
    writer->writeEndElement();
}
