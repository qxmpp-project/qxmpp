// SPDX-FileCopyrightText: 2020 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppNonza.h"
#include "QXmppPacket_p.h"
#include "QXmppUtils_p.h"

#include <QXmlStreamWriter>

using namespace QXmpp::Private;

/// \cond
QXmppPacket::QXmppPacket(const QXmppNonza &nonza, QXmppPromise<QXmpp::SendResult> interface)
    : QXmppPacket(serializeNonza(nonza), nonza.isXmppStanza(), std::move(interface))
{
}

QXmppPacket::QXmppPacket(const QByteArray &data, bool isXmppStanza, QXmppPromise<QXmpp::SendResult> interface)
    : m_promise(std::move(interface)),
      m_data(data),
      m_isXmppStanza(isXmppStanza)
{
}

QByteArray QXmppPacket::data() const
{
    return m_data;
}

bool QXmppPacket::isXmppStanza() const
{
    return m_isXmppStanza;
}

QXmppTask<QXmpp::SendResult> QXmppPacket::task()
{
    return m_promise.task();
}

void QXmppPacket::reportFinished(QXmpp::SendResult &&result)
{
    m_promise.finish(std::move(result));
}
/// \endcond
