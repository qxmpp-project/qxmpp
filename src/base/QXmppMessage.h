// SPDX-FileCopyrightText: 2009 Manjeet Dahiya <manjeetdahiya@gmail.com>
// SPDX-FileCopyrightText: 2010 Jeremy Lainé <jeremy.laine@m4x.org>
// SPDX-FileCopyrightText: 2018 Linus Jahn <lnj@kaidan.im>
// SPDX-FileCopyrightText: 2020 Melvin Keskin <melvo@olomono.de>
// SPDX-FileCopyrightText: 2023 Tibor Csötönyi <work@taibsu.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPMESSAGE_H
#define QXMPPMESSAGE_H

#include "QXmppFileShare.h"
#include "QXmppStanza.h"

#include <optional>

// Required for source compatibility
#include <QDateTime>

class QXmppMessagePrivate;
class QXmppBitsOfBinaryDataList;
class QXmppFallback;
class QXmppJingleMessageInitiationElement;
class QXmppMessageReaction;
class QXmppMixInvitation;
#ifdef BUILD_OMEMO
class QXmppOmemoElement;
#endif
class QXmppTrustMessageElement;
class QXmppOutOfBandUrl;
class QXmppCallInviteElement;

///
/// \brief The QXmppMessage class represents an XMPP message.
///
/// \ingroup Stanzas
///
class QXMPP_EXPORT QXmppMessage : public QXmppStanza
{
public:
#if QXMPP_DEPRECATED_SINCE(1, 5)
    /// \cond
    using EncryptionMethod = QXmpp::EncryptionMethod;

    static const EncryptionMethod NoEncryption = EncryptionMethod::NoEncryption;
    static const EncryptionMethod UnknownEncryption = EncryptionMethod::UnknownEncryption;
    static const EncryptionMethod OTR = EncryptionMethod::Otr;
    static const EncryptionMethod LegacyOpenPGP = EncryptionMethod::LegacyOpenPgp;
    static const EncryptionMethod OX = EncryptionMethod::Ox;
    static const EncryptionMethod OMEMO = EncryptionMethod::Omemo0;
    /// \endcond
#endif

    /// This enum describes a message type.
    enum Type {
        Error = 0,
        Normal,
        Chat,
        GroupChat,
        Headline
    };

    ///
    /// This enum describes a chat state as defined by \xep{0085}: Chat State
    /// Notifications.
    ///
    /// \since QXmpp 0.2
    ///
    enum State {
        None = 0,   ///< The message does not contain any chat state information.
        Active,     ///< User is actively participating in the chat session.
        Inactive,   ///< User has not been actively participating in the chat session.
        Gone,       ///< User has effectively ended their participation in the chat session.
        Composing,  ///< User is composing a message.
        Paused      ///< User had been composing but now has stopped.
    };

    ///
    /// This enum describes a chat marker as defined by \xep{0333}: Chat Markers.
    ///
    /// \since QXmpp 0.8.1
    ///
    enum Marker {
        NoMarker = 0,
        Received,
        Displayed,
        Acknowledged
    };

    ///
    /// \xep{0334}: Message Processing Hints
    ///
    /// \since QXmpp 1.1
    ///
    enum Hint {
        NoPermanentStore = 1 << 0,  ///< Do not allow permanent storage
        NoStore = 1 << 1,           ///< Do not store at all
        NoCopy = 1 << 2,            ///< Do not copy the message
        Store = 1 << 3              ///< Do store the message
    };

    QXmppMessage(const QString &from = QString(), const QString &to = QString(),
                 const QString &body = QString(), const QString &thread = QString());

    QXmppMessage(const QXmppMessage &other);
    QXmppMessage(QXmppMessage &&);
    ~QXmppMessage() override;

    QXmppMessage &operator=(const QXmppMessage &other);
    QXmppMessage &operator=(QXmppMessage &&);

    bool isXmppStanza() const override;

    QString body() const;
    void setBody(const QString &);

    QString e2eeFallbackBody() const;
    void setE2eeFallbackBody(const QString &fallbackBody);

    QString subject() const;
    void setSubject(const QString &);

    QString thread() const;
    void setThread(const QString &);

    QString parentThread() const;
    void setParentThread(const QString &);

    QXmppMessage::Type type() const;
    void setType(QXmppMessage::Type);

    // XEP-0066: Out of Band Data
    QString outOfBandUrl() const;
    void setOutOfBandUrl(const QString &);

    QVector<QXmppOutOfBandUrl> outOfBandUrls() const;
    void setOutOfBandUrls(const QVector<QXmppOutOfBandUrl> &urls);

    // XEP-0071: XHTML-IM
    QString xhtml() const;
    void setXhtml(const QString &xhtml);

    // XEP-0085: Chat State Notifications
    QXmppMessage::State state() const;
    void setState(QXmppMessage::State);

    // XEP-0091: Legacy Delayed Delivery | XEP-0203: Delayed Delivery
    QDateTime stamp() const;
    void setStamp(const QDateTime &stamp);

    // XEP-0184: Message Delivery Receipts
    bool isReceiptRequested() const;
    void setReceiptRequested(bool requested);

    QString receiptId() const;
    void setReceiptId(const QString &id);

    // XEP-0224: Attention
    bool isAttentionRequested() const;
    void setAttentionRequested(bool requested);

    // XEP-0231: Bits of Binary
    QXmppBitsOfBinaryDataList bitsOfBinaryData() const;
    QXmppBitsOfBinaryDataList &bitsOfBinaryData();
    void setBitsOfBinaryData(const QXmppBitsOfBinaryDataList &bitsOfBinaryData);

