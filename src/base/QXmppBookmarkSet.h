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

#ifndef QXMPPBOOKMARKSET_H
#define QXMPPBOOKMARKSET_H

#include <QList>
#include <QUrl>

#include "QXmppStanza.h"

/// \brief The QXmppBookmarkConference class represents a bookmark for a conference room,
/// as defined by XEP-0048: Bookmarks.
///
class QXMPP_EXPORT QXmppBookmarkConference
{
public:
    QXmppBookmarkConference();

    bool autoJoin() const;
    void setAutoJoin(bool autoJoin);

    QString jid() const;
    void setJid(const QString &jid);

    QString name() const;
    void setName(const QString &name);

    QString nickName() const;
    void setNickName(const QString &nickName);

private:
    bool m_autoJoin;
    QString m_jid;
    QString m_name;
    QString m_nickName;
};

/// \brief The QXmppBookmarkUrl class represents a bookmark for a web page,
/// as defined by XEP-0048: Bookmarks.
///
class QXMPP_EXPORT QXmppBookmarkUrl
{
public:
    QString name() const;
    void setName(const QString &name);

    QUrl url() const;
    void setUrl(const QUrl &url);

private:
    QString m_name;
    QUrl m_url;
};

/// \brief The QXmppbookmarkSets class represents a set of bookmarks, as defined
/// by XEP-0048: Bookmarks.
///
class QXMPP_EXPORT QXmppBookmarkSet
{
public:
    QList<QXmppBookmarkConference> conferences() const;
    void setConferences(const QList<QXmppBookmarkConference> &conferences);

    QList<QXmppBookmarkUrl> urls() const;
    void setUrls(const QList<QXmppBookmarkUrl> &urls);

    /// \cond
    static bool isBookmarkSet(const QDomElement &element);
    void parse(const QDomElement &element);
    void toXml(QXmlStreamWriter *writer) const;
    /// \endcond

private:
    QList<QXmppBookmarkConference> m_conferences;
    QList<QXmppBookmarkUrl> m_urls;
};

#endif
