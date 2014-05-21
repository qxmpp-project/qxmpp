/*
 * Copyright (C) 2008-2014 The QXmpp developers
 *
 * Author:
 *  Manjeet Dahiya
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

#include <QDateTime>
#include "QXmppStanza.h"

class QXmppMessagePrivate;

/// \brief The QXmppMessage class represents an XMPP message.
///
/// \ingroup Stanzas
///

class QXMPP_EXPORT QXmppMessage : public QXmppStanza
{
public:
    /// This enum described a message type.
    enum Type
    {
        Error = 0,
        Normal,
        Chat,
        GroupChat,
        Headline
    };

    /// This enum describes a chat state as defined by
    /// XEP-0085 : Chat State Notifications.
    enum State
    {
        None = 0,   ///< The message does not contain any chat state information.
        Active,     ///< User is actively participating in the chat session.
        Inactive,   ///< User has not been actively participating in the chat session.
        Gone,       ///< User has effectively ended their participation in the chat session.
        Composing,  ///< User is composing a message.
        Paused,     ///< User had been composing but now has stopped.
    };

    /// This enum describes a chat marker as defined by
    /// XEP-0333 : Char Markers
    enum Marker {
        NoMarker = 0,
        Received,
        Displayed,
        Acknowledged
    };

    QXmppMessage(const QString& from = QString(), const QString& to = QString(),
                 const QString& body = QString(), const QString& thread = QString());

    QXmppMessage(const QXmppMessage &other);
    ~QXmppMessage();

    QXmppMessage& operator=(const QXmppMessage &other);

    QString body() const;
    void setBody(const QString&);

    bool isAttentionRequested() const;
    void setAttentionRequested(bool requested);

    bool isReceiptRequested() const;
    void setReceiptRequested(bool requested);

    QString mucInvitationJid() const;
    void setMucInvitationJid(const QString &jid);

    QString mucInvitationPassword() const;
    void setMucInvitationPassword(const QString &password);

    QString mucInvitationReason() const;
    void setMucInvitationReason(const QString &reason);

    QString receiptId() const;
    void setReceiptId(const QString &id);

    QDateTime stamp() const;
    void setStamp(const QDateTime &stamp);

    QXmppMessage::State state() const;
    void setState(QXmppMessage::State);

    QString subject() const;
    void setSubject(const QString&);

    QString thread() const;
    void setThread(const QString&);

    QXmppMessage::Type type() const;
    void setType(QXmppMessage::Type);

    QString xhtml() const;
    void setXhtml(const QString &xhtml);

    // XEP-0333
    bool isMarkable() const;
    void setMarkable(const bool);

    QString markedId() const;
    void setMarkerId(const QString&);

    QString markedThread() const;
    void setMarkedThread(const QString&);

    Marker marker() const;
    void setMarker(const Marker);

    /// \cond
    void parse(const QDomElement &element);
    void toXml(QXmlStreamWriter *writer) const;
    /// \endcond

private:
    QSharedDataPointer<QXmppMessagePrivate> d;
};

#endif // QXMPPMESSAGE_H
