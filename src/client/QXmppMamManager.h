/*
 * Copyright (C) 2016-2017 The QXmpp developers
 *
 * Author:
 *  Niels Ole Salscheider
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

#ifndef QXMPPMAMMANAGER_H
#define QXMPPMAMMANAGER_H

#include <QDateTime>

#include "QXmppClientExtension.h"
#include "QXmppResultSet.h"

class QXmppMessage;

/// \brief The QXmppMamManager class makes it possible to access message
/// archives as defined by XEP-0313: Message Archive Management.
///
/// To make use of this manager, you need to instantiate it and load it into
/// the QXmppClient instance as follows:
///
/// \code
/// QXmppMamManager *manager = new QXmppMamManager;
/// client->addExtension(manager);
/// \endcode
///
/// \ingroup Managers

class QXMPP_EXPORT QXmppMamManager : public QXmppClientExtension
{
    Q_OBJECT

public:
    QString retrieveArchivedMessages(const QString &to = QString(),
                                     const QString &node = QString(),
                                     const QString &jid = QString(),
                                     const QDateTime &start = QDateTime(),
                                     const QDateTime &end = QDateTime(),
                                     const QXmppResultSetQuery &resultSetQuery = QXmppResultSetQuery());

    /// \cond
    QStringList discoveryFeatures() const;
    bool handleStanza(const QDomElement &element);
    /// \endcond

signals:
    /// This signal is emitted when an archived message is received
    void archivedMessageReceived(const QString &queryId,
                                 const QXmppMessage &message);

    /// This signal is emitted when all results for a request have been received
    void resultsRecieved(const QString &queryId,
                         const QXmppResultSetReply &resultSetReply,
                         bool complete);
};

#endif
