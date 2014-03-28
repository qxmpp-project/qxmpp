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

#include "QXmppRemoteMethod.h"
#include "QXmppClient.h"
#include "QXmppUtils.h"

#include <QDebug>
#include <QEventLoop>
#include <QTimer>

QXmppRemoteMethod::QXmppRemoteMethod(const QString &jid, const QString &method, const QVariantList &args, QXmppClient *client) :
        QObject(client), m_client(client)
{
    m_payload.setTo( jid );
    m_payload.setFrom( client->configuration().jid() );
    m_payload.setMethod( method );
    m_payload.setArguments( args );
}

QXmppRemoteMethodResult QXmppRemoteMethod::call( )
{
    // FIXME : spinning an event loop is a VERY bad idea, it can cause
    // us to lose incoming packets
    QEventLoop loop(this);
    connect( this, SIGNAL(callDone()), &loop, SLOT(quit()));
    QTimer::singleShot(30000,&loop, SLOT(quit())); // Timeout incase the other end hangs...

    m_client->sendPacket( m_payload );

    loop.exec( QEventLoop::ExcludeUserInputEvents | QEventLoop::WaitForMoreEvents );
    return m_result;
}

void QXmppRemoteMethod::gotError( const QXmppRpcErrorIq &iq )
{
    if ( iq.id() == m_payload.id() )
    {
        m_result.hasError = true;
        m_result.errorMessage = iq.error().text();
        m_result.code = iq.error().type();
        emit callDone();
    }
}

void QXmppRemoteMethod::gotResult( const QXmppRpcResponseIq &iq )
{
    if ( iq.id() == m_payload.id() )
    {
        m_result.hasError = false;
        // FIXME: we don't handle multiple responses
        m_result.result = iq.values().first();
        emit callDone();
    }
}
