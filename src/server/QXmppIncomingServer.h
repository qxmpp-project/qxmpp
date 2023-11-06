// SPDX-FileCopyrightText: 2010 Jeremy Lainé <jeremy.laine@m4x.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "QXmppStream.h"

class QXmppDialback;
class QXmppIncomingServerPrivate;
class QXmppOutgoingServer;

/// \brief The QXmppIncomingServer class represents an incoming XMPP stream
/// from an XMPP server.
///

class QXMPP_EXPORT QXmppIncomingServer : public QXmppStream
{
    Q_OBJECT

public:
    QXmppIncomingServer(QSslSocket *socket, const QString &domain, QObject *parent);
    ~QXmppIncomingServer() override;

    bool isConnected() const override;
    QString localStreamId() const;

Q_SIGNALS:
    /// This signal is emitted when a dialback verify request is received.
    void dialbackRequestReceived(const QXmppDialback &result);

    /// This signal is emitted when an element is received.
    void elementReceived(const QDomElement &element);

protected:
    /// \cond
    void handleStanza(const QDomElement &stanzaElement) override;
    void handleStream(const QDomElement &streamElement) override;
    /// \endcond

private Q_SLOTS:
    void slotDialbackResponseReceived(const QXmppDialback &dialback);
    void slotSocketDisconnected();

private:
    Q_DISABLE_COPY(QXmppIncomingServer)
    QXmppIncomingServerPrivate *d;
    friend class QXmppIncomingServerPrivate;
};
