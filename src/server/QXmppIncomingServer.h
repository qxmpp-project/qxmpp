// SPDX-FileCopyrightText: 2010 Jeremy Lainé <jeremy.laine@m4x.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPINCOMINGSERVER_H
#define QXMPPINCOMINGSERVER_H

#include "QXmppLogger.h"

#include <memory>

class QDomElement;
class QSslSocket;
class QXmppDialback;
class QXmppIncomingServerPrivate;
class QXmppNonza;

namespace QXmpp::Private {
struct StreamOpen;
}

///
/// \brief The QXmppIncomingServer class represents an incoming XMPP stream
/// from an XMPP server.
///
class QXMPP_EXPORT QXmppIncomingServer : public QXmppLoggable
{
    Q_OBJECT

public:
    QXmppIncomingServer(QSslSocket *socket, const QString &domain, QObject *parent);
    ~QXmppIncomingServer() override;

    bool isConnected() const;
    void disconnectFromHost();

    QString localStreamId() const;

    bool sendPacket(const QXmppNonza &);
    Q_SLOT bool sendData(const QByteArray &);

    /// This signal is emitted when the stream is connected.
    Q_SIGNAL void connected();
    /// This signal is emitted when the stream is disconnected.
    Q_SIGNAL void disconnected();
    /// This signal is emitted when a dialback verify request is received.
    Q_SIGNAL void dialbackRequestReceived(const QXmppDialback &result);
    /// This signal is emitted when an element is received.
    Q_SIGNAL void elementReceived(const QDomElement &element);

private:
    void handleStart();
    void handleStanza(const QDomElement &element);
    void handleStream(const QXmpp::Private::StreamOpen &element);

    void slotDialbackResponseReceived(const QXmppDialback &dialback);
    void slotSocketDisconnected();

    const std::unique_ptr<QXmppIncomingServerPrivate> d;
    friend class QXmppIncomingServerPrivate;
};

#endif
