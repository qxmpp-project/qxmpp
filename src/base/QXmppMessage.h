/*
 * Copyright (C) 2008-2020 The QXmpp developers
 *
 * Author:
 *  Manjeet Dahiya
 *  Jeremy Lainé
 *  Linus Jahn
 *
 * Source:
 *  https://github.com/qxmpp-project/qxmpp
 *
 * This file is a part of QXmpp library.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 */

#ifndef QXMPPMESSAGE_H
#define QXMPPMESSAGE_H

// Required for source compatibility
#include "QXmppStanza.h"

#include <QDateTime>

class QXmppMessagePrivate;
class QXmppBitsOfBinaryDataList;

///
/// \brief The QXmppMessage class represents an XMPP message.
///
/// \ingroup Stanzas
///
class QXMPP_EXPORT QXmppMessage : public QXmppStanza
{
public:
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

    ///
    /// This enum describes different end-to-end encryption methods. These can
    /// be used to mark a message explicitly as encrypted with a specific
    /// algothim. See \xep{0380}: Explicit Message Encryption for details.
    ///
    /// \since QXmpp 1.1
    ///
    enum EncryptionMethod {
        NoEncryption,       ///< No encryption
        UnknownEncryption,  ///< Unknown encryption
        OTR,                ///< \xep{0364}: Current Off-the-Record Messaging Usage
        LegacyOpenPGP,      ///< \xep{0027}: Current Jabber OpenPGP Usage
        OX,                 ///< \xep{0373}: OpenPGP for XMPP
        OMEMO               ///< \xep{0384}: OMEMO Encryption
    };

    QXmppMessage(const QString &from = QString(), const QString &to = QString(),
                 const QString &body = QString(), const QString &thread = QString());

    QXmppMessage(const QXmppMessage &other);
    ~QXmppMessage() override;

    QXmppMessage &operator=(const QXmppMessage &other);

    bool isXmppStanza() const override;

    QString body() const;
    void setBody(const QString &);

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
    QString mixUserJid() const;
    void setMixUserJid(const QString &);

    QString mixUserNick() const;
    void setMixUserNick(const QString &);

    // XEP-0380: Explicit Message Encryption
    EncryptionMethod encryptionMethod() const;
    void setEncryptionMethod(EncryptionMethod);
    QString encryptionMethodNs() const;
    void setEncryptionMethodNs(const QString &);

    QString encryptionName() const;
    void setEncryptionName(const QString &);

    // XEP-0382: Spoiler messages
    bool isSpoiler() const;
    void setIsSpoiler(bool);

    QString spoilerHint() const;
    void setSpoilerHint(const QString &);

    // XEP-0428: Fallback Indication
    bool isFallback() const;
    void setIsFallback(bool isFallback);

    /// \cond
    void parse(const QDomElement &element) override;
    void toXml(QXmlStreamWriter *writer) const override;
    /// \endcond

private:
    void parseExtension(const QDomElement &element, QXmppElementList &unknownExtensions);
    void parseXElement(const QDomElement &element, QXmppElementList &unknownElements);

    QSharedDataPointer<QXmppMessagePrivate> d;
};

#endif  // QXMPPMESSAGE_H
