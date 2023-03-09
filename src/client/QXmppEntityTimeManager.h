// SPDX-FileCopyrightText: 2010 Manjeet Dahiya <manjeetdahiya@gmail.com>
// SPDX-FileCopyrightText: 2022 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPENTITYTIMEMANAGER_H
#define QXMPPENTITYTIMEMANAGER_H

#include "QXmppClientExtension.h"

#include <variant>

template<class T>
class QXmppTask;
class QXmppEntityTimeIq;
struct QXmppError;

///
/// \brief The QXmppEntityTimeManager class provided the functionality to get
/// the local time of an entity as defined by \xep{0202}: Entity Time.
///
/// \ingroup Managers
///
class QXMPP_EXPORT QXmppEntityTimeManager : public QXmppClientExtension
{
    Q_OBJECT

public:
    QString requestTime(const QString &jid);

    using EntityTimeResult = std::variant<QXmppEntityTimeIq, QXmppError>;
    QXmppTask<EntityTimeResult> requestEntityTime(const QString &jid);

    /// \cond
    QStringList discoveryFeatures() const override;
    bool handleStanza(const QDomElement &element) override;
    std::variant<QXmppEntityTimeIq, QXmppStanza::Error> handleIq(QXmppEntityTimeIq iq);
    /// \endcond

Q_SIGNALS:
    /// \brief This signal is emitted when a time response is received. It's not
    /// emitted when the QFuture-based request is used.
    void timeReceived(const QXmppEntityTimeIq &);
};

#endif  // QXMPPENTITYTIMEMANAGER_H
