// SPDX-FileCopyrightText: 2020 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPATTENTIONMANAGER_H
#define QXMPPATTENTIONMANAGER_H

#include "QXmppClientExtension.h"
#include "QXmppMessageHandler.h"

#include <QTime>

class QXmppAttentionManagerPrivate;
class QXmppMessage;

class QXMPP_EXPORT QXmppAttentionManager : public QXmppClientExtension, public QXmppMessageHandler
{
    Q_OBJECT

public:
    QXmppAttentionManager(quint8 allowedAttempts = 3, QTime timeFrame = QTime(0, 15, 0));
    ~QXmppAttentionManager();

    QStringList discoveryFeatures() const override;

    quint8 allowedAttempts() const;
    void setAllowedAttempts(quint8 allowedAttempts);

    QTime allowedAttemptsTimeInterval() const;
    void setAllowedAttemptsTimeInterval(QTime interval);

    bool handleMessage(const QXmppMessage &) override;

public Q_SLOTS:
    QString requestAttention(const QString &jid, const QString &message = {});

Q_SIGNALS:
    void attentionRequested(const QXmppMessage &message, bool isTrusted);
    void attentionRequestRateLimited(const QXmppMessage &message);

private:
    QXmppAttentionManagerPrivate *const d;
};

#endif  // QXMPPATTENTIONMANAGER_H