    // XEP-0245: The /me Command
    static bool isSlashMeCommand(const QString &body);
    bool isSlashMeCommand() const;
    static QString slashMeCommandText(const QString &body);
    QString slashMeCommandText() const;

    // XEP-0249: Direct MUC Invitations
    QString mucInvitationJid() const;
    void setMucInvitationJid(const QString &jid);

    QString mucInvitationPassword() const;
    void setMucInvitationPassword(const QString &password);

    QString mucInvitationReason() const;
    void setMucInvitationReason(const QString &reason);

    // XEP-0280: Message Carbons
    bool isPrivate() const;
    void setPrivate(const bool);
    bool isCarbonForwarded() const;
    void setCarbonForwarded(bool);

    // XEP-0308: Last Message Correction
    QString replaceId() const;
    void setReplaceId(const QString &);

    // XEP-0333: Chat State Markers
    bool isMarkable() const;
    void setMarkable(const bool);

    QString markedId() const;
    void setMarkerId(const QString &);

    QString markedThread() const;
    void setMarkedThread(const QString &);

    Marker marker() const;
    void setMarker(const Marker);

    // XEP-0334: Message Processing Hints
    bool hasHint(const Hint hint) const;
    void addHint(const Hint hint);
    void removeHint(const Hint hint);
    void removeAllHints();

    // XEP-0353: Jingle Message Initiation
    std::optional<QXmppJingleMessageInitiationElement> jingleMessageInitiationElement() const;
    void setJingleMessageInitiationElement(const std::optional<QXmppJingleMessageInitiationElement> &jingleMessageInitiationElement);

    // XEP-0359: Unique and Stable Stanza IDs
    QString stanzaId() const;
    void setStanzaId(const QString &id);

    QString stanzaIdBy() const;
    void setStanzaIdBy(const QString &id);

    QString originId() const;
    void setOriginId(const QString &id);

    // XEP-0367: Message Attaching
    QString attachId() const;
    void setAttachId(const QString &);

    // XEP-0369: Mediated Information eXchange (MIX)
    QString mixParticipantId() const;

    QString mixUserJid() const;
    void setMixUserJid(const QString &);

    QString mixUserNick() const;
    void setMixUserNick(const QString &);

    // XEP-0380: Explicit Message Encryption
    QXmpp::EncryptionMethod encryptionMethod() const;
    void setEncryptionMethod(QXmpp::EncryptionMethod);
    QString encryptionMethodNs() const;
    void setEncryptionMethodNs(const QString &);

    QString encryptionName() const;
    void setEncryptionName(const QString &);

    // XEP-0382: Spoiler messages
    bool isSpoiler() const;
    void setIsSpoiler(bool);

    QString spoilerHint() const;
    void setSpoilerHint(const QString &);

    // XEP-0407: Mediated Information eXchange (MIX): Miscellaneous Capabilities
    std::optional<QXmppMixInvitation> mixInvitation() const;
    void setMixInvitation(const std::optional<QXmppMixInvitation> &mixInvitation);

    // XEP-0428: Fallback Indication
#if QXMPP_DEPRECATED_SINCE(1, 7)
    [[deprecated("Use fallbackMarkers()")]] bool isFallback() const;
    [[deprecated("Use setFallbackMarkers()")]] void setIsFallback(bool isFallback);
#endif
    const QVector<QXmppFallback> &fallbackMarkers() const;
    void setFallbackMarkers(const QVector<QXmppFallback> &);

    // XEP-0434: Trust Messages (TM)
    std::optional<QXmppTrustMessageElement> trustMessageElement() const;
    void setTrustMessageElement(const std::optional<QXmppTrustMessageElement> &trustMessageElement);

    // XEP-0444: Message Reactions
    std::optional<QXmppMessageReaction> reaction() const;
    void setReaction(const std::optional<QXmppMessageReaction> &reaction);

    // XEP-0447: Stateless file sharing
    const QVector<QXmppFileShare> &sharedFiles() const;
    void setSharedFiles(const QVector<QXmppFileShare> &sharedFiles);
    QVector<QXmppFileSourcesAttachment> fileSourcesAttachments() const;
    void setFileSourcesAttachments(const QVector<QXmppFileSourcesAttachment> &);

    // XEP-0482: Call Invites
    std::optional<QXmppCallInviteElement> callInviteElement() const;
    void setCallInviteElement(std::optional<QXmppCallInviteElement> callInviteElement);

    /// \cond
#ifdef BUILD_OMEMO
    // XEP-0384: OMEMO Encryption
    std::optional<QXmppOmemoElement> omemoElement() const;
    void setOmemoElement(const std::optional<QXmppOmemoElement> &omemoElement);
#endif

    void parse(const QDomElement &element) override final;
    virtual void parse(const QDomElement &element, QXmpp::SceMode);
    void toXml(QXmlStreamWriter *writer) const override final;
    virtual void toXml(QXmlStreamWriter *writer, QXmpp::SceMode) const;
    /// \endcond

    void parseExtensions(const QDomElement &element, QXmpp::SceMode sceMode);
    virtual bool parseExtension(const QDomElement &element, QXmpp::SceMode);
    virtual void serializeExtensions(QXmlStreamWriter *writer, QXmpp::SceMode, const QString &baseNamespace = {}) const;

private:
    QSharedDataPointer<QXmppMessagePrivate> d;
};

Q_DECLARE_METATYPE(QXmppMessage)

#endif  // QXMPPMESSAGE_H
