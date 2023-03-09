// SPDX-FileCopyrightText: 2016 Niels Ole Salscheider <niels_ole@salscheider-online.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPMAMMANAGER_H
#define QXMPPMAMMANAGER_H

#include "QXmppClientExtension.h"
#include "QXmppError.h"
#include "QXmppMamIq.h"
#include "QXmppResultSet.h"

#include <variant>

#include <QDateTime>

template<typename T>
class QXmppTask;
class QXmppMessage;
class QXmppMamManagerPrivate;

///
/// \brief The QXmppMamManager class makes it possible to access message
/// archives as defined by \xep{0313}: Message Archive Management.
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
///
/// \since QXmpp 1.0
///
class QXMPP_EXPORT QXmppMamManager : public QXmppClientExtension
{
    Q_OBJECT

public:
    struct QXMPP_EXPORT RetrievedMessages
    {
        QXmppMamResultIq result;
        QVector<QXmppMessage> messages;
    };

    using RetrieveResult = std::variant<RetrievedMessages, QXmppError>;

    QXmppMamManager();
    ~QXmppMamManager();

    QString retrieveArchivedMessages(const QString &to = QString(),
                                     const QString &node = QString(),
                                     const QString &jid = QString(),
                                     const QDateTime &start = QDateTime(),
                                     const QDateTime &end = QDateTime(),
                                     const QXmppResultSetQuery &resultSetQuery = QXmppResultSetQuery());
    QXmppTask<RetrieveResult> retrieveMessages(const QString &to = QString(),
                                               const QString &node = QString(),
                                               const QString &jid = QString(),
                                               const QDateTime &start = QDateTime(),
                                               const QDateTime &end = QDateTime(),
                                               const QXmppResultSetQuery &resultSetQuery = QXmppResultSetQuery());

    /// \cond
    QStringList discoveryFeatures() const override;
    bool handleStanza(const QDomElement &element) override;
    /// \endcond

Q_SIGNALS:
    /// This signal is emitted when an archived message is received
    void archivedMessageReceived(const QString &queryId,
                                 const QXmppMessage &message);

    /// This signal is emitted when all results for a request have been received
    void resultsRecieved(const QString &queryId,
                         const QXmppResultSetReply &resultSetReply,
                         bool complete);

private:
    std::unique_ptr<QXmppMamManagerPrivate> d;
};

#endif
