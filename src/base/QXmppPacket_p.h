// SPDX-FileCopyrightText: 2020 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPPACKET_H
#define QXMPPPACKET_H

#include "QXmppGlobal.h"
#include "QXmppPromise.h"
#include "QXmppSendResult.h"

#include <memory>

#include <QFutureInterface>

class QXmppNonza;

class QXmppPacket
{
public:
    QXmppPacket(const QXmppNonza &nonza, QXmppPromise<QXmpp::SendResult> = {});
    QXmppPacket(const QByteArray &data, bool isXmppStanza, QXmppPromise<QXmpp::SendResult> = {});

    QByteArray data() const;
    bool isXmppStanza() const;

    QXmppTask<QXmpp::SendResult> task();

    void reportFinished(QXmpp::SendResult &&);

private:
    QXmppPromise<QXmpp::SendResult> m_promise;
    QByteArray m_data;
    bool m_isXmppStanza;
};

#endif  // QXMPPPACKET_H
