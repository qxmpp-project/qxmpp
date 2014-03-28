/*
 * Copyright (C) 2008-2014 The QXmpp developers
 *
 * Author:
 *  Jeremy Lainé
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

#ifndef QXMPPARCHIVEIQ_H
#define QXMPPARCHIVEIQ_H

#include "QXmppIq.h"
#include "QXmppResultSet.h"

#include <QDateTime>

/// \brief The QXmppArchiveMessage class represents an archived message
/// as defined by XEP-0136: Message Archiving.

class QXMPP_EXPORT QXmppArchiveMessage
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

/// \brief The QXmppArchiveChat class represents an archived conversation
/// as defined by XEP-0136: Message Archiving.

class QXMPP_EXPORT QXmppArchiveChat
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
    void toXml(QXmlStreamWriter *writer, const QXmppResultSetReply &rsm = QXmppResultSetReply()) const;
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

class QXMPP_EXPORT QXmppArchiveChatIq : public QXmppIq
{
public:
    QXmppArchiveChat chat() const;
    void setChat(const QXmppArchiveChat &chat);

    QXmppResultSetReply resultSetReply() const;
    void setResultSetReply(const QXmppResultSetReply &rsm);

    /// \cond
    static bool isArchiveChatIq(const QDomElement &element);

protected:
    void parseElementFromChild(const QDomElement &element);
    void toXmlElementFromChild(QXmlStreamWriter *writer) const;
    /// \endcond

private:
    QXmppArchiveChat m_chat;
    QXmppResultSetReply m_rsmReply;
};

/// \brief Represents an archive list as defined by XEP-0136: Message Archiving.
///
/// \ingroup Stanzas

class QXMPP_EXPORT QXmppArchiveListIq : public QXmppIq
{
public:
    QXmppArchiveListIq();

    QList<QXmppArchiveChat> chats() const;
    void setChats(const QList<QXmppArchiveChat> &chats);

    QString with() const;
    void setWith( const QString &with );

    QDateTime start() const;
    void setStart(const QDateTime &start );

    QDateTime end() const;
    void setEnd(const QDateTime &end );

    QXmppResultSetQuery resultSetQuery() const;
    void setResultSetQuery(const QXmppResultSetQuery &rsm);

    QXmppResultSetReply resultSetReply() const;
    void setResultSetReply(const QXmppResultSetReply &rsm);

    /// \cond
    static bool isArchiveListIq(const QDomElement &element);
    /// \endcond

protected:
    /// \cond
    void parseElementFromChild(const QDomElement &element);
    void toXmlElementFromChild(QXmlStreamWriter *writer) const;
    /// \endcond

private:
    QString m_with;
    QDateTime m_start;
    QDateTime m_end;
    QList<QXmppArchiveChat> m_chats;
    QXmppResultSetQuery m_rsmQuery;
    QXmppResultSetReply m_rsmReply;
};

/// \brief Represents an archive remove IQ as defined by XEP-0136: Message Archiving.
///
/// \ingroup Stanzas

class QXMPP_EXPORT QXmppArchiveRemoveIq : public QXmppIq
{
public:
    QString with() const;
    void setWith( const QString &with );

    QDateTime start() const;
    void setStart(const QDateTime &start );

    QDateTime end() const;
    void setEnd(const QDateTime &end );

    /// \cond
    static bool isArchiveRemoveIq(const QDomElement &element);

protected:
    void parseElementFromChild(const QDomElement &element);
    void toXmlElementFromChild(QXmlStreamWriter *writer) const;
    /// \endcond

private:
    QString m_with;
    QDateTime m_start;
    QDateTime m_end;
};

/// \brief Represents an archive retrieve IQ as defined by XEP-0136: Message Archiving.
///
/// \ingroup Stanzas

class QXMPP_EXPORT QXmppArchiveRetrieveIq : public QXmppIq
{
public:
    QXmppArchiveRetrieveIq();

    QDateTime start() const;
    void setStart(const QDateTime &start);

    QString with() const;
    void setWith(const QString &with);

    QXmppResultSetQuery resultSetQuery() const;
    void setResultSetQuery(const QXmppResultSetQuery &rsm);

    /// \cond
    static bool isArchiveRetrieveIq(const QDomElement &element);

protected:
    void parseElementFromChild(const QDomElement &element);
    void toXmlElementFromChild(QXmlStreamWriter *writer) const;
    /// \endcond

private:
    QString m_with;
    QDateTime m_start;
    QXmppResultSetQuery m_rsmQuery;
};

/// \brief Represents an archive preference IQ as defined by XEP-0136: Message Archiving.
///
/// \ingroup Stanzas

class QXMPP_EXPORT QXmppArchivePrefIq : public QXmppIq
{
public:
    /// \cond
    static bool isArchivePrefIq(const QDomElement &element);

protected:
    void parseElementFromChild(const QDomElement &element);
    void toXmlElementFromChild(QXmlStreamWriter *writer) const;
    /// \endcond
};

#endif // QXMPPARCHIVEIQ_H
