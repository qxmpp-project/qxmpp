// SPDX-FileCopyrightText: 2020 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppNonza.h"
#include "QXmppPacket_p.h"

#include <QFuture>
#include <QXmlStreamWriter>

inline QByteArray serialize(const QXmppNonza &nonza)
{
    QByteArray out;
    QXmlStreamWriter xmlStream(&out);
    nonza.toXml(&xmlStream);
    return out;
}

/// \cond
QXmppPacket::QXmppPacket(const QXmppNonza &nonza)
    : QXmppPacket(nonza, std::make_shared<QFutureInterface<QXmpp::SendResult>>())
{
}

QXmppPacket::QXmppPacket(const QXmppNonza &nonza, std::shared_ptr<QFutureInterface<QXmpp::SendResult>> interface)
    : QXmppPacket(serialize(nonza), nonza.isXmppStanza(), std::move(interface))
{
}

QXmppPacket::QXmppPacket(const QByteArray &data, bool isXmppStanza, std::shared_ptr<QFutureInterface<QXmpp::SendResult>> interface)
    : m_interface(std::move(interface)),
      m_data(data),
      m_isXmppStanza(isXmppStanza)
{
    m_interface->reportStarted();
}

QByteArray QXmppPacket::data() const
{
    return m_data;
}

bool QXmppPacket::isXmppStanza() const
{
    return m_isXmppStanza;
}

QFuture<QXmpp::SendResult> QXmppPacket::future()
{
    return m_interface->future();
}

void QXmppPacket::reportFinished()
{
    m_interface->reportFinished();
}

void QXmppPacket::reportResult(const QXmpp::SendResult &result)
{
    m_interface->reportResult(result);
}
/// \endcond
