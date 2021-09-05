/*
 * Copyright (C) 2008-2021 The QXmpp developers
 *
 * Authors:
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

#include "QXmppPacket_p.h"
#include "QXmppNonza.h"

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
