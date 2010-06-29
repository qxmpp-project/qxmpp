/*
 * Copyright (C) 2008-2009 Manjeet Dahiya
 *
 * Author:
 *	Manjeet Dahiya
 *
 * Source:
 *	http://code.google.com/p/qxmpp
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

class QXmppMessage : public QXmppStanza
{
public:
    enum Type
    {
        Error = 0,
        Normal,
        Chat,
        GroupChat,
        Headline
    };

    // XEP-0085 : Chat State Notifications
    // http://xmpp.org/extensions/xep-0085.html
    enum State
    {
        None = 0,
        Active,
        Inactive,
        Gone,
        Composing,
        Paused,
    };

    /// Type of the message timestamp.
    enum StampType
    {
        LegacyDelayedDelivery,  ///< XEP-0091: Legacy Delayed Delivery
        DelayedDelivery,        ///< XEP-0203: Delayed Delivery
    };

    QXmppMessage(const QString& from = "", const QString& to = "",
                 const QString& body = "", const QString& thread = "");
    ~QXmppMessage();
    
    QXmppMessage::Type type() const;
    void setType(QXmppMessage::Type);

    QDateTime stamp() const;
    void setStamp(const QDateTime &stamp);

    QXmppMessage::State state() const;
    void setState(QXmppMessage::State);

    QString body() const;
    void setBody(const QString&);

    QString subject() const;
    void setSubject(const QString&);

    QString thread() const;
    void setThread(const QString&);

    void parse(const QDomElement &element);
    void toXml(QXmlStreamWriter *writer) const;

    // deprecated accessors, use the form without "get" instead
    QXmppMessage::Type Q_DECL_DEPRECATED getType() const;
    QXmppMessage::State Q_DECL_DEPRECATED getState() const;
    QString Q_DECL_DEPRECATED getBody() const;
    QString Q_DECL_DEPRECATED getSubject() const;
    QString Q_DECL_DEPRECATED getThread() const;

private:
    QString getTypeStr() const;
    void setTypeFromStr(const QString&);

    Type m_type;
    QDateTime m_stamp;
    StampType m_stampType;
    State m_state;

    QString m_body;
    QString m_subject;
    QString m_thread;
};

#endif // QXMPPMESSAGE_H
