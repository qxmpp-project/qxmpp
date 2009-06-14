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

    QXmppMessage(const QString& from = "", const QString& to = "", const QString& body = "", 
        const QString& thread = "");
    ~QXmppMessage();
    
    QXmppMessage::Type getType() const;
    QString getTypeStr() const;
    void setType(QXmppMessage::Type);
    void setTypeFromStr(const QString&);

    QString getBody() const;
    void setBody(const QString&);
    QString getSubject() const;
    void setSubject(const QString&);
    QString getThread() const;
    void setThread(const QString&);

    QByteArray toXml() const;
private:
    Type m_type;

    QString m_body;
    QString m_subject;
    QString m_thread;
};

#endif // QXMPPMESSAGE_H
