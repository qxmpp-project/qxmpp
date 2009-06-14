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


#ifndef QXMPPPRESENCE_H
#define QXMPPPRESENCE_H

#include "QXmppStanza.h"

class QXmppPresence : public QXmppStanza
{
public:
    enum Type
    {
        Error = 0,
        Available,
        Unavailable,
        Subscribe,
        Subscribed,
        Unsubscribe,
        Unsubscribed,
        Probe
    };

    class Status
    {
    public:
        enum Type
        {
            Offline = 0, 
            Online, 
            Away, 
            XA, 
            DND, 
            Chat,
            Invisible 
        };

        Status(QXmppPresence::Status::Type type = QXmppPresence::Status::Online,
            const QString statusText = "", int priority = 0);

        QXmppPresence::Status::Type getType() const;
        void setType(QXmppPresence::Status::Type);
        void setTypeFromStr(const QString&);
        QString getTypeStr() const;

        QString getStatusText() const;
        void setStatusText(const QString&);

        int getPriority() const;
        void setPriority(int);

    private:
        QXmppPresence::Status::Type m_type;
        QString m_statusText;
        int m_priority;
    };

    QXmppPresence(QXmppPresence::Type type = QXmppPresence::Available, 
        const QXmppPresence::Status& status = QXmppPresence::Status());
    ~QXmppPresence();

    QXmppPresence::Type getType() const;
    void setType(QXmppPresence::Type);
    void setTypeFromStr(const QString&);

    QXmppPresence::Status getStatus() const;
    void setStatus(const QXmppPresence::Status&);
    QByteArray toXml() const;

private:
    Type m_type;
    QXmppPresence::Status m_status;

    QString getTypeStr() const;
};

#endif // QXMPPPRESENCE_H
