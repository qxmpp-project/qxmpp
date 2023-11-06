// SPDX-FileCopyrightText: 2009 Ian Reinhart Geiser <geiseri@kde.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "QXmppRpcIq.h"

#include <QObject>
#include <QVariant>

class QXmppClient;

struct QXmppRemoteMethodResult
{
    QXmppRemoteMethodResult() : hasError(false), code(0) { }
    bool hasError;
    int code;
    QString errorMessage;
    QVariant result;
};

class QXMPP_EXPORT QXmppRemoteMethod : public QObject
{
    Q_OBJECT
public:
    QXmppRemoteMethod(const QString &jid, const QString &method, const QVariantList &args, QXmppClient *client);
    QXmppRemoteMethodResult call();

private Q_SLOTS:
    void gotError(const QXmppRpcErrorIq &iq);
    void gotResult(const QXmppRpcResponseIq &iq);

Q_SIGNALS:
    void callDone();

private:
    QXmppRpcInvokeIq m_payload;
    QXmppClient *m_client;
    QXmppRemoteMethodResult m_result;
};
