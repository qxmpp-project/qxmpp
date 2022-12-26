// SPDX-FileCopyrightText: 2009 Ian Reinhart Geiser <geiseri@kde.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppRemoteMethod.h"

#include "QXmppClient.h"
#include "QXmppUtils.h"

#include <QDebug>
#include <QEventLoop>
#include <QTimer>

QXmppRemoteMethod::QXmppRemoteMethod(const QString &jid, const QString &method, const QVariantList &args, QXmppClient *client) : QObject(client), m_client(client)
{
    m_payload.setTo(jid);
    m_payload.setFrom(client->configuration().jid());
    m_payload.setMethod(method);
    m_payload.setArguments(args);
}

QXmppRemoteMethodResult QXmppRemoteMethod::call()
{
    // FIXME : spinning an event loop is a VERY bad idea, it can cause
    // us to lose incoming packets
    QEventLoop loop(this);
    connect(this, &QXmppRemoteMethod::callDone, &loop, &QEventLoop::quit);
    QTimer::singleShot(30000, &loop, &QEventLoop::quit);  // Timeout in case the other end hangs...

    m_client->sendPacket(m_payload);

    loop.exec(QEventLoop::ExcludeUserInputEvents | QEventLoop::WaitForMoreEvents);
    return m_result;
}

void QXmppRemoteMethod::gotError(const QXmppRpcErrorIq &iq)
{
    if (iq.id() == m_payload.id()) {
        m_result.hasError = true;
        m_result.errorMessage = iq.error().text();
        m_result.code = iq.error().type();
        Q_EMIT callDone();
    }
}

void QXmppRemoteMethod::gotResult(const QXmppRpcResponseIq &iq)
{
    if (iq.id() == m_payload.id()) {
        m_result.hasError = false;
        // FIXME: we don't handle multiple responses
        const auto values = iq.values();
        m_result.result = values.first();
        Q_EMIT callDone();
    }
}
