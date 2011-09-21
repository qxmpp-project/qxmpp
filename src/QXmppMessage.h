/*
 * Copyright (C) 2008-2011 The QXmpp developers
 *
 * Author:
 *  Manjeet Dahiya
 *
 * Source:
 *  http://code.google.com/p/qxmpp
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

/// \brief The QXmppMessage class represents an XMPP message.
///
/// \ingroup Stanzas
///

class QXmppMessage : public QXmppStanza
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

    QXmppMessage(const QString& from = "", const QString& to = "",
                 const QString& body = "", const QString& thread = "");
    ~QXmppMessage();

    QString body() const;
    void setBody(const QString&);

    bool isAttentionRequested() const;
    void setAttentionRequested(bool requested);

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

    /// \cond
    void parse(const QDomElement &element);
    void toXml(QXmlStreamWriter *writer) const;
    /// \endcond

private:
    /// This enum describe a type of message timestamp.
    enum StampType
    {
        LegacyDelayedDelivery,  ///< XEP-0091: Legacy Delayed Delivery
        DelayedDelivery,        ///< XEP-0203: Delayed Delivery
    };

    QString getTypeStr() const;
    void setTypeFromStr(const QString&);

    Type m_type;
    QDateTime m_stamp;
    StampType m_stampType;
    State m_state;

    bool m_attentionRequested;
    QString m_body;
    QString m_subject;
    QString m_thread;
};

#endif // QXMPPMESSAGE_H
