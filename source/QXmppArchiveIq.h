/*
 * Copyright (C) 2010 Bolloré telecom
 *
 * Author:
 *	Jeremy Lainé
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

#ifndef QXMPPARCHIVEIQ_H
#define QXMPPARCHIVEIQ_H

#include "QXmppIq.h"

#include <QDateTime>

class QXmlStreamWriter;
class QDomElement;

class QXmppArchiveMessage
{
public:
    QDateTime datetime;
    bool local;
    QString body;
};

class QXmppArchiveChat
{
public:
    QString with;
    QDateTime start;

    QString subject;
    QList<QXmppArchiveMessage> messages;
    int version;
};

class QXmppArchiveChatIq : public QXmppIq
{
public:
    void parse( QDomElement &element );
    static bool isArchiveChatIq( QDomElement &element );

    QXmppArchiveChat chat() const;

private:
    QXmppArchiveChat m_chat;
};

class QXmppArchiveListIq : public QXmppIq
{
public:
    QXmppArchiveListIq();
    void toXmlElementFromChild(QXmlStreamWriter *writer) const;
    void parse( QDomElement &element );
    static bool isArchiveListIq( QDomElement &element );

    QList<QXmppArchiveChat> chats() const;

    int max() const;
    void setMax(int max);

    QString with() const;
    void setWith( const QString &with );

    QDateTime start() const;
    void setStart(const QDateTime &start );

    QDateTime end() const;
    void setEnd(const QDateTime &end );

private:
    int m_max;
    QString m_with;
    QDateTime m_start;
    QDateTime m_end;
    QList<QXmppArchiveChat> m_chats;
};

class QXmppArchiveRetrieveIq : public QXmppIq
{
public:
    QXmppArchiveRetrieveIq();
    void toXmlElementFromChild(QXmlStreamWriter *writer) const;

    int max() const;
    void setMax(int max);

    QDateTime start() const;
    void setStart( const QDateTime &start );

    QString with() const;
    void setWith( const QString &with );

private:
    int m_max;
    QString m_with;
    QDateTime m_start;
};

class QXmppArchivePrefIq : public QXmppIq
{
public:
    void toXmlElementFromChild(QXmlStreamWriter *writer) const;
    void parse( QDomElement &element );
    static bool isArchivePrefIq( QDomElement &element );
};

#endif // QXMPPARCHIVEIQ_H
