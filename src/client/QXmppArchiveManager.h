// SPDX-FileCopyrightText: 2010 Jeremy Lainé <jeremy.laine@m4x.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "QXmppArchiveIq.h"
#include "QXmppClientExtension.h"
#include "QXmppResultSet.h"

#include <QDateTime>

/// \brief The QXmppArchiveManager class makes it possible to access message
/// archives as defined by \xep{0136}: Message Archiving.
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
    QStringList discoveryFeatures() const override;
    bool handleStanza(const QDomElement &element) override;
    /// \endcond

Q_SIGNALS:
    /// This signal is emitted when archive list is received
    /// after calling listCollections()
    void archiveListReceived(const QList<QXmppArchiveChat> &, const QXmppResultSetReply &rsm = QXmppResultSetReply());

    /// This signal is emitted when archive chat is received
    /// after calling retrieveCollection()
    void archiveChatReceived(const QXmppArchiveChat &, const QXmppResultSetReply &rsm = QXmppResultSetReply());
};
