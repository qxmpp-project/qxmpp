// SPDX-FileCopyrightText: 2020 Manjeet Dahiya <manjeetdahiya@gmail.com>
// SPDX-FileCopyrightText: 2020 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QXmpp API.
//
// This header file may change from version to version without notice,
// or even be removed.
//
// We mean it.
//

#ifndef QXMPPCLIENT_P_H
#define QXMPPCLIENT_P_H

#include "QXmppPresence.h"

class QXmppClient;
class QXmppClientExtension;
class QXmppE2eeExtension;
class QXmppLogger;
class QXmppOutgoingClient;
class QTimer;

class QXmppClientPrivate
{
public:
    QXmppClientPrivate(QXmppClient *qq);

    /// Current presence of the client
    QXmppPresence clientPresence;
    QList<QXmppClientExtension *> extensions;
    QXmppLogger *logger;
    /// Pointer to the XMPP stream
    QXmppOutgoingClient *stream;

    QXmppE2eeExtension *encryptionExtension;

    // reconnection
    bool receivedConflict;
    int reconnectionTries;
    QTimer *reconnectionTimer;

    // Client state indication
    bool isActive;

    void addProperCapability(QXmppPresence &presence);
    int getNextReconnectTime() const;

    static QStringList discoveryFeatures();

private:
    QXmppClient *q;
};

#endif  // QXMPPCLIENT_P_H
