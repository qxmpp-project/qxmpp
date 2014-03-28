/*
 * Copyright (C) 2008-2014 The QXmpp developers
 *
 * Authors:
 *  Ian Reinhart Geiser
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

#ifndef QXMPPREMOTEMETHOD_H
#define QXMPPREMOTEMETHOD_H

#include <QObject>
#include <QVariant>

#include "QXmppRpcIq.h"

class QXmppClient;

struct QXmppRemoteMethodResult {
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
    QXmppRemoteMethodResult call( );

private slots:
    void gotError( const QXmppRpcErrorIq &iq );
    void gotResult( const QXmppRpcResponseIq &iq );

signals:
    void callDone();

private:
    QXmppRpcInvokeIq m_payload;
    QXmppClient *m_client;
    QXmppRemoteMethodResult m_result;

};

#endif // QXMPPREMOTEMETHOD_H
