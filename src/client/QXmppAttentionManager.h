/*
 * Copyright (C) 2008-2022 The QXmpp developers
 *
 * Author:
 *  Linus Jahn
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

#ifndef QXMPPATTENTIONMANAGER_H
#define QXMPPATTENTIONMANAGER_H

#include "QXmppClientExtension.h"

#include <QTime>

class QXmppAttentionManagerPrivate;
class QXmppMessage;

class QXMPP_EXPORT QXmppAttentionManager : public QXmppClientExtension
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

public Q_SLOTS:
    QString requestAttention(const QString &jid, const QString &message = {});

Q_SIGNALS:
    void attentionRequested(const QXmppMessage &message, bool isTrusted);
    void attentionRequestRateLimited(const QXmppMessage &message);

protected:
    void setClient(QXmppClient *client) override;
    bool handleStanza(const QDomElement &stanza) override;

private Q_SLOTS:
    void handleMessageReceived(const QXmppMessage &message);

private:
    QXmppAttentionManagerPrivate *const d;
};

#endif  // QXMPPATTENTIONMANAGER_H
