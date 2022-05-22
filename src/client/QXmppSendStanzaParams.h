// SPDX-FileCopyrightText: 2022 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPSENDSTANZAPARAMS_H
#define QXMPPSENDSTANZAPARAMS_H

#include "QXmppGlobal.h"

#include <QSharedDataPointer>

class QXmppSendStanzaParamsPrivate;

class QXMPP_EXPORT QXmppSendStanzaParams
{
public:
    QXmppSendStanzaParams();
    QXmppSendStanzaParams(const QXmppSendStanzaParams &other);
    ~QXmppSendStanzaParams();
    QXmppSendStanzaParams &operator=(const QXmppSendStanzaParams &);

    QVector<QString> encryptionJids() const;
    void sendEncryptionJids(QVector<QString>);

private:
    QSharedDataPointer<QXmppSendStanzaParamsPrivate> d;
};

#endif  // QXMPPSENDSTANZAPARAMS_H
