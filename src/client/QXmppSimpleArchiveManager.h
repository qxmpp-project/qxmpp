/*
 * Copyright (C) 2008-2012 The QXmpp developers
 *
 * Author:
 *  James Turner (james.turner@kdab.com)
 *  Truphone Labs (labs@truphone.com)
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

#ifndef QXMPPSIMPLEARCHIVEMANAGER_H
#define QXMPPSIMPLEARCHIVEMANAGER_H

#include <QDateTime>

#include "QXmppClientExtension.h"
#include "QXmppResultSet.h"
#include "QXmppMessage.h"

class QXmppSimpleArchiveQueryIq;

/// \brief The QXmppSimpleArchiveManager class makes it possible to access message
/// archives as defined by XEP-0313: Message Archive Management
///
/// To make use of this manager, you need to instantiate it and load it into
/// the QXmppClient instance as follows:
///
/// \code
/// QXmppSimpleArchiveManager *manager = new QXmppSimpleArchiveManager;
/// client->addExtension(manager);
/// \endcode
///
/// \note Few servers support message archiving. Check if the server in use supports
/// this XEP.
///
/// \ingroup Managers

class QXMPP_EXPORT QXmppSimpleArchiveManager : public QXmppClientExtension
{
    Q_OBJECT

public:
    void retrieveMessages(const QString &jid, const QDateTime &start = QDateTime(), const QDateTime &end = QDateTime(),
                         const QXmppResultSetQuery &rsm = QXmppResultSetQuery());

    /// \cond
    QStringList discoveryFeatures() const;
    bool handleStanza(const QDomElement &element);
    /// \endcond

signals:
    /// This signal is emitted when archive list is received
    /// after calling retrieveMessages()
    void archiveMessagesReceived(const QString &jid, const QList<QXmppMessage>&, const QXmppResultSetReply &rsm = QXmppResultSetReply());
    
private:
    struct PendingQuery {
        QString jid;
        QList<QXmppMessage> messages;
    };

    QMap<QString, PendingQuery> m_pendingQueries;
};

#endif
