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

#ifndef QXMPPARCHIVEMANAGER_H
#define QXMPPARCHIVEMANAGER_H

#include <QDateTime>

#include "QXmppClientExtension.h"
#include "QXmppResultSet.h"

class QXmppArchiveChat;
class QXmppArchiveChatIq;
class QXmppArchiveListIq;
class QXmppArchivePrefIq;

/// \brief The QXmppArchiveManager class makes it possible to access message
/// archives as defined by XEP-0136: Message Archiving.
///
/// To make use of this manager, you need to instantiate it and load it into
/// the QXmppClient instance as follows:
///
/// \code
/// QXmppArchiveManager *manager = new QXmppArchiveManager;
/// client->addExtension(manager);
/// \endcode
///
/// \note Few servers support message archiving. Check if the server in use supports
/// this XEP.
///
/// \ingroup Managers

class QXMPP_EXPORT QXmppArchiveManager : public QXmppClientExtension
{
    Q_OBJECT

public:
    void listCollections(const QString &jid, const QDateTime &start = QDateTime(), const QDateTime &end = QDateTime(),
                         const QXmppResultSetQuery &rsm = QXmppResultSetQuery());
    void listCollections(const QString &jid, const QDateTime &start, const QDateTime &end, int max);
    void removeCollections(const QString &jid, const QDateTime &start = QDateTime(), const QDateTime &end = QDateTime());
    void retrieveCollection(const QString &jid, const QDateTime &start, const QXmppResultSetQuery &rsm = QXmppResultSetQuery());
    void retrieveCollection(const QString &jid, const QDateTime &start, int max);

    /// \cond
    QStringList discoveryFeatures() const;
    bool handleStanza(const QDomElement &element);
    /// \endcond

signals:
    /// This signal is emitted when archive list is received
    /// after calling listCollections()
    void archiveListReceived(const QList<QXmppArchiveChat>&, const QXmppResultSetReply &rsm = QXmppResultSetReply());

    /// This signal is emitted when archive chat is received
    /// after calling retrieveCollection()
    void archiveChatReceived(const QXmppArchiveChat&, const QXmppResultSetReply &rsm = QXmppResultSetReply());
};

#endif
