/*
 * Copyright (C) 2008-2011 The QXmpp developers
 *
 * Author:
 *  Jeremy Lainé
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

#ifndef QXMPPARCHIVEIQ_H
#define QXMPPARCHIVEIQ_H

#include "QXmppIq.h"

#include <QDateTime>

class QXmlStreamWriter;
class QDomElement;

/// \brief The QXmppArchiveMessage represents an archived message
/// as defined by XEP-0136: Message Archiving.

class QXmppArchiveMessage
{
public:
    QXmppArchiveMessage();

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

/// \brief The QXmppArchiveChat represents an archived conversation
/// as defined by XEP-0136: Message Archiving.

class QXmppArchiveChat
{
public:
    QXmppArchiveChat();

    QList<QXmppArchiveMessage> messages() const;
    void setMessages(const QList<QXmppArchiveMessage> &messages);

    QDateTime start() const;
    void setStart(const QDateTime &start);

    QString subject() const;
    void setSubject(const QString &subject);

    QString thread() const;
    void setThread(const QString &thread);

    int version() const;
    void setVersion(int version);

    QString with() const;
    void setWith(const QString &with);

    /// \cond
    void parse(const QDomElement &element);
    void toXml(QXmlStreamWriter *writer) const;
    /// \endcond

private:
    QList<QXmppArchiveMessage> m_messages;
    QDateTime m_start;
    QString m_subject;
    QString m_thread;
    int m_version;
    QString m_with;
};

/// \brief Represents an archive chat as defined by XEP-0136: Message Archiving.
///
/// It is used to get chat as a QXmppArchiveChat.
///
/// \ingroup Stanzas

class QXmppArchiveChatIq : public QXmppIq
{
public:
    QXmppArchiveChat chat() const;
    void setChat(const QXmppArchiveChat &chat);

    /// \cond
    static bool isArchiveChatIq(const QDomElement &element);
    /// \endcond

protected:
    /// \cond
    void parseElementFromChild(const QDomElement &element);
    void toXmlElementFromChild(QXmlStreamWriter *writer) const;
    /// \endcond

private:
    QXmppArchiveChat m_chat;
};

/// \brief Represents an archive list as defined by XEP-0136: Message Archiving.
///
/// \ingroup Stanzas

class QXmppArchiveListIq : public QXmppIq
{
public:
    QXmppArchiveListIq();

    QList<QXmppArchiveChat> chats() const;
    void setChats(const QList<QXmppArchiveChat> &chats);

    int max() const;
    void setMax(int max);

    QString with() const;
    void setWith( const QString &with );

    QDateTime start() const;
    void setStart(const QDateTime &start );

    QDateTime end() const;
    void setEnd(const QDateTime &end );

    /// \cond
    static bool isArchiveListIq(const QDomElement &element);
    /// \endcond

protected:
    /// \cond
    void parseElementFromChild(const QDomElement &element);
    void toXmlElementFromChild(QXmlStreamWriter *writer) const;
    /// \endcond

private:
    int m_max;
    QString m_with;
    QDateTime m_start;
    QDateTime m_end;
    QList<QXmppArchiveChat> m_chats;
};

/// \brief Represents an archive retrieve IQ as defined by XEP-0136: Message Archiving.
///
/// \ingroup Stanzas

class QXmppArchiveRetrieveIq : public QXmppIq
{
public:
    QXmppArchiveRetrieveIq();

    int max() const;
    void setMax(int max);

    QDateTime start() const;
    void setStart(const QDateTime &start);

    QString with() const;
    void setWith(const QString &with);

    /// \cond
    static bool isArchiveRetrieveIq(const QDomElement &element);
    /// \endcond

protected:
    /// \cond
    void parseElementFromChild(const QDomElement &element);
    void toXmlElementFromChild(QXmlStreamWriter *writer) const;
    /// \endcond

private:
    int m_max;
    QString m_with;
    QDateTime m_start;
};

/// \brief Represents an archive preference IQ as defined by XEP-0136: Message Archiving.
///
/// \ingroup Stanzas

class QXmppArchivePrefIq : public QXmppIq
{
public:
    /// \cond
    static bool isArchivePrefIq(const QDomElement &element);
    /// \endcond

protected:
    /// \cond
    void parseElementFromChild(const QDomElement &element);
    void toXmlElementFromChild(QXmlStreamWriter *writer) const;
    /// \endcond
};

#endif // QXMPPARCHIVEIQ_H
