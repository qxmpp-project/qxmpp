/*
 * Copyright (C) 2008-2010 The QXmpp developers
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

/// \brief Represents an archive chat as defined by XEP-0136: Message Archiving.
///
/// It is used to get chat as a QXmppArchiveChat.
///
/// \ingroup Stanzas

class QXmppArchiveChatIq : public QXmppIq
{
public:
    QXmppArchiveChat chat() const;

    static bool isArchiveChatIq(const QDomElement &element);

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

    int max() const;
    void setMax(int max);

    QString with() const;
    void setWith( const QString &with );

    QDateTime start() const;
    void setStart(const QDateTime &start );

    QDateTime end() const;
    void setEnd(const QDateTime &end );

    static bool isArchiveListIq(const QDomElement &element);

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
    void setStart( const QDateTime &start );

    QString with() const;
    void setWith( const QString &with );

    static bool isArchiveRetrieveIq(const QDomElement &element);

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
    static bool isArchivePrefIq(const QDomElement &element);

protected:
    /// \cond
    void parseElementFromChild(const QDomElement &element);
    void toXmlElementFromChild(QXmlStreamWriter *writer) const;
    /// \endcond
};

#endif // QXMPPARCHIVEIQ_H
