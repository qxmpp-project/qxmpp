// SPDX-FileCopyrightText: 2020 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPPACKET_H
#define QXMPPPACKET_H

#include "QXmppGlobal.h"
#include "QXmppSendResult.h"

#include <memory>

#include <QFutureInterface>

class QXmppNonza;

class QXmppPacket
{
public:
    QXmppPacket(const QXmppNonza &nonza);
    QXmppPacket(const QXmppNonza &nonza, std::shared_ptr<QFutureInterface<QXmpp::SendResult>>);
    QXmppPacket(const QByteArray &data, bool isXmppStanza, std::shared_ptr<QFutureInterface<QXmpp::SendResult>>);

    QByteArray data() const;
    bool isXmppStanza() const;

    QFuture<QXmpp::SendResult> future();

    void reportFinished();
    void reportResult(const QXmpp::SendResult &);

private:
    std::shared_ptr<QFutureInterface<QXmpp::SendResult>> m_interface;
    QByteArray m_data;
    bool m_isXmppStanza;
};

#endif  // QXMPPPACKET_H
