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
    QString body() const;
    void setBody(const QString &body);

    QDateTime date() const;
    void setDate(const QDateTime &date);

    bool isReceived() const;
    void setReceived(bool isReceived);

private:
    QString m_body;
    QDateTime m_date;
    bool m_received;
};

class QXmppArchiveChat
{
public:
    QList<QXmppArchiveMessage> messages() const;
    QDateTime start() const;
    QString subject() const;
    int version() const;
    QString with() const;

    void parse(const QDomElement &element);

private:
    QList<QXmppArchiveMessage> m_messages;
    QDateTime m_start;
    QString m_subject;
    int m_version;
    QString m_with;
};

class QXmppArchiveChatIq : public QXmppIq
{
public:
    static bool isArchiveChatIq(const QDomElement &element);
    void parse(const QDomElement &element);

    QXmppArchiveChat chat() const;

private:
    QXmppArchiveChat m_chat;
};

class QXmppArchiveListIq : public QXmppIq
{
public:
    QXmppArchiveListIq();

    QList<QXmppArchiveChat> chats() const;

    int max() const;
    void setMax(int max);

    QString with() const;
    void setWith( const QString &with );

    QDateTime start() const;
    void setStart(const QDateTime &start );

    QDateTime end() const;
    void setEnd(const QDateTime &end );

    static bool isArchiveListIq(const QDomElement &element);
    void parse(const QDomElement &element);
    void toXmlElementFromChild(QXmlStreamWriter *writer) const;

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

    int max() const;
    void setMax(int max);

    QDateTime start() const;
    void setStart( const QDateTime &start );

    QString with() const;
    void setWith( const QString &with );

    void toXmlElementFromChild(QXmlStreamWriter *writer) const;

private:
    int m_max;
    QString m_with;
    QDateTime m_start;
};

class QXmppArchivePrefIq : public QXmppIq
{
public:
    static bool isArchivePrefIq(const QDomElement &element);
    void parse(const QDomElement &element);
    void toXmlElementFromChild(QXmlStreamWriter *writer) const;
};

#endif // QXMPPARCHIVEIQ_H
